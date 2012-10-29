#include "pic18fcon.h"
#include "serialcon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const char* CONF_REG_STR[] = {"CONFIG1L", "CONFIG1H", "CONFIG2L", "CONFIG2H", "CONFIG3H", "CONFIG4L",
			  "CONFIG5L", "CONFIG5H", "CONFIG6L", "CONFIG6H", "CONFIG7L", "CONFIG7H"};

const unsigned char CONF_REG_ADR[] = {0x00, 0x01, 0x02, 0x03, 0x05, 0x06, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D};

const unsigned char CONF_REG_DEF[] = {0x00, 0x0B, 0x01, 0x00, 0xFF, 0x81, 0xC4, 0xFF, 0x0F, 0xC0, 0x0F, 0xE0, 0x0F, 0x40};


int echo(unsigned char c1, unsigned char c2)
{
	unsigned char buf[3];
	buf[0] = c1; buf[1] = c2; buf[2] = '\0';
	serWrite(&buf[0], 2);
	buf[0] = '\0'; buf[1] = '\0';
	
	if (!serRead(&buf[0], 3, TRUE)) 
		return FALSE; 
	else if (buf[0] == 'S' && buf[1] == 'F')
	{
		printf("Synchronization failed.\n");
		return FALSE;
	} else if (buf[0] != c1 || buf[1] != c2)
	{
		printf("Invalid response '%c%c'.\n", buf[0], buf[1]);
		return FALSE;
	}

	return TRUE;
}

int picIdent()
{
	printf("Sending identification request.\n");


	if (!echo('I', 'D'))
		return FALSE;

	unsigned char buf[13];
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
	buf[0] = '\0'; buf[1] = '\0';
	
	if (!serRead(&buf[0], 3, TRUE)) 
		return FALSE; 
	else if (buf[0] == 'I' && buf[1] == 'A')
	{
		printf("Invalid block.\n");
		return FALSE;
	}
	else if (buf[0] == 'S' && buf[1] == 'F')
	{
		printf("Synchronization failed.\n");
		return FALSE;
	}
	else if (buf[0] != 'R' || buf[1] != 'B')
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
	serRead(&buf[0], 3, TRUE);
	printf("\n");
	return TRUE;
}

int picWriteChunk(uint8_t (*chunk)[WR_CHNK_SIZE], const uint32_t adr)
{
	uint8_t i;
	uint8_t buf[6];

	uint16_t cCRC = 0xFFFF;
	uint16_t rCRC = 0;

	if (adr > 0x007FFF)
	  {
	    printf("Invalid base address 0x%06x. Skipped.\n", adr);
	    return TRUE;
	  }

	// Sending Address

	buf[0] = 'C';
	buf[1] = (adr & 0xFF000000) >> 24;
	buf[2] = (adr & 0x00FF0000) >> 16;
	buf[3] = (adr & 0x0000FF00) >> 8;
	buf[4] = (adr & 0x000000FF);
	buf[5] = 'W';
	
	//printf("Sending address...\n");
	serWrite(&buf[0], 6);

	// Calculate checksum
	cCRC =  crc16_update(cCRC, buf[1]);
	cCRC =  crc16_update(cCRC, buf[2]);
	cCRC =  crc16_update(cCRC, buf[3]);
	cCRC =  crc16_update(cCRC, buf[4]);
	for (i = 0; i < WR_CHNK_SIZE; i++)
	  {
	  cCRC = crc16_update(cCRC, (*chunk)[i]);
	  // printf("%02X ", (*chunk)[i]);
	  }
       
	// printf("\n @0x%08X\n", adr);

	
	if (!serRead(&buf[0], 3, TRUE))
		return FALSE;
	else if (buf[0] == 'A' && buf[1] == 'H')
	{
	  printf("\nChunk base (0x%06x) rejected. Skipped.\n", adr);
		return TRUE;
	}
	else if (buf[0] != 'C' || buf[1] != 'W')
	{
		printf("\nInvalid response '%c%c', expected CW.\n", buf[0], buf[1]);
		return FALSE;
	}

	// Write data
	//	printf("Sending data...\n");
	serWrite(&(*chunk)[0], WR_CHNK_SIZE);

	if (!serRead(&buf[0], 5, TRUE))
		return FALSE;
	else if (buf[0] != 'C' || buf[3] != 'S')
	{
		printf("\nInvalid response '%c%c', expected CS.\n", buf[0], buf[3]);
		return FALSE;
	}

	// Verify Checksum
	rCRC = (buf[1] << 8) | buf[2];
	if (rCRC != cCRC)
	  {
	    printf("\nChecksum missmatch! (C: 0x%x / R: 0x%x) Sending abort....\n", cCRC, rCRC);
	    serWrite("CM", 2);

	    if (!serRead(&buf[0], 3, TRUE))
		return FALSE;
	    else if (buf[0] != 'S' || buf[1] != 'T')
	      {
		printf("\nInvalid response '%c%c', expected ST.\n", buf[0], buf[1]);
		return FALSE;
	      }
	    printf("Write aborted.\n");
	    return FALSE;
	  }

	//	printf("Checksum OK. Sending confirmation...\n");
	if (!echo('A','C'))
	  {
	    printf("\nConfirmation failed. \n");
	    return FALSE;
	  }

	//	printf("Confirmed.\n");

	return TRUE;
}

int picBulkErase(uint8_t blockNo)
{
  if (blockNo >= 4 && blockNo != 0x0B && blockNo != 0x0C && blockNo != 0xCE)
    {
      printf("Unexpected block %d!\n", blockNo);
      return FALSE;
    }
 
  if (!echo('E', 'R')) return FALSE;

  serWrite(&blockNo, 1);
  unsigned char buf[3];
  
  if (!serRead(&buf[0], 3, TRUE))
	return FALSE;
  else if (buf[0] == 'I' || buf[1] == 'A')
  {
	printf("Block rejected by programmer.\n");
	return FALSE;
  }
  else if (buf[0] == 'S' || buf[1] == 'F')
  {
	printf("Synchronization failed.\n");
	return FALSE;
  }
  else if (buf[0] == 'E' || buf[1] == 'C')
  {
	printf("Erase completed.\n");
	return TRUE;
  }
  else
   {
     printf("Invalid response '%c%c'.\n", buf[0], buf[1]);
     return FALSE;
   }

}

int picWriteConf(uint8_t cReg, uint8_t cVal)
{
  if (!echo('C', 'G'))
     return FALSE;

  unsigned char buf[5];

  buf[0] = 'A';
  buf[1] = cReg;
  buf[2] = 'C';
  buf[3] = cVal;
  serWrite(&buf[0], 4);

 if (!serRead(&buf[0], 5, TRUE))
	return FALSE;
  else if (buf[0] != 'C' || buf[2] != 'A')
  {
	 printf("\nInvalid response '%c%c'.\n", buf[0], buf[2]);
	return FALSE;
  }
  else if (buf[1] != cVal || buf[3] != cReg)
    {
      printf("\nTransmission failed.\n");
      serWrite("IV", 2);
    }

 serWrite("AC", 2);

if (!serRead(&buf[0], 3, TRUE))
	return FALSE;
  else if (buf[0] == 'I' || buf[1] == 'A')
  {
	printf("\nConfiguration rejected by programmer.\n");
	return FALSE;
  }
  else if (buf[0] == 'S' || buf[1] == 'F')
  {
	printf("\nSynchronization failed.\n");
	return FALSE;
  }
  else if (buf[0] == 'C' || buf[1] == 'W')
  {
	return TRUE;
  }
  else
   {
     printf("\nInvalid response '%c%c'.\n", buf[0], buf[1]);
     return FALSE;
   }

}

