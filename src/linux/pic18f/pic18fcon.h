#ifndef __PICCON__
#define __PICCON__

#define WR_CHNK_SIZE 32

extern const char* CONF_REG_STR[];
extern const unsigned char CONF_REG_ADR[];
extern const unsigned char CONF_REG_DEF[];


int picIdent();

#endif
