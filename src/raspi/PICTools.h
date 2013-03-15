#pragma once

#include <inttypes.h>
#include <wiringPi.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "PIC18Fconst.h"

#define pinPGC 6
#define pinPGD 5
#define pinPGM 4

#define del delayMicroseconds(clkDel)

volatile unsigned long clkDel;

int pgmEnable();
void pwrOffTarget();
void cmdOut(const uint8_t cmd, const uint16_t dat);
uint8_t cmdIn(uint8_t cmd, uint8_t dat);
void clkFlashWrite();
void setTablePtr8(const uint8_t up, const uint8_t high, const uint8_t low);
void setTablePtr(uint32_t memAdr);
inline uint16_t readWord(uint32_t memAdr);
inline uint8_t readByte();
inline void setAccessToFlash();
inline void enableWrite();
inline void disableWrite();
inline void performRowErase();
void writeTest();
void bulkErase(const uint16_t erOpt);
void writeConfig (const uint32_t confReg, const uint8_t conf);

int initialize(unsigned long cdel)
{	
	clkDel = cdel;
	if (wiringPiSetup () == -1)
		return 1 ;	
	return 0;
}

void setClockDelay(unsigned long cdel)
{
	clkDel = cdel;
}

// Clock benchmark
unsigned long clkBench()
{
	struct timeval  tvst, tve;	
	
	int i;
	gettimeofday(&tvst, NULL);
	for (i = 0; i < 50; i++)
	{
		clkLO; clkHI; del; clkLO; del;
	}
	gettimeofday(&tve, NULL);	
	const unsigned long tdif = ((tve.tv_sec - tvst.tv_sec) * 1000000 + (tve.tv_usec - tvst.tv_usec)) / 50;	
	return 1000000 / tdif; 
}