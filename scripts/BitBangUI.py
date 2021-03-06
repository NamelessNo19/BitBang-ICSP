#!/usr/bin/python3

from DialogUtil import Dlg        
from time import sleep
from ICSP.Connector import Pic18fICSP;
from math import ceil
from ICSP.Utils import *
import HexFile
import os.path


def dumpMenu():
    menuOptions = ["Dump to Hex File", "Full code memory", "Code memory sequence"]
    
    hasEE = pic.device.HAS_DATA_EEPROM
    
    if hasEE:
        menuOptions.append("Data EEPROM")       
    menuOptions.append("Back")
    
    sel = dlg.menu("What do you want to dump?", menuOptions)
    
    if sel == 0:
        dumpToHex()
    elif sel == 1:
        fullDumpToFile()
    elif sel == 2:
        dumpSequence()
    elif hasEE and sel == 3:
        dumpEEPROM()
    else:
        return
    
def clockSet():
    while True:
        input = dlg.inputBox("Enter the clock target (Hz):", "50000")
        
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
    chnkCount = pic.device.BLOCK_SIZE * (pic.device.MAX_BLOCK_INDEX + 1) // chnkSize
    
    dlg.createGauge("Dumping " + str(chnkSize * chnkCount) + " Bytes to " + file.name)
    
    for i in range(chnkCount):
        dlg.updateGauge((i * 100) // chnkCount)
        buf = pic.readFlash(i * chnkSize, chnkSize)
        file.write(buf)
           
    dlg.closeGauge()
    file.close()
    dlg.msgBox("Dumping finished.")

def dumpEEPROM():
    file = queryOutputFile()    
    if file == None:
        return
          
    dlg.infoBox("Dumping Data EEPROM...")
    ebuf = pic.readEEPROM(0, pic.device.DATA_EEPROM_SIZE)       
    file.write(ebuf)      
    file.close()
    dlg.msgBox("Dumping finished.")
    
    

def dumpSequence():
    startAdr = -1
    endAdr = -1
    # Query start address
    while True:
        input = dlg.inputBox("Please enter the start address (0 - %d):" %  pic.device.MAX_CODE_ADR, "0x")
        if input == None:
            return
        try:
            startAdr = eval(input)
        except SyntaxError:
            startAdr = -1
           
        if startAdr < 0 or startAdr > pic.device.MAX_CODE_ADR:
            dlg.msgBox("Invalid address.")
        else:
            break
        
    # Query end address
    while True:
        input = dlg.inputBox("Please enter the end address (%d - %d):" %  (startAdr, pic.device.MAX_CODE_ADR), "0x")
        if input == None:
            return
        try:
            endAdr = eval(input)
        except SyntaxError:
            endAdr = -1
            
        if endAdr < startAdr or endAdr > pic.device.MAX_CODE_ADR:
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
        cmMenOptions = ["Edit", "Load from Hex file", "Load from target", "Restore defaults", "Write to target", "Back"]
        sel = dlg.menu("Target Configuration", cmMenOptions)
        if sel == 0:
            confEditMenu()
        elif sel == 1:
            hf = queryHexFile()
            if hf != None:
                try:
                    conf.fromHexFile(hf)
                except MissingConfigurationException as mce:
                   dlg.msgBox("Configuration Error: " + str(mce))
                else:    
                    dlg.msgBox("Configuration loaded.")
        elif sel == 2:
            conf.fromBinaryDict(pic.readConfiguration())
            dlg.msgBox("Configuration loaded.")
        elif sel == 3:
            conf.loadDefaults()
            dlg.msgBox("Restored default configuration.")
        elif sel == 4:
            dlg.infoBox("Writing configuration to target...")
            pic.writeConfiguration(conf.toBinaryDict())
            dlg.msgBox("Configuration written to target.")
        else:
            return

            
 
def confEditMenu():
    dlg.noCancel = True
    while True:
        ceMenOptions = []
        backWd = dlg.minWidth
        dlg.minWidth = 30
        for cOpt, cVal in zip(conf.optList, conf.optVals):
            ceMenOptions.append(cOpt.name + "\t= " + PicConfig.getOptValDesc(cOpt, cVal))
        ceMenOptions.append("Back")
     
        sel = dlg.menu("Select Option", ceMenOptions)
        dlg.minWidth = backWd
        
        if sel == None or sel >= len(ceMenOptions) - 1:
            dlg.noCancel = False
            return
        else:
            if conf.optList[sel] in pic.device.RD_ONLY_CONF:
                dlg.msgBox("This value cannot be changed.")
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
                    
def eraseMenu():
    erMenOptions = ["Chip Erase", "Erase Code Memory", "Erase Data EEPROM", "Erase Configuration", "Back"]
    
    while True:
        sel = dlg.menu("Select an erase option", erMenOptions)
        dlg.defaultNo = True
        
        if sel == 0:
            if dlg.yesNo("Do you really want to perform a Chip Erase?"):
                pic.eraseChip()
                dlg.msgBox("Chip Erase complete.")
        elif sel == 1:
            blockList = ["Boot Block"]
            for i in range(pic.device.MAX_BLOCK_INDEX + 1):
                blockList.append("Block " + str(i))
            blockSel = dlg.checkList("Select the blocks to erase", blockList)
            if blockSel != None and len(blockSel) > 0 and dlg.yesNo("Do you really want to erase the selected blocks?"):
                for blk in blockSel:
                    pic.eraseBlock(blk - 1)
                dlg.msgBox("Code Memory Erase complete.")
        elif sel == 2:
            if not pic.device.HAS_DATA_EEPROM:
                dlg.msgBox("No Data EEPROM present on target.")
            elif dlg.yesNo("Do you really want to erase the Data EEPROM?"):
                pic.eraseDataEEPROM()
                dlg.msgBox("Data EEPROM Erase complete.")
        elif sel == 3:
            if dlg.yesNo("Do you really want to erase the configuration registers?"):
                pic.eraseConfiguration()
                dlg.msgBox("Configuration Erase complete.")       
        else:
            return 
    
        dlg.defaultNo = False    
        
def queryHexFile():
    while True:
        fpath = dlg.fselect("/home/")
        if fpath == None:
            return None
        
        try:
            hf = HexFile.HexFile(fpath, 1 << pic.device.ROW_WRITE_LENGTH_EXP)
        except HexFile.Error as e:
            dlg.msgBox("Parsing Hex File failed: \n" + str(e))
        else:
            return hf
        
# Monster Function :/        
def writeHex():
    hf = queryHexFile()
    
    if hf == None:
        return
    
    hxData = {codeAdr : hf.chunks[codeAdr] for codeAdr in hf.chunks if codeAdr < pic.device.MAX_CODE_ADR }    
    
    title = str(len(hxData) * hf.chunkSize) + " Bytes parsed from Hex File."
    hasConf = pic.device.CONFIG_HEX_OFFSET in hf.chunks
    
    hasEE = False
    if pic.device.HAS_DATA_EEPROM:
        hasEE = hf.hasDataInRange(pic.device.DATA_EEPROM_HEX_OFFSET, pic.device.DATA_EEPROM_SIZE)
    
    
    
    wrtOpts = ["Perform Chip Erase", "Write code memory", "Verify written memory"]
    
    if hasConf:
        wrtOpts.append("Write Configuration")
        wrtOpts.append("Verify Configuration")
        hexCfg = PicConfig(pic.device)
        try:
            hexCfg.fromHexFile(hf)
        except MissingConfigurationException as mce:
            dlg.msgBox("Configuration Error: " + str(mce))
            return
        
    if hasEE:
        wrtOpts.append("Write Data EEPROM")
        wrtOpts.append("Verify Data EEPROM")
        eeDat = hf.read(pic.device.DATA_EEPROM_HEX_OFFSET, pic.device.DATA_EEPROM_SIZE)
        
     
    dlg.defaultNo = True    
    dlgSel = dlg.checkList(title, wrtOpts, [True] * len(wrtOpts))
    dlg.defaultNo = False
    
    if dlgSel == None:
        return
    
    performCE = 0 in dlgSel
    writeCM = 1 in dlgSel
    verCM = 2 in dlgSel
    if hasConf:
        writeCFG = 3 in dlgSel
        verCFG = 4 in dlgSel
        writeEE = 5 in dlgSel
        verEE = 6 in dlgSel
    else:
        writeCFG = False
        verCFG = False
        writeEE = 3 in dlgSel
        verEE = 4 in dlgSel
    
    datLen = len(hxData)
    
    if performCE: # Chip Erase
        dlg.infoBox("Performing Chip Erase...")
        pic.eraseChip()
    if writeCM:  # Write Code Memory   
        dlg.createGauge("Writing %d chunks to target..." % datLen)
        cCount = 0
        for chnkAdr, chnkDat in hxData.items():
            pic.writeFlashRow(chnkAdr, chnkDat)
            cCount += 1
            dlg.updateGauge((cCount * 100) // datLen)
        dlg.closeGauge()
    if verCM:   # Verify Code Memory
        dlg.createGauge("Verifying %d chunks..." % datLen)
        cCount = 0
        for chnkAdr, chnkDat in hxData.items():
            readBack = pic.readFlash(chnkAdr, hf.chunkSize)
                        
            if readBack != chnkDat:
                dlg.closeGauge()
                msgString = "Verification failed at %s! Aborting" % hex(chnkAdr)
                dlg.msgBox(msgString)
                return
            cCount += 1
            dlg.updateGauge((cCount * 100) // datLen)
        dlg.closeGauge()
    if writeCFG: # Write Configuration
        dlg.infoBox("Writing Configuration...")
        pic.writeConfiguration(hexCfg.toBinaryDict())
    if verCFG:  # Verify Configuration
        rbCfg = pic.readConfiguration()
        if rbCfg != hexCfg.toBinaryDict():
            dlg.msgBox("Configuration mismatch!")
            return
    if writeEE: # Write Data EEPROM
        dlg.infoBox("Writing Data EEPROM...")
        pic.writeDataEEPROM(eeDat)
    if verEE:   # Verify Data EEPROM
        rbEE = pic.readEEPROM(0, pic.device.DATA_EEPROM_SIZE)
        if rbEE != eeDat:
            dlg.msgBox("Data EEPROM mismatch!")
            return
    
    dlg.msgBox("All operations completed.")
    return

# ----- End Of writeHex ----       


def dumpToHex():
    # Query output
    while True:
        path = dlg.fselect("/home/")
        if path == None:
            return None  
        
        if not (path.lower().endswith(".hex") or path.endswith('\\')):
            path += ".hex"
            
        if os.path.isdir(path) or os.path.isfile(path):
            dlg.msgBox("File already exists or invalid path.")
        else:
            break;      
    dmpHf = HexFile.HexFile(path, readOnly = False)
    
    # Dump Code Memory
    chnkSize = 128
    chnkCount = pic.device.BLOCK_SIZE * (pic.device.MAX_BLOCK_INDEX + 1) // chnkSize   
    dlg.createGauge("Dumping " + str(chnkSize * chnkCount) + " Bytes to Hex File.")   
    for i in range(chnkCount):
        dlg.updateGauge((i * 100) // chnkCount)
        buf = pic.readFlash(i * chnkSize, chnkSize)
        
        allPad = True
        for b in buf:
            if int(b) != 0xFF:
                allPad = False
                break 
            
        if not allPad:   
            dmpHf.add(i * chnkSize, buf)     
                
    dlg.closeGauge()
    
    # Dump Data EEPROM
    if pic.device.HAS_DATA_EEPROM:
        dlg.infoBox("Dumping Data EEPROM...")
        ebuf = pic.readEEPROM(0, pic.device.DATA_EEPROM_SIZE) 
        dmpHf.add(pic.device.DATA_EEPROM_HEX_OFFSET, ebuf) 
    
    # Dump Configuration
    dlg.infoBox("Dumping Configuration...")
    confDict = pic.readConfiguration()
    conBAr = bytearray(1)
    for cAdr in confDict:
        conBAr[0] = confDict[cAdr]
        dmpHf.chunks[cAdr] = bytes(conBAr)
    dmpHf.writeOut()
    
    
    
    
    
        
     
            
# ----- MAIN -----           
    
        
if __name__ == '__main__':
    
    print("Starting BitBang-ICSP ...")
    print("Initializing ICSP ...")
    pic = Pic18fICSP();
    
    if not pic.initialized:
        print("FATAL: Initialization failed.")
        exit()
    pic.setClockTarget(50000)
    print("Clock set to %d Hz." % pic.getClock())
    print("Connecting to target ...")
    
    if not pic.connect():
        print("FATAL: Unable to connect to target.")
        exit()
    
    print("Connected to: " + pic.getTargetName())
    conf = PicConfig(pic.device)
    print("Starting UI ...")
    
    dlg = Dlg()
    dlg.backtitle = "BitBang-ICSP"
    dlg.infoBox("Use at your own risk!", 1)
    quit = False
    mainMenuOptions = ["Dump Memory", "Write Hex File", "Erase Memory", "Configuration Editor" ,"Set Clock", "Quit"]
    while not quit:
        sel = dlg.menu("Connected to " + pic.getTargetName() + ".", mainMenuOptions)   
        if sel == 0:
            dumpMenu()
        elif sel == 1:
            writeHex()
        elif sel == 2:
            eraseMenu()
        elif sel == 3:
            configMenu()
        elif sel == 4:
            clockSet()
        else:
            quit = True
             
     
    pic.disconnect()
    

  
