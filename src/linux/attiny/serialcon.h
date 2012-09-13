#ifndef _SERIALCON_
#define _SERIALCON_

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#include <stddef.h>
#include <inttypes.h>

int serOpen(char *port);
void serClose();
void serWrite(char *buf, size_t len);
int serRead(char *buf, size_t len, int fillBuffer);
uint16_t crc16_update(uint16_t crc, uint8_t a);
void flushInBuf();

#endif