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

import sys
sys.path.append('..')

from mmfparser.data.exe import ExecutableData
from mmfparser.data.gamedata import GameData
from mmfparser.data.mfa import MFA
from mmfparser.translators.pame2mfa import translate
from mmfparser.bytereader import ByteReader

import sys
import os
from PySide.QtCore import *
from PySide.QtGui import *
 
def main():
    print 'MMF2 EXE->MFA translator by Mathias Kaerlev'
    print '(only for internal use)'
    print ''

    app = QApplication(sys.argv)

    input = QFileDialog.getOpenFileName(caption = 'Select input EXE file')[0]
    if not input:
        return
    output = QFileDialog.getExistingDirectory(
        caption = 'Select output directory')
    if not output:
        return
    fp = ByteReader(open(input, 'rb'))
    if input.endswith('.ccn'):
        newGame = GameData(fp)
    else:
        newExe = ExecutableData(fp)
        
        for file in newExe.packData.items:
            name = file.filename.split('\\')[-1]
            print 'Writing pack file %r' % name
            open(os.path.join(output, name), 'wb').write(file.data)

        newGame = newExe.gameData
        
    if newGame.files is not None:
        for file in newGame.files.items:
            name = file.name.split('\\')[-1]
            print 'Writing embedded file %r' % name
            open(os.path.join(output, name), 'wb').write(str(file.data))
            
    def out(value):
        print value

    newMfa = translate(newGame, print_func = out)
    newMfa.write(ByteReader(open(os.path.join(output, 'out.mfa'), 'wb')))
    print 'Finished!'

if __name__ == '__main__':
    main()