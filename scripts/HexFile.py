from os import path


class Error(Exception):
    """Base class for exceptions in this module."""
    pass

class InputError(Error):
    """Exception raised when unable to read file."""
    def __init__(self, fPath):
        self.fPath = fPath
        
    def __str__(self):
        return "Unable to open %s for reading." % self.fPath

class FormatError(Error):
    """Exception raised when file content does not match hex file formatting"""
    def __init__(self, fName, line, msg):
        self.fName = fName
        self.line = line
        self.msg = msg
        
    def __str__(self):
        return "Bad formatting in " + self.fName + " (Line " + str(self.line) + "): " + self.msg
       
class ChecksumError(Error):
    """Exception raised at checksum mismatch"""
    def __init__(self, fName, line, csIs, csExp):
        self.fName = fName
        self.line = line
        self.csIs = csIs
        self.csExp = csExp
        
    def __str__(self):
        return "Checksum mismatch in " + self.fName + " (Line " + str(self.line) + "): " + "Expected: " +\
             hex(self.csExp) + " Is: " + hex(self.csIs) + "."

 
class HexFile(object):
    '''
    Parses an Intel HexFile to binary data.
    '''

    def __init__(self, fPath, bytesPerChunk = 4):
        '''
        Constructor
        '''
        self.chunkSize = bytesPerChunk
        self.chunks= {}
        try:
            self.file = open(fPath, 'r')
        except IOError:
            self.file = None
            raise InputError(fPath)
        else:
            self.name = path.basename(fPath)
            self._parseFile()
            self.file.close()
            
    def _parseFile(self):
        lineNo = 1
        self.curBaseAdr = 0
        
        for line in self.file: 
            if line[0] != ':':
                raise FormatError(self.name, lineNo, 'Missing colon at beginning of line.')
            line = line.strip()[1:]
            
            if len(line) < 10:
                raise FormatError(self.name, lineNo, 'Unexpected end of line.')
            
            try:
                lineDat = bytearray.fromhex(line.strip())
            except ValueError as ve:
                raise FormatError(self.name, lineNo, str(ve)) from ve
            if self._parseLine(lineDat, lineNo) == False:
                break
            lineNo += 1
        else:
          raise FormatError(self.name, lineNo, 'Unexpected end of file.')  
                
    
    def _parseLine(self, data, lineNo): 
        # Checksum        
        checksumEx = data[-1]
        checksumIs = -1 * sum(data[:-1]) % 256
        if checksumIs != checksumEx:
            raise ChecksumError(self.name, lineNo, checksumEx, checksumIs)
        
        # Parsing
        size = data[0]
        base = data[1] * 256 + data[2]
        type = data[3]
        
        if size != len(data) - 5:
            raise FormatError(self.name, lineNo, "Given size does not match actual length.") 
        
        if type == 0:
            # Data Record
            self._addSegment(base, data[4:-1], lineNo)
        elif type == 1:
            # EOF
            return False
        elif type == 2:
            # Extended Segment Address Record
            if size != 2:
                raise FormatError(self.name, lineNo, 'Length of ESAR does not equal 2.') 
            else:
                self.curBaseAdr = (self.curBaseAdr & 0xFFFF0000) | (data[4] * 256 + data[5])
        elif type == 4:
            # Extended Linear Address Record
            if size != 2:
                raise FormatError(self.name, lineNo, 'Length of ELAR does not equal 2.') 
            else:
                self.curBaseAdr = (self.curBaseAdr & 0x0000FFFF) | ((data[4] * 256 + data[5]) << 16) 
        else:
            raise FormatError(self.name, lineNo, "Unsupported record type.")
        
        return True
 
 
    def _addSegment(self, base, data, lineNo):
        adr = base + self.curBaseAdr
        offset = adr % self.chunkSize
        adr -= offset
        pos = 4 - offset
        self._updateChunk(adr, offset, data[0:pos], lineNo)
        adr += pos
        
        while pos < len(data):
            btw = min(len(data) - pos, self.chunkSize)
            self._updateChunk(adr, 0, data[pos:pos+btw], lineNo)
            adr += btw
            pos += btw
        
    
    def _updateChunk(self, chunkAdr, offset, data, lineNo):
        assert offset + len(data) <= self.chunkSize
        assert chunkAdr % self.chunkSize == 0
        
        newChunk = (1 << (8 * self.chunkSize) - 1)
        
        for b in data:
            newChunk <<= 8
            newChunk += b

        newChunk <<= offset * 8
        newChunk | (1 << offset * 8) - 1   
        newChunk &= 0xFFFFFFFF
        
        if chunkAdr in self.chunks:
            # Update existing Chunk
            bitMask = (1 << (8 * len(data))) - 1
            bitMask <<= offset * 8
            tmpChunk = self.chunks[chunkAdr]
            if tmpChunk & bitMask != bitMask:
                raise FormatError(self.name, lineNo, "Memory at 0x%0.8X reassigned." % (chunkAdr + offset))
            else:
                newChunk &= tmpChunk
        
        self.chunks.update({chunkAdr : newChunk})
            
         
# Testing       
if __name__ == '__main__':
    print('Go.')
    try:
        hf = HexFile('D:\\test.hex')
    except FormatError as fe:
        print(fe)
    else:
        for key in hf.chunks:
            print("0x%0.8X: 0x%0.8x" % (key, hf.chunks[key]))
            
        
        