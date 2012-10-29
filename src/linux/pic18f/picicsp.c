#define _POSIX_C_SOURCE 2

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include "serialcon.h"
#include "pic18fcon.h"
#include "hexdec.h"

typedef struct conf_s 
{
  char *port;
  int dump;
  int write;
  int erase;
  unsigned char blockNo;
  char* hexfile;
  int hashex;
  int ident;
  int resConf;
} conf_t;

int parseArgs(int argc, char **argv, conf_t *conf);
int verifyConf(conf_t *conf);
int decodeHex(const char* path,  datSeq_t* data);
void writeHex(datSeq_t* hexdat);
void dump(const char* path, unsigned char blockNo);
void resetConfig();
void writeConfig(uint8_t* dat);

int main (int argc, char **argv)
{
	conf_t conf ;
	
	printf(" *** BitBang-ICSP (PIC18F) BUILD %s, %s *** \n", __DATE__, __TIME__);

	if (!parseArgs(argc, argv, &conf))
		return 1;
		
	if (!verifyConf(&conf))
		return 2;

	 datSeq_t* hexdt = malloc(1024 * sizeof(datSeq_t));

	 int hasConf = FALSE;
	 uint8_t confDat[14] = {[0 ... 13] = 0xFF};
	 int hasDatEEPROM = FALSE;
	 uint8_t eeDat[256];
	 int hexAlloced = FALSE;

	if (conf.hashex && conf.write)
	  {
	    if (!decodeHex(conf.hexfile, hexdt))
	      {
		printf("Aborted.\n");
		free(hexdt);
		return 0;
	      }
	    else
	      {
		hexAlloced = TRUE;
	      }

	    // Check for configuration bit
	    hasConf = (seqToByteArray(hexdt, &confDat[0], 0x300000L, 14) > 0);
	    if (hasConf)
	      {
		printf("This Hexfile includes configuration bits. Do you want to write them? (Y/N): ");
		char in;
		scanf("%c", &in);
		if (in != 'y' && in != 'Y')
		  {
		    printf("Ignoring configuration bits.\n");
		    hasConf = FALSE;
		  }  
	      }
	    else
	      {
		printf("No configuration bits found.\n");
	      }

	    // Check for data EEPROM
	    hasDatEEPROM = (seqToByteArray(hexdt, &eeDat[0], 0xF00000L, 256) > 0);
	    if (hasDatEEPROM)
	      {
		printf("This Hexfile includes a segemt for the data EEPROM. Writing this is currently not supported. Do you wnat to continue anyway? (Y/N): ");
		char in;
		scanf("%c", &in);
		if (in != 'y' && in != 'Y')
		  {
		  printf("Aborted.\n");
		  if (hexAlloced)
		      cleanUpSeq(hexdt);
		  free(hexdt);
		    
		return 0;
		  }  
	      }
	  }
	
	if (conf.erase)
		{

		  if (conf.blockNo == 'B')
		    printf("Are you sure you want to erase the boot block? (Y/N): ");
		  else if (conf.blockNo == 'C')
		     printf("Are you sure you want to erase the configuration bits? (Y/N): ");
		  else if (conf.blockNo == 'X')
		     printf("Are you sure you want to erase ALL of the chip's memory? (Y/N): ");
		  else
		    printf("Are you sure you want to erase block %d? (Y/N): ", (unsigned int)conf.blockNo);

			char in;
			scanf("%c", &in);
			if (in != 'y' && in != 'Y')
			{
				printf("Aborted.\n");
				if (hexAlloced)
				  cleanUpSeq(hexdt);
				free(hexdt);
				return 0;
			}
		}

	if (conf.resConf)
		{
		    printf("Are you sure you want to rewrite the configuration bits? (Y/N): ");
			char in;
			scanf("%c", &in);
			if (in != 'y' && in != 'Y')
			{
				printf("Aborted.\n");
				if (hexAlloced)
				  cleanUpSeq(hexdt);
				free(hexdt);
				return 0;
			}
		}


	if (!serOpen(conf.port))
	{
	  if (hexAlloced)
	    cleanUpSeq(hexdt);
	  free(hexdt);
	  return 3;
	}
		
	// Waiting		
	printf("Waiting for programmer...");
	fflush(stdout);
	
	char buffer[6];
	int rdy = FALSE;
	
	do {
	if (!serRead(&buffer[0], 2, TRUE))
	{
		serClose();
		if (hexAlloced)
		  cleanUpSeq(hexdt);
		free(hexdt);
		return 0;
	} 
	else if (buffer[0] != '!')
	{
		printf(".");
		fflush(stdout);
	}
	else
	{
		printf("Ready.\n");
		rdy = TRUE;
	}
	}
	while (!rdy);
	
	if (conf.ident) 
		picIdent();
	else if (conf.dump)
		dump(conf.hexfile, conf.blockNo);
	else if (conf.write)
	  {
	    writeHex(hexdt);
	    if (hasConf)
	      writeConfig(&confDat[0]);
	  }
	else if (conf.erase)
	  {
	    if (conf.blockNo == 'B') conf.blockNo = 0x0b;
	    else if (conf.blockNo == 'C') conf.blockNo = 0x0c;
            else if (conf.blockNo == 'X') conf.blockNo = 0xce;
	    picBulkErase(conf.blockNo);
	  }
	else if (conf.resConf)
	  writeConfig((uint8_t*) &CONF_REG_DEF[0]);
		
		
		
	// Finishing
	serClose();
	printf("Done.\n");
	if (hexAlloced)
	  cleanUpSeq(hexdt);
	 free(hexdt);
	return 0;
}

int parseArgs(int argc, char **argv, conf_t *conf)
{

	int index;
	int ai;
	
	conf->port = NULL;
	conf->dump = FALSE;
	conf->ident = FALSE;
	conf->write = FALSE;
	conf->erase = FALSE;
	conf->hashex = FALSE;
	conf->hexfile = NULL;
	conf->resConf = FALSE;
	conf->blockNo = 0;
	 
	 while ((ai = getopt (argc, argv, "ip:d:e:h:wC")) != -1)
         switch (ai)
           {
           case 'i':
             conf->ident = TRUE;
             break;
           case 'e':
             conf->erase = TRUE;
	     if (optarg[0] == 'b' || optarg[0] == 'B')
	       conf->blockNo = 'B';
	     else if (optarg[0] == 'c' || optarg[0] == 'C')
	       conf->blockNo = 'C';
	     else if (optarg[0] == 'x' || optarg[0] == 'X')
	       conf->blockNo = 'X';
	     else
	      conf->blockNo = atoi(optarg);
             break;
           case 'p':
             conf->port = optarg;
             break;
           case 'd':
	     conf->dump = TRUE;
	     if (optarg[0] == 'b' || optarg[0] == 'B')
	       conf->blockNo = 'B';
	     else if (optarg[0] == 'f' || optarg[0] == 'F')
	       conf->blockNo = 'F';
	     else
	      conf->blockNo = atoi(optarg);
             break;
	   case 'w':
	     conf->write = TRUE;
             break;
	   case 'h':
	     conf->hashex = TRUE;
	     conf->hexfile = optarg;
	     break;
	   case 'C':
	     conf->resConf = TRUE;
	     break;
           case '?':
             if (optopt == 'p' || optopt == 'd' || optopt == 'w')
               fprintf (stderr, "Option -%c requires an argument.\n", optopt);
             else if (isprint (optopt))
               fprintf (stderr, "Unknown option `-%c'.\n", optopt);
             else
               fprintf (stderr,
                        "Unknown option character `\\x%x'.\n",
                        optopt);
             return FALSE;
           default:
		     fprintf (stderr, "Parser failed.");
             return(FALSE);
           }
		   
	 for (index = optind; index < argc; index++) 
	 {
         printf ("Non-option argument %s\n", argv[index]);
		 return FALSE;
	 }
	 
	 return TRUE;

}

int verifyConf(conf_t *conf)
{
	if (conf->port == NULL)
	{
		printf("Missing Port argument (e.g '-p /dev/ttyACM0').\n");
		return FALSE;
	}
	
	if (!conf->dump && !conf->ident && !conf->write && !conf->erase && !conf->resConf)
	{
		printf("No operation specified.\n");
		return FALSE;
	}

	
	if (conf->dump && conf->ident)
	{
		printf("Incompatible arguments '-d' and '-i'.\n");
		return FALSE;
	}
	
	if (conf->write && conf->ident)
	{
		printf("Incompatible arguments '-w' and '-i'.\n");
		return FALSE;
	}
	
	if (conf->write && conf->dump)
	{
		printf("Incompatible arguments '-w' and '-d'.\n");
		return FALSE;
	}
	
	if (conf->erase && conf->ident)
	{
		printf("Incompatible arguments '-e' and '-i'.\n");
		return FALSE;
	}

	if (conf->erase && conf->dump)
	{
		printf("Incompatible arguments '-e' and '-d'.\n");
			return FALSE;
	}

	if (conf->erase && conf->write)
	{
		printf("Incompatible arguments '-e' and '-w'.\n");
			return FALSE;
	}

	if (conf->erase && conf->resConf)
	{
		printf("Incompatible arguments '-e' and '-C'.\n");
			return FALSE;
	}

	if (conf->resConf && conf->ident)
	{
		printf("Incompatible arguments '-C' and '-i'.\n");
		return FALSE;
	}

	if (conf->resConf && conf->dump)
	{
		printf("Incompatible arguments '-C' and '-d'.\n");
			return FALSE;
	}

	if (conf->resConf && conf->write)
	{
		printf("Incompatible arguments '-C' and '-w'.\n");
			return FALSE;
	}


	if (conf->ident && conf->hashex)
	{
		printf("Incompatible arguments '-i' and '-h'.\n");
		return FALSE;
	}

	if (conf->erase && conf->hashex)
	{
		printf("Incompatible arguments '-e' and '-h'.\n");
		return FALSE;
	}

	if (conf->dump && !conf->hashex)
	{
		printf("You need to specify an output file for dumping.\n");
		return FALSE;
	}

	if (conf->write && !conf->hashex)
	{
		printf("You need to specify an input file for writing.\n");
		return FALSE;
	}

	
	return TRUE;
}

int decodeHex(const char* path, datSeq_t* data)
{
  int hfd = open(path, O_RDONLY);
  
  if (hfd < 0)
    {
      printf("Cannot open Hex file '%s'.\n", path);
      return FALSE;
    }


  long count = parseHexFile(hfd, data);

  if (count < 0)
    {
      printf("Could not parse Hexfile.\n");
      close(hfd);
      return FALSE;
    }
  else if (count == 0)
    {
      printf("Unable to read Data from Hexfile.\n");
      close(hfd);
      return FALSE;
    }
  else
    {
      printf("Got %ld bytes from Hexfile.\n", count);
    }

  //   int i;
  //  for (i = 0; data[i].length > 0;  i++)
  //  printf("%d: Base: 0x%08x  -  Length: %d\n", i,  data[i].baseAdr, data[i].length);

  // Creating a local binary copy

	const size_t blockSize = 8192;
	FILE* outf = fopen("HEXDMP.BIN", "wb+");

	if(outf == 0)
	{
		printf("Cannot open \"%s\" for writing.", path);
		return;
	}

	uint8_t* dat = malloc(blockSize);
	size_t byteCount = 0;
	size_t i;
do
  {

    for (i = 0; i < blockSize; i++) dat[i] = 0xFF;

    seqToByteArray(data, dat, byteCount, blockSize);
    byteCount += blockSize;

    if (fwrite(dat, 1, blockSize, outf) != blockSize) {
       	printf("Failed.\n");
	break;
    }
  }
 while (byteCount < 32768);

	free(dat);
        fclose(outf);

     


  close(hfd);
  return TRUE;
}


int isPageEmpty(unsigned char* dat, unsigned char pgno)
{
  int i;
  for (i = 0; i < 32; i++)
    if (dat[pgno * 32 + i] != 0xFF)
      return FALSE;
  return TRUE;
}

void writeHex( datSeq_t* hexdat)
{
  printf("Enabling write access...");
	uint32_t base;
	uint8_t chunk[WR_CHNK_SIZE];
	int i = 0;
	int j;

       if (!echo('W', 'R')) return;
       printf("OK.\n");


	int cwrt = 0;

	base = hexdat[i].baseAdr;
	while (hexdat[i].length > 0 && base <= 0x8000)
	{
	  for ( j = 0; j < WR_CHNK_SIZE; j++) chunk[j] = 0xFF; // Init

	  	printf("\rWriting Chunk %d...", cwrt);
		fflush(stdout);
		seqToByteArray(hexdat, &chunk[0], base, WR_CHNK_SIZE);
		

		if (!echo('N', 'C'))
		  return;

		if (!picWriteChunk(&chunk, base))
		 {
		  printf("\nChunk write failed.\n");
		  break;
		  }
	     
		//		printf("%cwrt @ 0x%08x: ",cwrt, base);
		//	for ( j = 0; j < WR_CHNK_SIZE; j++) printf("%02x", chunk[j]);
		//	printf("\n\n");
		
		cwrt++;
		base += WR_CHNK_SIZE;
     
	       	while (base > hexdat[i].baseAdr + hexdat[i].length  && hexdat[i].length > 0)
		  {
		    i++;
		    base = hexdat[i].baseAdr < base ? base : (hexdat[i].baseAdr & 0xFFFFFFE0); // align
		  }
	}
	printf("\n%d chunk(s) written.\n", cwrt);
	printf("Sending write stop...");
		if (!echo('W', 'S'))
	  printf("Failed.\n");
		else
	   printf("Confirmed.\n");

	return;
}


void dump(const char* path, unsigned char blockNo)
{
	const size_t blockSize = 8192;

	FILE* outf = fopen(path, "wb+");

	if(outf == 0)
	{
		printf("Cannot open \"%s\" for writing.", path);
		return;
	}

	int fullDump = (blockNo == 'F');
	int fullCount = 0;


	unsigned char* dat = malloc(blockSize);
do
  {

    if (!picDumpBlock(dat, fullDump ? fullCount : blockNo))
	{
		printf("Dumping failed.\n");
		free(dat);
		return;
	}

	printf("Writing to file... ");

	if (fwrite(dat, 1, blockSize, outf) == blockSize)
		printf("Done.\n");
	else
		printf("Failed.\n");
	fullCount++;
  }
 while (fullDump && fullCount < 4);

	free(dat);
fclose(outf);
	return;
}

void writeConfig(uint8_t* dat)
{
  printf("Preparing to write configuration bits...\n");
  if (!echo('C', 'W'))
    return;
  int i;
  uint8_t cByte;
 for (i = 0; i < 12; i++)
    {
      cByte = dat[CONF_REG_ADR[i]];
      if (cByte != 0xFF)
	{
	  printf("Writing 0x%02X to %s... ",cByte, CONF_REG_STR[i]);
	  if (picWriteConf(CONF_REG_ADR[i], cByte))
	    {
	      printf("OK\n");
	    }
	  else
	    {
	      printf("Aborting.\n");
	      break;
	    }
	}
    }
  serWrite("ST", 2);

}
