#define WR_CHNK_SIZE 32

int picIdent();
int tinyReadPage(unsigned char pageNo);
int tinyWritePage(unsigned char pageNo, unsigned char* data);
int tinyChipErase();
