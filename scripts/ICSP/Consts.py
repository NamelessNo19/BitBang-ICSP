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
    
class Pic18f2xxx_4xxx(Pic18fBase):
    
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
    
    CFG_PWRTEN  =  ConfigValue("PWRTEN", "MEM_CONFIG2L", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 0, 1, 1,
                                 "Power-up Timer")
    
    CFG_BOREN  =   ConfigValue("BOREN", "MEM_CONFIG2L", 
                                [("Disabled", 0), ("Software Only", 1),
                                 ("Hardware Only", 2), ("Enabled", 3)],
                                 1, 2, 3,
                                 "Brown-out Reset")
    
    CFG_BORV   =   ConfigValue("BORV", "MEM_CONFIG2L", 
                                [("Level 4", 0), ("Level 3", 1),
                                 ("Level 2", 2), ("Level 1", 3)],
                                 3, 2, 3,
                                 "Brown-out Reset Voltage")
    
    CFG_VREGEN =   ConfigValue("VREGEN", "MEM_CONFIG2L", 
                                [("Disabled", 0), ("Enabled", 1)],
                                 5, 1, 0,
                                 "USB Internal Voltage Regulator")
    
    CFG_WDTEN  =   ConfigValue("WDTEN", "MEM_CONFIG2H", 
                                [("Disabled", 0), ("Enabled", 1)],
                                 0, 1, 1,
                                 "Watchdog Timer")
    
    CFG_WDTPS =    ConfigValue("WDTPS", "MEM_CONFIG2H", 
                               [("1:1", 0), ("1:2", 1), ("1:4", 2),
                                ("1:8", 3), ("1:16", 4), ("1:32", 5),
                                ("1:64", 6), ("1:128", 7), ("1:256", 8),
                                ("1:512", 9), ("1:1024", 10), ("1:2048", 11),
                                ("1:4096", 12), ("1:8192", 13), ("1:16384", 14),
                                ("1:32768", 15)], 1, 4, 15,
                               "Watchdog Timer Postscale")
    
    CFG_CCP2MX =  ConfigValue("CCP2MX", "MEM_CONFIG3H",
                              [("RB3", 0), ("RC1", 1)], 0, 1, 1,
                              "CCP2 Multiplex") 
    
    CFG_PBADEN =  ConfigValue("PBADEN", "MEM_CONFIG3H",
                              [("Digital I/O", 0), ("Analog In", 1)], 1, 1, 1,
                              "PORTB A/D") 
    
    CFG_LPT1OSC = ConfigValue("LPT1OSC", "MEM_CONFIG3H",
                              [("Higher Power", 0), ("Low Power", 1)], 2, 1, 0,
                              "Timer1 Oscillator mode")
    
    CFG_MCLRE  =  ConfigValue("MCLRE", "MEM_CONFIG3H",
                              [("RE3", 0), ("MCLR", 1)], 7, 1, 1,
                              "MCLR Pin mode")
    
    CFG_STVREN  = ConfigValue("STVREN", "MEM_CONFIG4L", 
                             [("Disabled", 0), ("Enabled", 1)],
                              0, 1, 1,
                              "Reset on Stack Full/Underflow")
    
    CFG_LVP    =  ConfigValue("LVP", "MEM_CONFIG4L", 
                                [("Disabled", 0), ("Enabled", 1)],
                                 2, 1, 1,
                                 "Single-Supply ICSP")
    
    CFG_ICPRT  =  ConfigValue("ICPRT", "MEM_CONFIG4L", 
                                [("Disabled", 0), ("Enabled", 1)],
                                 5, 1, 0,
                                 "ICPORT")
    
    CFG_XINST   = ConfigValue("XINST", "MEM_CONFIG4L", 
                                [("Disabled", 0), ("Enabled", 1)],
                                 6, 1, 0,
                                 "Extended Instruction Set")
    
    CFG_DEBUG   = ConfigValue("DEBUG", "MEM_CONFIG4L", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 7, 1, 1,
                                 "Background Debugger")
    
    CFG_CP0     = ConfigValue("CP0", "MEM_CONFIG5L", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 0, 1, 1,
                                 "Block 0 Code Protection") 
    
    CFG_CP1     = ConfigValue("CP1", "MEM_CONFIG5L", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 1, 1, 1,
                                 "Block 1 Code Protection") 
    
    CFG_CP2     = ConfigValue("CP2", "MEM_CONFIG5L", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 2, 1, 1,
                                 "Block 2 Code Protection") 
    
    CFG_CP3     = ConfigValue("CP3", "MEM_CONFIG5L", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 3, 1, 1,
                                 "Block 3 Code Protection")
    
    CFG_CPB     = ConfigValue("CPB", "MEM_CONFIG5H", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 6, 1, 1,
                                 "Boot Block Code Protection") 
    
    CFG_CPD     = ConfigValue("CPD", "MEM_CONFIG5H", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 7, 1, 1,
                                 "Data EEPROM Code Protection")
    
    CFG_WRT0     = ConfigValue("WRT0", "MEM_CONFIG6L", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 0, 1, 1,
                                 "Block 0 Write Protection") 
    
    CFG_WRT1     = ConfigValue("WRT1", "MEM_CONFIG6L", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 1, 1, 1,
                                 "Block 1 Write Protection") 
    
    CFG_WRT2     = ConfigValue("WRT2", "MEM_CONFIG6L", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 2, 1, 1,
                                 "Block 2 Write Protection") 
    
    CFG_WRT3     = ConfigValue("WRT3", "MEM_CONFIG6L", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 3, 1, 1,
                                 "Block 3 Write Protection")
    
    CFG_WRTC     = ConfigValue("WRTC", "MEM_CONFIG6H", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 5, 1, 1,
                                 "Configuration Register Write Protection")
    
    CFG_WRTB     = ConfigValue("WRTB", "MEM_CONFIG6H", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 6, 1, 1,
                                 "Boot Block Write Protection") 
    
    CFG_WRTD     = ConfigValue("WRTD", "MEM_CONFIG6H", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 7, 1, 1,
                                 "Data EEPROM Write Protection")
    
    CFG_EBTR0     = ConfigValue("EBTR0", "MEM_CONFIG7L", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 0, 1, 1,
                                 "Block 0 Table Read Protection") 
    
    CFG_EBTR1     = ConfigValue("EBTR1", "MEM_CONFIG7L", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 1, 1, 1,
                                 "Block 1 Table Read Protection") 
    
    CFG_EBTR2     = ConfigValue("EBTR2", "MEM_CONFIG7L", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 2, 1, 1,
                                 "Block 2 Table Read Protection") 
    
    CFG_EBTR3     = ConfigValue("EBTR3", "MEM_CONFIG7L", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 3, 1, 1,
                                 "Block 3 Table Read Protection")
    
    CFG_EBTRB     = ConfigValue("EBTRB", "MEM_CONFIG7H", 
                                [("Disabled", 1), ("Enabled", 0)],
                                 6, 1, 1,
                                 "Boot Block Table Read Protection")   
    
    
    CONFIG_VALS = [CFG_PLLDIV, CFG_CPUDIV, CFG_USBDIV, CFG_FOSC, CFG_FCMEN,
                    CFG_IESO, CFG_PWRTEN, CFG_BOREN, CFG_BORV, CFG_VREGEN,
                    CFG_WDTEN,  CFG_WDTPS, CFG_CCP2MX, CFG_PBADEN, CFG_LPT1OSC,
                    CFG_MCLRE, CFG_STVREN, CFG_LVP, CFG_ICPRT, CFG_XINST, CFG_DEBUG,
                    CFG_CP0, CFG_CP1, CFG_CP2, CFG_CP3, CFG_CPB, CFG_CPD,
                    CFG_WRT0, CFG_WRT1, CFG_WRT2, CFG_WRT3, CFG_WRTC, CFG_WRTB, CFG_WRTD,
                    CFG_EBTR0, CFG_EBTR1, CFG_EBTR2, CFG_EBTR3, CFG_EBTRB]
    
    RD_ONLY_CONF = [CFG_LVP, CFG_ICPRT]
    
    DATA_EEPROM_HEX_OFFSET   = 0xF00000
    CONFIG_HEX_OFFSET        = 0x300000
    

class Pic18f4550(Pic18f2xxx_4xxx):
    NAME                    = "PIC18F4550"
    DEV_ID                  = 0x1200
    MAX_CODE_ADR            = 0x007FFF
    BLOCK_SIZE              = 8192
    ROW_WRITE_LENGTH_EXP    = 5
    ROW_ERASE_LENGTH_EXP    = 6
    MAX_BLOCK_INDEX         = 3
    HAS_DATA_EEPROM         = True
    DATA_EEPROM_SIZE        = 256
    
class Pic18f2550(Pic18f2xxx_4xxx):
    NAME                    = "PIC18F2550"
    DEV_ID                  = 0x1240
    MAX_CODE_ADR            = 0x007FFF
    BLOCK_SIZE              = 8192
    ROW_WRITE_LENGTH_EXP    = 5
    ROW_ERASE_LENGTH_EXP    = 6
    MAX_BLOCK_INDEX         = 3
    HAS_DATA_EEPROM         = True
    DATA_EEPROM_SIZE        = 256
    
class UnknownPic(Pic18fBase):
    NAME                    = "Unknown Target"
    DEV_ID                  = 0x0
    MAX_CODE_ADR            = 0xFFFFFFFF
    BLOCK_SIZE              = 8192
    ROW_READ_LENGTH_EXP     = 0
    ROW_ERASE_LENGTH_EXP    = 0
    MAX_BLOCK_INDEX         = 5
    HAS_DATA_EEPROM         = False
    

    
TargetList = [Pic18f4550, Pic18f2550]
    