#include <util/crc16.h>
#include "PIC18Fconst.h"

// #define SEVSEG
#ifdef SEVSEG
#include "sevsegconst.h"
#endif

#define pinMCLR 11
#define pinPGD 10
#define pinPGC 9
#define pinPGM 8



#ifdef SEVSEG
#define pin4094SI 4
#define pin4094CK 3
#endif

//#define pinStart 9


#define pinVSense 12
#define pinSpeaker 6

#define WR_CHNK_SIZE 32

int ledpwr;
uint8_t inbuf[6];
int err;

uint8_t datBuf[WR_CHNK_SIZE];
uint32_t chnkAdr;


void setup()
{
  pinMode(pinMCLR, OUTPUT);
  pinMode(pinPGD, INPUT);
  pinMode(pinPGC, OUTPUT);
  pinMode(pinPGM, OUTPUT);
  //pinMode(pinStart, INPUT);
  pinMode(pinVSense, INPUT);
  pinMode(pinSpeaker, OUTPUT);
  pinMode(13, OUTPUT);
  
  

  digitalWrite(pinMCLR, LOW);
  digitalWrite(pinPGD, LOW);
  digitalWrite(pinPGM, LOW);
  digitalWrite(pinPGC, LOW);
  //digitalWrite(pinStart, LOW);
  digitalWrite(pinVSense, LOW);
  digitalWrite(pinSpeaker, LOW);
  
  #ifdef SEVSEG
  pinMode(pin4094SI, OUTPUT);
  pinMode(pin4094CK, OUTPUT);
  digitalWrite(pin4094SI, LOW);
  digitalWrite(pin4094CK, LOW);
  #endif

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
    
    #ifdef SEVSEG
    segWrite(SEG_o);
    #endif
    
    delay(300);
    digitalWrite(13, LOW);
    
    #ifdef SEVSEG
    segWrite(SEG_o_high);
    #endif
    
    delay(300);
  }

  digitalWrite(13, LOW);
  
   #ifdef SEVSEG
   segWrite(SEG_b);
   #endif

  Serial.readBytes((char*) &inbuf[0], 2);

  if (inbuf[0] == 'I' && inbuf[1] == 'D')
  {
   
    ident();
  }
  else if (inbuf[0] == 'R' && inbuf[1] == 'B')
  {
    while (Serial.available() == 0) delay(100);
    Serial.readBytes((char*) &inbuf[0], 1);
    #ifdef SEVSEG
    segWrite(segNum(inbuf[0], false));
    #endif
    rdBlock(inbuf[0]);
  }
  else if (inbuf[0] == 'W' && inbuf[1] == 'R')
  {
    picwrite();
  }
  else if (inbuf[0] == 'E' && inbuf[1] == 'R')
  { 
    erBlock();
  }
  else if (inbuf[0] == 'C' && inbuf[1] == 'W')
  { 
    writeConf();
  }
  else
  {
    // Flushing inbput buffer
    tone(pinSpeaker, 330);
    delay(250);
    while (Serial.available() > 0) Serial.read();
  }


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

  const uint8_t memlocs[] = {
    0, 1, 2, 3, 5, 6, 8, 9, 10, 11, 12, 13        };

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

    if (Serial.readBytes((char*) &inbuf[0], 2) != 2)
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
  if (!initTarget()) return;
   Serial.print("WR");
   
   setAccessToFlash(); 
   boolean cont = true;
   
   while(cont)
   {
    
    // New Chunk or stop 
    if (Serial.readBytes((char*) &inbuf[0], 2) != 2)
    {
      Serial.print("TC");
      break;
    }
    
     if (inbuf[0] != 'N' || inbuf[1] != 'C')
     {
       cont = false;
     }
     else
     {
       Serial.print("NC");
       if (!receiveChunk())
       {
         // Receive Failed
         pwrOffTarget();
         #ifdef SEVSEG
         segWrite(SEG_F);
         #endif
         delay(1000);
         return;
       }
     
       // Write
       setTablePtr(chnkAdr);
       writeChunkToFlash();  
     }
   } // while(cont)
   
   
   // Confirm
   Serial.print("WS");
   #ifdef SEVSEG
   segWrite(SEG_d);
   delay(1000);
   #endif
   
   pwrOffTarget();
   
}

/*
void writeChunk()
{
   cmdOut(CMD_OUT_TBWR_POSI2, (datBuf[1]   << 8) | datBuf[0]);
   cmdOut(CMD_OUT_TBWR_POSI2, (datBuf[3]   << 8) | datBuf[2]);
   cmdOut(CMD_OUT_TBWR_POSI2, (datBuf[5]   << 8) | datBuf[4]);
   cmdOut(CMD_OUT_TBWR_POSI2, (datBuf[7]   << 8) | datBuf[6]);
   cmdOut(CMD_OUT_TBWR_POSI2, (datBuf[9]   << 8) | datBuf[8]);
   cmdOut(CMD_OUT_TBWR_POSI2, (datBuf[11]  << 8) | datBuf[10]);
   cmdOut(CMD_OUT_TBWR_POSI2, (datBuf[13]  << 8) | datBuf[12]);
   cmdOut(CMD_OUT_TBWR_POSI2, (datBuf[15]  << 8) | datBuf[14]);
   cmdOut(CMD_OUT_TBWR_POSI2, (datBuf[17]  << 8) | datBuf[16]);
   cmdOut(CMD_OUT_TBWR_POSI2, (datBuf[19]  << 8) | datBuf[18]);
   cmdOut(CMD_OUT_TBWR_POSI2, (datBuf[21]  << 8) | datBuf[20]);
   cmdOut(CMD_OUT_TBWR_POSI2, (datBuf[23]  << 8) | datBuf[22]);
   cmdOut(CMD_OUT_TBWR_POSI2, (datBuf[25]  << 8) | datBuf[24]);
   cmdOut(CMD_OUT_TBWR_POSI2, (datBuf[27]  << 8) | datBuf[26]);
   cmdOut(CMD_OUT_TBWR_POSI2, (datBuf[29]  << 8) | datBuf[28]);
   cmdOut(CMD_OUT_TBWR_SP,    (datBuf[31]  << 8) | datBuf[30]);
   clkFlashWrite();
} */

void writeChunkToFlash()
{
  int i;
  uint16_t dat;
  for (i = 0; i < ((WR_CHNK_SIZE / 2) - 1); i++)
   {
     dat = datBuf[(2 * i) + 1];
     dat <<= 8;
     dat |= datBuf[2 * i];       
     
  //   dat = chnkAdr & 0x0000FFFF;
     
     cmdOut(CMD_OUT_TBWR_POSI2, dat);
   } 
   
   dat = datBuf[WR_CHNK_SIZE - 1];
   dat <<= 8;
   dat |= datBuf[WR_CHNK_SIZE - 2];
   
  //  dat = chnkAdr & 0x0000FFFF;
   
   cmdOut(CMD_OUT_TBWR_SP, dat);
   
   
   clkFlashWrite();
}

boolean receiveChunk()
{
  // Read address
  if (Serial.readBytes((char*) &inbuf[0], 6) != 6)
  {
    Serial.print("TC"); // Read Timeout
    return false;
  }
  
  if (inbuf[0] != 'C' || inbuf[5] != 'W')
  {
    Serial.print("IC"); // Invalid command
    return false;
  }
  
  uint16_t checksum = 0xFFFF;
  
  chnkAdr = inbuf[1];
  chnkAdr <<= 8;
  chnkAdr |= inbuf[2];
  chnkAdr <<= 8;
  chnkAdr |= inbuf[3];
  chnkAdr <<= 8;
  chnkAdr |= inbuf[4];  
  
  if (chnkAdr > 0x007FFF)
  {
    Serial.print("AH"); // Invalid address
    return true;
  }
  
  // CRC16 address
  checksum = _crc16_update(checksum, inbuf[1]);
  checksum = _crc16_update(checksum, inbuf[2]);
  checksum = _crc16_update(checksum, inbuf[3]);
  checksum = _crc16_update(checksum, inbuf[4]);
  
  Serial.print("CW"); // Acknowledge address
  Serial.flush();
  
  // Read Chunk data
  if (Serial.readBytes((char*) &datBuf[0], WR_CHNK_SIZE) != WR_CHNK_SIZE)
  {
    Serial.print("TC"); // Read Timeout
    return false;
  }
  
  // CRC-16 data
  int i;
  for (i = 0; i < WR_CHNK_SIZE; i++)
    checksum = _crc16_update(checksum, datBuf[i]);
    
  // Send checksum
  Serial.write('C');
  Serial.write(checksum >> 8);
  Serial.write(checksum & 0x00FF);
  Serial.write('S');
  Serial.flush();
  
  // Wait for confirmation
  if (Serial.readBytes((char*) &inbuf[0], 2) != 2)
  {
    Serial.print("TC"); // Read Timeout
    return false;
  }
  
  if (inbuf[0] != 'A' || inbuf[1] != 'C')
  {
    Serial.print("ST"); // Invalid Checksum
    return false;
  }

  Serial.print("AC");
  Serial.flush();
  return true;  
}
  
 
void writeConf()
{
  if (!initTarget()) return;
  Serial.print("CW");
  
  int rc;
  bool confStop = false;
  while (!confStop) 
  {
    rc = Serial.readBytes((char*) &inbuf[0], 2);
    if (rc != 2 || inbuf[0] != 'C' || inbuf[1] != 'G')
      confStop = true;
    else
      if (!writeConfReg())
      {
        tone(pinSpeaker, 700);
        delay(150);
        noTone(pinSpeaker);
        delay(150);
        tone(pinSpeaker, 700);
        delay(150);
        noTone(pinSpeaker);
        confStop = true;
      }
  }
  pwrOffTarget();
}
  
bool writeConfReg()
{
  Serial.print("CG");
  
  int rc = Serial.readBytes((char*) &inbuf[0], 4);
  
  if (rc != 4)
  {
    Serial.print("TC");
    return false;
  }
  
  uint8_t reg = inbuf[1];
  uint8_t dat = inbuf[3];
  
  if (inbuf[0] != 'A' || inbuf[2] != 'C')
  {
    Serial.print("ST");
    return false;
  }
  
  Serial.write('C');
  Serial.write(dat);
  Serial.write('A');
  Serial.write(reg);
  
  rc = Serial.readBytes((char*) &inbuf[0], 2);
  
  if (inbuf[0] != 'A' || inbuf[1] != 'C')
    return false;
  
  
  if (reg > 0x0D || reg == 0x04 || reg == 0x07)
  {
    Serial.print("IA");
    return false;
  } 
  
  writeConfig(0x300000L + (uint32_t) reg, dat);
  
  Serial.print("CW");
  Serial.flush();
  return true;
}  
  
  

void erBlock()
{

  uint16_t eropt;
  Serial.print("ER");

  int res = Serial.readBytes((char*) &inbuf[0], 1);
  
  if (res != 1)
  {
    Serial.print("TC");
    return;
  }
  
  switch(inbuf[0])
  {
  case 0: 
    eropt = BLKER_CEP_B0; 
    break;
  case 1: 
    eropt = BLKER_CEP_B1; 
    break;
  case 2: 
    eropt = BLKER_CEP_B2; 
    break;
  case 3: 
    eropt = BLKER_CEP_B3; 
    break;
    //case 4: eropt = BLKER_CEP_B4; break;
    //case 5: eropt = BLKER_CEP_B5; break;
  case 0x0B: 
    eropt = BLKER_BB;  
    break;
  case 0x0C: 
    eropt = BLKER_CB;  
    break;
  case 0xCE: 
    eropt = BLKER_CE;  
    break;
  default:
    Serial.print("IA");
    return;
  }

  if (!initTarget()) return;
  bulkErase(eropt);
  pwrOffTarget();
  

  Serial.print("EC");
}




