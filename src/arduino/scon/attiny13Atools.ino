
uint8_t rawByte(uint8_t data) {
  uint8_t buf = 0;
  uint8_t mask = 0;
  int i;
  for (i = 0; i < 8; i++) {
    clkLo;
    mask = (uint8_t) 0x80 >> i;
    del;
    // Write
    if ((data & mask) == 0) {
      digitalWrite(pinMOSI, LOW);
     } else {
       digitalWrite(pinMOSI, HIGH);
     }
    clkHi;
    // Read
    if (digitalRead(pinMISO)) {
      buf |= mask;
    }
    del;
  }
  clkLo;
  digitalWrite(pinMOSI, LOW);
  return buf;
}

void pwrOffTarget() {
  digitalWrite(pinSCK, LOW);
  digitalWrite(pinMOSI, LOW);
  digitalWrite(pinRST, HIGH);
  delay(25);
  digitalWrite(pinVDD, LOW);
  delay(25);
  digitalWrite(pinRST, LOW);
}

void bootTarget() {
  digitalWrite(pinVDD, LOW);
  digitalWrite(pinSCK, LOW);
  digitalWrite(pinMOSI, LOW);
  digitalWrite(pinRST, LOW);
  delay(10);
  digitalWrite(pinVDD, HIGH);
  delay(50);
}

int checkID() {
  
  uint8_t dat[3];
  
  readIdent(&dat[0]);
  
  return dat[0] == manID &&
         dat[1] == memID &&
         dat[2] == devID;
}  

void readIdent(uint8_t *dat)
{
  
  rawByte(0x30);
  rawByte(0x00);
  rawByte(0x00);
  dat[0] = rawByte(0x00); 
  
  rawByte(0x30);
  rawByte(0x00);
  rawByte(0x01);
  dat[1] = rawByte(0x00); 
  
  rawByte(0x30);
  rawByte(0x00);
  rawByte(0x02);
  dat[2] = rawByte(0x00); 
}

int pgmEnable() {
  rawByte(0xAC);
  rawByte(0x53);
  uint8_t echo = rawByte(0x00);
  rawByte(0x00);
  
  return echo == 0x53;
}

uint16_t readFlashWord(uint16_t adr) {
 const uint8_t hAdr = (uint8_t) ((adr & 0x0100) >> 8);
 const uint8_t lAdr = (uint8_t) (adr & 0x00FF);
 uint8_t low = 0;
 uint8_t high = 0;
 
 rawByte(0x20);
 rawByte(hAdr);
 rawByte(lAdr);
 low = rawByte(0x00);
 
 rawByte(0x28);
 rawByte(hAdr);
 rawByte(lAdr);
 high = rawByte(0x00);
 
 // Little Endian
 uint16_t dat = high;
 dat |= ((uint16_t) low) << 8;
 return dat;
}

void readFlashPage(uint8_t pageNo, uint16_t *buf)
{
   uint16_t wadr = (pageNo & 0x1F) << 4;
   int i;
   for (i = 0; i < 16; i++)
     buf[i] = readFlashWord(wadr + i);
}   

void loadFlashWord(uint16_t adr, uint16_t data) {

  if (data == 0xFFFF)
    return;
 
 // Little Endian
 const uint8_t lData = (uint8_t) ((data & 0xFF00) >> 8);
 const uint8_t hData = (uint8_t) (data & 0x00FF);
 
 const uint8_t lsbAdr = (uint8_t) (adr & 0x000F);
 
 // Load Low
 rawByte(0x40);
 rawByte(0x00);
 rawByte(lsbAdr);
 rawByte(lData);
 
 // Load High 
 rawByte(0x48);
 rawByte(0x00);
 rawByte(lsbAdr);
 rawByte(hData);

 return;
}

void writeFlashPageNear(uint16_t adr) {
 const uint8_t msbAdr = (uint8_t) (adr & 0x00F0);
 const uint8_t hAdr   = (uint8_t) ((adr & 0x0100) >> 8);
 
 rawByte(0x4C);
 rawByte(hAdr);
 rawByte(msbAdr);
 rawByte(0x00);
 
 delay(FLASH_WRITE_DEL);
}

void chipErase() {
 rawByte(0xAC);
 rawByte(0x80);
 rawByte(0x00);
 rawByte(0x00);
 delay(ERASE_DEL);
}

void writePage(int pageNo, prog_uint16_t *pg) {
 const uint16_t baseAdr = pageNo << 4;
 
 int i = 0;
 uint16_t data = 0;
 
 for (i = 0; i < 16; i++) {
   data = pgm_read_word_near(pg + i);
   loadFlashWord(i, data);
 }
 writeFlashPageNear(baseAdr);
}

 
  
