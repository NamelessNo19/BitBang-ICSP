#!/usr/bin/python3

from ctypes import *
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


class Pic18f(object):
    
    def __init__(self):
        self.initlialized = False
        self.targetEnabled = False;
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
            
    def getClock(self):
        if not self.initialized:
            raise NotInitializedError
            return None
        else:
            return self.lib.clkBench()
        
    def start(self):
        if not self.initialized:
            raise NotInitializedError
            return None
        else:
            if self.lib.pgmEnable() == True:
                print("ICSP connection established.")
                self.targetEnabled = True
            else:
                print("ICSP connection failed.")
                
    def stop(self):
        if not self.initialized:
            raise NotInitializedError
            return None
        else:
            self.lib.pwrOffTarget()
            print("ICSP disconnected.")
            self.targetEnabled = False
            
    def readDataWord(self, adress):
        if not self.initialized:
            raise NotInitializedError
            return None
        if not self.targetEnabled:
            raise NotConnectedError
            return None
        return self.lib.readWord(adress)

        
                          

# Testing       
if __name__ == '__main__':
    pic = Pic18f()
    print(str(pic.getClock()) + " Hz")
