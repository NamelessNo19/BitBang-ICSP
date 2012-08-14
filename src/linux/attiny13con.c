#include "serialcon.h"
#include <stdio.h>

int tinyIdent()
{
	printf("Sending identification request.\n");
	serWrite("ID", 2);
	unsigned char buf[4];
	buf[0] = '\0'; buf[1] = '\0'; buf[2] = '\0'; buf[3] = '\0';
	
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

	if (!serRead(buf, 4, TRUE))
		return FALSE;
	
	printf("\n");
	printf("------------------------------\n");
	printf("Man. ID:\t%#04X\n", buf[0]);
	printf("Memory ID:\t%#04X\n",  buf[1]);
	printf("Device ID:\t%#04X\n", buf[2]);
	printf("------------------------------\n");
	printf("\n");

	return TRUE;
}

int tinyReadPage(unsigned char pageNo)
{
	unsigned char buf[33];
	pageNo &= 0x1F;
	printf("Sending page %d read request.\n", pageNo);
	serWrite("RP", 2);
	serWrite(&pageNo, 1);
	
	if (!serRead(buf, 3, TRUE))
		return FALSE;
	else if (buf[0] == 'S' && buf[1] == 'F')
	{
		printf("Synchronization failed.\n");
		return FALSE;
	}
	else if (buf[0] == 'I' && buf[1] == 'M')
	{
		printf("Device ID mismatch.\n");
		return FALSE;
	}	else if (buf[0] != 'R' || buf[1] != 'P')
	{
		printf("Invalid response '%c%c'.\n", buf[0], buf[1]);
		return FALSE;
	}
	
	printf("Receiving data...\n");
	
	if (!serRead(buf, 33, TRUE))
		return FALSE;
		
	uint16_t word;
	int i;
	printf("\n");
	printf("Word Adr.|\t\t      Data\n");
	printf("---------+-------------------------------------\n");
	for (i = 0; i < 16; i++)
	{
		if (i % 4 == 0) 
			printf(  "%#06x   |\t", (pageNo << 4) + i);
		word = 0;
		word |= buf[2*i+1];
		word |= buf[2*i] << 8;
		printf("%#06x\t", word);
		if (i % 4 == 3) 
			printf("\n");
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
		printf("Verification failed. (Page already written?)");
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
