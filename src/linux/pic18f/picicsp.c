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
} conf_t;

int parseArgs(int argc, char **argv, conf_t *conf);
int verifyConf(conf_t *conf);
int decodeHex(char* path, unsigned char* data);
void writeHex(unsigned char* hexdat);
void dump(const char* path, unsigned char blockNo);


unsigned char tDat[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFF, 0xFF, 0x01, 0x23, 0x00, 0x00, 0xB0, 0x0B, 
				   0xB0, 0x0B, 0xFF, 0xFF, 0x01, 0x23, 0xAB, 0x47, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};


int main (int argc, char **argv)
{
	conf_t conf ;
	
	printf(" *** BitBang-ICSP (PIC18F) BUILD %s, %s *** \n", __DATE__, __TIME__);

	if (!parseArgs(argc, argv, &conf))
		return 1;
		
	if (!verifyConf(&conf))
		return 2;

	unsigned char hexdat[1024] = {[0 ... 1023] = 0xFF};
	if (conf.hashex && conf.write)
	  if (!decodeHex(conf.hexfile, &hexdat[0]))
	    {
	      printf("Aborted.\n");
	      return 0;
	    }
	
	if (conf.erase || conf.write)
		{
			printf("Are you sure you want to delete the chip's memory? (Y/N): ");
			char in;
			scanf("%c", &in);
			if (in != 'y' && in != 'Y')
			{
				printf("Aborted.\n");
				return;
			}
		}


	if (!serOpen(conf.port))
		return 3;
		
	// Waiting		
	printf("Waiting for programmer...");
	fflush(stdout);
	
	char buffer[6];
	int rdy = FALSE;
	
	do {
	if (!serRead(&buffer[0], 2, TRUE))
	{
		serClose();
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
	  ;//	tinyWritePage(conf.pageNo, &tDat[0]);
	else if (conf.erase)
		tinyChipErase();
	else if (conf.hashex) 
	    if (!tinyChipErase())
	      printf("Chip erase failed. Aborting.\n");
	    else
	      writeHex(&hexdat[0]);
		
		
		
	// Finishing
	serClose();
	printf("Done.\n");
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
	conf->blockNo = 0;
	 
	 while ((ai = getopt (argc, argv, "ip:d:w:eh:c")) != -1)
         switch (ai)
           {
           case 'i':
             conf->ident = TRUE;
             break;
           case 'e':
             conf->erase = TRUE;
             break;
           case 'p':
             conf->port = optarg;
             break;
           case 'd':
	     conf->dump = TRUE;
	     conf->blockNo = atoi(optarg);
             break;
	   case 'w':
	     conf->write = TRUE;
	     conf->blockNo = atoi(optarg);
             break;
	   case 'h':
	     conf->hashex = TRUE;
	     conf->hexfile = optarg;
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
	
	if (!conf->dump && !conf->ident && !conf->write && !conf->erase && !conf->hashex)
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

	if (conf->ident && conf->hashex)
	{
		printf("Incompatible arguments '-i' and '-h'.\n");
		return FALSE;
	}
	if (conf->write && conf->hashex)
	{
		printf("Incompatible arguments '-w' and '-h'.\n");
		return FALSE;
	}
	if (conf->erase && conf->hashex)
	{
		printf("Incompatible arguments '-e' and '-h'.\n");
		return FALSE;
	}

	if (conf->dump && !conf->hashex)
	{
		printf("You need to specify an output directory for dumping.\n");
		return FALSE;
	}

	
	return TRUE;
}

int decodeHex(char* path, unsigned char* data)
{
  int hfd = open(path, O_RDONLY);
  
  if (hfd < 0)
    {
      printf("Cannot open Hex file '%s'.\n", path);
      return FALSE;
    }

  datSeq_t hexseq[64];

  long count =  parseHexFile(hfd, &hexseq[0]);
  seqToByteArray(&hexseq[0], (uint8_t*) data, 0, 1024);
  cleanUpSeq(&hexseq[0]);

  if (count < 0)
    {
      printf("Could not parse Hexfile.\n");
      return FALSE;
    }
  else if (count == 0)
    {
      printf("Unable to read Data from Hexfile.\n");
      return FALSE;
    }
  else
    {
      printf("Got %ld bytes from Hexfile.\n", count);
    }


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

void writeHex(unsigned char* hexdat)
{
  unsigned char pgno = 0;
  unsigned int skipcnt;
  unsigned char pgDat[32];
  int i;
  while (pgno < 32)
    {
      skipcnt = 0;
      while (isPageEmpty(hexdat, pgno + skipcnt) && pgno + skipcnt < 32)
	skipcnt++;
      pgno += skipcnt;

      if (skipcnt != 0)
	{
	  printf("Skipped %u empty pages.\n", skipcnt);
	  if (pgno > 31) break;
	}
      printf("Attempting to write page %u.\n", pgno);
      memcpy(&pgDat[0], &hexdat[pgno * 32], 32);
      
      /*     for (i = 0; i < 32; i++)
	{
	  printf("%#04x\t", pgDat[i+32*pgno]);
	  if (i % 8 == 7)
	  printf("\n"); 
	  } */

      if(!tinyWritePage(pgno, &pgDat[0]))
	{
	  printf("Writing failed. Aborting.\n");
	  return;
	}


      pgno++;
      usleep(150 * 1000);
    }
  printf("Hexfile successfully written.\n");

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

	unsigned char* dat = malloc(blockSize);

	if (!picDumpBlock(dat, blockNo))
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

	free(dat);
	return;
}
