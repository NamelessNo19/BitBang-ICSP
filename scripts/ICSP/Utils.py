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