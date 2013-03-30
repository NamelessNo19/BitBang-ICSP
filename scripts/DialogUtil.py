from os.path import isfile
from os import popen
import subprocess
from textwrap import fill
from time import sleep
from math import floor
import shlex


class TtySizeError(Exception):
    def __str__(self):
        return ("Terminal too small for dialog.")

class Dlg(object):
    @staticmethod
    def callDialog(message, type, width, height, heightpad, widthpad,
                    comArgs=[], msgArgs = [], pipeStdin = False):  
        
        callString = "dialog"
        
        
        for s in comArgs:
            callString += " " + s

        callString += " --" + type + " \"" + message + "\" "     
        callString += str(height + heightpad) + " " + str(width + widthpad)
        
        for s in msgArgs:
            callString += " " + s

        proc = subprocess.Popen(callString, shell=True, stderr=subprocess.PIPE, 
                                stdin=subprocess.PIPE if pipeStdin else None )
        return proc
    
    def __init__(self):
        if not isfile("/usr/bin/dialog"):
            print("WARNING: Cannot find dialog in /usr/bin/.")
       
        self.backtitle = None
        self.minWidth = 1
            
        self.resize()
        
        if self.termHeight < 15 or self.termWidth < 31:
            raise TtySizeError
        
        self.gaugeProc = None
        self.gaugeText = None
        
    def resize(self):
        termSize = popen("stty size", "r").read().split()
        self.termHeight = eval(termSize[0])
        self.termWidth = eval(termSize[1])

        
    def defArgs(self):
        tmpList = ["--cr-wrap"]
        
        if self.backtitle != None:
            tmpList.append("--backtitle \"" + self.backtitle + "\"")
            
        return tmpList
        
    def dialog(self, message, type, width = 0, height = 0, heightpad = 3, widthpad = 4, comArgs=[], msgArgs = []):
        if width <= 0:
            width = len(message)       
        width = min(self.termWidth - widthpad , width) 
        width = max(self.minWidth, width)
        height = min(height, self.termHeight - heightpad)      
        if len(message) > width:
            message = fill(message, width, replace_whitespace=False)  
        if height <= 0:
            height = message.count('\n')
        
        return Dlg.callDialog(message, type, width, height, heightpad, widthpad, self.defArgs() + comArgs, msgArgs)

    def infoBox(self, message, sleepTime = 0):
        proc = self.dialog(message, "infobox")
        if sleepTime > 0:
            sleep(sleepTime)
        proc.communicate();
        
    def msgBox(self, message):
        proc = self.dialog(message, "msgbox", heightpad = 5)
        proc.communicate();
            
    def menu(self, message, elements):
        mList = [str(len(elements))]
        i = 1
        backMinWd = self.minWidth
        for el in elements:
            mList.append(str(i) + " \"" + el + "\"")
            if len(el) > self.minWidth:
                self.minWidth = len(el)
            i += 1
            
        proc = self.dialog(message, "menu", msgArgs = mList, heightpad = 8 + len(elements), widthpad = 9)
        menuSel = proc.communicate()[1].decode()
        self.minWidth = backMinWd
        if len(menuSel) > 0:
            if menuSel[0] == '\n':
                raise TtySizeError
            return eval(menuSel) - 1
        else:
            return None
        
    def fselect(self, path = "/"):
        
        #proc = Dlg.dialog(path, "fselect", max(floor(self.termWidth * 0.6), 15) , max(floor(self.termHeight * 0.6), 31) , 0, 0)
        width = max(floor(self.termWidth * 0.6) - 15, 0)
        height = max(floor(self.termHeight * 0.8) - 15, 0)
        
        proc = Dlg.callDialog(path, "fselect", width, height, 0, 0, self.defArgs(), [], False)
        outPath = proc.communicate()[1].decode()
        if len(outPath) > 0:
            if outPath[0] == '\n':
                raise TtySizeError
            return outPath
        else:
            return None
    
    def createGauge(self, message, width = 0):
        
        if not width >= 0:
            width = floor(self.termWidth * 0.6) 
             
        self.gaugeProc = Dlg.callDialog(message, "gauge", width, 5, 0, 6,
                                         self.defArgs(), [], True)
        self.gaugeText = message
    
    def updateGauge(self, percentage, newMessage = None):
        if not newMessage == None:
            self.gaugeText = newMessage           
        self.gaugeProc.stdin.write(bytes("XXX\n" + str(percentage) + "\n" + self.gaugeText + "\nXXX\n", "ascii"))
        
    def closeGauge(self):
        self.gaugeProc.communicate()
        self.gaugeProc.stdin.close()
        self.gaugeProc = None
        
    def clearShell(self):
        subprocess.call("clear")