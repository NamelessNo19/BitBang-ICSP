#include "attiny13Atools.h"
#include <util/crc16.h>

int ledpwr;
char inbuf[2];
int err;

uint16_t datBuf[16];


void setup()
{
  pinMode(pinStart, INPUT);
  pinMode(pinMISO, INPUT);
  pinMode(pinSCK, OUTPUT);
  pinMode(pinMOSI, OUTPUT);
  pinMode(pinRST, OUTPUT);
  pinMode(pinVDD, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(pinStart, LOW);
  digitalWrite(pinMISO, LOW);
  digitalWrite(pinSCK, LOW);
  digitalWrite(pinMOSI, LOW);
  digitalWrite(pinVDD, LOW);
  digitalWrite(pinRST, LOW);
  err = -1;
  ledpwr = 0;
 Serial.begin(9600);
 Serial.print("SYNC");
 Serial.flush();
 delay(100);
 Serial.print("!"); 
}

void loop()
{
  inbuf[0] = 0;
  Serial.flush();
  while (Serial.available() < 2)
  {
  ledpwr = !ledpwr;
  digitalWrite(13, ledpwr);
  delay(500);
  }
  
  Serial.readBytes(&inbuf[0], 2);
  
  if (inbuf[0] == 'I' && inbuf[1] == 'D')
  {
    ident();
  }
  else if (inbuf[0] == 'R' && inbuf[1] == 'P')
  {
    
    while (Serial.available() == 0) delay(100);
    Serial.readBytes(&inbuf[0], 1);
    rdPage(inbuf[0]);
  }
  else if (inbuf[0] == 'W' && inbuf[1] == 'P')
  {
    while (Serial.available() == 0) delay(100);
    Serial.readBytes(&inbuf[0], 1);
    wrPage(inbuf[0]);
  }
}

int initTarget()
{
  err = 0;
  bootTarget();
  // Send PGM Enable
  if (!pgmEnable()) {
    err = 1;
    pwrOffTarget();
    Serial.print("SF");
    Serial.flush();
    return false;  
  }
  
  if(!checkID())
  {
    err = 2;
    pwrOffTarget();
    Serial.print("IM");
    Serial.flush();
    return false;  
  }
  
  return true;
}

void ident()
{
  err = 0;
  bootTarget();
  // Send PGM Enable
  if (!pgmEnable()) {
    err = 1;
    pwrOffTarget();
    Serial.print("SF");
    Serial.flush();
    return;  
  }
  
  uint8_t dat[3];
  
  readIdent(&dat[0]);
  pwrOffTarget();
  Serial.print("ID");
  Serial.write(dat[0]);
  Serial.write(dat[1]);
  Serial.write(dat[2]); 
  Serial.flush();  
}

void rdPage(uint8_t pgNo)
{
    if (!initTarget()) return;
    
    Serial.print("RP");
    
    readFlashPage(pgNo, &datBuf[0]);
    pwrOffTarget();
    int i;
    for (i = 0; i < 16; i++)
    {
      Serial.write((char) ((datBuf[i] & 0xFF00) >> 8));
      Serial.write((char)(datBuf[i] & 0x00FF));
      Serial.flush();
    }  
}

void wrPage(uint8_t pgNo)
{
  digitalWrite(13, HIGH);
  while (Serial.available() < 32) delay(100);
  
  uint16_t crc = 0xffff;
  
  char inbuf[2];
  int i;
  for (i = 0; i < 16; i++)
  {
    Serial.readBytes(&inbuf[0], 2);     
    datBuf[i] = 0 | ((uint16_t) inbuf[0] << 8) | inbuf[1];
    crc = _crc16_update(crc, inbuf[0]);
    crc = _crc16_update(crc, inbuf[1]);
  }
  
  Serial.write('W');
  Serial.write((char) ((crc & 0xFF00) >> 8));
  Serial.write((char)(crc & 0x00FF));
  Serial.write('P');
  Serial.flush();
  
  while (Serial.available() < 2) delay(100);
  Serial.readBytes(&inbuf[0], 2);
  if (inbuf[0] != 'A' || inbuf[1] != 'C')
    return;
    
  if (!initTarget())
    return;  
  
  
  for (i = 0; i < 16; i++)
    loadFlashWord(i, datBuf[i]);
 
  writeFlashPageNear(pgNo << 4);   
  
  pwrOffTarget();
  Serial.print("AC");

  return;  
  
}
