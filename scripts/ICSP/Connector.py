#!/usr/bin/python3

from ctypes import *
from ICSP.Consts import *
from distutils.util import strtobool
from sys import stdin
import os

class Error(Exception):
    """Base class for exceptions in this module."""
    pass

class NotInitializedError(Error):
    """Exception raised when connection not initialized."""
    def __str__(self):
        return "Connection not initialized!"
    
class NotConnectedError(Error):
    """Exception raised when not connected to target."""
    def __str__(self):
        return "ICSP connection not started!"

class DataEEPROMWriteTimeout(Error):
    def __str__(self):
        return "Writing of Data EEPROM exceeded time limit!"

class Pic18fICSP(object):
    
    def __init__(self):
        self.initialized = False
        self.targetEnabled = False;
        self.targetId = 0
        self.device = None;
        if os.geteuid() != 0:
            print ("You need root privileges to access GPIO-Interface. Sorry.")
            return
        try:
            self.lib = cdll.LoadLibrary("../lib/libPIC18FTools.so")
        except OSException:
            pass
            print("Unable to load PIC library.")
            return
        
        if self.lib.initialize(100) != 0:
            print("Initialization failed.")
        else:
            print("PIC18F Library initialized.")
            self.initialized = True
    
    def __del__(self):
        if self.targetEnabled:
            self.disconnect()
        
            
    def getClock(self):
        if not self.initialized:
            raise NotInitializedError
            return None
        elif self.targetEnabled:
            print("Cannot perform clock benchmark  while connected!")
        else:
            return self.lib.clkBench()
        
    def setClockTarget(self, tClock):
        if not self.initialized:
            raise NotInitializedError
        else:
            self.lib.setClockDelay(max(500000 // (tClock + 1), 2))
        
    def connect(self):
        if not self.initialized:
            raise NotInitializedError
            return False
        else:
            self.targetId = self.lib.pgmEnable()
            if self.targetId != 0:
                print("ICSP connection established.")
                self.targetEnabled = True
                self.device = self.getTarget()
                return True
                
            else:
                print("ICSP connection failed.")
                self.disconnect()
                return False
                
    def disconnect(self):
        if not self.initialized:
            raise NotInitializedError
            return None
        else:
            self.lib.pwrOffTarget()
            print("ICSP disconnected.")
            self.targetEnabled = False
            self.targetId = 0
            
    def checkState(self):
        if not self.initialized:
            raise NotInitializedError
            return False
        if not self.targetEnabled:
            raise NotConnectedError
            return False
        return True
        
    def readDataWord(self, adress):
        if not self.checkState():
            return None
        if (adress > self.device.MAX_CODE_ADR) or (adress < 0):
            return 0
        return self.lib.readWord(adress)
    
    def readFlash(self, adress, length):
        if not self.checkState():
            return None
        buf = create_string_buffer(length)
        rdlen = self.lib.readFlashSeq(buf, adress, length);
        if (rdlen != length):
            print("WARN: Only %d bytes read at %s." % (rdlen, hex(adress)))
        return buf.raw
    
    def readEEPROM(self, adress, length):
        if not self.device.HAS_DATA_EEPROM:
            return None
        self.lib.setAccessToEEPROM()
        ebuf = bytearray(length)
        for i in range(length):
            self.lib.setEEPROMAdrPtr(adress + i)
            ebuf[i] = self.lib.readEEPROM()  
        self.lib.setAccessToFlash()
        return bytes(ebuf)       
    
    def getTarget(self):
        if self.targetId == 0:
            return None
        for dev in TargetList:
            if dev.DEV_ID == self.targetId:
                return dev
        return UnknownPic
                
    def getTargetName(self):
        if self.device == None:
            return "[Not Connected]"
        else:
           return self.device.NAME
    
    def writeFlashRow(self, adr, dat):
        if not self.checkState():
            return None
        if adr >= 0:
            adr = adr & self.device.MAX_CODE_ADR & (0xFFFFFFFF << self.device.ROW_WRITE_LENGTH_EXP)
        buf = bytearray(dat)
        if not len(buf) > 0:
            print ("Nothing written.")
            return None
            
        if len(buf) % 2 == 1:
            buf.append(0xFF)
        
        if adr >= 0:   
            self.lib.setAccessToFlash()
            self.lib.setTablePtr(adr)
            
        maxSize = 1 << self.device.ROW_WRITE_LENGTH_EXP 
        for i in range(0, (len(buf) // 2) - 1):
            if i * 2 > maxSize - 4:
                break
            self.lib.cmdOut(self.device.CMD_OUT_TBWR_POSI2, buf[2*i] + (buf[2*i + 1] << 8))  
            
        self.lib.cmdOut(self.device.CMD_OUT_TBWR_SP, buf[-2] + (buf[-1] << 8))
        self.lib.clkFlashWrite()
        
    
    def writeDataEEPROM(self, buf, skipPad = True):
        if not self.device.HAS_DATA_EEPROM:
            return
        self.lib.setAccessToEEPROM()
        for i in range(len(buf)):
            if skipPad and buf[i] != 0xFF:
                if i >= self.device.DATA_EEPROM_SIZE:
                    break
                self.lib.loadEEPROMWriteBuffer(i, buf[i])
                if self.lib.performEEPROMWrite(255) != 255:
                    self.lib.setAccessToFlash()
                    raise DataEEPROMWriteTimeout
        self.lib.setAccessToFlash()
                    
                
                
             
        
        
    
    def eraseFlashRow(self, adr):
        if not self.checkState():
            return None
        adr = adr & self.device.MAX_CODE_ADR & (0xFFFFFFFF << self.device.ROW_ERASE_LENGTH_EXP)
        self.lib.setAccessToFlash()
        self.lib.enableWrite()
        self.lib.setTablePtr(adr)
        self.lib.performRowErase(adr)   
        self.lib.disableWrite()
          
    def eraseBlock(self, index):
        if not self.checkState():
            return None
        
        if index < -1 or index > self.device.MAX_BLOCK_INDEX:
            raise ValueError("Invalid block index %d." % index)
            return
        
        eropt = 0
        if index == -1:
            eropt = self.device.BLKER_CEP_BB
        elif index == 0:
            eropt = self.device.BLKER_CEP_B0
        elif index == 1:
            eropt = self.device.BLKER_CEP_B1
        elif index == 2:
            eropt = self.device.BLKER_CEP_B2
        elif index == 3:
            eropt = self.device.BLKER_CEP_B3
        elif index == 4:
            eropt = self.device.BLKER_CEP_B4
        elif index == 5:
            eropt = self.device.BLKER_CEP_B5
            
        self.lib.bulkErase(eropt)
        
    def eraseChip(self):
        if not self.checkState():
            return None                       
        self.lib.bulkErase(self.device.BLKER_CE)
        
    def eraseDataEEPROM(self):
        if not self.device.HAS_DATA_EEPROM:
            return
        if not self.checkState():
            return None                       
        self.lib.bulkErase(self.device.BLKER_DEP)
        
    def eraseConfiguration(self):
        if not self.checkState():
            return None                       
        self.lib.bulkErase(self.device.BLKER_CB)
        
    def readConfiguration(self):
        if not self.checkState():
            return None
        
        dict = {}
        for cReg in self.device.CONFIG_REG_ADRS:
            regAdr = self.device.CONFIG_REG_ADRS[cReg]
            self.lib.setTablePtr(regAdr)
            dict[regAdr] = self.lib.readByte()
            
        return dict
    
    def writeConfiguration(self, confDict):
        if not self.checkState():
            return None
            
        for cReg in self.device.CONFIG_REG_ADRS:
            regAdr = self.device.CONFIG_REG_ADRS[cReg]
            if not regAdr in confDict:
                continue     
            self.lib.writeConfig(regAdr, confDict[regAdr])
        
         
          
# Testing       
if __name__ == '__main__':
    pic = Pic18fICSP()
    print(str(pic.getClock()) + " Hz")
