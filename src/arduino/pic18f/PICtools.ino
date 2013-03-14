#define clkHI digitalWrite(pinPGC, HIGH);
#define clkLO digitalWrite(pinPGC, LOW);
#define datIN pinMode(pinPGD, INPUT)
#define datOUT pinMode(pinPGD, OUTPUT);

#ifdef __RASPI__
#define del delayMicroseconds(clkDel)
#elif
#define del delayMicroseconds(10)
#endif

#define DEVID 0x1200

#ifndef __RASPI__
#include <inttypes.h>
#include "PIC18Fconst.h"
#endif 

int cntr;

int pgmEnable()
{

#ifndef __RASPI__
  if (!digitalRead(pinVSense))
    return false;
    
  digitalWrite(pinMCLR, LOW);
#endif 
 
  digitalWrite(pinPGD, LOW);
  digitalWrite(pinPGM, LOW);
  digitalWrite(pinPGC, LOW);
  datIN;
  del; del;
  digitalWrite(pinPGM, HIGH);
  delayMicroseconds(DELAY_P15);
  
#ifndef __RASPI__  
  digitalWrite(pinMCLR, HIGH);
#elif
  printf("\n-> Power Vpp now...");
  fflush(stdout);
  sleep(3);
  printf(" GO!\n");
#endif
  
  delayMicroseconds(DELAY_P12);
  datOUT;
  
  return (readWord(MEM_DEVID1) & 0xFFE0) == DEVID;
}

void pwrOffTarget()
{
  clkLO;
  datIN;
  del;
#ifndef __RASPI__  
  digitalWrite(pinMCLR, LOW);
#elif
  printf("\n-> Disconnect Vpp now...");
  fflush(stdout);
  sleep(3);
  printf(" Finished!\n");
#endif
  del;
  digitalWrite(pinPGM, LOW);
  del;
}


void cmdOut(const uint8_t cmd, const uint16_t dat)
{
  datOUT;
  int i;
  // Shift out command
  
  uint8_t mask8 = 0x01;
  
  for (i = 0; i < 4; i++)
  {
    if ((cmd & mask8) == 0)
      digitalWrite(pinPGD, LOW);
    else
     digitalWrite(pinPGD, HIGH);
    
    clkHI; del; clkLO; del;
    
    mask8 <<= 1;
  }
  
  del; del; del;
  
  uint16_t mask16 = 0x0001;
  // Shift out data
  for (i = 0; i < 16; i++)
  {
    if ((dat & mask16) == 0)
      digitalWrite(pinPGD, LOW);
    else
     digitalWrite(pinPGD, HIGH);
    
    clkHI; del; clkLO; del;
    
    mask16 <<= 1;
  }
  
}

uint8_t cmdIn(uint8_t cmd, uint8_t dat)
{
  datOUT;
  int i;
  // Shift out command
  for (i = 0; i < 4; i++)
  {
    if ((cmd & 0x01) == 0)
      digitalWrite(pinPGD, LOW);
    else
     digitalWrite(pinPGD, HIGH);
    
    clkHI; del; clkLO; del;
    
    cmd = cmd >> 1;
  }
  
  // Shift out data
  for (i = 0; i < 8; i++)
  {
    if ((dat & 0x01) == 0)
      digitalWrite(pinPGD, LOW);
    else
     digitalWrite(pinPGD, HIGH);
    
    clkHI; del; clkLO; del;
    
    dat = dat >> 1;
  }
  
  uint8_t rd = 0;
  datIN;
  del;
  uint8_t mask = 0x01;
  
  for (i = 0; i < 8; i++)
  {
    clkHI;
    del;
    if (digitalRead(pinPGD))
      rd |= mask;
    mask = mask << 1;
    clkLO
    del;
  }
  
  datOUT;
  return rd;
  
}

void clkFlashWrite()
{
  datOUT;
  digitalWrite(pinPGD, LOW);
  int i;
  // 3 Clocks
  for (i = 0; i < 3; i++)
  {
    clkHI; del; clkLO; del;
    
  }
  
  clkHI;
  delayMicroseconds(DELAY_P9);
  del;
  clkLO;
  delayMicroseconds(DELAY_P10);
  del;
  
  // 16 Clocks
  for (i = 0; i < 16; i++)
  {
    clkHI; del; clkLO; del;
    
  }
  
}

void setTablePtr(const uint8_t up, const uint8_t high, const uint8_t low)
{
  cmdOut(CMD_OUT_CI, 0x0E00 | up);
  cmdOut(CMD_OUT_CI, 0x6EF8);
  cmdOut(CMD_OUT_CI, 0x0E00 | high);
  cmdOut(CMD_OUT_CI, 0x6EF7);
  cmdOut(CMD_OUT_CI, 0x0E00 | low);
  cmdOut(CMD_OUT_CI, 0x6EF6);

} 

void setTablePtr(uint32_t memAdr)
{
  cmdOut(CMD_OUT_CI, 0x0E00 | (uint16_t) (0x0000FFL & memAdr));
  cmdOut(CMD_OUT_CI, 0x6EF6);
  memAdr >>= 8;
  cmdOut(CMD_OUT_CI, 0x0E00 | (uint16_t) (0x0000FFL & memAdr));
  cmdOut(CMD_OUT_CI, 0x6EF7);
  memAdr >>= 8;
  cmdOut(CMD_OUT_CI, 0x0E00 | (uint16_t) (0x0000FFL & memAdr));
  cmdOut(CMD_OUT_CI, 0x6EF8);
}


inline uint16_t readWord(uint32_t memAdr)
{
  uint16_t res;
  setTablePtr(0x3FFFFEL & memAdr); 
  res = cmdIn(CMD_IN_TBRD_POSI, 0);
  res  |= cmdIn(CMD_IN_TBRD, 0) << 8;
  return res;
}

inline uint8_t readByte()
{
	return cmdIn(CMD_IN_TBRD_POSI, 0);
}

inline void setAccessToFlash()
{
  cmdOut(CMD_OUT_CI, 0x8EA6);  // BSF  EECON1, EEPGD
  cmdOut(CMD_OUT_CI, 0x9CA6);  // BCF  EECON1, CFGS
}

inline void enableWrite()
{
	cmdOut(CMD_OUT_CI, 0x84A6);  // BSF  EECON1, WREN
}

inline void disableWrite()
{
	cmdOut(CMD_OUT_CI, 0x94A6);  // BCF  EECON1, WREN
}

inline void performRowErase()
{
	cmdOut(CMD_OUT_CI, 0x88A6);  // BSF  EECON1, FREE
	cmdOut(CMD_OUT_CI, 0x82A6);  // BSF  EECON1, WR
	clkFlashWrite();
}


void writeTest()
{
  setAccessToFlash();
  setTablePtr(0x000800);
  cmdOut(CMD_OUT_TBWR_POSI2, 0xFB0B);
  cmdOut(CMD_OUT_TBWR_SP_POSI2, 0xFACA);
  clkFlashWrite();
}

void bulkErase(const uint16_t erOpt)
{
	uint16_t optLo = erOpt & 0x00FF;
	uint16_t optHi = ((erOpt & 0xFF00) >> 8) & 0x00FF;

        optLo |= optLo << 8;
        optHi |= optHi << 8;

	setTablePtr(MEM_BLKER_OPTH);
	cmdOut(CMD_OUT_TBWR, (optHi << 8) | optHi);
	setTablePtr(MEM_BLKER_OPTL);
	cmdOut(CMD_OUT_TBWR, (optLo << 8) | optLo);

        cmdOut(CMD_OUT_CI, 0); // NOP

	// Clocking erase
	datOUT;
	digitalWrite(pinPGD, LOW);
	int i;
	// 4 Clocks
	for (i = 0; i < 4; i++)
	  {
	    clkHI; del; clkLO; del;

	  }

	delayMicroseconds(DELAY_P11);
	delayMicroseconds(DELAY_P10);

	// 16 Clocks
	for (i = 0; i < 16; i++)
	  {
	    clkHI; del; clkLO; del;
	  }
}

void writeConfig (const uint32_t confReg, const uint8_t conf)
{
 if (confReg < 0x300000L)
   return;
   
  setAccessToFlash();
  setTablePtr(confReg);
  
  uint16_t payload = conf;
  if ((confReg % 2) == 1)
    payload <<= 8;
    
 cmdOut(CMD_OUT_TBWR_SP, payload);
 clkFlashWrite(); 
  
}
