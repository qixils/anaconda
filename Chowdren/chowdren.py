import sys
sys.path.append('..')

import os
import shutil
from mmfparser.data.exe import ExecutableData
from mmfparser.data.gamedata import GameData
from mmfparser.bytereader import ByteReader
from mmfparser.data.chunkloaders.objectinfo import (PLAYER, KEYBOARD, CREATE,
    TIMER, GAME, SPEAKER, SYSTEM, QUICKBACKDROP, BACKDROP, ACTIVE, TEXT, 
    QUESTION, SCORE, LIVES, COUNTER, RTF, SUBAPPLICATION, EXTENSION_BASE,
    INK_EFFECTS)
from mmfparser.data.chunkloaders.objects import (COUNTER_FRAMES, 
    ANIMATION_NAMES, NUMBERS, HIDDEN, VERTICAL_BAR, HORIZONTAL_BAR, 
    VERTICAL_GRADIENT, HORIZONTAL_GRADIENT, RECTANGLE_SHAPE, SOLID_FILL,
    GRADIENT_FILL)
from mmfparser.data.chunkloaders.frame import NONE_PARENT
from mmfparser.bitdict import BitDict
from cStringIO import StringIO
import textwrap
import Image
from mmfparser.extension import loadLibrary, LoadedExtension
from mmfparser.data.font import LogFont
from key import key_to_qt
import string
import functools
import itertools
from collections import defaultdict, Counter

WRITE_IMAGES = True
WRITE_FONTS = True
WRITE_SOUNDS = True
WAIT_UNIMPLEMENTED = False
WAIT_UNNAMED = True
WRITE_CONFIG = True

LICENSE = ("""\
// Copyright (c) Mathias Kaerlev 2012.
//
// This file is part of Chowdren.
//
// Chowdren is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Chowdren is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Chowdren.  If not, see <http://www.gnu.org/licenses/>.\
""")


def wait_unimplemented():
    if WAIT_UNIMPLEMENTED:
        raw_input('(press enter to continue)')

def wait_unnamed():
    if WAIT_UNNAMED:
        raw_input('(press enter to continue)')

def copytree(src, dst):
    names = os.listdir(src)
    try:
        os.makedirs(dst)
    except OSError:
        pass
    errors = []
    for name in names:
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)
        try:
            if os.path.isdir(srcname):
                copytree(srcname, dstname)
            else:
                shutil.copy2(srcname, dstname)
        # catch the Error from the recursive copytree so that we can
        # continue with other files
        except shutil.Error, err:
            errors.extend(err.args[0])
        except EnvironmentError, why:
            errors.append((srcname, dstname, str(why)))
    try:
        shutil.copystat(src, dst)
    except OSError, why:
        if WindowsError is not None and isinstance(why, WindowsError):
            # Copying file access times may fail on Windows
            pass
        else:
            errors.extend((src, dst, str(why)))
    if errors:
        raise shutil.Error, errors

class CodeWriter(object):
    indentation = 0
    def __init__(self, filename = None):
        if filename is None:
            self.fp = StringIO()
        else:
            self.fp = open(filename, 'wb+')
    
    def format_line(self, line):
        return self.get_spaces() + line
    
    def putln(self, *lines, **kw):
        wrap = kw.get('wrap', False)
        indent = kw.get('indent', True)
        if wrap:
            indent = self.get_spaces(1)
        for line in lines:
            if wrap:
                line = ('\n' + indent).join(textwrap.wrap(line))
            if indent:
                line = self.format_line(line)
            self.fp.write(line + '\n')

    def putdefine(self, name, value):
        if value is None:
            return
        if isinstance(value, str):
            value = '"%s"' % value
        self.putln('#define %s %s' % (name, value))
    
    def putindent(self, extra = 0):
        self.fp.write(self.get_spaces(extra))
    
    def put(self, value, indent = False):
        if indent:
            self.putindent()
        self.fp.write(value)
    
    def get_data(self):
        fp = self.fp
        pos = fp.tell()
        fp.seek(0)
        data = fp.read()
        fp.seek(pos)
        return data

    def putcode(self, writer):
        data = writer.get_data().splitlines()
        for line in data:
            self.putln(line)
    
    def putclass(self, name, subclass = None):
        text = 'class %s' % name
        if subclass is not None:
            text += ' : public %s' % subclass
        self.putln(text)
        self.start_brace()

    def start_brace(self):
        self.putln('{')
        self.indent()

    def end_brace(self, semicolon = False):
        self.dedent()
        text = '}'
        if semicolon:
            text += ';'
        self.putln(text)
    
    def putdef(self, name, value, wrap = False):
        new_value = '%r' % (value,)
        self.putln('%s = %s' % (name, new_value), wrap = wrap)
    
    def putfunc(self, name, *arg, **kw):
        fullarg = list(arg)
        if kw:
            for k, v in kw.iteritems():
                fullarg.append('%s = %s' % (k, v))
        self.putln('def %s(%s):' % (name, ', '.join(fullarg)))
        self.indent()

    def putmeth(self, name, *arg, **kw):
        fullarg = list(arg)
        self.putln('%s(%s)' % (name, ', '.join(fullarg)))
        self.start_brace()

    def put_label(self, name):
        self.dedent()
        self.putln('%s: ;' % name)
        self.indent()

    def put_access(self, name):
        self.dedent()
        self.putln('%s:' % name)
        self.indent()

    def start_guard(self, name):
        self.putln('#ifndef %s' % name)
        self.putln('#define %s' % name)
        self.putln('')

    def close_guard(self, name):
        self.putln('')
        self.putln('#endif // %s' % name)
    
    def putend(self):
        self.putln('pass')
        self.dedent()
        self.putln('')
    
    def indent(self):
        self.indentation += 1
        
    def dedent(self):
        self.indentation -= 1
        if self.indentation < 0:
            raise ValueError('indentation cannot be lower than 0')
        
    def get_spaces(self, extra = 0):
        return (self.indentation + extra) * '    '
    
    def close(self):
        self.fp.close()

    def get_line_count(self):
        return self.get_data().count('\n')
    
    # misc. helpers

    def putfont(self, font):
        if font is None:
            # default font
            face = 'Arial'
            size = 8
            bold = False
            italic = False
            underline = False
        else:
            face = font.faceName
            size = font.getSize()
            bold = font.isBold()
            italic = bool(font.italic)
            underline = bool(font.underline)
        self.putln('font = Font(%r, %r, %r, %r, %r)' % (face, size, bold, 
            italic, underline))    

extensions = {
    'kcfile' : {
        'actions' : {
            4 : 'CreateFile',
            10 : 'AppendText',
            5 : 'DeleteFile'
            
        },
        'conditions' : {
            2 : 'FileReadable',
            4 : 'NameIsFile'
        },
        'expressions' : {}
    },
    'kcpica' : {
        'actions' : {
            5 : 'ActivePictureSetTransparency',
            0 : 'ActivePictureCreateBackdrop'
        },
        'conditions' : {
        },
        'expressions' : {}
    },
    'ModFusionEX' : {
        'actions' : {
            4 : 'QuickLoadMod',
            15 : 'SetModuleVolume',
            23 : 'ModuleCrossFade',
            13 : 'ModuleFadeStop',
            12 : 'ModuleStop',
            11 : 'ModulePlay'
        },
        'conditions' : {
            1 : 'ModIsPlaying'
        },
        'expressions' : {}
    },
    'moosock' : {
        'actions' : {
            0 : 'SockAccept',
            1 : 'Connect',
            2 : 'Disconnect',
            4 : 'SockSendText',
            5 : 'SockSendLine',
            12 : 'SockSetTimeout',
            3 : 'SockListen',
            18 : 'DeleteSocket',
            13 : 'SelectSocket',
            16 : 'SockSetProperty',
            17 : 'SockSelectSockWithProperty'
        },
        'conditions' : {
            0 : 'SockConnected',
            3 : 'SockCanAccept',
            1 : 'SockDisconnected',
            2 : 'SockReceived',
            4 : 'SockIsConnected'
        },
        'expressions' : {
            1 : 'SockGetLocalIP',
            2 : 'SockGetRemoteIP',
            3 : 'SockReceiveText',
            4 : 'SockReceiveLine',
            40 : 'SockGetProperty',
            39 : 'SockCountSockets'
        }
    },
    'kcedit' : {
        'actions' : {
            23 : 'LimitTextSize',
            16 : 'SetFocusOn',
            30 : 'SetFocusOff',
            4 : 'SetEditText',
            0 : 'EditLoadFile',
            20 : 'EditReadOnlyOff',
            19 : 'EditReadOnlyOn',
            33 : 'EditScrollToEnd',
            17 : 'EditEnable',
            18 : 'EditDisable',
            12 : 'EditMakeVisible',
            13 : 'EditMakeInvisible'
        },
        'conditions' : {
            0 : 'EditIsVisible',
            3 : 'EditModified',
            4 : 'EditHasFocus',
            5 : 'EditIsNumber'
        },
        'expressions' : {
            0 : 'EditGetText',
            6 : 'EditTextNumeric'
        }
    },
    'kcini' : {
        'actions' : {
            6 : 'SetINIFile',
            0 : 'SetINIGroup',
            1 : 'SetINIItem',
            9 : 'SetINIItemString',
            10 : 'SetINIGroupItemValue',
            7 : 'SetINIItemValue'
        },
        'conditions' : {},
        'expressions' : {
            0 : 'GetINIValue',
            1 : 'GetINIString',
            2 : 'GetINIValueItem',
            4 : 'GetINIStringItem'
        }
    },
    'parser' : {
        'actions' : {
            6 : 'AddDelimiter',
            5 : 'ResetDelimiters',
            0 : 'ParserSetText'
            
        },
        'conditions' : {},
        'expressions' : {
            0 : 'ParserGetString',
            31 : 'ParserGetLastElement',
            29 : 'ParserGetElement',
            30 : 'ParserGetFirstElement',
            24 : 'ParserGetElementCount'
        }
    },
    'Blowfish' : {
        'actions' : {
            0 : 'AddEncryptionKey',
            1 : 'BlowfishRemoveKey'
        },
        'conditions' : {},
        'expressions' : {
            10 : 'BlowfishRandomKey',
            6 : 'BlowfishFilterString',
            2 : 'BlowfishEncryptString',
            5 : 'BlowfishDecryptString',
        }
    },
    'binary' : {
        'actions' : {
            3 : 'BinaryInsertString',
            40 : 'BinaryReplaceString',
            22 : 'BinaryDecodeBase64',
            33 : 'BinaryClear',
            21 : 'BinaryEncodeBase64'
        },
        'conditions' : {},
        'expressions' : {
            1 : 'BinaryGetStringAt',
            0 : 'BinaryGetSize'
        }
    },
    'iconlist' : {
        'actions' : {
            0 : 'IconListSetLine',
            1 : 'IconListAddLine',
            6 : 'IconListReset',
            13 : 'IconListDehighlight'
        },
        'conditions' : {
            1 : 'IconListSelectionChanged',
            2 : 'IconListOnDoubleClick'
        },
        'expressions' : {
            0 : 'IconListGetIndex'
        }
    },
    'kcriched' : {
        'actions' : {
            24 : 'RichGoCharacter',
            44 : 'RichSetFontColorString',
            46 : 'RichSetText'
        },
        'conditions' : {
            19 : 'RichLinkClicked'
        },
        'expressions' : {
            16 : 'RichGetCharacterCount',
            13 : 'RichGetLinkText'
        }
    },
    'kclist' : {
        'actions' : {
            5 : 'ListReset',    
            6 : 'ListAddLine',
            24 : 'ListScrollEnd',
            8 : 'DeleteLine',
            3 : 'ListLoadFiles',
            0 : 'ListLoad',
            14 : 'ListFocusOff',
            13 : 'ListFocusOn',
            4 : 'ListSaveListFile',
            7 : 'ListInsertLine'
        },
        'conditions' : {
            3 : 'ListSelectionChanged'
        },
        'expressions' : {
            4 : 'ListGetLine',
            15 : 'ListFindExact',
            0 : 'ListCurrentLineIndex',
            7 : 'ListGetLineCount',
            1 : 'ListGetCurrentLine'
        }
    },
    'kcwctrl' : {
        'actions' : {
        },
        'conditions' : {
            3 : 'ApplicationActive',
            2 : 'IsWindowVisible'
        },
        'expressions' : {
        }
    },
    'Gstrings' : {
        'actions' : {
            0 : 'SetGlobalString'
        },
        'conditions' : {
        },
        'expressions' : {
            0 : 'GetGlobalString'
        }
    },
    'kcbutton' : {
        'actions' : {
            14 : 'ButtonCheck',
            4 : 'ButtonDisable',
            3 : 'ButtonEnable'
        },
        'conditions' : {
            0 : 'ButtonBoxChecked',
            1 : 'ButtonClicked'
        },
        'expressions' : {
        }
    },
    'Encryption' : {
        'actions' : {
            0 : 'EncryptText',
            1 : 'DecryptText'
        },
        'conditions' : {
        },
        'expressions' : {
        }
    },
    'XXCRC' : {
        'actions' : {
            0 : 'CalculateCRC'
        },
        'conditions' : {
        },
        'expressions' : {
            0 : 'GetFileCRC'
        }
    },
    'funcloop' : {
        'actions' : {
            0 : 'CallFunction',
            13 : 'StartFuncLoop',
            1 : 'CallFunctionInt',
            14 : 'StopCurrentFuncLoop',
            9 : 'ReturnFunction',
            # 1 : 'GetFuncLoopIndex',
            # 1 : 'GetCurrentFuncLoopIndex'
        },
        'conditions' : {
            0 : 'OnFunction',
            1 : 'OnFuncLoop'
        },
        'expressions' : {
            1 : 'GetCurrentFuncLoopIndex',
            2 : 'GetIntArgumentA'
        }
    },
    'IIF' : {
        'actions' : {
        },
        'conditions' : {
        },
        'expressions' : {
            0 : 'IIFIntegerCompareInteger', # 'Return Types...', 'Integers', 'Compare Integers(n,operator,n,true,false)',
            3 : 'IIFStringCompareInteger',
            4 : 'IIFStringCompareString'
        }
    },
    'Capture' : {
        'actions' : {
            0 : 'CaptureSetFilename',
            5 : 'CaptureFrame',
            8 : 'CaptureStartAutomatic'
        },
        'conditions' : {
        },
        'expressions' : {
        }
    },
    'timex' : {
        'actions' : {
            1 : 'StartGlobalTimer',
            2 : 'StopGlobalTimer',
            4 : 'ResetGlobalTimer'
        },
        'conditions' : {
        },
        'expressions' : {
            2 : 'GetGlobalTimer'
        }
    },
    'Overlay' : {
        'actions' : {
            48 : 'OverlaySetTransparent',
            8 : 'OverlayClearRGB',
            12 : 'OverlayDrawRectangle'
        },
        'conditions' : {
            1 : 'OverlayMatchColorRGB'
        },
        'expressions' : {
        }
    },
    'Dsound' : {
        'actions' : {
            5 : 'SoundAutoPlay',
            7 : 'SoundAutoPlayPosition',
            0 : 'SoundSetDirectory',
            8 : 'SoundSetListenerPosition'
        },
        'conditions' : {
        },
        'expressions' : {
        }
    },
    'Layer' : {
        'actions' : {
            27 : 'SortByAlterableDecreasing'
        },
        'conditions' : {
        },
        'expressions' : {
        }
    },
    'KcBoxA' : {
        'actions' : {
            55 : 'BoxSetText'
        },
        'conditions' : {
            3 : 'BoxLeftClicked'
        },
        'expressions' : {
            29 : 'BoxGetText'
        }
    },
    'ctrlx' : {
        'actions' : {
            7 : 'ControlSetPlayerUp',
            8 : 'ControlSetPlayerDown',
            9 : 'ControlSetPlayerLeft',
            10 : 'ControlSetPlayerRight',
            11 : 'ControlSetPlayerFire1',
            12 : 'ControlSetPlayerFire2'
        },
        'conditions' : {
            9 : 'ControlAnyKeyDown'
        },
        'expressions' : {
            0 : 'ControlLastKeyString'
        }
    },
    'kcplugin' : {
        'actions' : {
            4 : 'OpenURL'
        },
        'conditions' : {
        },
        'expressions' : {
        }
    },
    'txtblt' : {
        'actions' : {
            0 : 'BlitChangeText'
        },
        'conditions' : {
        },
        'expressions' : {
        }
    },
}

native_extension_cache = {}
MMF_PATHS = (
    # ('C:\Programs\Multimedia Fusion 1.5\Programs\Extensions', '.cox'),
    ('C:\Programs\Multimedia Fusion Developer 2\Extensions', '.mfx')
)

def load_native_extension(name):
    for path_index, (path, ext) in enumerate(MMF_PATHS):
        try:
            return native_extension_cache[name]
        except KeyError:
            library = loadLibrary(os.path.join(path, name + ext))
            if library is None:
                continue
            print 'loaded native extension (%s): %s' % (path_index, name)
            extension = LoadedExtension(library, False)
            native_extension_cache[name] = extension
            return extension
    native_extension_cache[name] = None
    return None

def get_image_list(values):
    return '[%s]' % ', '.join(['image%s' % image for image in values])

def get_qt_key(value):
    if value == 1:
        return 'Qt.LeftButton'
    elif value == 4:
        return 'Qt.MiddleButton'
    elif value == 2:
        return 'Qt.RightButton'
    return 'Qt.Key_%s' % key_to_qt(value)

VALID_CHARACTERS = string.ascii_letters + string.digits
DIGITS = string.digits

def get_method_name(value, check_digits = False):
    new_name = ''
    add_underscore = False
    for c in value:
        if c.isupper():
            c = c.lower()
            add_underscore = True
        if c in VALID_CHARACTERS:
            if add_underscore:
                if new_name:
                    new_name += '_'
                add_underscore = False
            new_name += c
        else:
            add_underscore = True
    if check_digits:
        new_name = check_digits(new_name, 'meth_')
    return new_name

def get_class_name(value):
    new_name = ''
    go_upper = True
    for c in value:
        if c in VALID_CHARACTERS:
            if go_upper:
                c = c.upper()
                go_upper = False
            new_name += c
        else:
            go_upper = True
    return check_digits(new_name, 'Obj')

def check_digits(value, prefix):
    if not value:
        return value
    if value[0] in DIGITS:
        value = prefix + value
    return value

def is_qualifier(handle):
    return handle & 32768 == 32768

def get_qualifier(handle):
    return handle & 2047

ACTIVE_BOX_COLORS = {
    0 : (0xC8, 0xC8, 0xC8),
    1 : (0x00,0x00,0x00),
    2 : (0x99,0xb4,0xd1),
    3 : (0xbf,0xcd,0xdb), # SystemColor.activeCaptionBorder,
    4 : (0xf0,0xf0,0xf0),
    5 : (0xff,0xff,0xff),
    6 : (0x64,0x64,0x64), # SystemColor.inactiveCaptionBorder,
    7 : (0x00,0x00,0x00),
    8 : (0x00,0x00,0x00),
    9 : (0x00,0x00,0x00),
    10 : (0xb4,0xb4,0xb4), # new
    11 : (0xf4,0xf7,0xfc), # new
    12 : (0xab,0xab,0xab), # mdi one, doesn't quite match. There is no java mdi background colour./ AppWorksapce
    13 : (0x33,0x99,0xff), # SystemColor.textText,
    14 : (0xff,0xff,0xff), # new  #SystemColor.textHighlight,
    15 : (0xf0,0xf0,0xf0), # SystemColor.textHighlightText,
    16 : (0xa0,0xa0,0xa0), # SystemColor.textInactiveText,
    17 : (0x80,0x80,0x80),
    18 : (0x00,0x00,0x00),
    19 : (0x43,0x4e,0x54),
    20 : (0xff,0xff,0xff),
    21 : (0x69,0x69,0x69),
    22 : (0xe3,0xe3,0xe3),
    23 : (0x00,0x00,0x00),
    24 : (0xff,0xff,0xe1),
}

def get_color_number(value):
    return (value & 0xFF, (value & 0xFF00) >> 8, (value & 0xFF0000) >> 16)

def get_system_color(index):
    if index == 0xFFFF:
        return None
    if index & (1 << 31) != 0:
        return get_color_number(index)
    try:
        return ACTIVE_BOX_COLORS[index]
    except KeyError:
        return (0, 0, 0)

def read_system_color(reader):
    return get_system_color(reader.readInt(True))

OBJECT_CLASSES = {
    TEXT : 'Text',
    ACTIVE : 'Active',
    BACKDROP : 'Backdrop',
    COUNTER : 'Counter',
    SUBAPPLICATION : 'SubApplication',
    'kcedit' : 'Edit',
    'kcpica' : 'ActivePicture',
    'parser' : 'StringParser',
    'binary' : 'BinaryObject',
    'kcbutton' : 'ButtonControl',
    'kcini' : 'INI',
    'Blowfish' : None,
    'kcfile' : None,
    'ModFusionEX' : None,
    'ControlX' : None,
    'Gstrings' : None,
    'kcplugin' : None,
    'ctrlx' : None,
    'funcloop' : None,
    'IIF' : None,
    'Layer' : None,
    'Dsound' : None,
    'timex' : None,
    'Encryption' : None,
    'XXCRC' : 'ChecksumCalculator',
    'kcwctrl' : None,
    'moosock' : 'Socket',
    'kclist' : 'ListControl',
    'iconlist' : 'IconList',
    'kcriched' : 'RichEdit',
    'KcBoxA' : 'ActiveBox',
    'txtblt' : 'TextBlitter',
    'Overlay' : 'OverlayRedux',
    'Capture' : None,
}

active_picture_flags = BitDict(
    'Resize',
    'HideOnStart',
    'TransparentBlack',
    'TransparentFirstPixel',
    'FlippedHorizontally',
    'FlippedVertically',
    'Resample',
    'WrapModeOff',
)

edit_flags = BitDict(
    'HorizontalScrollbar',
    'HorizontalAutoscroll',
    'VerticalScrollbar',
    'VerticalAutoscroll',
    'ReadOnly',
    'Multiline',
    'Password',
    'Border',
    'HideOnStart',
    'Uppercase',
    'Lowercase',
    'Tabstop',
    'SystemColor',
    '3DLook',
    'Transparent',
    None,
    'AlignCenter',
    'AlignRight'
)

list_flags = BitDict(
    'FreeFlag',
    'VerticalScrollbar',
    'Sort',
    'Border',
    'HideOnStart',
    'SystemColor',
    '3DLook',
    'ScrollToNewline'
)

rich_edit_flags = BitDict(
    'ReadOnly',
    'HorizontalBar',
    'VerticalBar',
    'HorizontalAutoscroll',
    'VerticalAutoscroll',
    'Visible',
    'Border',
    'Internal',
    'SystemColor',
    'V20',
    'AutoLinkURL',
    'Transparent',
    'MultiLevelUndo',
    'NoColorChangeWhenDisabled'
)

button_flags = BitDict(
    'HideOnStart',
    'DisableOnStart',
    'TextOnLeft',
    'Transparent',
    'SystemColor'
)

active_box_flags = BitDict(
    'AlignTop',
    'AlignVerticalCenter', 
    'AlignBottom', 
    None, 
    'AlignLeft', 
    'AlignHorizontalCenter', 
    'AlignRight', 
    None, 
    'Multiline', 
    'NoPrefix', 
    'EndEllipsis', 
    'PathEllipsis', 
    'Container', 
    'Contained', 
    'Hyperlink', 
    None, 
    'AlignImageTopLeft', 
    'AlignImageCenter', 
    'AlignImagePattern', 
    None, 
    'Button',
    'Checkbox',
    'ShowButtonBorder', 
    'ImageCheckbox',
    'HideImage',
    'ForceClipping',
    None,
    None,
    'ButtonPressed', 
    'ButtonHighlighted',
    'Disabled'
)

key_flags = BitDict(
    'Up',
    'Down',
    'Left',
    'Right',
    'Fire1',
    'Fire2',
    'Fire3',
    'Fire4'
)

PUSHTEXT_BUTTON = 0
CHECKBOX_BUTTON = 1
RADIO_BUTTON = 2
PUSHBITMAP_BUTTON = 3
PUSHTEXTBITMAP_BUTTON = 4

button_type_names = {
    PUSHTEXTBITMAP_BUTTON : 'Text',
    CHECKBOX_BUTTON : 'Check',
    RADIO_BUTTON : 'Radio',
    PUSHBITMAP_BUTTON : 'Bitmap',
    PUSHTEXTBITMAP_BUTTON : 'BitmapText'
}

COMPARISONS = [
    '==',
    '!=',
    '<=',
    '<',
    '>=',
    '>'
]

ACCELERATORS = [0.0078125, 0.01171875, 0.015625, 0.0234375, 0.03125, 0.0390625,
    0.046875, 0.0625, 0.078125, 0.09375, 0.1875, 0.21875, 0.25, 0.28125,
    0.3125, 0.34375, 0.375, 0.40625, 0.4375, 0.46875, 0.5625, 0.625, 0.6875, 
    0.75, 0.8125, 0.875, 0.9375, 1.0, 1.0625, 1.125, 1.25, 1.3125, 1.375,
    1.4375, 1.5, 1.5625, 1.625, 1.6875, 1.75, 1.875, 2.0, 2.125, 2.1875,
    2.3125, 2.4375, 2.5, 2.625, 2.6875, 2.8125, 2.875, 3.0, 3.0625, 3.1875,
    3.3125, 3.375, 3.5, 3.625, 3.6875, 3.8125, 3.875, 4.0, 4.375, 4.75,
    5.125, 5.625, 6.0, 6.375, 6.75, 7.125, 7.625, 8.0, 8.75, 9.5, 10.5, 11.25,
    12.0, 12.75, 13.5, 14.5, 15.25, 16.0, 25.5625, 19.1953125, 20.375,
    22.390625, 24.0, 25.59765625, 27.1953125, 28.7734375, 30.390625, 32.0,
    38.421875, 45.59375, 52.015625, 58.4375, 64.859375, 71.28125, 77.703125,
    84.0, 100.0, 100.0]

DEFAULT_CHARMAP = ''.join([chr(item) for item in xrange(32, 256)])

class EventContainer(object):
    is_static = False
    def __init__(self, name, inactive):
        self.name = name
        self.inactive = inactive
        self.always_groups = []

class EventGroup(object):
    global_id = None
    local_id = None
    container_start = False
    container_end = False
    generated = False
    def __init__(self, conditions, actions, container = None):
        self.conditions = conditions
        self.actions = actions
        self.container = container
        self.config = {}

class StringWrapper(object):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return self.value

    def __repr__(self):
        return '"%s"' % self.value.replace('"', '')

def to_c(format_spec, *args):
    new_args = []
    for arg in args:
        if isinstance(arg, str):
            arg = StringWrapper(arg)
        elif isinstance(arg, bool):
            if arg:
                arg = 'true'
            else:
                arg = 'false'
        new_args.append(arg)
    return format_spec % tuple(new_args)

class Converter(object):
    iterated_object = None
    def __init__(self, filename, outdir):
        self.filename = filename
        self.outdir = outdir
        
        self.checked_aces = {'actions' : Counter(), 
                             'conditions' : Counter(),
                             'expressions' : Counter()}

        self.selected = {}

        fp = open(filename, 'rb')
        if filename.endswith('.exe'):
            exe = ExecutableData(ByteReader(fp))
            game = exe.gameData
        elif filename.endswith('.ccn'):
            game = GameData(ByteReader(fp))
        else:
            raise NotImplementedError('invalid extension')

        self.game = game
        
        # shutil.rmtree(outdir, ignore_errors = True)
        copytree(os.path.join(os.getcwd(), 'base'), outdir)
        
        # fonts
        if WRITE_FONTS:
            fonts_file = self.open_code('fonts.h')
            fonts_file.putln('#include "common.h"')
            fonts_file.putln('')
            all_fonts = []
            if game.fonts:
                for font in game.fonts.items:
                    logfont = font.value
                    font_name = 'font%s' % font.handle
                    all_fonts.append(font_name)
                    fonts_file.putln(to_c('Font %s = Font(%r, %s, %s, %s, %s);',
                        font_name, logfont.faceName, logfont.getSize(), 
                        logfont.isBold(), bool(logfont.italic), 
                        bool(logfont.underline)))
                
            fonts_file.putln('')
            fonts_file.close()
        
        # images

        if WRITE_IMAGES:
            images_file = self.open_code('images.h')
            images_file.putln('#include "common.h"')
            images_file.putln('')
            images_file.start_guard('IMAGES_H')
            all_images = []
            if game.images:
                for image in game.images.items:
                    handle = image.handle
                    image_name = 'image%s' % handle
                    all_images.append(image_name)
                    pil_image = Image.fromstring('RGBA', (image.width, 
                        image.height), image.getImageData())
                    pil_image.save(self.get_filename('images', 
                        '%s.png' % handle))
                    images_file.putln(to_c(
                        'static Image %s(%r, %s, %s, %s, %s);',
                        image_name, str(handle), image.xHotspot, 
                        image.yHotspot, image.actionX, image.actionY))
            
            images_file.close_guard('IMAGES_H')
            images_file.putln('')
            images_file.close()
        
        # sounds
        if WRITE_SOUNDS:
            if game.sounds:
                for sound in game.sounds.items:
                    self.open('sounds', '%s.wav' % sound.name).write(
                        str(sound.get_wav()))
        
        # objects
        objects_file = self.open_code('objects.h')
        objects_file.putln('#include "common.h"')
        objects_file.putln('#include "images.h"')
        objects_file.putln('#include "fonts.h"')
        # objects_file.putln('#include "sounds.h"')
        objects_file.putln('')
        
        self.object_names = {}
        self.instance_names = {}

        type_id = itertools.count()
        
        for frameitem in game.frameItems.items:
            name = frameitem.name
            handle = frameitem.handle
            if name is None:
                class_name = 'Object%s' % handle
            else:
                class_name = get_class_name(name) + '_' + str(handle)
            common = frameitem.properties.loader
            object_type = frameitem.properties.getType()
            if object_type == EXTENSION_BASE:
                object_key = self.get_extension(frameitem).name
            else:
                object_key = object_type
            try:
                subclass = OBJECT_CLASSES[object_key]
            except KeyError:
                print repr(frameitem.name), object_key, handle
                continue
            if subclass is None:
                continue

            try:
                visible = common.newFlags['VisibleAtStart']
            except (AttributeError, KeyError):
                visible = True

            try:
                movement = common.movements.items[0]
                movement_type = movement.getName()
                if movement_type == 'Static':
                    raise StopIteration
                movement_name = '%sMovement' % class_name
                objects_file.putclass(movement_name, movement_type)
                move_loader = movement.loader
                objects_file.putdef('move_at_start', 
                    bool(movement.movingAtStart))
                if movement_type == 'Ball':
                    objects_file.putdef('start_speed', move_loader.speed)
                    objects_file.putdef('randomizer', move_loader.randomizer)
                    objects_file.putdef('angles', 2 ** (move_loader.angles + 3))
                    objects_file.putdef('security', move_loader.security)
                    objects_file.putdef('deceleration', 
                        ACCELERATORS[move_loader.deceleration])
                elif movement_type == 'Path':
                    objects_file.putdef('min_speed', move_loader.minimumSpeed)
                    objects_file.putdef('max_speed', move_loader.maximumSpeed)
                    objects_file.putdef('loop', bool(move_loader.loop))
                    objects_file.putdef('reposition', 
                        bool(move_loader.repositionAtEnd))
                    objects_file.putdef('reverse', 
                        bool(move_loader.reverseAtEnd))
                    objects_file.putln('steps = [')
                    objects_file.indent()
                    for step in move_loader.steps:
                        details = {
                            'x' : step.destinationX,
                            'y' : step.destinationY,
                            'pause' : step.pause,
                            'name' : step.name
                        }
                        objects_file.putln('%s,' % details)
                    objects_file.dedent()
                    objects_file.putln(']')
                elif movement_type == 'Mouse':
                    objects_file.putdef('player', movement.player - 1)
                    objects_file.putdef('x1', move_loader.x1)
                    objects_file.putdef('y1', move_loader.y1)
                    objects_file.putdef('x2', move_loader.x2)
                    objects_file.putdef('y2', move_loader.y2)
                objects_file.dedent()
                objects_file.putln('')
            except (AttributeError, IndexError, StopIteration):
                movement_name = None

            self.object_names[handle] = class_name
            self.instance_names[handle] = get_method_name(class_name)
            objects_file.putclass(class_name, subclass)
            objects_file.put_access('public')
            objects_file.putln('static const int type_id = %s;' % type_id.next())
            objects_file.putln(to_c('%s(int x, int y) : %s(%r, x, y, type_id)',
                class_name, subclass, name))
            objects_file.start_brace()
            objects_file.end_brace()
            if movement_name is not None:
                objects_file.putln('movement_class = %s' % movement_name)
            if frameitem.flags['Global']:
                objects_file.putdef('is_global', True)
            if object_type == TEXT:
                text = common.text
                lines = [paragraph.value for paragraph in text.items]
                if len(lines) != 1:
                    raise NotImplementedError(
                        'text can only be 1 paragraph')
                objects_file.putdef('width', text.width)
                objects_file.putdef('height', text.height)
                objects_file.putln('font = font%s' % text.items[0].font)
                objects_file.putdef('color', text.items[0].color)
                objects_file.putdef('text', lines[0])
                
                paragraph = text.items[0]
                if paragraph.flags['HorizontalCenter']:
                    horizontal = 'Qt.AlignHCenter'
                elif paragraph.flags['RightAligned']:
                    horizontal = 'Qt.AlignRight'
                else:
                    horizontal = 'Qt.AlignLeft'
                if paragraph.flags['VerticalCenter']:
                    vertical = 'Qt.AlignVCenter'
                elif paragraph.flags['BottomAligned']:
                    vertical = 'Qt.AlignBottom'
                else:
                    vertical = 'Qt.AlignTop'
                objects_file.putln('alignment = %s | %s' % (horizontal,
                    vertical))
                    
            elif object_type == ACTIVE:
                # self.write_active(objects_file, common)
                pass
            elif object_type == BACKDROP:
                objects_file.putdef('obstacle_type', 
                    common.getObstacleType())
                objects_file.putdef('collision_mode', 
                    common.getCollisionMode())
                objects_file.putln('image = image%s' % common.image)
            elif object_type == COUNTER:
                counters = common.counters
                counter = common.counter
                if counters:
                    display_type = counters.displayType
                    if display_type == NUMBERS:
                        objects_file.putln('frames = {')
                        objects_file.indent()
                        for char_index, char in enumerate(COUNTER_FRAMES):
                            objects_file.putln("%r : image%s," % (char,
                                counters.frames[char_index]))
                        objects_file.dedent()
                        objects_file.putln('}')
                    elif display_type == HORIZONTAL_BAR:
                        shape_object = counters.shape
                        shape = shape_object.shape
                        fill_type = shape_object.fillType
                        if shape != RECTANGLE_SHAPE:
                            raise NotImplementedError
                        objects_file.putdef('width', counters.width)
                        objects_file.putdef('height', counters.height)
                        if fill_type == GRADIENT_FILL:
                            objects_file.putdef('color1', shape_object.color1)
                            objects_file.putdef('color2', shape_object.color2)
                        elif fill_type == SOLID_FILL:
                            objects_file.putdef('color1', shape_object.color1)
                        else:
                            raise NotImplementedError
                    else:
                        raise NotImplementedError
                objects_file.putdef('initial', counter.initial)
                objects_file.putdef('minimum', counter.minimum)
                objects_file.putdef('maximum', counter.maximum)
            elif object_type == SUBAPPLICATION:
                subapp = common.subApplication
                objects_file.putdef('width', subapp.width)
                objects_file.putdef('height', subapp.height)
                objects_file.putdef('start_frame', subapp.startFrame)
                # self.options.setFlags(reader.readInt(True))
            elif object_type == EXTENSION_BASE:
                extension = self.get_extension(frameitem)
                extension_name = extension.name
                if common.extensionData is not None:
                    data = ByteReader(common.extensionData)
                if extension_name == 'kcpica':
                    objects_file.putdef('width', data.readInt())
                    objects_file.putdef('height', data.readInt())
                    active_picture_flags.setFlags(data.readInt(True))
                    visible = not active_picture_flags['HideOnStart']
                    transparent_color = data.readColor()
                    image = data.readString(260) or None
                    objects_file.putdef('filename', image)
                elif extension_name == 'kcini':
                    data.skipBytes(2)
                    filename = data.readString()
                    if filename != 'Name.ini':
                        objects_file.putdef('filename', filename)
                elif extension_name == 'kcedit':
                    objects_file.putdef('width', data.readShort(True))
                    objects_file.putdef('height', data.readShort(True))
                    objects_file.putfont(LogFont(data, old = True))
                    data.skipBytes(4 * 16)
                    objects_file.putdef('foreground', data.readColor())
                    background = data.readColor()
                    data.skipBytes(40)
                    edit_flags.setFlags(data.readInt())
                    visible = not edit_flags['HideOnStart']
                    transparent = edit_flags['Transparent']
                    if transparent:
                        background = None
                    objects_file.putdef('background', background)
                    objects_file.putdef('transparent', transparent)
                    objects_file.putdef('border', edit_flags['Border'])
                elif extension_name == 'kclist':
                    objects_file.putdef('width', data.readShort())
                    objects_file.putdef('height', data.readShort())
                    objects_file.putfont(LogFont(data, old = True))
                    objects_file.putdef('font_color', data.readColor())
                    data.skipBytes(40 + 16 * 4)
                    objects_file.putdef('background', data.readColor())
                    list_flags.setFlags(data.readInt())
                    visible = not list_flags['HideOnStart']
                    line_count = data.readShort(True)
                    index_offset = -1 if data.readInt() == 1 else 0
                    objects_file.putdef('index_offset', index_offset)
                    data.skipBytes(4 * 3)
                    objects_file.putln('lines = [')
                    objects_file.indent()
                    for i in xrange(line_count):
                        if i == line_count - 1:
                            objects_file.putln('%r' % data.readString())
                        else:
                            objects_file.putln('%r,' % data.readString())
                    objects_file.dedent()
                    objects_file.putln(']')
                elif extension_name == 'iconlist':
                    data.skipBytes(4) # version?
                    objects_file.putdef('width', data.readShort(True))
                    objects_file.putdef('height', data.readShort(True))
                    image16 = data.readShort(True)
                    image32 = data.readShort(True)
                    data.skipBytes(4)
                    list_type = data.readInt(True) # 1: simple, 2: combo
                    if list_type == 1:
                        type_name = 'Simple'
                    elif list_type == 2:
                        type_name = 'Combo'
                    else:
                        raise NotImplementedError('invalid icon list type')
                    objects_file.putdef('list_type', type_name)
                    icon_size = int(data.readInt(True))
                    objects_file.putdef('icon_size', icon_size)
                    if icon_size == 16:
                        objects_file.putln('image = image%s' % image16)
                    elif icon_size == 32:
                        objects_file.putln('image = image%s' % image32)
                    data.skipBytes(4)
                    focus = bool(data.readInt(True))
                    invisible = bool(data.readInt(True))
                    data.skipBytes(24)
                    font = LogFont(data)
                    if not font.faceName:
                        font = None
                    objects_file.putfont(font)
                    data.skipBytes(52)
                    objects_file.putdef('font_color', data.readColor())
                elif extension_name == 'kcriched':
                    objects_file.putdef('width', data.readShort(True))
                    objects_file.putdef('height', data.readShort(True))
                    rich_edit_flags.setFlags(data.readInt(True))
                    string_unknown = data.readString(260)
                    visible = rich_edit_flags['Visible']
                    undo_levels = data.readInt(True)
                    unknown_integer = data.readInt(True)
                    string_size = data.readInt(True)
                    objects_file.putfont(LogFont(data))
                    data.skipBytes(64)
                    objects_file.putdef('foreground', data.readColor())
                    objects_file.putdef('background', data.readColor())
                    objects_file.putdef('read_only', 
                        rich_edit_flags['ReadOnly'])
                    string_unknown2 = data.readString(40)
                    if rich_edit_flags['Internal']:
                        objects_file.putdef('text', data.readString(
                            string_size))
                elif extension_name == 'kcbutton':
                    # data.openEditor()
                    objects_file.putdef('width', data.readShort(True))
                    objects_file.putdef('height', data.readShort(True))
                    button_type = data.readShort()
                    objects_file.putdef('type', button_type_names[button_type])
                    button_count = data.readShort()
                    button_flags.setFlags(data.readShort())
                    visible = not button_flags['HideOnStart']
                    objects_file.putfont(LogFont(data, old = True))
                    foreground = data.readColor()
                    background = data.readColor()
                    if button_flags['SystemColor']:
                        background = (255, 255, 255)
                    objects_file.putdef('foreground', foreground)
                    objects_file.putdef('background', background)
                    data.skipBytes(104)
                    images = [data.readShort() for _ in xrange(3)]
                    if button_type in (PUSHBITMAP_BUTTON, PUSHTEXTBITMAP_BUTTON):
                        images = [('image%s' % item) if item != -1 else 'None'
                                  for item in images]
                        objects_file.putln('images = [%s]' % ', '.join(images))
                    data.skipBytes(10)
                    strings = []
                    if button_type == RADIO_BUTTON:
                        strings = None
                        # strings = [data.readString() 
                        #     for _ in xrange(button_count)]
                    else:
                        strings.append(data.readString())
                    if strings:
                        objects_file.putdef('strings', strings)
                elif extension_name == 'KcBoxA':
                    objects_file.putdef('width', data.readShort(True))
                    objects_file.putdef('height', data.readShort(True))
                    active_box_flags.setFlags(data.readInt(True))
                    if not active_box_flags['Button']:
                        objects_file.putdef('disabled', True)
                    fill = read_system_color(data)
                    objects_file.putdef('fill', fill)
                    border1 = read_system_color(data)
                    border2 = read_system_color(data)
                    image = data.readShort()
                    margin_left = data.readShort()
                    margin_top = data.readShort()
                    margin_right = data.readShort()
                    margin_bottom = data.readShort()
                    data.skipBytes(6)
                    font = LogFont(data, old = True)
                    objects_file.putfont(font)
                    data.skipBytes(42)
                    text, = data.readReader(data.readInt(True)).readString(
                        ).rsplit('\\n', 1)
                    if text:
                        objects_file.putdef('text', text)
                    version = data.readInt()
                    hyperlink_color = read_system_color(data)
                elif extension_name == 'txtblt':
                    data.skipBytes(4)
                    objects_file.putdef('width', data.readShort(True))
                    objects_file.putdef('height', data.readShort(True))
                    data.seek(1424)
                    char_width = data.readInt(True)
                    char_height = data.readInt(True)
                    objects_file.putdef('character_size', 
                        (char_width, char_height))
                    data.seek(1468)
                    image = data.readShort(True)
                    objects_file.putln('image = image%s' % image)
                    character_map = data.readString()
                    if character_map != DEFAULT_CHARMAP:
                        raise NotImplementedError

            if not visible:
                objects_file.putdef('visible', visible)
            
            objects_file.end_brace(True)
            objects_file.putln('static ObjectList %s_instances;' % 
                get_method_name(class_name))

            objects_file.putln('')
        objects_file.close()
        
        processed_frames = []
        
        # frames
        for i, frame in enumerate(game.frames):
            frame.load()
            self.current_frame = frame
            processed_frames.append(i+1)
            frame_file = self.open_code('frame%s.h' % (i + 1))
            frame_file.putln('#include "common.h"')
            frame_file.putln('#include "objects.h"')
            frame_file.putln('')
            
            startup_instances = []
            self.multiple_instances = set()
            startup_set = set()
            
            for instance in frame.instances.items:
                frameitem = instance.getObjectInfo(game.frameItems)
                if frameitem.handle not in self.object_names:
                    continue
                common = frameitem.properties.loader
                try:
                    create_startup = not common.flags['DoNotCreateAtStart']
                except AttributeError:
                    create_startup = True
                if create_startup and instance.parentType == NONE_PARENT:
                    if frameitem in startup_set:
                        self.multiple_instances.add(frameitem.handle)
                    startup_set.add(frameitem)
                    startup_instances.append((instance, frameitem))

            events = frame.events

            self.qualifiers = {}
            for qualifier in events.qualifiers.values():
                object_infos = qualifier.resolve_objects(self.game.frameItems)
                object_names = [self.object_names[item] 
                    for item in object_infos]
                frame_file.putln('qualifier_%s = (%s)' % (qualifier.qualifier,
                    ', '.join(object_names)), wrap = True)
                frame_file.putln('')
                self.qualifiers[qualifier.qualifier] = object_infos
            
            class_name = 'Frame%s' % (i+1)
            frame_file.putclass(class_name, 'Frame')
            frame_file.put_access('public')
            frame_file.putln(to_c('%s(GameManager * manager) : '
                'Frame(%r, %s, %s, %s, manager)',
                class_name, frame.name, frame.width, frame.height, i))
            # frame_file.putln(to_c('%s::name = %r;', class_name, frame.name))
            # frame_file.putln(to_c('%s::width = %r;', class_name, frame.width))
            # frame_file.putln(to_c('%s::height = %r;', class_name, frame.height))
            # frame_file.putln(to_c('%s::index = %r;', class_name, i))
            frame_file.start_brace()
            frame_file.end_brace()
            # frame_file.putdef('name', frame.name)
            # frame_file.putdef('index', i)
            # frame_file.putdef('width', frame.width)
            # frame_file.putdef('height', frame.height)
            # frame_file.putdef('background', frame.background)

            generated_groups = defaultdict(list)
            always_groups = []
            always_groups_dict = defaultdict(list)
            action_dict = defaultdict(list)
            current_container = None
            self.containers = containers = {}
            changed_containers = set()
            for group_index, group in enumerate(events.items):
                first_condition = group.conditions[0]
                name = self.get_condition_name(first_condition)
                if name == 'NewGroup':
                    group_loader = first_condition.items[0].loader
                    current_container = EventContainer(group_loader.name,
                        group_loader.flags['Inactive'])
                    containers[group_loader.offset+2] = current_container
                    continue
                elif name == 'GroupEnd':
                    always_container = current_container.always_groups
                    if always_container:
                        always_container[0].container_start = True
                        always_container[-1].container_end = True
                    current_container = None
                    continue
                new_group = EventGroup(group.conditions[:], group.actions[:],
                    current_container)
                new_group.global_id = group_index+1
                for action in new_group.actions[:]:
                    action_name = self.get_action_name(action)
                    if action_name in ('CreateObject', 'Shoot', 'DisplayText'):
                        loader = action.items[0].loader
                        try:
                            create_info = self.current_frame.instances.fromHandle(
                                loader.objectInstance).objectInfo
                        except ValueError:
                            create_info = loader.objectInfo
                        if create_info == 0xFFFF:
                            # the case for DisplayText
                            create_info = action.objectInfo
                        loader.objectInfo = create_info
                        self.multiple_instances.add(create_info)
                    elif action_name in ('StopLoop', 'StopCurrentFuncLoop',
                                         'ReturnFunction'):
                        new_group.actions.remove(action)
                        new_group.actions.append(action)
                    elif action_name in ('DeactivateGroup', 'ActivateGroup'):
                        pointer = action.items[0].loader.pointer
                        changed_containers.add(pointer)
                    action_dict[action_name].append(action)
                is_always = first_condition.flags['Always']
                if name in ('OnCollision',):
                    is_always = True
                if is_always:
                    if current_container:
                        current_container.always_groups.append(new_group)
                    always_groups.append(new_group)
                    new_group.local_id = len(always_groups_dict[name])+1
                    always_groups_dict[name].append(new_group)
                else:
                    new_group.generated = True
                    new_group.local_id = len(generated_groups[name])+1
                    generated_groups[name].append(new_group)

            for k, v in containers.iteritems():
                if k not in changed_containers:
                    if v.inactive:
                        raise NotImplementedError
                    v.is_static = True

            frame_file.putmeth('void initialize')
                    
            timer_equals = generated_groups.pop('TimerEquals', [])
            for timer_group in timer_equals:
                seconds = timer_group.conditions[0].items[0].loader.timer / 1000.0
                frame_file.putln('self.add_timed_call(self.on_timer_%s, %s)' % (
                    (timer_group.local_id), seconds))
                    
            # frame_file.putln('groups = {')
            # frame_file.indent()
            # for container in containers.values():
            #     frame_file.putln('%r : %r,' % (container.name, 
            #         not container.inactive))
            # frame_file.dedent()
            # frame_file.putln('}')
            
            frame_file.end_brace()
            
            start_groups = generated_groups.pop('StartOfFrame', None)
            frame_file.putmeth('void on_start')
            for instance, frameitem in startup_instances:
                frame_file.putln('add_object(new %s(%s, %s));' %
                    (self.object_names[frameitem.handle], instance.x, 
                    instance.y))
            if start_groups:
                for group in start_groups:
                    self.write_event(frame_file, group, True)
            frame_file.end_brace()
            
            for group in timer_equals:
                frame_file.putmeth('on_timer_%s' % (group.local_id))
                self.write_event(frame_file, group, True)
                frame_file.putend()

            # loops

            loops = defaultdict(list)
            for loop_group in generated_groups.pop('OnLoop', []):
                exp = loop_group.conditions[0].items[0].loader.items[0]
                loop_name = get_method_name(exp.loader.value)
                loops[loop_name].append(loop_group)

            for loop_name, groups in loops.iteritems():
                frame_file.putmeth('bool loop_%s' % loop_name)
                for group in groups:
                    self.write_event(frame_file, group, True)
                frame_file.putln('return true;')
                frame_file.end_brace()

            # func loops

            func_loops = defaultdict(list)
            for loop_group in generated_groups.pop('OnFuncLoop', []):
                exp = loop_group.conditions[0].items[0].loader.items[0]
                loop_name = get_method_name(exp.loader.value)
                func_loops[loop_name].append(loop_group)

            for loop_name, groups in func_loops.iteritems():
                frame_file.putmeth('loop_%s' % loop_name, 'loop_index')
                for group in groups:
                    self.write_event(frame_file, group, True)
                frame_file.putln('return true;')
                frame_file.end_brace()

            # functions

            functions = defaultdict(list)
            for loop_group in generated_groups.pop('OnFunction', []):
                exp = loop_group.conditions[0].items[0].loader.items[0]
                function_name = get_method_name(exp.loader.value)
                functions[function_name].append(loop_group)

            for function_name, groups in functions.iteritems():
                frame_file.putmeth('function_%s' % function_name, 
                    'int_arg = None')
                for group in groups:
                    self.write_event(frame_file, group, True)
                frame_file.putend()
            
            # active
            anim_groups = generated_groups.pop('AnimationFinished', [])
            if anim_groups:
                frame_file.putmeth('on_animation_end', 'instance')
                for group in anim_groups:
                    first_condition = group.conditions[0]
                    object_info = first_condition.objectInfo
                    anim_index = first_condition.items[0].loader.value
                    animation_name = ANIMATION_NAMES[anim_index]
                    frame_file.putln('if type(instance) == %s '
                                     'and instance.animation_value == %r:' %
                        (self.get_object_name(object_info), animation_name))
                    frame_file.indent()
                    self.write_event(frame_file, group, True)
                    frame_file.dedent()
                frame_file.putend()

            # mouse

            object_clicked = defaultdict(list)
            for group in generated_groups.pop('ObjectClicked', []):
                button = self.convert_parameter(group.conditions[0].items[0])
                object_info = group.conditions[0].items[1].loader.objectInfo
                object_clicked[object_info].append(group)

            mouse_clicked = defaultdict(list)
            for group in generated_groups.pop('MouseClicked', []):
                button = self.convert_parameter(group.conditions[0].items[0])
                mouse_clicked[button].append(group)
            
            if object_clicked or mouse_clicked:
                frame_file.putmeth('on_mouse_press', 'x', 'y', 'button')
                for object_info, groups in object_clicked.iteritems():
                    frame_file.putln('if %s.is_over(x, y):' % (
                        self.get_object(object_info)))
                    frame_file.indent()
                    for group in groups:
                        self.write_event(frame_file, group, True)
                    frame_file.dedent()
                for button, groups in mouse_clicked.iteritems():
                    frame_file.putln('if button == %s:' % button)
                    frame_file.indent()
                    for group in groups:
                        self.write_event(frame_file, group, True)
                    frame_file.dedent()
                frame_file.putend()
            
            def write_object_handler(group_name, meth_name):
                handler_groups = generated_groups.pop(group_name, [])
                if handler_groups:
                    frame_file.putmeth(meth_name, 'instance')
                    for group in handler_groups:
                        object_info = group.conditions[0].objectInfo
                        frame_file.putln('if type(instance) == %s:' %
                            self.get_object_name(object_info))
                        frame_file.indent()
                        self.write_event(frame_file, group, True)
                        frame_file.dedent()
                    frame_file.putend()

            def write_handler(group_name, meth_name, *arg):
                handler_groups = generated_groups.pop(group_name, [])
                if handler_groups:
                    frame_file.putmeth(meth_name, *arg)
                    for group in handler_groups:
                        self.write_event(frame_file, group)
                    frame_file.putend()

            # callbacks

            write_object_handler('ButtonClicked', 'on_button_click')
            write_object_handler('BoxLeftClicked', 'on_box_click')
            write_object_handler('SockReceived', 'on_sock_receive')
            write_object_handler('SockConnected', 'on_sock_connect')
            write_object_handler('SockDisconnected', 'on_sock_disconnect')
            write_object_handler('SockCanAccept', 'on_sock_connection')
            write_object_handler('ListSelectionChanged', 'on_list_selection')
            write_handler('PlayerKeyPressed', 'on_player_press')
            
            if generated_groups:
                print 'unimplemented generated groups in %r: %s' % (
                    frame.name, generated_groups.keys())

            for name, group_list in generated_groups.iteritems():
                meth_name = 'on_%s' % name
                frame_file.putmeth(meth_name, 'instance')
                for group in group_list:
                    first_condition = group.conditions[0]
                    try:
                        if first_condition.hasObjectInfo():
                            object_info = first_condition.objectInfo
                            object_name = self.get_object_name(object_info)
                            frame_file.putln('if type(instance) == %s:' %
                                object_name)
                            frame_file.indent()
                            self.write_event(frame_file, group)
                            frame_file.dedent()
                            continue
                    except KeyError:
                        pass
                    self.write_event(frame_file, group)
                frame_file.putend()
            
            frame_file.putmeth('void handle_events')
            if always_groups:
                for group in always_groups:
                    self.write_event(frame_file, group)
            frame_file.end_brace()
            
            frame_file.end_brace(True) # end of frame

        # general configuration
        if WRITE_CONFIG:
            header = game.header
            config_file = self.open_code('config.h')
            config_file.putln('#include "common.h"')
            frame_classes = []
            for frame_index in processed_frames:
                frame_class_name = 'Frame%s' % frame_index
                frame_classes.append(frame_class_name)
                config_file.putln('#include "frame%s.h"' % frame_index)
            config_file.putln('')
            config_file.putdefine('NAME', game.name)
            config_file.putdefine('COPYRIGHT', game.copyright)
            config_file.putdefine('ABOUT', game.aboutText)
            config_file.putdefine('AUTHOR', game.author)
            config_file.putdefine('WINDOW_WIDTH', header.windowWidth)
            config_file.putdefine('WINDOW_HEIGHT', header.windowHeight)
            # config_file.putdef('BORDER_COLOR', header.borderColor)
            config_file.putdefine('START_LIVES', header.initialLives)
            config_file.putln('')
            config_file.putln('static Frame ** frames = NULL;')
            config_file.putmeth('Frame ** get_frames', 'GameManager * manager')
            config_file.putln('if (frames) return frames;')

            config_file.putln('frames = new Frame*[%s];' % len(frame_classes))

            for i, frame in enumerate(frame_classes):
                config_file.putln('frames[%s] = new %s(manager);' % (i, frame))

            config_file.putln('return frames;')

            config_file.end_brace()
            config_file.close()

        fp.close()

        # post-mortem stats
        print ''
        print 'stats:'
        print 'ACTIONS'
        print self.checked_aces['actions'].most_common()
        print ''
        print 'CONDITIONS'
        print self.checked_aces['conditions'].most_common()
        print ''
        print 'EXPRESSIONS'
        print self.checked_aces['expressions'].most_common()
    
    def write_event(self, outwriter, group, triggered = False):
        self.current_event_id = group.global_id
        self.has_selection = {}
        actions, conditions = group.actions, group.conditions
        container = group.container
        has_container_check = bool(container) and not container.is_static
        if triggered:
            conditions = conditions[1:]
        writer = CodeWriter()
        writer.putln('// event %s' % group.global_id)
        event_break = 'goto event_%s_end;' % group.global_id
        collisions = defaultdict(list)
        if conditions or has_container_check:
            names = [self.get_condition_name(condition)
                for condition in conditions]
            if has_container_check:
                writer.putln('if not self.groups[%r]: %s' % 
                    container.name, event_break)
            elif container:
                writer.putln('// group: %s' % container.name)
            for condition_index, condition in enumerate(conditions):
                condition_name = names[condition_index]
                if condition_name == 'Always':
                    continue
                negated = not condition.otherFlags['Not']
                parameters = condition.items
                try:
                    comparison = COMPARISONS[parameters[-1].loader.comparison]
                except (IndexError, AttributeError):
                    pass
                object_name = None
                has_instance = False
                has_multiple = False
                try:
                    if condition.hasObjectInfo():
                        object_info = condition.objectInfo
                        object_name = self.get_object(object_info)
                except KeyError:
                    pass
                if condition_name in ('IsOverlapping', 'OnCollision'):
                    real_neg = not negated
                    other_info = parameters[0].loader.objectInfo
                    selected_name = '%s_instances' % get_method_name(
                        self.get_object_name(object_info))
                    other_selected = '%s_instances' % get_method_name(
                        self.get_object_name(other_info))
                    if real_neg:
                        prefix = ''
                    else:
                        prefix = '%s, %s = ' % (selected_name, other_selected)
                    writer.putln('%sself.check_overlap('
                                 '%s, %s, negated = %s)' % (prefix,
                        self.get_object(object_info, True), 
                        self.get_object(other_info, True), not real_neg))
                    if not real_neg:
                        self.has_selection[object_info] = selected_name
                        self.has_selection[other_info] = other_selected
                    continue
                if object_name is not None:
                    if condition_name in ('NumberOfObjects',):
                        pass
                    elif self.has_multiple_instances(object_info):
                        has_multiple = True
                        selected_name = '%s_instances' % get_method_name(
                            self.get_object_name(object_info))
                        if object_info not in self.has_selection:
                            make_dict = self.get_object(object_info, True)
                        else:
                            make_dict = None
                        self.has_selection[object_info] = selected_name
                if has_multiple:
                    if make_dict is not None:
                        writer.putln('%s = %s;' % (selected_name, 
                            make_dict))
                    self.iterated_object = object_info
                    writer.putln('item = %s.begin();' %
                        selected_name)
                    writer.putln('while (item != %s.end()) {' % 
                        selected_name)
                    writer.indent()
                    object_name = '(*item)'
                writer.putindent()
                if negated:
                    writer.put('if (!(')
                else:
                    writer.put('if (')
                if condition_name == 'PickRandom':
                    object_name = None
                if object_name is not None:
                    if condition_name == 'NumberOfObjects':
                        writer.put('len(%s)' % (
                            self.get_object(condition.objectInfo, True)
                        ))
                    else:
                        if condition_name == 'ObjectInvisible':
                            writer.put('not ')
                        writer.put('%s->' % object_name)
                if condition_name == 'FileReadable':
                    exp = self.convert_parameter(parameters[0])
                    writer.put('os.path.isfile(%s)' % exp)
                elif condition_name == 'Compare':
                    a = self.convert_parameter(parameters[0])
                    b = self.convert_parameter(parameters[1])
                    writer.put('%s %s %s' % (a, comparison, b))
                elif condition_name == 'CompareGlobalValue':
                    a = self.convert_parameter(parameters[0])
                    b = self.convert_parameter(parameters[1])
                    writer.put('self.get_global_value(%s) %s %s' % (a, comparison,
                        b))
                elif condition_name == 'CompareSpeed':
                    b = self.convert_parameter(parameters[0])
                    writer.put('movement.speed %s %s' % (
                        comparison, b))
                elif condition_name == 'CompareY':
                    b = self.convert_parameter(parameters[0])
                    writer.put('y %s %s' % (
                        comparison, b))
                elif condition_name == 'CompareX':
                    b = self.convert_parameter(parameters[0])
                    writer.put('x %s %s' % (
                        comparison, b))
                elif condition_name == 'MovementStopped':
                    writer.put('movement.is_stopped()')
                elif condition_name == 'NumberOfObjects':
                    a = self.convert_parameter(parameters[0])
                    writer.put(' %s %s' % (comparison, a))
                elif condition_name == 'CompareAlterableValue':
                    a = self.convert_parameter(parameters[0])
                    b = self.convert_parameter(parameters[1])
                    writer.put('values->get(%s) %s %s' % (a, 
                        comparison, b))
                elif condition_name == 'KeyPressed':
                    key = parameters[0].loader.key.getValue()
                    qt_key = get_qt_key(key)
                    writer.put('%s in self.scene.key_presses' % qt_key)
                elif condition_name == 'KeyDown':
                    key = parameters[0].loader.key.getValue()
                    qt_key = get_qt_key(key)
                    writer.put('%s in self.scene.key_downs' % qt_key)
                elif condition_name == 'CompareCounter':
                    b = self.convert_parameter(parameters[0])
                    writer.put('get_value() %s %s' % (comparison, b))
                elif condition_name == 'Every':
                    writer.put('self.every(%s)' % 
                        self.convert_parameter(parameters[0]))
                elif condition_name == 'Always':
                    writer.put('True')
                elif condition_name == 'MouseOnObject':
                    objectInfo = parameters[0].loader.objectInfo
                    writer.put('%s.mouse_over()' % 
                        self.get_object(objectInfo))
                elif condition_name == 'ApplicationActive':
                    writer.put('self.is_active()')
                elif condition_name == 'NotAlways':
                    writer.put('self.not_always()')
                elif condition_name == 'IsWindowVisible':
                    writer.put('self.is_window_visible()')
                elif condition_name == 'EditHasFocus':
                    writer.put('has_focus()')
                elif condition_name in ('SubApplicationVisible', 
                                        'ObjectVisible', 'EditIsVisible'):
                    writer.put('visible')
                elif condition_name in ('ObjectInvisible',):
                    writer.put('visible')
                elif condition_name == 'SockIsConnected':
                    writer.put('is_connected()')
                elif condition_name == 'FlagOff':
                    writer.put('flags[%s] == False' % (
                        self.convert_parameter(parameters[0])))
                elif condition_name == 'FlagOn':
                    writer.put('flags[%s] == True' % (
                        self.convert_parameter(parameters[0])))
                elif condition_name == 'SockReceived':
                    writer.put('has_bytes()')
                elif condition_name == 'NumberOfLives':
                    writer.put('self.players[%s].lives %s %s' % (
                        condition.objectInfo, comparison,
                        self.convert_parameter(parameters[0])))
                elif condition_name == 'ModIsPlaying':
                    writer.put('self.is_mod_playing(%s)' % (
                        self.convert_parameter(parameters[0])))
                elif condition_name == 'GroupActivated':
                    name = self.containers[parameters[0].loader.pointer].name
                    writer.put('self.groups[%r]' % name)
                elif condition_name == 'Once':
                    writer.put('self.check_once()')
                elif condition_name == 'ButtonBoxChecked':
                    writer.put('get_value()')
                elif condition_name == 'EditIsNumber':
                    writer.put('is_number()')
                elif condition_name == 'EditModified':
                    writer.put('is_modified()')
                elif condition_name == 'NameIsFile':
                    writer.put('os.path.isfile(%s)' % (
                        self.convert_parameter(parameters[0])))
                elif condition_name == 'PickObjectsInZone':
                    writer.put('"self.pick_objects(zone = %s)"' % (
                        self.convert_parameter(parameters[0])))
                elif condition_name == 'MouseInZone':
                    writer.put('self.mouse_in_zone(%s)' % (
                        self.convert_parameter(parameters[0])))
                elif condition_name == 'PlayerKeyPressed':
                    key_flags.setFlags(parameters[0].loader.value)
                    keys = []
                    for k, v in key_flags.iteritems():
                        if v:
                            keys.append(k)
                    writer.put('self.players[%s].key_pressed(%r)' % (
                        condition.objectInfo, keys))
                elif condition_name == 'PlayerKeyDown':
                    key_flags.setFlags(parameters[0].loader.value)
                    keys = []
                    for k, v in key_flags.iteritems():
                        if v:
                            keys.append(k)
                    writer.put('self.players[%s].key_down(%r)' % (
                        condition.objectInfo, keys))
                elif condition_name == 'ControlAnyKeyDown':
                    writer.put('len(self.scene.key_downs) > 1')
                elif condition_name == 'WhileMousePressed':
                    key = parameters[0].loader.key.getValue()
                    qt_key = get_qt_key(key)
                    writer.putln('%s in self.scene.mouse_downs' % (
                        qt_key))
                elif condition_name == 'Never':
                    writer.put('False')
                elif condition_name == 'FacingInDirection':
                    writer.put('direction == %s' % (
                        self.get_direction(parameters[0])))
                elif condition_name == 'RestrictFor':
                    writer.put('self.restrict_for(%s)' % (
                        self.convert_parameter(parameters[0])))
                elif condition_name in ('IsOverlapping', 'OnCollision'):
                    other_info = parameters[0].loader.objectInfo
                    collisions[object_info].append(other_info)
                    writer.put('is_overlapping(%s)' % self.get_object(
                        other_info, True))
                elif condition_name == 'OutsidePlayfield':
                    writer.put('is_outside()')
                elif condition_name == 'InsidePlayfield':
                    writer.put('is_inside()')
                elif condition_name == 'LeavingPlayfield':
                    value = self.convert_parameter(parameters[0])
                    if value != 15:
                        # up down left right boop boop
                        raise NotImplementedError
                    writer.put('is_leaving()')
                elif condition_name == 'PickRandom':
                    writer.put('index == 0')
                else:
                    if condition_name not in self.checked_aces['conditions']:
                        print ('condition %r not implemented' % condition_name),
                        self.print_parameters(condition)
                        wait_unimplemented()
                    self.checked_aces['conditions'][condition_name] += 1
                    debug_parameters = [str(self.convert_parameter(parameter))
                        for parameter in parameters]
                    debug_name = '%s(%s)' % (condition_name, ', '.join(
                        debug_parameters))
                    writer.put(debug_name)
                if has_multiple:
                    if negated:
                        writer.put(')')
                    writer.put(') item = %s.erase(item);\n' % selected_name)
                    writer.putln('else ++item;')
                    writer.end_brace()
                    writer.indented = False
                    writer.putln('if (%s.empty()) %s' % (selected_name, 
                        event_break))
                    self.iterated_object = None
                else:
                    writer.put('): %s\n' % event_break)

        for action in actions:
            parameters = action.items
            action_name = self.get_action_name(action)
            has_multiple = False
            object_info = None
            if action.hasObjectInfo():
                object_info = action.objectInfo
                if self.has_multiple_instances(object_info):
                    has_multiple = True
                elif object_info not in self.object_names:
                    object_info = None
            if action_name in ('DisplayText', 'Shoot'):
                has_multiple = False
                object_info = None
            if has_multiple:
                if object_info in self.has_selection:
                    list_name = self.has_selection[object_info]
                else:
                    list_name = self.get_object(object_info, True)
                writer.putln('for (item = %s.begin(); item != %s.end(); '
                             'item++) {' % (list_name, list_name))

                writer.indent()
                writer.putindent()
                writer.put('(*item)->')
                self.iterated_object = object_info
            elif object_info is not None:
                writer.putindent()
                writer.put('%s.' % self.get_object(object_info))
            else:
                writer.putindent()
            if action_name == 'HideCursor':
                writer.put('self.hide_cursor()')
            elif action_name == 'ShowCursor':
                writer.put('self.show_cursor()')
            elif action_name == 'CreateFile':
                filename = self.convert_parameter(parameters[0])
                writer.put('open(%s, "wb").close()' % filename)
            elif action_name == 'AppendText':
                data = self.convert_parameter(parameters[0])
                filename = self.convert_parameter(parameters[1])
                writer.putln('fp = open(%s, "ab")' % filename,
                    indent = False)
                writer.putln('fp.write(%s)' % data,
                             'fp.close()')
            elif action_name == 'QuickLoadMod':
                filename = self.convert_parameter(parameters[0])
                cache = self.convert_parameter(parameters[1])
                track = self.convert_parameter(parameters[2])
                writer.put('self.load_mod(%s, %s, %s)' % (filename, cache,
                    track))
            elif action_name == 'NextFrame':
                writer.put('self.next_frame()')
            elif action_name == 'LimitTextSize':
                writer.put('limit_size(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SetFocusOn':
                writer.put('set_focus(True)')
            elif action_name == 'SetMinimumValue':
                writer.put('set_minimum(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SetMaximumValue':
                writer.put('set_maximum(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SetCounterValue':
                writer.put('set_value(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SubtractCounterValue':
                writer.put('subtract_value(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'AddCounterValue':
                writer.put('add_value(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SetINIFile':
                writer.put('set_filename(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SetINIGroup':
                writer.put('set_group(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SetINIGroupItemValue':
                writer.put('set_group_item_value(%s, %s, %s)' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1]),
                    self.convert_parameter(parameters[2])))
            elif action_name == 'SetINIItem':
                writer.put('set_item(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SetEditText':
                writer.put('set_value(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SetString':
                writer.put('set_value(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SetGlobalValue':
                writer.put('self.values[%s] = %s' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name == 'AddGlobalValue':
                writer.put('self.values[%s] += %s' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name == 'SetAlterableValue':
                writer.put('values->set(%s, %s);' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name == 'SubtractFromAlterable':
                writer.put('values[%s] -= %s' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name == 'AddToAlterable':
                writer.put('values[%s] += %s' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name == 'AddDelimiter':
                writer.put('add_delimiter(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'EditLoadFile':
                writer.put('load_file(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'AddEncryptionKey':
                writer.put('add_encryption_key(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SetLives':
                writer.put('self.players[%s].lives = %s' % (
                    action.objectInfo, self.convert_parameter(parameters[0])))
            elif action_name == 'IgnoreControls':
                writer.put('self.players[%s].set_ignore(True)' % (
                    action.objectInfo))
            elif action_name == 'RestoreControls':
                writer.put('self.players[%s].set_ignore(False)' % (
                    action.objectInfo))
            elif action_name == 'SetModuleVolume':
                writer.put('self.set_mod_volume(%s, %s)' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name == 'ModuleCrossFade':
                src = self.convert_parameter(parameters[0])
                dst = self.convert_parameter(parameters[1])
                duration = self.convert_parameter(parameters[2])
                writer.put('self.cross_fade_mod(%s, %s, %s)' % (
                    src, dst, duration))
            elif action_name == 'ModulePlay':
                src = self.convert_parameter(parameters[0])
                writer.put('self.play_mod(%s)' % src)
            elif action_name == 'EndApplication':
                writer.put('self.end_application()')
            elif action_name == 'SetSemiTransparency':
                a = self.convert_parameter(parameters[0])
                writer.put('set_transparency(%s)' % a)
            elif action_name == 'Disconnect':
                writer.put('disconnect()')
            elif action_name == 'Connect':
                writer.put('connect(%s, %s)' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name == 'DeactivateGroup':
                name = self.containers[parameters[0].loader.pointer].name
                writer.put('self.groups[%r] = False' % name)
            elif action_name == 'ActivateGroup':
                name = self.containers[parameters[0].loader.pointer].name
                writer.put('self.groups[%r] = True' % name)
            elif action_name == 'EditReadOnlyOff':
                writer.put('set_read_only(False)')
            elif action_name == 'EditReadOnlyOn':
                writer.put('set_read_only(True)')
            elif action_name == 'StartLoop':
                real_name = self.convert_parameter(parameters[0])
                name = get_method_name(real_name)
                times = self.convert_parameter(parameters[1])
                writer.putln('for (int i = 0; i < %s; i++) {' % times, 
                    indent = False)
                writer.indent()
                writer.putln('loop_indexes[%s] = i;' % real_name)
                writer.putln('if (!loop_%s()) %s' % (name, event_break))
                writer.end_brace()
            elif action_name == 'StartFuncLoop':
                real_name = self.convert_parameter(parameters[0])
                name = get_method_name(real_name)
                times = self.convert_parameter(parameters[1])
                writer.putln('for loop_index in xrange(%s):\n' % times,
                    indent = False)
                writer.indent()
                writer.put('if self.loop_%s(loop_index) == False: break' % (
                    name), True)
                writer.dedent()
            elif action_name == 'StopLoop':
                real_name = self.convert_parameter(parameters[0])
                writer.put('return False # %s' % real_name)
            elif action_name == 'StopCurrentFuncLoop':
                writer.put('return False')
            elif action_name == 'ReturnFunction':
                writer.put('return')
            elif action_name == 'ParserSetText':
                writer.put('set_value(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SetTextColor':
                writer.put('set_color(%s)' % (
                    self.convert_parameter(parameters[0]),))
            elif action_name == 'JumpToFrame':
                frame_parameter = parameters[0].loader
                if frame_parameter.isExpression:
                    value = '(%s - 1)' % self.convert_parameter(frame_parameter)
                else:
                    value = str(self.game.frameHandles[frame_parameter.value])
                writer.put('self.set_frame(%s)' % value)
            elif action_name == 'ForceAnimation':
                a = ANIMATION_NAMES[parameters[0].loader.value]
                writer.put('force_animation(%r)' % a)
            elif action_name == 'RestoreAnimation':
                writer.put('restore_animation()')
            elif action_name == 'BinaryInsertString':
                writer.put('insert(%s, %s)' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name == 'BinaryReplaceString':
                writer.put('replace(%s, %s)' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name == 'BinaryDecodeBase64':
                writer.put('decode_base64()')
            elif action_name == 'ResetDelimiters':
                writer.put('clear_delimiters()')
            elif action_name == 'SockSendText':
                writer.put('send_text(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'DisableFlag':
                writer.put('flags[%s] = False' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'RichGoCharacter':
                writer.put('go_to_character(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SetINIItemString':
                writer.put('set_item_string(%s, %s)' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name == 'SetINIItemValue':
                writer.put('set_item_value(%s, %s)' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name == 'ListReset':
                writer.put('reset()')
            elif action_name == 'ListAddLine':
                writer.put('add_line(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'BinaryClear':
                writer.put('clear()')
            elif action_name == 'RichSetFontColorString':
                color = eval(eval(self.convert_parameter(parameters[0])))
                writer.put('set_color(%r)' % (color,))
            elif action_name == 'RichSetText':
                writer.put('set_text(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'IconListReset':
                writer.put('reset()')
            elif action_name == 'IconListAddLine':
                writer.put('add_line(%s, %s)' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name == 'IconListSetLine':
                writer.put('set_line(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'ModuleFadeStop':
                writer.put('self.stop_mod(%s, %s) # with fade' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name == 'ModuleStop':
                writer.put('self.stop_mod(%s)' % 
                    self.convert_parameter(parameters[0]))
            elif action_name == 'IconListDehighlight':
                writer.put('set_focus(False)')
            elif action_name == 'EditScrollToEnd':
                writer.put('scroll_end()')
            elif action_name == 'ListScrollEnd':
                writer.put('scroll_end()')
            elif action_name == 'SockSendLine':
                writer.put('send_line(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'EnableFlag':
                writer.put('flags[%s] = True' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'DeleteLine':
                writer.put('delete_line(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'ListLoadFiles':
                writer.put('load_file_list(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'ListLoad':
                writer.put('load(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'EditDisable':
                writer.put('disable()')
            elif action_name == 'EncryptText':
                writer.put('encrypt_file(%s, %s)' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name == 'DecryptText':
                writer.put('decrypt_file(%s, %s)' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name == 'ButtonCheck':
                writer.put('set_value(True)')
            elif action_name == 'ListFocusOff':
                writer.put('set_focus(False)')
            elif action_name == 'EditEnable':
                writer.put('enable()')
            elif action_name == 'ButtonDisable':
                writer.put('disable()')
            elif action_name == 'ButtonEnable':
                writer.put('enable()')
            elif action_name == 'ListFocusOn':
                writer.put('set_focus(True)')
            elif action_name == 'SockAccept':
                writer.put('accept()')
            elif action_name in ('CallFunction', 'CallFunctionInt'):
                arguments = ''
                if len(parameters) > 1:
                    arguments = self.convert_parameter(parameters[1])
                real_name = self.convert_parameter(parameters[0])
                exp = parameters[0].loader.items
                if len(exp) != 2 or exp[0].getName() != 'String':
                    func_get = 'getattr(self, "function_" + %s)' % real_name
                else:
                    func_get = 'self.function_%s' % get_method_name(real_name)
                writer.put('%s(%s)' % (func_get, arguments))
            elif action_name == 'CalculateCRC':
                writer.put('calculate(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'RestartFrame':
                writer.put('self.restart_frame()')
            elif action_name == 'Destroy':
                writer.put('destroy()')
            elif action_name == 'SockSetTimeout':
                writer.put('set_timeout(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SoundSetDirectory':
                writer.put('self.audio.set_default_directory(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'ControlSetPlayerUp':
                num = parameters[0].loader.items[0].loader.value
                writer.put('self.players[%s].up = key_from_name(%s)' % (
                    num - 1,
                    self.convert_parameter(parameters[1])))
            elif action_name == 'ControlSetPlayerDown':
                num = parameters[0].loader.items[0].loader.value
                writer.put('self.players[%s].down = key_from_name(%s)' % (
                    num - 1,
                    self.convert_parameter(parameters[1])))
            elif action_name == 'ControlSetPlayerLeft':
                num = parameters[0].loader.items[0].loader.value
                writer.put('self.players[%s].left = key_from_name(%s)' % (
                    num - 1,
                    self.convert_parameter(parameters[1])))
            elif action_name == 'ControlSetPlayerRight':
                num = parameters[0].loader.items[0].loader.value
                writer.put('self.players[%s].right = key_from_name(%s)' % (
                    num - 1,
                    self.convert_parameter(parameters[1])))
            elif action_name == 'ControlSetPlayerFire1':
                num = parameters[0].loader.items[0].loader.value
                writer.put('self.players[%s].fire1 = key_from_name(%s)' % (
                    num - 1,
                    self.convert_parameter(parameters[1])))
            elif action_name == 'ControlSetPlayerFire2':
                num = parameters[0].loader.items[0].loader.value
                writer.put('self.players[%s].fire2 = key_from_name(%s)' % (
                    num - 1,
                    self.convert_parameter(parameters[1])))
            elif action_name == 'StopGlobalTimer':
                writer.put('self.stop_timer(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SetFocusOff':
                # kcedit
                writer.put('set_focus(False)')
            elif action_name == 'SetInkEffect':
                shorts = parameters[0].loader
                effect_name = repr(INK_EFFECTS[shorts.value1])
                if effect_name == 'Semitransparent':
                    parameters = (effect_name, repr(shorts.value1))
                else:
                    parameters = (effect_name,)
                writer.put('set_effect(%s)' % (
                    ', '.join(parameters)))
            elif action_name == 'SetDirection':
                writer.put('set_direction(%s)' % (
                    self.get_direction(parameters[0])))
            elif action_name == 'SetSpeed':
                writer.put('movement.set_speed(%s)' % (
                    self.get_direction(parameters[0])))
            elif action_name == 'SockListen':
                writer.put('listen(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'Stop':
                writer.put('stop()')
            elif action_name == 'Start':
                writer.put('movement.start()')
            elif action_name == 'SetMaximumValue':
                writer.put('set_maximum(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'DeleteFile':
                writer.put('os.remove(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'CaptureSetFilename':
                writer.put('self.capture_filename = %s' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'BoxSetText':
                writer.put('set_text(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SockSetProperty':
                writer.put('set_property(%s, %s)' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name == 'SockSelectSock':
                writer.put('select_sock(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SetGlobalString':
                writer.put('self.strings[%s] = %s' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name in ('Hide', 'HideSubApplication',
                                 'EditMakeInvisible'):
                writer.put('set_visible(False)')
            elif action_name in ('Show', 'ShowSubApplication',
                                 'EditMakeVisible'):
                writer.put('set_visible(True)')
            elif action_name == 'SetX':
                writer.put('set_x(%s);' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SetY':
                writer.put('set_y(%s);' % (
                    self.convert_parameter(parameters[0])))
            elif action_name in ('CreateObject', 'DisplayText', 'Shoot'):
                is_shoot = action_name == 'Shoot'
                details = self.convert_parameter(parameters[0])
                x = str(details['x'])
                y = str(details['y'])
                parent = details.get('parent', None)
                if parent and not is_shoot:
                    x = '%s.x + %s' % (parent, x)
                    y = '%s.y + %s' % (parent, y)
                    # arguments.append('parent = %s' % details['parent'])
                    # if details.get('use_action_point', False):
                    #     arguments.append('use_action = True')
                    # if details.get('set_direction', False):
                    #     arguments.append('use_direction = True')
                if is_shoot:
                    create_object = details['shoot_object']
                else:
                    create_object = details['create_object']
                arguments = [x, y]
                list_name = '%s_instances' % get_method_name(create_object)
                if is_shoot:
                    create_method = '%s->shoot' % self.get_object(
                        action.objectInfo)
                    arguments.append(str(details['shoot_speed']))
                else:
                    create_method = 'create_object'
                writer.put('%s = %s(new %s(%s)); // %s' % (
                    list_name, create_method, create_object, 
                    ', '.join(arguments), details))
                self.has_selection[parameters[0].loader.objectInfo] = list_name
                if action_name == 'DisplayText':
                    paragraph = parameters[1].loader.value
                    if paragraph != 0:
                        raise NotImplementedError
            elif action_name == 'OverlayClearRGB':
                writer.put('clear(%s, %s, %s)' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1]),
                    self.convert_parameter(parameters[2])))
            elif action_name == 'SetPosition':
                details = self.convert_parameter(parameters[0])
                x = str(details['x'])
                y = str(details['y'])
                parent = details.get('parent', None)
                if parent:
                    x = '%s.x + %s' % (parent, x)
                    y = '%s.y + %s' % (parent, y)
                    # arguments.append('parent = %s' % details['parent'])
                    # if details.get('use_action_point', False):
                    #     arguments.append('use_action = True')
                    # if details.get('set_direction', False):
                    #     arguments.append('use_direction = True')
                arguments = [x, y]
                writer.put('set_position(%s) # %s' % (
                    ', '.join(arguments), details))
            elif action_name == 'LookAt':
                details = self.convert_parameter(parameters[0])
                x = str(details['x'])
                y = str(details['y'])
                parent = details.get('parent', None)
                if parent:
                    x = '%s.x + %s' % (parent, x)
                    y = '%s.y + %s' % (parent, y)
                    # arguments.append('parent = %s' % details['parent'])
                    # if details.get('use_action_point', False):
                    #     arguments.append('use_action = True')
                    # if details.get('set_direction', False):
                    #     arguments.append('use_direction = True')
                arguments = [x, y]
                writer.put('look_at(%s) # %s' % (
                    ', '.join(arguments), details))
            elif action_name == 'AddBackdrop':
                writer.put('add_backdrop()')
            elif action_name == 'SortByAlterableDecreasing':
                writer.put('self.sort_by_alterable(%s, default = %s)' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name == 'OverlaySetTransparent':
                writer.put('set_transparency(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'SoundAutoPlay':
                writer.put('self.audio.play_sound(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'PlaySample':
                writer.put('self.audio.play_sound(%r)' % (
                    self.convert_parameter(parameters[0]) + '.wav'))
            elif action_name == 'SpreadValue':
                writer.put('values[%s] = index+%s' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            elif action_name == 'BlitChangeText':
                writer.put('set_value(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'BringToFront':
                writer.put('bring_to_front()')
            elif action_name == 'BringToBack':
                writer.put('bring_to_back()')
            elif action_name == 'SoundSetListenerPosition':
                x = self.convert_parameter(parameters[0])
                y = self.convert_parameter(parameters[1])
                writer.put('self.audio.set_listener(%s, %s)' % (x, y))
            elif action_name == 'SoundAutoPlayPosition':
                name = self.convert_parameter(parameters[0])
                x = self.convert_parameter(parameters[1])
                y = self.convert_parameter(parameters[2])
                writer.put('self.audio.play_sound(%s, pos = (%s, %s))' % (
                    name, x, y))
            elif action_name == 'Bounce':
                other_infos = collisions[object_info]
                if len(other_infos) > 1:
                    raise NotImplementedError
                if not other_infos:
                    other_info = ''
                else:
                    other_info = self.get_object(other_infos[0], True)
                writer.put('movement.bounce(%s)' % other_info)
            elif action_name == 'ActivePictureCreateBackdrop':
                writer.put('create_backdrop(%s)' % (
                    self.convert_parameter(parameters[0])))
            elif action_name == 'ListInsertLine':
                writer.put('insert_line(%s, %s)' % (
                    self.convert_parameter(parameters[0]),
                    self.convert_parameter(parameters[1])))
            else:
                if action_name not in self.checked_aces['actions']:
                    print ('action %r not implemented' % action_name),
                    self.print_parameters(action)
                    wait_unimplemented()
                self.checked_aces['actions'][action_name] += 1
                debug_parameters = [str(self.convert_parameter(parameter))
                    for parameter in parameters]
                debug_name = '%s(%s)' % (action_name, ', '.join(
                    debug_parameters))
                writer.put(debug_name)
            writer.put('\n')
            if has_multiple:
                writer.end_brace()
                self.iterated_object = None

        writer.putln('event_%s_end: ;' % group.global_id)

        outwriter.putcode(writer)
    
    def convert_parameter(self, container):
        loader = container.loader
        out = ''
        if loader.isExpression:
            for item_index, item in enumerate(loader.items[:-1]):
                item_name = self.get_expression_name(item)
                if item_name == 'ObjectCount':
                    out += 'len(%s)' % self.get_object(item.objectInfo, True)
                    continue
                elif item_name == 'FixedValue':
                    out += 'id(%s)' % self.get_object(item.objectInfo)
                    continue
                if item.hasObjectInfo():
                    try:
                        object_info = item.objectInfo
                        out += '%s->' % self.get_object(item.objectInfo)
                    except KeyError:
                        pass
                if item_name in ('String', 'Long', 'Double'):
                    out += to_c('%r', item.loader.value)
                elif item_name == 'CurrentText':
                    out += 'text'
                elif item_name == 'EditTextNumeric':
                    out += 'get_number()'
                elif item_name == 'EditGetText':
                    out += 'get_value()'
                elif item_name == 'Plus':
                    out += '+'
                elif item_name == 'Minus':
                    out += '-'
                elif item_name == 'Multiply':
                    out += '*'
                elif item_name == 'Virgule':
                    if out[-1] == '(':
                        out += ')'
                    out += ', '
                elif item_name == 'Parenthesis':
                    out += '('
                elif item_name == 'EndParenthesis':
                    out += ')'
                elif item_name == 'LeftString':
                    out += 'left_string('
                elif item_name == 'ApplicationDirectory':
                    prev_exp = self.get_expression_name(
                        loader.items[item_index-2])
                    if prev_exp == 'ApplicationDrive':
                        out += "(os.path.splitdrive(os.getcwd())[1]+'\\\\')"
                    else:
                        out += "(os.getcwd()+'\\\\')"
                elif item_name == 'ApplicationDrive':
                    out += "os.path.splitdrive(os.getcwd())[0]"
                elif item_name == 'GetINIString':
                    out += 'get()'
                elif item_name == 'GetINIValueItem':
                    out += 'get_value_item('
                elif item_name == 'GetINIStringItem':
                    out += 'get_string_item('
                elif item_name == 'NewLine':
                    out += "'\\r\\n'"
                elif item_name == 'StringLength':
                    out += 'len('
                elif item_name == 'MidString':
                    out += 'mid_string('
                elif item_name == 'LoopIndex':
                    out += 'self.get_loop_index('
                elif item_name == 'GetCurrentFuncLoopIndex':
                    out += 'loop_index'
                elif item_name == 'CounterValue':
                    out += 'get_value()'
                elif item_name == 'ParserGetElement':
                    out += 'get_element(-1 + '
                elif item_name == 'ParserGetFirstElement':
                    out += 'get_element(0)'
                elif item_name == 'ParserGetLastElement':
                    out += 'get_element(-1)'
                elif item_name == 'ParserGetElementCount':
                    out += 'get_count()'
                elif item_name == 'BinaryGetStringAt':
                    out += 'get_string('
                elif item_name == 'BinaryGetSize':
                    out += 'get_size()'
                elif item_name == 'ToNumber':
                    out += 'to_number('
                elif item_name == 'SockGetLocalIP':
                    out += 'get_local_ip()'
                elif item_name == 'SockReceiveText':
                    out += 'get_bytes('
                elif item_name == 'RichGetCharacterCount':
                    out += 'get_character_count()'
                elif item_name == 'IconListGetIndex':
                    out += 'get_index()'
                elif item_name == 'ListGetLine':
                    out += 'get_line('
                elif item_name == 'ListFindExact':
                    out += 'find_exact('
                elif item_name == 'SockReceiveLine':
                    out += 'get_line('
                elif item_name == 'ParserGetString':
                    out += 'get_value()'
                elif item_name == 'RightString':
                    out += 'right_string('
                elif item_name == 'GetGlobalString':
                    out += 'self.get_global_string('
                elif item_name == 'ToString':
                    out += 'str('
                elif item_name == 'Random':
                    out += 'randrange('
                elif item_name == 'ListCurrentLineIndex':
                    out += 'get_index()'
                elif item_name == 'GetFileCRC':
                    out += 'get_crc()'
                elif item_name == 'GlobalValue':
                    out += 'self.get_global_value(%s)' % item.loader.value
                elif item_name == 'AlterableValue':
                    out += 'values->get(%s)' % item.loader.value
                elif item_name == 'GetFlag':
                    out += 'get_flag('
                elif item_name == 'XPosition':
                    out += 'get_x()'
                elif item_name == 'YPosition':
                    out += 'get_y()'
                elif item_name == 'GetINIValue':
                    out += 'get_value()'
                elif item_name == 'BoxGetText':
                    out += 'get_text()'
                elif item_name in ('IIFIntegerCompareInteger', 
                                   'IIFStringCompareString',
                                   'IIFStringCompareInteger'):
                    out += 'immediate_compare('
                elif item_name == 'BlowfishRandomKey':
                    out += 'make_random_key('
                elif item_name == 'BlowfishFilterString':
                    out += 'filter_string('
                elif item_name == 'Max':
                    out += 'max('
                elif item_name == 'Abs':
                    out += 'abs('
                elif item_name == 'Divide':
                    out += '/'
                elif item_name == 'ToInt':
                    out += 'to_int('
                elif item_name == 'ControlLastKeyString':
                    out += 'key_string(self.scene.key_downs[-1])'
                elif item_name == 'ListGetCurrentLine':
                    out += 'get_line(None)'
                elif item_name == 'BlowfishEncryptString':
                    out += 'encrypt_string('
                elif item_name == 'BlowfishDecryptString':
                    out += 'decrypt_string('
                elif item_name == 'SockGetProperty':
                    out += 'get_property('
                elif item_name == 'Cos':
                    out += 'cos('
                elif item_name == 'Sin':
                    out += 'sin('
                elif item_name == 'Modulus':
                    out += '%'
                elif item_name == 'GetGlobalTimer':
                    out += 'self.get_timer('
                elif item_name == 'GetIntArgumentA':
                    out += 'int_arg'
                elif item_name == 'ListGetLineCount':
                    out += 'get_count()'
                elif item_name == 'GetDirection':
                    out += 'direction'
                elif item_name == 'PlayerLives':
                    out += 'self.players[%s].lives' % item.objectInfo
                elif item_name == 'Speed':
                    out += 'movement.speed'
                else:
                    if item_name not in self.checked_aces['expressions']:
                        print 'expression not implemented:', item_name, item.loader
                        wait_unimplemented()
                    self.checked_aces['expressions'][item_name] += 1
                    out += '%s(' % item_name
        else:
            parameter_name = type(container.loader).__name__
            if parameter_name == 'Object':
                return self.get_object_name(loader.objectInfo)
            elif parameter_name == 'Sample':
                return loader.name
            elif parameter_name in ('Position', 'Shoot', 'Create'):
                details = {}
                if parameter_name == 'Position':
                    position = loader
                elif parameter_name == 'Shoot':
                    details['shoot_speed'] = loader.shootSpeed
                    details['shoot_object'] = self.get_object_name(
                        loader.objectInfo)
                elif parameter_name == 'Create':
                    create_info = loader.objectInfo
                    details['create_object'] = self.get_object_name(create_info)
                if parameter_name in ('Shoot', 'Create'):
                    position = loader.position
                flags = position.flags
                if flags['Action']:
                    details['use_action_point'] = True
                if flags['Direction']:
                    details['transform_position_direction'] = True
                if flags['InitialDirection']:
                    details['use_direction'] = True
                parent = None
                if position.objectInfoParent != 0xFFFF:
                    parent = self.get_object(position.objectInfoParent)
                    details['parent'] = parent
                details['x'] = position.x
                details['y'] = position.y
                return details
            elif parameter_name == 'KeyParameter':
                value = loader.key.getValue()
                if value == 1:
                    value = 'Qt.LeftButton'
                elif value == 4:
                    value = 'Qt.MiddleButton'
                elif value == 2:
                    value = 'Qt.RightButton'
                else:
                    value = get_qt_key(value)
                return value
            elif parameter_name == 'Zone':
                return repr((loader.x1, loader.y1, loader.x2, loader.y2))
            elif parameter_name == 'Time':
                return repr(loader.timer / 1000.0)
            elif parameter_name == 'Every':
                return repr(loader.delay / 1000.0)
            elif parameter_name == 'Click':
                return 'Qt.%sButton' % loader.getButton()
            elif parameter_name in ('Int', 'Short', 'Colour'):
                return loader.value
            else:
                raise NotImplementedError('parameter: %s' % parameter_name)
        start_clauses = out.count('(')
        end_clauses = out.count(')')
        for _ in xrange(start_clauses - end_clauses):
            out += ')'
        return out

    def get_direction(self, parameter):
        loader = parameter.loader
        if loader.isExpression:
            return self.convert_parameter(parameter)
        else:
            value = loader.value
            directions = []
            for i in xrange(32):
                if value & (1 << i) != 0:
                    directions.append(i)
            if len(directions) > 1:
                return 'random.choice(%r)' % (tuple(directions),)
            else:
                return repr(directions[0])

    def has_multiple_instances(self, handle):
        return is_qualifier(handle) or handle in self.multiple_instances

    def get_object(self, handle, as_list = False):
        if is_qualifier(handle) and not handle in self.has_selection:
            resolved_qualifier = []
            for new_handle in self.resolve_qualifier(handle):
                if self.iterated_object == new_handle:
                    return '(*item)'
                elif new_handle in self.has_selection:
                    resolved_qualifier.append(new_handle)
            if len(resolved_qualifier) == 1:
                handle = resolved_qualifier[0]
        if self.iterated_object == handle:
            return '(*item)'
        elif handle in self.has_selection:
            ret = self.has_selection[handle]
            if not as_list:
                ret = '%s[-1]' % ret
            return ret
        else:
            arguments = [self.get_object_handle(handle)]
            object_index = ''
            if as_list:
                pass
                # arguments.append('True')
            elif (self.has_multiple_instances(handle) and
                  self.iterated_object is not None):
                # arguments.append('True')
                object_index = '[index]'
            ret = 'get_instances(%s)%s' % (', '.join(arguments), object_index)
            return ret

    def get_object_name(self, handle):
        if is_qualifier(handle):
            return 'qualifier_%s' % get_qualifier(handle)
        else:
            return '%s' % self.object_names[handle]

    def get_object_handle(self, handle):
        if is_qualifier(handle):
            return 'qualifier_%s' % get_qualifier(handle)
        else:
            return '%s::type_id' % self.object_names[handle]

    def resolve_qualifier(self, handle):
        return self.qualifiers[get_qualifier(handle)]

    def print_parameters(self, ace):
        print [parameter.getName() for parameter in ace.items]

    def get_condition_name(self, item):
        return self.get_ace_name(item, 'conditions')

    def get_action_name(self, item):
        return self.get_ace_name(item, 'actions')

    def get_expression_name(self, item):
        return self.get_ace_name(item, 'expressions')
    
    def get_ace_name(self, item, key):
        if item.getType() == EXTENSION_BASE:
            num = item.getExtensionNum()
            if num >= 0:
                extension = self.get_extension(item)
                extension_name = extension.name
                try:
                    return extensions[extension_name][key][num]
                except KeyError:
                    print 'getting %r %r %r' % (extension_name, key, num)
                    native = load_native_extension(extension_name)
                    if native is None:
                        menu_entry = repr([])
                    else:
                        menu_name = key[:-1] + 'Menu'
                        menu_dict = getattr(native, menu_name)
                        try:
                            menu_entry = menu_dict[num]
                        except KeyError, e:
                            print 'could not load menu', num, extension_name, key
                            menu_entry = repr([])
                    print 'unnamed:', extension_name, num, menu_entry, key
                    wait_unnamed()
                    return None
        ret = item.getName()
        if ret is None:
            print 'ret:', item.objectType, item.num
            import code
            code.interact(local = locals())
        return ret
    
    def get_extension(self, item):
        return item.getExtension(self.game.extensions)
    
    def write_active(self, writer, common):
        animations = common.animations.loadedAnimations
        writer.putln('animations = {')
        writer.indent()
        for animation_index, animation in animations.iteritems():
            directions = animation.loadedDirections
            writer.putln('%r : {' % animation.getName())
            writer.indent()
            for direction_index, direction in directions.iteritems():
                writer.putln('%s : {' % direction_index)
                writer.indent()
                writer.putln("'min_speed' : %r," % direction.minSpeed)
                writer.putln("'max_speed' : %r," % direction.maxSpeed)
                writer.putln("'repeat' : %r," % direction.repeat)
                writer.putln("'back_to' : %r," % direction.backTo)
                writer.putln("'frames' : %s" % get_image_list(direction.frames))
                writer.dedent()
                writer.putln('},')
                
            writer.dedent()
            writer.putln('},')

        writer.dedent()
        writer.putln('}')
    
    def open_code(self, *path):
        return CodeWriter(self.get_filename(*path))

    def open(self, *path):
        return open(self.get_filename(*path), 'wb')
    
    def get_filename(self, *path):
        return os.path.join(self.outdir, *path)

def main():
    import argparse
    parser = argparse.ArgumentParser(description='Chowdren')
    parser.add_argument('filename', type = str, help = 'input file to convert')
    parser.add_argument('outdir', type = str, help = 'destination directory')
    args = parser.parse_args()
    
    Converter(args.filename, args.outdir)
    

if __name__ == '__main__':
    main()