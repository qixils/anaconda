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
from chowdren.writers.events import default_writers
from chowdren.writers.events import system as system_writers
from chowdren.writers.objects.system import system_objects
from chowdren.common import (get_method_name, get_class_name, check_digits, 
    to_c, make_color)

WRITE_IMAGES = True
WRITE_FONTS = True
WRITE_SOUNDS = True
WRITE_CONFIG = True

WAIT_UNIMPLEMENTED = False
WAIT_UNNAMED = False

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

native_extension_cache = {}
MMF_PATHS = (
    ('C:\Programs\Multimedia Fusion Developer 2\Extensions', '.mfx'),
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

def get_qt_key(value):
    if value == 1:
        return 'Qt.LeftButton'
    elif value == 4:
        return 'Qt.MiddleButton'
    elif value == 2:
        return 'Qt.RightButton'
    return 'Qt.Key_%s' % key_to_qt(value)

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

# OBJECT_CLASSES = {
#     TEXT : 'Text',
#     ACTIVE : 'Active',
#     BACKDROP : 'Backdrop',
#     COUNTER : 'Counter',
#     SUBAPPLICATION : 'SubApplication',
#     'kcedit' : 'Edit',
#     'kcpica' : 'ActivePicture',
#     'parser' : 'StringParser',
#     'binary' : 'BinaryObject',
#     'kcbutton' : 'ButtonControl',
#     'kcini' : 'INI',
#     'Blowfish' : None,
#     'kcfile' : None,
#     'ModFusionEX' : None,
#     'ControlX' : None,
#     'Gstrings' : None,
#     'kcplugin' : None,
#     'ctrlx' : None,
#     'funcloop' : None,
#     'IIF' : None,
#     'Layer' : None,
#     'Dsound' : None,
#     'timex' : None,
#     'Encryption' : None,
#     'XXCRC' : 'ChecksumCalculator',
#     'kcwctrl' : None,
#     'moosock' : 'Socket',
#     'kclist' : 'ListControl',
#     'iconlist' : 'IconList',
#     'kcriched' : 'RichEdit',
#     'KcBoxA' : 'ActiveBox',
#     'txtblt' : 'TextBlitter',
#     'Overlay' : 'OverlayRedux',
#     'Capture' : None,
# }

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

class Converter(object):
    iterated_object = None
    def __init__(self, filename, outdir):
        self.filename = filename
        self.outdir = outdir

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
            object_type = frameitem.properties.getType()
            try:
                if object_type == EXTENSION_BASE:
                    # object_writer = self.get_extension(frameitem).name
                    continue
                else:
                    object_writer = system_objects[object_type](self, frameitem)
            except KeyError:
                print repr(frameitem.name), object_type, handle
                continue
            common = object_writer.common
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
            subclass = object_writer.class_name
            self.object_names[handle] = class_name
            self.instance_names[handle] = get_method_name(class_name)
            object_writer.write_pre(objects_file)
            objects_file.putclass(class_name, subclass)
            objects_file.put_access('public')
            objects_file.putln('static const int type_id = %s;' % type_id.next())
            object_writer.write_constants(objects_file)
            parameters = [to_c('%r', name), 'x', 'y', 'type_id']
            parameters = ', '.join(object_writer.get_parameters() + parameters)
            init_list = [to_c('%s(%s)', subclass, parameters)]
            for name, value in object_writer.get_init_list():
                init_list.append(to_c('%s(%r)', name, value))
            init_list = ', '.join(init_list)
            objects_file.putln(to_c('%s(int x, int y) : %s', class_name, 
                init_list))
            objects_file.start_brace()
            object_writer.write_init(objects_file)
            objects_file.end_brace()
            object_writer.write_class(objects_file)
            if movement_name is not None:
                objects_file.putln('movement_class = %s' % movement_name)
            if frameitem.flags['Global']:
                objects_file.putdef('is_global', True)
            if object_type == TEXT:
                text = common.text
                lines = [paragraph.value for paragraph in text.items]
                objects_file.putdef('width', text.width)
                objects_file.putdef('height', text.height)
                objects_file.putln('font = font%s' % text.items[0].font)
                objects_file.putdef('color', text.items[0].color)
                objects_file.putdef('text', lines[0])
                
                paragraph = text.items[0]
                if paragraph.flags['HorizontalCenter']:
                    horizontal = 'ALIGN_HORIZONTAL_CENTER'
                elif paragraph.flags['RightAligned']:
                    horizontal = 'ALIGN_RIGHT'
                else:
                    horizontal = 'ALIGN_LEFT'
                if paragraph.flags['VerticalCenter']:
                    vertical = 'ALIGN_VERTICAL_CENTER'
                elif paragraph.flags['BottomAligned']:
                    vertical = 'ALIGN_BOTTOM'
                else:
                    vertical = 'ALIGN_TOP'
                objects_file.putln('alignment = %s | %s' % (horizontal,
                    vertical))

            if False:
                if object_type == COUNTER:
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
            object_writer.write_post(objects_file)

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
                # frame_file.putln('qualifier_%s = (%s)' % (qualifier.qualifier,
                #     ', '.join(object_names)), wrap = True)
                frame_file.putln('')
                self.qualifiers[qualifier.qualifier] = object_infos
            
            class_name = 'Frame%s' % (i+1)
            frame_file.putclass(class_name, 'Frame')
            frame_file.put_access('public')
            frame_file.putln(to_c('%s(GameManager * manager) : '
                'Frame(%r, %s, %s, %s, %s, manager)',
                class_name, frame.name, frame.width, frame.height, 
                make_color(frame.background), i))
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
                        print v.name, 'is weird'
                        # raise NotImplementedError
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
        print default_writers['actions'].checked.most_common()
        print ''
        print 'CONDITIONS'
        print default_writers['conditions'].checked.most_common()
        print ''
        print 'EXPRESSIONS'
        print default_writers['expressions'].checked.most_common()
    
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
                condition_writer = self.get_condition_writer(condition)
                if condition_name == 'Always':
                    continue
                negated = not condition.otherFlags['Not']
                parameters = condition.items
                object_name = None
                has_instance = False
                has_multiple = False
                try:
                    if condition.hasObjectInfo():
                        object_info = condition.objectInfo
                        object_name = self.get_object(object_info)
                except KeyError:
                    pass
                # if condition_name in ('IsOverlapping', 'OnCollision'):
                #     real_neg = not negated
                #     other_info = parameters[0].loader.objectInfo
                #     selected_name = self.get_list_name(self.get_object_name(
                #         object_info))
                #     other_selected = self.get_list_name(self.get_object_name(
                #         other_info))
                #     if real_neg:
                #         prefix = ''
                #     else:
                #         prefix = '%s, %s = ' % (selected_name, other_selected)
                #     writer.putln('%sself.check_overlap('
                #                  '%s, %s, negated = %s)' % (prefix,
                #         self.get_object(object_info, True), 
                #         self.get_object(other_info, True), not real_neg))
                #     if not real_neg:
                #         self.has_selection[object_info] = selected_name
                #         self.has_selection[other_info] = other_selected
                #     continue
                if object_name is not None:
                    if False:#condition_name in ('NumberOfObjects',):
                        pass
                    elif self.has_multiple_instances(object_info):
                        has_multiple = True
                        selected_name = self.get_list_name(
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
                        if condition_writer.negate:
                            writer.put('not ')
                        writer.put('%s->' % object_name)
                    condition_writer.write(writer)
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
            action_writer = self.get_action_writer(action)
            has_multiple = False
            object_info = None
            if action.hasObjectInfo():
                object_info = action.objectInfo
                if self.has_multiple_instances(object_info):
                    has_multiple = True
                elif object_info not in self.object_names:
                    object_info = None
            if action_writer.iterate_objects is False:
            # if action_name in ('DisplayText', 'Shoot'):
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
            action_writer.write(writer)
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
                if item.hasObjectInfo():
                    try:
                        object_info = item.objectInfo
                        out += '%s->' % self.get_object(item.objectInfo)
                    except KeyError:
                        pass
                self.last_out = out
                out += self.get_expression_writer(item).get_string()
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

    def get_list_name(self, object_name):
        return '%s_instances' % get_method_name(object_name)

    def get_object_handle(self, handle):
        if is_qualifier(handle):
            return 'qualifier_%s' % get_qualifier(handle)
        else:
            return '%s::type_id' % self.object_names[handle]

    def resolve_qualifier(self, handle):
        return self.qualifiers[get_qualifier(handle)]

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
                    # return extensions[extension_name][key][num]
                    raise KeyError()
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

    def get_action_writer(self, item):
        return self.get_ace_writer(item, 'actions')

    def get_condition_writer(self, item):
        return self.get_ace_writer(item, 'conditions')

    def get_expression_writer(self, item):
        return self.get_ace_writer(item, 'expressions')

    def get_ace_writer(self, item, ace_type):
        writer_module = None
        if item.getType() == EXTENSION_BASE:
            raise NotImplementedError()
            num = item.getExtensionNum()
            if num >= 0:
                extension = self.get_extension(item)
                writer_module = extension.name
                key = num
        if writer_module is None:
            writer_module = system_writers
            key = item.getName()
        try:
            klass = getattr(writer_module, ace_type)[key]
        except (KeyError, AttributeError):
            klass = default_writers[ace_type]
        return klass(self, item)
    
    def get_extension(self, item):
        return item.getExtension(self.game.extensions)
    
    def open_code(self, *path):
        return CodeWriter(self.get_filename(*path))

    def open(self, *path):
        return open(self.get_filename(*path), 'wb')
    
    def get_filename(self, *path):
        return os.path.join(self.outdir, *path)