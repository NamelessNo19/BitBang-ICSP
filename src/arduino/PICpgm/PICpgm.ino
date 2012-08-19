#define pinMCLR 11
#define pinPGD 7
#define pinPGC 5
#define pinPGM 3

#define pinStart 9

#define clkHI digitalWrite(pinPGC, HIGH);
#define clkLO digitalWrite(pinPGC, LOW);
#define datIN pinMode(pinPGD, INPUT)
#define datOUT pinMode(pinPGD, OUTPUT);
#define del delayMicroseconds(500)


#include <inttypes.h>
#include "PIC18Fconst.h"

void setup()
{
  pinMode(pinMCLR, OUTPUT);
  pinMode(pinPGD, INPUT);
  pinMode(pinPGC, OUTPUT);
  pinMode(pinPGM, OUTPUT);
  pinMode(pinStart, INPUT);
  digitalWrite(pinMCLR, LOW);
  digitalWrite(pinPGD, LOW);
  digitalWrite(pinPGM, LOW);
  digitalWrite(pinPGC, LOW);
  digitalWrite(pinStart, LOW);
}

void enterPM()
{
  digitalWrite(pinMCLR, LOW);
  digitalWrite(pinPGD, LOW);
  digitalWrite(pinPGM, LOW);
  digitalWrite(pinPGC, LOW);
  datIN;
  del; del; del; del;
  digitalWrite(pinPGM, HIGH);
  del;
  digitalWrite(pinMCLR, HIGH);
  del;
  datOUT;
}

void exitPM()
{
  clkLO;
  datIN;
  del;
  digitalWrite(pinMCLR, LOW);
  del;
  digitalWrite(pinPGM, LOW);
  del;
}


void cmdOut(uint8_t cmd, uint16_t dat)
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
  for (i = 0; i < 16; i++)
  {
    if ((dat & 0x01) == 0)
      digitalWrite(pinPGD, LOW);
    else
     digitalWrite(pinPGD, HIGH);
    
    clkHI; del; clkLO; del;
    
    dat = dat >> 1;
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


inline uint16_t readID()
{

  uint16_t res;

  setTablePtr(MEM_DEVID1); 
  res = cmdIn(CMD_IN_TBRD_POSI, 0);
  res  |= cmdIn(CMD_IN_TBRD, 0) << 8;
  return res;
}

void loop()
{
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
 while (!digitalRead(pinStart)) delay(100);
 digitalWrite(13, LOW);
 enterPM();
 del;
 uint16_t id = readID();
 exitPM();
 Serial.begin(9600);
 Serial.print(id, HEX);
 Serial.write('\n');
 delay(500);
 Serial.end();
}
