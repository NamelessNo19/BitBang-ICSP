#include <util/crc16.h>
#include "PIC18Fconst.h"

#define pinMCLR 11
#define pinPGD 7
#define pinPGC 5
#define pinPGM 3

#define pinStart 9
#define pinVSense 12
#define pinSpeaker 6

int ledpwr;
char inbuf[6];
int err;

char datBuf[32];
uint8_t tdat;

void setup()
{
  pinMode(pinMCLR, OUTPUT);
  pinMode(pinPGD, INPUT);
  pinMode(pinPGC, OUTPUT);
  pinMode(pinPGM, OUTPUT);
  pinMode(pinStart, INPUT);
  pinMode(pinVSense, INPUT);
  pinMode(pinSpeaker, OUTPUT);
  pinMode(13, OUTPUT);
  
  digitalWrite(pinMCLR, LOW);
  digitalWrite(pinPGD, LOW);
  digitalWrite(pinPGM, LOW);
  digitalWrite(pinPGC, LOW);
  digitalWrite(pinStart, LOW);
  digitalWrite(pinVSense, LOW);
  digitalWrite(pinSpeaker, LOW);
  
  
  
  err = -1;
  
 Serial.setTimeout(2000);
 Serial.begin(9600);
 Serial.write('S'); 
 Serial.write('Y'); 
 Serial.write('N');
 Serial.write('C'); 
 Serial.flush();
 tone(pinSpeaker, 880);
 delay(150);
 noTone(pinSpeaker);
 Serial.write('!'); 
}

void loop()
{
  digitalWrite(13, HIGH);
  inbuf[0] = 0;
  Serial.flush();
  while (Serial.available() < 2)
  {
  digitalWrite(13, HIGH);
  delay(200);
  digitalWrite(13, LOW);
  delay(200);
  }
  
  digitalWrite(13, LOW);
  
  Serial.readBytes(&inbuf[0], 2);
  
  if (inbuf[0] == 'I' && inbuf[1] == 'D')
  {
    ident();
  }
  else if (inbuf[0] == 'R' && inbuf[1] == 'B')
  {
    
    while (Serial.available() == 0) delay(100);
    Serial.readBytes(&inbuf[0], 1);
    rdBlock(inbuf[0]);
  }
  else if (inbuf[0] == 'W' && inbuf[1] == 'R')
  {
    picwrite();
  }
  /*
  else if (inbuf[0] == 'E' && inbuf[1] == 'R')
  {
    if (initTarget())
    {
      chipErase();
      pwrOffTarget();
      Serial.print("ER");
    }
    else
    pwrOffTarget();
  }
  */
  
  tone(pinSpeaker, 1188);
 delay(150);
 noTone(pinSpeaker);
}

int initTarget()
{
  err = 0;
  // Send PGM Enable
  if (!pgmEnable()) {
    err = 1;
    pwrOffTarget();
    Serial.print("SF");
    Serial.flush();
    return false;  
  }
  
  
  return true;
}

void ident()
{
  err = 0;
  // Send PGM Enable
  if (!pgmEnable()) {
    err = 1;
    pwrOffTarget();
    Serial.print("SF");
    Serial.flush();
    return;  
  }
  

  uint16_t id = readWord(MEM_DEVID1);
  
  Serial.print("ID");
  Serial.write((id & 0xFF00) >> 8);
  Serial.write(id & 0xFF);
  Serial.flush();  

  const uint8_t memlocs[] = {0, 1, 2, 3, 5, 6, 8, 9, 10, 11, 12, 13};

  uint8_t i;
  for (i = 0; i < 12; i++)
  {
	setTablePtr(MEM_CONFIG1L + memlocs[i]);
	Serial.write(cmdIn(CMD_IN_TBRD, 0));
   
  }
  
  pwrOffTarget();
}

void rdBlock(const uint8_t blockNo)
{
    if (!initTarget()) return;
    


    uint32_t blkSA;
    if (blockNo < 4)
        blkSA = blockNo * 0x2000;
    else
    {
    	Serial.print("IA");
    	pwrOffTarget();
    	return;
    }

    Serial.print("RB");



    uint16_t pgNo;
    uint8_t i;
    
    setTablePtr(blkSA);
    for (pgNo = 0; pgNo < 256; pgNo++)
    {
    	Serial.flush();

    	for (i = 0; i < 32; i++)
    		Serial.write(readByte());

    	if (Serial.readBytes(&inbuf[0], 2) != 2)
    	{
    		Serial.print("TC");
    		break;
    	}

    	if (inbuf[0] != 'A' || inbuf[1] != 'C')
    	{
    		Serial.print("TC");
    		break;
    	}
    }

    Serial.print("AC");
    pwrOffTarget();
}


void picwrite()
{
// if (!initTarget()) return;
  
// setAccessToFlash();
  
  Serial.print("WR");
  inbuf[0] = 0;
  int i;

  uint32_t base;
  while (1)
  {
    if (Serial.readBytes(&inbuf[0], 6) != 6)
    {
      Serial.print("TC");
      break;
      return;
     }
     
     if (inbuf[0] != 'C' || inbuf[5] != 'W')
     {
       if (inbuf[0] != 'S' || inbuf[5] != 'T');
         Serial.print("IC");
       break;
     }
     
     base = inbuf[4];
     base |= inbuf[3] << 8;
     base |= inbuf[2] << 16;
     base |= inbuf[1] << 24;
     
     if (base > 0x7FFF)
     {
       Serial.print("IA");
       break;
     }
     Serial.print("CW");
     
    // setTablePtr(base);
     
    // Read Data
    if (Serial.readBytes(&datBuf[0], 32) != 32)
    {
      Serial.print("TC");
      break;
      return;
    }
    
   for (i = 0; i < 15; i++)
    {
     tdat = 0;
     tdat += datBuf[2 * i + 1] * 256;
     tdat += datBuf[2 * i];
  
     // dat = 0x83 | (0x6A << 8);//
      //cmdOut(CMD_OUT_TBWR_POSI2, dat);
      
      if (datBuf[2 * i] == 0x83 && datBuf[2 * i + 1] == 0x6A)
        tone(pinSpeaker, 1440);
      else
        tone(pinSpeaker, 1440);
       delay(200);
       noTone(pinSpeaker);
       delay(200);
       
       if (tdat == 0x6A83)
        tone(pinSpeaker, 1440);
      else
        tone(pinSpeaker, 440);
       delay(200);
       noTone(pinSpeaker);
       delay(200);
       
       return; 

    }
    
    dat = datBuf[30] | (datBuf[31] << 8);
    cmdOut(CMD_OUT_TBWR_SP, dat);
    clkFlashWrite(); 
    Serial.print("AW");
  }
 //disableWrite();
 //pwrOffTarget();
  Serial.print("AC");
}
