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

#define CMD_CI B0000
#define CMD_SOTBAT 0010b
#define CMD_TBRD B1000
#define CMD_TBRD_POSI B1001
#define CMD_TBRD_POSD 1010b 
#define CMD_TBRD_PREI 1011b
#define CMD_TBWR 1100b
#define CMD_TBWR_POSI2 B1101
#define CMD_TBWR_SP_POSI2 1110b
#define CMD_TBWR_SP 1111b

#include <inttypes.h>

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

uint16_t readID()
{
  uint8_t lo = 0;
  uint8_t hi = 0;
  uint16_t res = 0;
  
  cmdOut(CMD_CI, 0x0E3F);
  cmdOut(CMD_CI, 0x6EF8);
  cmdOut(CMD_CI, 0x0EFF);
  cmdOut(CMD_CI, 0x6EF7);
  cmdOut(CMD_CI, 0x0EFE);
  cmdOut(CMD_CI, 0x6EF6);
  
  lo = cmdIn(CMD_TBRD_POSI, 0);
  hi = cmdIn(CMD_TBRD, 0);
  
  res = hi;
  res <<= 8;
  res |= lo;
  
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
 Serial.end();
}
