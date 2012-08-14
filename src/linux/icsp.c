#define _POSIX_C_SOURCE 2

#include <stdio.h>
#include <unistd.h>

#include "serialcon.h"
#include "attiny13con.h"

typedef struct conf_s 
{
	char *port;
	int read;
	int write;
	int erase;
	unsigned char pageNo;
	int ident;
} conf_t;

int parseArgs(int argc, char **argv, conf_t *conf);
int verifyConf(conf_t *conf);

unsigned char tDat[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFF, 0xFF, 0x01, 0x23, 0x00, 0x00, 0xB0, 0x0B, 
				   0xB0, 0x0B, 0xFF, 0xFF, 0x01, 0x23, 0xAB, 0x47, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};


int main (int argc, char **argv)
{
	conf_t conf ;
	
	if (!parseArgs(argc, argv, &conf))
		return 1;
		
	if (!verifyConf(&conf))
		return 2;
	
	if (conf.erase)
		{
			printf("Are you sure you want to perform a chip erase? (Y/N): ");
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
		tinyIdent();
	else if (conf.read)
		tinyReadPage(conf.pageNo);
	else if (conf.write)
		tinyWritePage(conf.pageNo, &tDat[0]);
	else if (conf.erase)
		tinyChipErase();
		
		
		
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
	conf->read = FALSE;
	conf->ident = FALSE;
	conf->write = FALSE;
	conf->erase = FALSE;
	conf->pageNo = 0;
	 
	 while ((ai = getopt (argc, argv, "ip:r:w:e")) != -1)
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
           case 'r':
			 conf->read = TRUE;
			 conf->pageNo = atoi(optarg);
             break;
			case 'w':
			 conf->write = TRUE;
			 conf->pageNo = atoi(optarg);
             break;
           case '?':
             if (optopt == 'p' || optopt == 'r' || optopt == 'w')
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
	
	if (!conf->read && !conf->ident && !conf->write && !conf->erase)
	{
		printf("No operation specified.\n");
		return FALSE;
	}
	
	if (conf->read && conf->ident)
	{
		printf("Incompatible arguments '-r' and '-i'.\n");
		return FALSE;
	}
	
	if (conf->write && conf->ident)
	{
		printf("Incompatible arguments '-w' and '-i'.\n");
		return FALSE;
	}
	
	if (conf->write && conf->read)
	{
		printf("Incompatible arguments '-w' and '-r'.\n");
		return FALSE;
	}
	
	if (conf->erase && conf->ident)
	{
		printf("Incompatible arguments '-e' and '-i'.\n");
		return FALSE;
	}

	if (conf->erase && conf->read)
	{
		printf("Incompatible arguments '-e' and '-r'.\n");
			return FALSE;
	}

	if (conf->erase && conf->write)
	{
		printf("Incompatible arguments '-e' and '-w'.\n");
			return FALSE;
	}

	
	return TRUE;
}
