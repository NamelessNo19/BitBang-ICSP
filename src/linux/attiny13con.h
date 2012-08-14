int tinyIdent();
int tinyReadPage(unsigned char pageNo);
int tinyWritePage(unsigned char pageNo, char* data);
uint16_t crc16_update(uint16_t crc, uint8_t a)