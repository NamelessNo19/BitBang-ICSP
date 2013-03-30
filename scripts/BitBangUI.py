#!/usr/bin/python3

from DialogUtil import Dlg        
from time import sleep
from ICSP.Connector import Pic18fICSP;
import os.path

menuOptions = ["Dump to file", "Quit"]

def dmpToFile():
    pathOk = False
    while not pathOk:
        path = dlg.fselect("/home/")
        if path == None:
            return    
        
        if os.path.isdir(path) or os.path.isfile(path):
            dlg.msgBox("File already exists or invalid path.")
        else:
            try:
                file = open(path, "wb")
                pathOk = True
            except IOError as e:
                dlg.msgBox("IOError:\n" + str(e))
    

    
    chnkSize = 128
    chnkCount = pic.getTarget().BLOCK_SIZE * (pic.getTarget().MAX_BLOCK_INDEX + 1) // chnkSize
    
    dlg.createGauge("Dumping " + str(chnkSize * chnkCount) + " Bytes to " + path, 65)
    
    for i in range(0, chnkCount):
        dlg.updateGauge((i * 100) // chnkCount)
        buf = pic.readFlash(i * chnkSize, chnkSize)
        file.write(buf)

        
    
    dlg.closeGauge()
    file.close()
    dlg.msgBox("Dumping finished.")
        
    
        
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
    print("Starting UI ...")
    
    dlg = Dlg()
    dlg.backtitle = "BitBang-ICSP"
    dlg.infoBox("Use at your own risk!", 1)
    quit = False
    
    while not quit:
        sel = dlg.menu("Connected to " + pic.getTargetName() + ".", menuOptions)
    
        # Dump to file
        if sel == 0:
            dmpToFile()
        else:
            quit = True
             
     
    pic.disconnect()
    

  