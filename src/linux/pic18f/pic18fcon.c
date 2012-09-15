#include "pic18fcon.h"
#include "serialcon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* CONF_REG_STR[] = {"CONFIG1L", "CONFIG1H", "CONFIG2L", "CONFIG2H", "CONFIG3H", "CONFIG4L",
			  "CONFIG5L", "CONFIG5H", "CONFIG6L", "CONFIG6H", "CONFIG7L", "CONFIG7H"};

int picIdent()
{
	printf("Sending identification request.\n");
	serWrite("ID", 2);
	unsigned char buf[13];
	buf[0] = '\0'; buf[1] = '\0'; buf[2] = '\0';
	
	if (!serRead(&buf[0], 3, TRUE)) 
		return FALSE; 
	else if (buf[0] == 'S' && buf[1] == 'F')
	{
		printf("Synchronization failed.\n");
		return FALSE;
	} else if (buf[0] != 'I' || buf[1] != 'D')
	{
		printf("Invalid response '%c%c'.\n", buf[0], buf[1]);
		return FALSE;
	}

	if (!serRead(buf, 3, TRUE))
		return FALSE;

	char* mod;
	uint16_t mdID = (buf[0] << 8 | buf[1]) & 0xFFE0;

	switch(mdID)
	  {
	  case 0x1200:
	    mod = "PIC18F4550";
	    break;
	  default:
		mod = "Unknown";
	    break;
	  }
	
	unsigned char rev = buf[1] & 0x7F;

	// Reading configuration
	if (!serRead(&buf[0], 13, TRUE))
		 return FALSE;

	printf("------\nModel: %s - Revision: %d\n", mod, rev);

	int i;

	for (i = 0; i < 12; i++)
		printf("%s: 0x%02X\n", CONF_REG_STR[i], buf[i]);
	printf("------\n");

	return TRUE;
}

int picDumpBlock(unsigned char* blkData, unsigned char blockNo)
{
	unsigned char buf[33];
	printf("Sending block %d read request.\n", blockNo);
	serWrite("RB", 2);
	serWrite(&blockNo, 1);
	
	if (!serRead(buf, 3, TRUE))
		return FALSE;
	else if (buf[0] == 'S' && buf[1] == 'F')
	{
		printf("Synchronization failed.\n");
		return FALSE;
	}
	else if (buf[0] == 'I' && buf[1] == 'A')
	{
		printf("Invalid Block.\n");
		return FALSE;
	}	else if (buf[0] != 'R' || buf[1] != 'B')
	{
		printf("Invalid response '%c%c'.\n", buf[0], buf[1]);
		return FALSE;
	}
	
	


	printf("Receiving block data...");


	uint16_t pgNo;



	for (pgNo = 0; pgNo < 256; pgNo++)
	{
	  fflush(stdout);

	  if (!serRead(buf, 33, TRUE))
	    {
	      printf("\n");
	      return FALSE;
	    }
	  memcpy(blkData + (32 * pgNo), &buf[0], 32);
	  serWrite("AC", 2);
	  printf("\rReceiving block data... %d%%", (pgNo * 100) / 255);
	}

	printf("\n");
	return TRUE;
}



int tinyWritePage(unsigned char pageNo, unsigned char* data)
{
	unsigned char buf[5];
	pageNo &= 0x1F;
	serWrite("WP", 2);
	serWrite(&pageNo, 1);
	serWrite(data, 32);
	printf("Transmitting data for page %d...\n", pageNo);
	
	
	uint16_t crcCal = 0xFFFF;
	int i;
	for (i = 0; i < 32; i++) 
	  crcCal = crc16_update(crcCal, data[i]);

	
	
	
	if (!serRead(buf, 5, TRUE))
		return FALSE;
	
	if (buf[0] != 'W' || buf[3] != 'P')
	{
		printf("Invalid response '%c%c'.\n", buf[0], buf[3]);
		return FALSE;
	}
	
	uint16_t crcSer = 0x00 | (buf[1] << 8) | buf[2]; 
	
	if (crcSer == crcCal)
		printf("Checksum OK. Start writing...\n");
	else
	{
		printf("Checksum mismatch. Expected %#06x, got %#06x.\n", crcCal, crcSer);
		printf("Cancel writing.\n");
		serWrite("NA", 2);
		return FALSE;
	}
	
	serWrite("AC", 2);
	
	if (!serRead(buf, 3, TRUE))
	{
		printf("Verification timed out.\n");
		return FALSE;
	}	
	else if (buf[0] == 'S' && buf[1] == 'F')
	{
		printf("Synchronization failed.\n");
		return FALSE;
	}
	else if (buf[0] == 'I' && buf[1] == 'M')
	{
		printf("Device ID mismatch.\n");
		return FALSE;
	}
	else if (buf[0] == 'C' || buf[1] == 'F')
	{
		printf("Verification failed. (Page already written?)\n");
		return FALSE;
	}
	else if (buf[0] != 'A' || buf[1] != 'C')
	{
		printf("Unexpected response '%c%c'.\n", buf[0], buf[1]);
		return FALSE;
	}
	
	
	
	printf("Page verified.\n");
	return TRUE;
}

int tinyChipErase()
{
	printf("Sending chip erase request.\n");
	serWrite("ER", 2);

	char buf[3];
	if (!serRead(buf, 3, TRUE))
			return FALSE;

	if (buf[0] == 'E' && buf[1] == 'R')
	{
		printf("Chip erase confirmed.\n");
		return TRUE;
	}
	else if (buf[0] == 'S' && buf[1] == 'F')
	{
		printf("Synchronization failed.\n");
		return FALSE;
	}
	else
	{
		printf("Unexpected response '%c%c'.\n", buf[0], buf[1]);
		return FALSE;
	}
}
