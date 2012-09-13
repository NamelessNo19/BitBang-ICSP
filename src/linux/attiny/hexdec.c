#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <inttypes.h>

#include "hexdec.h"

int hexFd;


int skipLine();
long parseHexLine(unsigned char* data);
uint8_t readByte();


long parseHexFile (int hexFileDesc, unsigned char* buffer)
{
	
	hexFd = hexFileDesc;
	int cnt;
	long total = 0;
   
	while ((cnt = parseHexLine(buffer)) >= 0)
    	total += cnt;		
	
   if (cnt == -1)
   {
	   return total;
   }
   else 
   {
	   return -1;
   }
   
}

int skipLine()
{
	char tmp;
	while (read(hexFd, &tmp, 1) > 0)
	{
		if (tmp == '\n') 
			return 1;
	}
	
	return 0;
}

long parseHexLine(unsigned char* data)
{
	char tmp[2];
	uint8_t checksum = 0;
	
	if (read(hexFd, &tmp[0], 1) < 1)
	{
		//printf("Unexpected EOF.\n");
		return -1;
	}
	
	if (tmp[0] != ':')
	{
		//printf("Line does not start with a colon (%c).\n", tmp[0]);
		skipLine();
		return 0;
	}
	
	
	// Length
	uint8_t count = readByte();
	
	checksum += count;
	uint8_t byte = 0;
	byte = readByte();
	checksum += byte;
	uint16_t adr = 0 | (byte << 8);
	byte = readByte();
	checksum += byte;
	adr |= byte;
	
	// Type
	uint8_t type = readByte();

	
	if (type == 1)
		return -1; // EOF
	else if (type != 0)
	{
		skipLine();
		return 0;
	}
	
	// Data
	int i;
	for (i = 0; i < count; i++)
	{
		byte = readByte();
		checksum += byte;
		data[adr + i] = byte;
	}
		
	
	// Checksum
	checksum += readByte();
	
    if (checksum != 0)
		return -2;

	
	skipLine();
	return count;
}

uint8_t readByte()
{
	uint8_t byte = 0;
	char hex[2];
	if (read(hexFd, &hex[0], 2) < 2)
	{
	//	printf("Unexpected EOF (Byte).\n");
		return 0xFF;
	}
		
	if (hex[0] >= '0' && hex[0] <= '9')
		byte |= (hex[0] - '0') << 4;
	else if (hex[0] >= 'A' && hex[0] <= 'F')
		byte |= (hex[0] - 'A' + 10) << 4;
	else
	{
		//printf("Invalid char '%c'", hex[0]);
		return 0xFF;
	}
	
	if (hex[1] >= '0' && hex[1] <= '9')
		byte |= (hex[1] - '0');
	else if (hex[1] >= 'A' && hex[1] <= 'F')
		byte |= (hex[1] - 'A' + 10);
	else
	{
		//printf("Invalid char '%c'", hex[1]);
		return 0xFF;
	}
	
	return byte;
}

