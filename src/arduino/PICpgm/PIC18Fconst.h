#ifndef __PIC18FCONSTANTS__
#define __PIC18FCONSTANTS__

// Input Commands
#define CMD_IN_SOTBAT B0010 		// Shift Out TABLAT Register
#define CMD_IN_TBRD B1000			// Table Read
#define CMD_IN_TBRD_POSI B1001		// Table Read, Post-Increment
#define CMD_IN_TBRD_POSD B1010		// Table Read, Post-Decrement
#define CMD_IN_TBRD_PREI B1011		// Table Read, Pre-Increment

// Output Commands
#define CMD_OUT_CI B0000			// Core Instruction
#define CMD_OUT_TBWR B1100			// Table Write
#define CMD_OUT_TBWR_POSI2 B1101	// Table Write, Post-Increment by 2
#define CMD_OUT_TBWR_SP_POSI2 B1110 // Table Write, Start Programming, Post-Increment by 2
#define CMD_OUT_TBWR_SP B1111		// Table Write, Start Programming

// Memory Locations

#define MEM_CONFIG1L 0x300000L
#define MEM_CONFIG1H 0x300001L
#define MEM_CONFIG2L 0x300002L
#define MEM_CONFIG2H 0x300003L
#define MEM_CONFIG3H 0x300005L
#define MEM_CONFIG4L 0x300006L
#define MEM_CONFIG5L 0x300008L
#define MEM_CONFIG5H 0x300009L
#define MEM_CONFIG6L 0x30000AL
#define MEM_CONFIG6H 0x30000BL
#define MEM_CONFIG7L 0x30000CL
#define MEM_CONFIG7H 0x30000DL


#define MEM_DEVID1 0x3FFFFEL
#define MEM_DEVID2 0x3FFFFFL

#define MEM_BLKER_OPTL 0x3C0004L
#define MEM_BLKER_OPTH 0x3C0005L


// Bulk Erase Options

#define BLKER_CE 0x3F8F
#define BLKER_DEP 0x0084
#define BLKER_BB 0x0081
#define BLKER_CB 0x0082
#define BLKER_CEP_B0 0x0180
#define BLKER_CEP_B1 0x0280 
#define BLKER_CEP_B2 0x0480 
#define BLKER_CEP_B3 0x0880 
#define BLKER_CEP_B4 0x1080 
#define BLKER_CEP_B5 0x2080 


// Delays (microseconds)

#define DELAY_P2 1
#define DELAY_P2A 1
#define DELAY_P2B 1
#define DELAY_P3 1
#define DELAY_P4 1
#define DELAY_P5 1
#define DELAY_P5A 1
#define DELAY_P6 1
#define DELAY_P9 1000
#define DELAY_P10 100
#define DELAY_P11 5000
#define DELAY_P11A 4000
#define DELAY_P12 2
#define DELAY_P13 1
#define DELAY_P14 1
#define DELAY_P15 2



#endif
