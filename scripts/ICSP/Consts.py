class ConfigValue(object):
    def __init__(self, name, register, values, offset, length, default, desc):
        self.name = name
        self.register = register
        self.values = values
        self.offset = offset
        self.length = length
        self.default = default
        self.desc = desc

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
    
    CONFIG_REG_ADRS = {
    "MEM_CONFIG1L" : 0x300000,
    "MEM_CONFIG1H" : 0x300001,
    "MEM_CONFIG2L" : 0x300002,
    "MEM_CONFIG2H" : 0x300003,
    "MEM_CONFIG3H" : 0x300005,
    "MEM_CONFIG4L" : 0x300006,
    "MEM_CONFIG5L" : 0x300008,
    "MEM_CONFIG5H" : 0x300009,
    "MEM_CONFIG6L" : 0x30000A,
    "MEM_CONFIG6H" : 0x30000B,
    "MEM_CONFIG7L" : 0x30000C,
    "MEM_CONFIG7H" : 0x30000D
    }    
    
    MEM_DEVID1              = 0x3FFFFE
    MEM_DEVID2              = 0x3FFFFF
    MEM_BLKER_OPTL          = 0x3C0004
    MEM_BLKER_OPTH          = 0x3C0005
    
    # Configuration
    
    CFG_PLLDIV =    ConfigValue("PLLDIV", "MEM_CONFIG1L", 
                                [("1", 0), ("2", 1), ("3", 2),
                                 ("4", 3), ("5", 4), ("6", 5),
                                 ("10", 6), ("12", 7)], 0, 3, 0,
                                 "PLL Prescaler Divisor")
    
    CFG_CPUDIV =    ConfigValue("CPUDIV", "MEM_CONFIG1L", 
                                [("1 / 2", 0), ("2 / 3", 1),
                                 ("3 / 4", 2), ("4 / 6", 3)], 3, 2, 0,
                                 "System Clock Postscaler Divisor (PLL Disabled / PLL Enabled)")
   
    CFG_USBDIV =    ConfigValue("USBDIV", "MEM_CONFIG1L", 
                                [("Primary Osc.", 0), ("PLL / 2", 1)],
                                 5, 1, 0,
                                 "USB Clock source (Full-Speed mode only)")
    
    CFG_FOSC   =    ConfigValue("FOSC", "MEM_CONFIG1H", 
                                [("XT", 0), ("XTPLL", 2), ("ECIO", 4),
                                 ("EC", 5), ("ECPIO", 6), ("ECPLL", 7),
                                 ("INTIO", 8), ("INTCKO", 9), ("INTXT", 10),
                                 ("INTHS", 11), ("HS", 12), ("HSPLL", 14)], 0, 4, 5,
                                 "Osclillator Selection")
    
    CFG_FCMEN  =    ConfigValue("FCMEN", "MEM_CONFIG1H", 
                                [("Disabled", 0), ("Enabled", 1)],
                                 6, 1, 0,
                                 "Fail-Safe Clock Monitor")
    
    CFG_IESO  =    ConfigValue("IESO", "MEM_CONFIG1H", 
                                [("Disabled", 0), ("Enabled", 1)],
                                 7, 1, 0,
                                 "Oscillator Switchover")
    
    CONFIG_VALS = [CFG_PLLDIV, CFG_CPUDIV, CFG_USBDIV, CFG_FOSC, CFG_FCMEN, CFG_IESO]
    

class Pic18f4550(Pic18f4xxx):
    NAME                    = "PIC18F4550"
    DEV_ID                  = 0x1200
    MAX_CODE_ADR            = 0x007FFF
    BLOCK_SIZE              = 8192
    ROW_READ_LENGTH_EXP     = 5
    ROW_ERASE_LENGTH_EXP    = 6
    MAX_BLOCK_INDEX         = 3
    HAS_DATA_EEPROM         = True
    
class UnknownPic(Pic18fBase):
    NAME                    = "Unknown Target"
    DEV_ID                  = 0x0
    MAX_CODE_ADR            = 0xFFFFFFFF
    BLOCK_SIZE              = 8192
    ROW_READ_LENGTH_EXP     = 0
    ROW_ERASE_LENGTH_EXP    = 0
    MAX_BLOCK_INDEX         = 5
    HAS_DATA_EEPROM         = False
    

    
TargetList = [Pic18f4550]
    