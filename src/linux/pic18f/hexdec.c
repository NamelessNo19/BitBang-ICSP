#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>


#include "hexdec.h"

int hexFd;


int skipLine();
long parseHexLine(datSeq_t* buf);
uint8_t readByte();


uint16_t ulba; // Upper Linear Base Adress


size_t parseHexFile (int hexFileDesc, datSeq_t* buf)
{
	
	hexFd = hexFileDesc;
	int cnt;
	long total = 0;
	long pos = 0;
	int segCount = 0;
	ulba = 0;
   
	while ((cnt = parseHexLine(buf + segCount)) >= 0)
	{
    	total += cnt;
		if (cnt > 0)
			segCount++;
	}

	buf[segCount].length = 0;
	sortSeq(buf);

	/*
	printf("Allocated %d segments.\n", segCount);

	int i;
	for(i = 0; i < segCount; i++)
		printf("Seg. %d:\t %d Bytes \t @ 0x%08x\n", i, buf[i].length, buf[i].baseAdr); */
	
   if (cnt == -1)
   {
	   return total;
   }
   else 
   {
	   return -1;
   }
   
}

void cleanUpSeq(datSeq_t* buf)
{
  // printf("Starting cleanup.\n");
	int i;
	for (i = 0; buf[i].length != 0; i++)
	  {
	    //	    printf("Free buf[%d]\n", i);
	    free(buf[i].data);
	  }
	printf("Released %d Hexfile segments.\n", i);
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

long parseHexLine(datSeq_t* buf)
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
	checksum += type;

	
	if (type == 1)
		return -1; // EOF
	else if (type == 0)	// Data
	{
		buf->baseAdr = (ulba << 16) + adr;
		buf->length = count;
		//	printf("Alloc %d Bytes for base 0x%08x\n", count, buf->baseAdr);
		buf->data = malloc(count);

		int i;
		for (i = 0; i < count; i++)
		{
			byte = readByte();
			checksum += byte;
			buf->data[i] = byte;
		}

	}
	else if (type == 4) // Ext. Adr.
	{
		byte = readByte();
		checksum += byte;
		ulba = byte << byte;
		byte = readByte();
		checksum += byte;
		ulba += byte;
		count = 0;
	}
	else
	{
	skipLine();
	return 0;
	}
		
	
	// Checksum
	checksum += readByte();
	

	skipLine();

    if (checksum != 0)
	{
		printf("Checksum missmatch.\n");
		return -2;
	}
	
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


int comp_seq(const void *a, const void* b)
{
 const datSeq_t* seqA = (const datSeq_t*) a;
 const datSeq_t* seqB = (const datSeq_t*) b;

 if (seqA->length == 0 && seqB->length == 0)
	return 0;

 if (seqA->length == 0 || seqB->length == 0)
	return (seqA->length == 0) ? 1 : -1;

 return seqA->baseAdr - seqB->baseAdr;
}

void sortSeq(datSeq_t* buf)
{
	int i;
	for (i = 0; buf[i].length != 0; i++);

	if (i == 0)
		return;

	qsort(buf, i, sizeof(datSeq_t), comp_seq);
}

size_t seqToByteArray(datSeq_t* buf, uint8_t* raw, size_t start, size_t len)
{
	const size_t limit = start + len;

	size_t i;
	for (i = 0; (buf[i].baseAdr + buf[i].length - 1) < start; i++)
		if (buf[i].length == 0)
			return 0;

	if (start + len <= buf[i].baseAdr)
		return 0;


	size_t written = 0;
	size_t pos = (start < buf[i].baseAdr) ? buf[i].baseAdr : start;
	size_t gap = 0;

	while (pos < limit)
	{
		while (pos < buf[i].baseAdr + buf[i].length)
		{
			raw[pos - start] = buf[i].data[pos - buf[i].baseAdr];
			written++;
			pos++;
			if (pos - start >= len)
				return written;
		}

		gap = buf[i].baseAdr + buf[i].length;
		i++;
		if (buf[i].length == 0)
			return written;
		gap = buf[i].baseAdr - gap;

		pos += gap;
	}

	return written;

}
