#ifndef _ATTINY13TOOLS_
#define _ATTINY13TOOLS_

#include <inttypes.h>
#include <avr/pgmspace.h>

#define del delayMicroseconds(200) // 5 kHz
#define clkHi digitalWrite(pinSCK, HIGH)
#define clkLo digitalWrite(pinSCK, LOW)

#define manID 0x1E
#define memID 0x90
#define devID 0x07

#define FLASH_WRITE_DEL 5
#define ERASE_DEL 10

PROGMEM prog_uint16_t page0[] =
 {0x0ee1, 0x07bb, 0x08bb, 0x0000, 0xb09b, 0x02c0, 0xc398, 0xfbcf,
  0xc39a, 0xf9cf, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff};


const int pinStart = 4;
const int pinMISO = 7;
const int pinSCK = 8;
const int pinMOSI = 9;
const int pinRST = 11;
const int pinVDD = 12;

uint8_t rawByte(uint8_t data);
void pwrOffTarget();
int checkID();
void pwrOffTarget();
void bootTarget();
int pgmEnable();
uint16_t readFlashWord(uint16_t adr);
void loadFlashWord(uint16_t adr, uint16_t data);
void writeFlashPageNear(uint16_t adr);
void chipErase();
void readIdent(uint8_t *dat);
void writePage(int pageNo, prog_uint16_t *pg);

#endif
