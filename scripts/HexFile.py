from os import path


class Error(Exception):
    """Base class for exceptions in this module."""
    pass

class InputError(Error):
    """Exception raised when unable to open file."""
    def __init__(self, fPath):
        self.fPath = fPath
        
    def __str__(self):
        return "Unable to open %s." % self.fPath
    
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
        return "Checksum mismatch in " + self.fName + " (Line " + str(self.line) + "): " + "Expected: " + \
             hex(self.csExp) + " Is: " + hex(self.csIs) + "."
             
class AccessError(Error):
    def __str__(self):
        return "Cannot write to read-only file."

 
class HexFile(object):
    '''
    Parses an Intel HexFile to binary data.
    '''

    def __init__(self, fPath, bytesPerChunk=32, padByte=0xFF, readOnly=True):
        '''
        Constructor
        '''
        self.chunkSize = bytesPerChunk
        self.chunks = {}
        self.pad = padByte
        self.path = fPath
        self.rdOnly = readOnly
        
        if path.isdir(fPath):
            raise InputError(fPath);
        
        if readOnly and (not path.isfile(fPath)):
            raise InputError(fPath);
            
        try:
            self.file = open(fPath, 'r' if readOnly else 'a+')
            self.file.seek(0)
        except IOError:
            self.file = None
            raise InputError(fPath)
        else:
            self.name = path.basename(fPath)
            try:
                self._parseFile()
            except UnicodeDecodeError:
                raise FileFormatError(fPath)


    def __del__(self):
        if not self.file == None:
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
            if lineNo != 1:
                raise FormatError(self.name, lineNo, 'Unexpected end of file.')  
        
        self.file.seek(0)
                
    
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
                self.curBaseAdr = (self.curBaseAdr & 0xFFF0000F) | ((data[4] * 256 + data[5]) << 4)
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
            self._updateChunk(adr, 0, data[pos:pos + btw], lineNo)
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
        
    def read(self, offset, length, pad=True):
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
    
    def hasDataInRange(self, startAdr, length):
        offset = startAdr % self.chunkSize
        base = startAdr - offset
        
        while base < startAdr + length:
            if base in self.chunks:
                return True
            base += self.chunkSize
        
        return False
    
    def add(self, adr, data, skipPad = True):
        if self.rdOnly:
            raise AccessError()
        
        padSkipRev = 0
        for b in reversed(data):
            if int(b) == self.pad:
                padSkipRev += 1
            else:
                break
            
        if padSkipRev == len(data):
            return
        elif padSkipRev != 0:
            data = data[0 : -1 * padSkipRev]
        
        self.curBaseAdr = adr & 0xFFFF0000;
        self._addSegment(adr & 0xFFFF, data, -1)
        
    
    def writeOut(self):
        if self.rdOnly:
            raise AccessError()
        
        self.file.seek(0)
        self.file.truncate()
        base = 0
        self.file.write(":020000040000FA\n")    # SOF
        
        for cAdr in sorted(self.chunks.keys()):
            cnBase = cAdr >> 16
            loAdr = cAdr & 0xFFFF
             
            if base != cnBase:
                ckSum = 6
                ckSum += cnBase & 0xFF
                ckSum += cnBase >> 8
                ckSum = (0x100 - ckSum) & 0xFF
                self.file.write(":02000004%02X%02X%02X\n" % ((cnBase >> 8), (cnBase & 0xFF), ckSum))
                base = cnBase
            
            cLength =  len(self.chunks[cAdr])
            ckSum = cLength
            ckSum += loAdr & 0xFF
            ckSum += loAdr >> 8
            self.file.write(":%02X%04X00" % (cLength, loAdr))
            datStr = ""
            
            for b in self.chunks[cAdr]:
                ckSum += b                    
                datStr += "%02X" % int(b)
                
            ckSum = (0x100 - ckSum) & 0xFF
            self.file.write("%s%02X\n" % (datStr, ckSum))
        
        self.file.write(":00000001FF\n")  # EOF
        
        
        
        
        
# Testing       
if __name__ == '__main__':
    print('Go.')
    newHf = HexFile('D:\\blub.hex', readOnly = False)
    
    for key in newHf.chunks:
            print("0x%0.8X: " % key, newHf.chunks[key])
            
    newHf.add(0x00000008, bytes('ABCDEF', 'ascii'))
    newHf.writeOut()
