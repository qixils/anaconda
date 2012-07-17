# Copyright (c) Mathias Kaerlev 2012.

# This file is part of Anaconda.

# Anaconda is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# Anaconda is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with Anaconda.  If not, see <http://www.gnu.org/licenses/>.

"""
Python module for reading KcArray's file format
"""

from mmfparser.bytereader import ByteReader
from mmfparser.bitdict import BitDict
from mmfparser.loader import DataLoader
from mmfparser.exceptions import InvalidData

MAGIC = 'CNC ARRAY'

MAJOR_VERSION = 2
MINOR_VERSION = 0

class MMFArray(DataLoader):
    items = None
    size = None
    default = None
    def initialize(self):
        self.flags = BitDict('Numeric', 'Text', 'Base1', 'Global')
        self.flags.setFlags(0) # to prevent BitDict from complaining :)
        self.flags['Base1'] = True # default
        self.flags['Text'] = True # default
        self.default = ''
        self.flags_updated()
        
    def read(self, reader):
        if reader.readString() != MAGIC:
            raise InvalidData('data is invalid')
        if reader.readShort() != MAJOR_VERSION:
            raise InvalidData('major version incompatibility')
        if reader.readShort() != MINOR_VERSION:
            raise InvalidData('minor version incompatibility')
        xDimension = reader.readInt(True)
        yDimension = reader.readInt(True)
        zDimension = reader.readInt(True)
        self.size = (xDimension, yDimension, zDimension)
        items = self.items = {}
        self.flags.setFlags(reader.readInt())
        if self.flags['Numeric']:
            self.default = 0
        elif self.flags['Text']:
            self.default = ''
        else:
            raise NotImplementedError(
                'invalid array type (should be "Text" or "Numeric")')
        for z in xrange(zDimension):
            for y in xrange(yDimension):
                for x in xrange(xDimension):
                    if self.flags['Numeric']:
                        item = reader.readInt(True)
                    elif self.flags['Text']:
                        item = reader.read(reader.readInt(True))
                    else:
                        raise InvalidData('invalid flags')
                    items[(x, y, z)] = item

    def flags_updated(self):
        self.indexOffset = int(self.flags['Base1'])
    
    def write(self, reader):
        reader.writeString(MAGIC)
        reader.writeShort(MAJOR_VERSION)
        reader.writeShort(MINOR_VERSION)
        xDimension, yDimension, zDimension = self.size
        reader.writeInt(xDimension)
        reader.writeInt(yDimension)
        reader.writeInt(zDimension)
        reader.writeInt(self.flags.getFlags())
        for z in xrange(zDimension):
            for y in xrange(yDimension):
                for x in xrange(xDimension):
                    try:
                        item = self.items[(x, y, z)]
                    except KeyError:
                        item = self.default
                    if self.flags['Numeric']:
                        reader.writeInt(int(item))
                    elif self.flags['Text']:
                        item = str(item)
                        reader.writeInt(len(item))
                        reader.write(item)
                    else:
                        raise NotImplementedError('invalid flags')

    def get(self, x, y, z):
        x -= self.indexOffset
        y -= self.indexOffset
        z -= self.indexOffset
        return self.items[(x, y, z)]
    
    def set(self, x, y, z, value):
        x -= self.indexOffset
        y -= self.indexOffset
        z -= self.indexOffset
        self.items[(x, y, z)] = value
            
    def setup(self, xDimension, yDimension, zDimension, arrayType):
        if arrayType == 'Text':
            self.flags['Numeric'] = False
            self.flags['Text'] = True
            self.default = ''
        elif arrayType == 'Numeric':
            self.flags['Numeric'] = True
            self.flags['Text'] = False
            self.default = 0
        else:
            raise NotImplementedError(
                'invalid array type (should be "Text" or "Numeric")')
        self.size = (xDimension, yDimension, zDimension)
        self.clear()
    
    def clear(self):
        self.items = {}