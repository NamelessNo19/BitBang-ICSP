class Pic18fBase(object):
    CMD_IN_SOTBAT           = 0b0010
    CMD_IN_TBRD             = 0b1000
    CMD_IN_TBRD_POSI        = 0b1001
    CMD_IN_TBRD_POSD        = 0b1010
    CMD_IN_TBRD_PREI        = 0b1011
    CMD_OUT_CI              = 0b0000
    CMD_OUT_TBWR            = 0b1100
    CMD_OUT_TBWR_POSI2      = 0b1101
    CMD_OUT_TBWR_SP_POSI2   = 0b1110
    CMD_OUT_TBWR_SP         = 0b1111
    BLKER_CE                = 0x3F8F
    BLKER_DEP               = 0x0084
    BLKER_BB                = 0x0081
    BLKER_CB                = 0x0082
    BLKER_CEP_B0            = 0x0180
    BLKER_CEP_B1            = 0x0280
    BLKER_CEP_B2            = 0x0480
    BLKER_CEP_B3            = 0x0880
    BLKER_CEP_B4            = 0x1080
    BLKER_CEP_B5            = 0x2080
    
class Pic18f4xxx(Pic18fBase):
    MEM_CONFIG1L            = 0x300000
    MEM_CONFIG1H            = 0x300001
    MEM_CONFIG2L            = 0x300002
    MEM_CONFIG2H            = 0x300003
    MEM_CONFIG3H            = 0x300005
    MEM_CONFIG4L            = 0x300006
    MEM_CONFIG5L            = 0x300008
    MEM_CONFIG5H            = 0x300009
    MEM_CONFIG6L            = 0x30000A
    MEM_CONFIG6H            = 0x30000B
    MEM_CONFIG7L            = 0x30000C
    MEM_CONFIG7H            = 0x30000D
    MEM_DEVID1              = 0x3FFFFE
    MEM_DEVID2              = 0x3FFFFF
    MEM_BLKER_OPTL          = 0x3C0004
    MEM_BLKER_OPTH          = 0x3C0005
    

class Pic18f4550(Pic18f4xxx):
    NAME                    = "PIC18F4550"
    DEV_ID                  = 0x1200
    MAX_CODE_ADR            = 0x007FFF
    ROW_LENGTH_EXP          = 5
    
class UnknownPic(Pic18fBase):
    NAME                    = "Unknown Target"
    DEV_ID                  = 0x0
    MAX_CODE_ADR            = 0xFFFFFFFF
    ROW_LENGTH_EXP          = 0
    
TargetList = [Pic18f4550]
    