#!/usr/bin/python3

from DialogUtil import Dlg        
from time import sleep
from ICSP.Connector import Pic18fICSP;
from math import ceil
import os.path

class PicConfig(object):
    def __init__(self, target):
        self.optList = target.CONFIG_VALS
        self.optVals = []
        for opt in self.optList:
            self.optVals.append(opt.default)
     
    @staticmethod       
    def getOptValDesc(opt, val):
        for vDesc, vVal in opt.values:
            if val == vVal:
                return vDesc
        return "???"

def dumpMenu():
    menuOptions = ["Full code memory", "Code memory sequence", "Back"]
    sel = dlg.menu("What do you want to dump?", menuOptions)
    
    if sel == 0:
        fullDumpToFile()
    elif sel == 1:
        dumpSequence()
    else:
        return
    
def clockSet():
    while True:
        input = dlg.inputBox("Enter the clock target (Hz):", "20000")
        
        if input == None:
            return
        
        clk = -1
        try:
            clk = int(input)
        except ValueError:
            clk = -1
                
        if clk <= 0:
            dlg.msgBox("Invalid clock.")
        else:
            break
        
    pic.setClockTarget(clk)
    
      

def queryOutputFile():
    pathOk = False
    while not pathOk:
        path = dlg.fselect("/home/")
        if path == None:
            return None  
        
        if os.path.isdir(path) or os.path.isfile(path):
            dlg.msgBox("File already exists or invalid path.")
        else:
            try:
                file = open(path, "wb")
            except IOError as e:
                dlg.msgBox("IOError:\n" + str(e))
            else:
                return file
    

def fullDumpToFile():
    file = queryOutputFile()    
    if file == None:
        return
      
    chnkSize = 128
    chnkCount = pic.getTarget().BLOCK_SIZE * (pic.getTarget().MAX_BLOCK_INDEX + 1) // chnkSize
    
    dlg.createGauge("Dumping " + str(chnkSize * chnkCount) + " Bytes to " + file.name)
    
    for i in range(chnkCount):
        dlg.updateGauge((i * 100) // chnkCount)
        buf = pic.readFlash(i * chnkSize, chnkSize)
        file.write(buf)
           
    dlg.closeGauge()
    file.close()
    dlg.msgBox("Dumping finished.")
    

def dumpSequence():
    startAdr = -1
    endAdr = -1
    # Query start address
    while True:
        input = dlg.inputBox("Please enter the start address (0 - %d):" %  pic.getTarget().MAX_CODE_ADR, "0x")
        if input == None:
            return
        try:
            startAdr = eval(input)
        except SyntaxError:
            startAdr = -1
           
        if startAdr < 0 or startAdr > pic.getTarget().MAX_CODE_ADR:
            dlg.msgBox("Invalid address.")
        else:
            break
        
    # Query end address
    while True:
        input = dlg.inputBox("Please enter the end address (%d - %d):" %  (startAdr, pic.getTarget().MAX_CODE_ADR), "0x")
        if input == None:
            return
        try:
            endAdr = eval(input)
        except SyntaxError:
            endAdr = -1
            
        if endAdr < startAdr or endAdr > pic.getTarget().MAX_CODE_ADR:
            dlg.msgBox("Invalid address.")
        else:
            break
        
    file = queryOutputFile()    
    if file == None:
        return
    
    chnkSize = 64
    chnkCount = ceil((endAdr - startAdr + 1) / chnkSize)
     
    
    dlg.createGauge("Dumping " + str(endAdr - startAdr + 1) + " Bytes to " + file.name)
    
    for i in range(chnkCount):
        dlg.updateGauge((i * 100) // chnkCount)
        buf = pic.readFlash(i * chnkSize + startAdr, chnkSize if i != chnkCount - 1 else (endAdr - startAdr + 1) % chnkSize)
        file.write(buf)
           
    dlg.closeGauge()
    file.close()
    dlg.msgBox("Dumping finished.")
    
    
def configMenu():
    while True:
        cmMenOptions = ["Edit", "Load from Hex file", "Load from target", "Restore defaults", "Write to Hex file", "Write to target", "Back"]
        sel = dlg.menu("Target Configuration", cmMenOptions)
        if sel == 0:
            confEditMenu()
        else:
            return


            
 
def confEditMenu():
    dlg.noCancel = True
    while True:
        ceMenOptions = []
        backWd = dlg.minWidth
        dlg.minWidth = 30
        for cOpt, cVal in zip(conf.optList, conf.optVals):
            ceMenOptions.append(cOpt.name + " \t= " + PicConfig.getOptValDesc(cOpt, cVal))
        ceMenOptions.append("Back")
     
        sel = dlg.menu("Select Option", ceMenOptions)
        dlg.minWidth = backWd
        
        if sel == None or sel >= len(ceMenOptions) - 1:
            dlg.noCancel = False
            return
        else:
            valList = conf.optList[sel].values
            cevalMenList = []
            for ceval in valList:
                cevalMenList.append(ceval[0])
            dlg.noCancel = False
            valSel = dlg.menu(conf.optList[sel].desc, cevalMenList)
            dlg.noCancel = True
            
            if valSel != None:
                conf.optVals[sel] = valList[valSel][1]
                
     
            
                      
 # ----- MAIN -----           
    
        
if __name__ == '__main__':
    
    print("Starting BitBang-ICSP ...")
    print("Initializing ICSP ...")
    pic = Pic18fICSP();
    
    if not pic.initialized:
        print("FATAL: Initialization failed.")
        exit()
    pic.setClockTarget(20000)
    print("Clock set to %d Hz." % pic.getClock())
    print("Connecting to target ...")
    
    if not pic.connect():
        print("FATAL: Unable to connect to target.")
        exit()
    
    print("Connected to: " + pic.getTargetName())
    conf = PicConfig(pic.getTarget())
    print("Starting UI ...")
    
    dlg = Dlg()
    dlg.backtitle = "BitBang-ICSP"
    dlg.infoBox("Use at your own risk!", 1)
    quit = False
    mainMenuOptions = ["Dump Memory", "Write Memory", "Erase Memory", "Configuration Editor" ,"Set Clock", "Quit"]
    while not quit:
        sel = dlg.menu("Connected to " + pic.getTargetName() + ".", mainMenuOptions)   
        if sel == 0:
            dumpMenu()
        elif sel == 1:
            dlg.msgBox("Not implemented. :-(")
        elif sel == 2:
            dlg.msgBox("Not implemented. :-(")
        elif sel == 3:
            configMenu()
        elif sel == 4:
            clockSet()
        else:
            quit = True
             
     
    pic.disconnect()
    

  