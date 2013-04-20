class Error(Exception):
    """Base class for exceptions in this module."""
    pass

class MissingConfigurationException(Error):
    def __init__(self, register):
        self.reg = register
    
    def __str__(self):
        return "Missing configuration value for register " + self.reg + "."


class PicConfig(object):
    def __init__(self, target):
        self.target = target
        self.optList = target.CONFIG_VALS
        self.loadDefaults()
            
    def loadDefaults(self):
        self.optVals = []
        for opt in self.optList:
            self.optVals.append(opt.default)
     
    @staticmethod       
    def getOptValDesc(opt, val):
        for vDesc, vVal in opt.values:
            if val == vVal:
                return vDesc
        return "???"
    
    def toBinaryDict(self):
        binDict = {}
        for opt, val in zip(self.optList, self.optVals):
            regAdr = self.target.CONFIG_REG_ADRS[opt.register]
            if regAdr not in binDict:
                binDict[regAdr] = 0       
            binDict[regAdr] += val << opt.offset
        return binDict
            
            
    def fromBinaryDict(self, binDict):
        for i in range(len(self.optList)):
            opt = self.optList[i]
            regAdr = self.target.CONFIG_REG_ADRS[opt.register]
            if regAdr not in binDict:
                continue       
            
            mask = ((1 << opt.length) - 1) << opt.offset
            self.optVals[i] = binDict[regAdr] & mask
            self.optVals[i] >>= opt.offset
            
    def fromHexFile(self, hexFile):
        cfgDict = {}
        for regName, regAdr in self.target.CONFIG_REG_ADRS.items():
            regVal = hexFile.read(regAdr, 1, pad = False)[0]
            if regVal != None:
                cfgDict[regAdr] = regVal
            else:
                raise MissingConfigurationException(regName)
        self.fromBinaryDict(cfgDict)
                

            
        
            
    
