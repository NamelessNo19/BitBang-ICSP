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
    
class FileFormatError(Error):
    """Exception raised when input in not a Hex File."""
    def __init__(self, fPath):
        self.fPath = fPath
        
    def __str__(self):
        return "%s is not a valid Hex File." % self.fPath

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

    def __init__(self, fPath, bytesPerChunk = 4, padByte = 0xFF):
        '''
        Constructor
        '''
        self.chunkSize = bytesPerChunk
        self.chunks= {}
        self.pad = padByte
        try:
            self.file = open(fPath, 'r')
        except IOError:
            self.file = None
            raise InputError(fPath)
        else:
            self.name = path.basename(fPath)
            try:
                self._parseFile()
            except UnicodeDecodeError:
                raise FileFormatError(fPath)
            finally:
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
        lineType = data[3]
        
        if size != len(data) - 5:
            raise FormatError(self.name, lineNo, "Given size does not match actual length.") 
        
        if lineType == 0:
            # Data Record
            self._addSegment(base, data[4:-1], lineNo)
        elif lineType == 1:
            # EOF
            return False
        elif lineType == 2:
            # Extended Segment Address Record
            if size != 2:
                raise FormatError(self.name, lineNo, 'Length of ESAR does not equal 2.') 
            else:
                self.curBaseAdr = (self.curBaseAdr & 0xFFFF0000) | (data[4] * 256 + data[5])
        elif lineType == 4:
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
        pos = self.chunkSize - offset
        
        if pos != 0:
            self._updateChunk(adr, offset, data[0:pos], lineNo)
            adr += self.chunkSize
        
        while pos < len(data):
            btw = min(len(data) - pos, self.chunkSize)
            self._updateChunk(adr, 0, data[pos:pos+btw], lineNo)
            adr += btw
            pos += btw
        
    
    def _updateChunk(self, chunkAdr, offset, data, lineNo):
        try:
            assert offset + len(data) <= self.chunkSize
            assert chunkAdr % self.chunkSize == 0
        except AssertionError:
            print("Line: %d, Adr: %s, Offset: %s, Len: %d" % (lineNo, hex(chunkAdr), hex(offset), len(data)))
            raise
            
        newChunk = bytearray([self.pad] * self.chunkSize)
        newChunk[offset:offset + len(data)] = data

        if chunkAdr in self.chunks:
            i = offset
            tmpChunk = bytearray(self.chunks[chunkAdr])
            for i in range(offset, offset + len(data)):
                if tmpChunk[i] == self.pad:
                    tmpChunk[i] = newChunk[i]
                elif tmpChunk[i] != newChunk[i]:
                    raise FormatError(self.name, lineNo, "Multiple assignment of location 0x%0.8X ." % (chunkAdr + offset))
            newChunk = tmpChunk
                    
        self.chunks.update({chunkAdr : bytes(newChunk)})
        
    def read(self, offset, length, pad = True):
        chunkOffset = offset % self.chunkSize
        chunkBase = offset - chunkOffset

        retBA = bytearray(length)
        rdlen = 0            
        while rdlen < length:
            btr = min(length - rdlen, self.chunkSize - chunkOffset)
            if chunkBase not in self.chunks:
                if pad:
                    retBA[rdlen : rdlen + btr] = bytearray([self.pad] * btr)
                else:
                    return None
            else:
                retBA[rdlen : rdlen + btr] = self.chunks[chunkBase][chunkOffset : chunkOffset + btr]
            rdlen += btr
            chunkBase += self.chunkSize
            chunkOffset = 0
        return bytes(retBA)
            
            
        
        
        
        
# Testing       
if __name__ == '__main__':
    print('Go.')
    try:
        hf = HexFile('D:\\blink.hex')
    except FormatError as fe:
        print(fe)
    else:
        for key in hf.chunks:
            print("0x%0.8X: " % key, hf.chunks[key])
            
            
        
        