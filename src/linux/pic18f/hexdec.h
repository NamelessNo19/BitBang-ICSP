#ifndef __HEXDEC__
#define __HEXDEC__

#define LINE_BUF_SIZE 100
#define MAX_DATA 1024

#include <inttypes.h>
#include <stddef.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

typedef struct dataSeq_s
{
	uint32_t baseAdr;
	uint32_t length;
	uint8_t *data;
} datSeq_t;


size_t parseHexFile(int hexFileDesc, datSeq_t* buf);
void sortSeq(datSeq_t* buf);
void cleanUpSeq(datSeq_t* buf);
size_t seqToByteArray(datSeq_t* buf, uint8_t* raw, size_t start, size_t len);

#endif
