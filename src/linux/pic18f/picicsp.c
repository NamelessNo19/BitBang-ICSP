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


int main (int argc, char **argv)
{
	conf_t conf ;
	
	printf(" *** BitBang-ICSP (PIC18F) BUILD %s, %s *** \n", __DATE__, __TIME__);

	if (!parseArgs(argc, argv, &conf))
		return 1;
		
	if (!verifyConf(&conf))
		return 2;

	 datSeq_t* hexdt = malloc(256 * sizeof(datSeq_t));



	if (conf.hashex && conf.write)
	  if (!decodeHex(conf.hexfile, hexdt))
	    {
	      printf("Aborted.\n");
	      free(hexdt);
	      return 0;
	    }
	
	if (conf.erase)
		{
			printf("Are you sure you want to delete the chip's memory? (Y/N): ");
			char in;
			scanf("%c", &in);
			if (in != 'y' && in != 'Y')
			{
				printf("Aborted.\n");
				free(hexdt);
				return;
			}
		}


	if (!serOpen(conf.port))
	{
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
	  writeHex(hexdt);
	else if (conf.erase)
	;//	tinyChipErase();
		
		
		
	// Finishing
	serClose();
	printf("Done.\n");
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
	
	if (!conf->dump && !conf->ident && !conf->write && !conf->erase)
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

int decodeHex(char* path, const datSeq_t* data)
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

void writeHex(datSeq_t* hexdat)
{

	uint32_t base;
	uint8_t chunk[32];
	int i = 0;


	base = hexdat[i].baseAdr;
	while (hexdat[i].length > 0 && hexdat[i].baseAdr + hexdat[i].length <= 0x8000)
	{
		for ( j = 0; j < 32; j++) chunk[j] = 0xFF;
		seqToByteArray(hexdat, &chunk[0], base, 32);
		picWriteChunk(&chunk, base);

		base += 32;

		if (base >= hexdat[i].baseAdr + hexdat[i].length )
		{
			i++;
			base = hexdat[i].baseAdr;
		}

	}

}






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
