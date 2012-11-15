import os
import sys
import shutil
import contextlib
from mmfparser.data.exe import ExecutableData
from mmfparser.data.gamedata import GameData
from mmfparser.bytereader import ByteReader
from mmfparser.data.chunkloaders.objectinfo import (PLAYER, KEYBOARD, CREATE,
    TIMER, GAME, SPEAKER, SYSTEM, QUICKBACKDROP, BACKDROP, ACTIVE, TEXT, 
    QUESTION, SCORE, LIVES, COUNTER, RTF, SUBAPPLICATION, EXTENSION_BASE,
    INK_EFFECTS, NONE_EFFECT, SEMITRANSPARENT_EFFECT, INVERTED_EFFECT, 
    XOR_EFFECT, AND_EFFECT, OR_EFFECT, MONOCHROME_EFFECT, ADD_EFFECT, 
    SUBTRACT_EFFECT, HWA_EFFECT, SHADER_EFFECT)
from mmfparser.data.chunkloaders.shaders import INT, FLOAT, INT_FLOAT4, IMAGE
from mmfparser.data.chunkloaders.frame import NONE_PARENT
from mmfparser.bitdict import BitDict
from cStringIO import StringIO
import textwrap
import Image
from mmfparser.data.font import LogFont
import string
import functools
import itertools
from collections import defaultdict, Counter
from chowdren.writers.events import default_writers
from chowdren.writers.events import system as system_writers
from chowdren.writers.events.system import SystemObject
from chowdren.writers.objects.system import system_objects
from chowdren.common import (get_method_name, get_class_name, check_digits, 
    to_c, make_color, get_image_name)
from chowdren.writers.extensions import load_extension_module
from chowdren.key import VK_TO_GLFW
from chowdren import extra
from chowdren import shader
import platform

WRITE_FONTS = True
WRITE_SOUNDS = True

# enabled for porting
NATIVE_EXTENSIONS = False

if NATIVE_EXTENSIONS and sys.platform == 'win32':
    from mmfparser.extension import loadLibrary, LoadedExtension

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
        self.fp = StringIO()
        self.filename = filename
    
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
        self.putln('%s: ;' % name)

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
        data = self.get_data()
        self.fp.close()
        if self.filename is None:
            return
        # fp = open(self.filename, 'rb')
        # original_data = fp.read()
        # fp.close()
        # if original_data == data:
        #     return
        fp = open(self.filename, 'wb')
        fp.write(data)
        fp.close()

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

if platform.node() == 'matpow22':
    MMF_BASE = 'D:\Multimedia Fusion Developer 2'
else:
    MMF_BASE = 'C:\Programs\Multimedia Fusion Developer 2'

MMF_PATH = os.path.join(MMF_BASE, 'Extensions')
MMF_EXT = '.mfx'

EXTENSION_ALIAS = {
    'INI++15' : 'INI++'
}

IGNORE_EXTENSIONS = set([
    'kcwctrl'
])

def load_native_extension(name):
    if not NATIVE_EXTENSIONS or sys.platform != 'win32':
        return None
    name = EXTENSION_ALIAS.get(name, name)
    if name in IGNORE_EXTENSIONS:
        return None
    try:
        return native_extension_cache[name]
    except KeyError:
        pass
    cwd = os.getcwd()
    os.chdir(MMF_BASE)
    library = loadLibrary(os.path.join(MMF_PATH, name + MMF_EXT))
    os.chdir(cwd)
    if library is None:
        raise NotImplementedError(name)
    print 'Loading', name
    extension = LoadedExtension(library, False)
    native_extension_cache[name] = extension
    return extension

def convert_key(value):
    return VK_TO_GLFW[value][0]

def is_qualifier(handle):
    return handle & 32768 == 32768

def get_qualifier(handle):
    return handle & 2047

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

button_flags = BitDict(
    'HideOnStart',
    'DisableOnStart',
    'TextOnLeft',
    'Transparent',
    'SystemColor'
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

def get_color_tuple(value):
    return value & 0xFF, (value & 0xFF00) >> 8, (value & 0xFF0000) >> 16

def get_directions(value):
    directions = []
    for i in xrange(32):
        if value & (1 << i) != 0:
            directions.append(i)
    return directions

def parse_direction(value):
    if value in (0, -1):
        return 'randrange(32)'
    directions = get_directions(value)
    if len(directions) > 1:
        return 'pick_random(%s, %s)' % (len(directions),
            ', '.join([str(item) for item in directions]))
    return directions[0]

DEFAULT_CHARMAP = ''.join([chr(item) for item in xrange(32, 256)])

class EventContainer(object):
    is_static = False
    def __init__(self, name, inactive, parent):
        self.name = name
        if parent is None:
            self.tree = [self]
        else:
            self.tree = parent.tree + [self]
        names = [item.name for item in self.tree]
        self.code_name = 'group_' + get_method_name('_'.join(names))
        self.inactive = inactive
        self.always_groups = []
        self.parent = parent
        self.end_label = '%s_end' % self.code_name

class ContainerMark(object):
    is_container_mark = True
    def __init__(self, container, mark):
        self.container = container
        self.mark = mark

class EventGroup(object):
    global_id = None
    local_id = None
    generated = False
    or_exit = None
    write_actions = True
    action_label = None
    is_container_mark = False
    def __init__(self, conditions, actions, container, global_id, 
                 or_index, or_len, not_always):
        self.conditions = conditions
        self.actions = actions
        for ace_list in (conditions, actions):
            for ace in ace_list:
                ace.group = self
        self.container = container
        self.config = {}
        self.global_id = global_id
        self.not_always = not_always
        name = 'event_%s' % global_id
        if or_len == 1:
            self.name = name
            return
        self.name = 'or_event_%s_%s' % (global_id, or_index)
        self.or_exit = 'goto or_event_%s_%s_end;' % (global_id, or_len-1)
        self.action_label = name
        if or_index != or_len-1:
            self.write_actions = False

    def set_generated(self, value):
        if value:
            self.disable_or()
        self.generated = value

    def disable_or(self):
        self.or_exit = self.action_label = None
        self.write_actions = True

class Converter(object):
    iterated_object = None
    debug = False
    def __init__(self, filename, outdir, image_file = 'Sprites.dat', 
                 win_ico = None, company = None, version = None, 
                 copyright = None):
        self.filename = filename
        self.outdir = outdir

        self.has_single_selection = {}
        self.has_selection = {}
        self.container_tree = []

        fp = open(filename, 'rb')
        if filename.endswith('.exe'):
            exe = ExecutableData(ByteReader(fp), loadFrames = False)
            game = exe.gameData
        elif filename.endswith('.ccn'):
            game = GameData(ByteReader(fp))
        else:
            raise NotImplementedError('invalid extension')

        self.game = game
        
        # shutil.rmtree(outdir, ignore_errors = True)
        copytree(os.path.join(os.getcwd(), 'base'), outdir)

        # application info
        company = company or game.author
        version = version or '1.0.0.0'
        version_number = ', '.join(version.split('.'))
        copyright = copyright or game.author
        print repr((company, version, copyright, game.name))
        res_path = self.get_filename('chowdren.rc')
        res_data = open(res_path, 'rb').read()
        res_data = res_data.format(company = company, version = version, 
            copyright = copyright, description = game.name,
            version_number = version_number)
        open(res_path, 'wb').write(res_data)
        
        # fonts
        if WRITE_FONTS:
            fonts_file = self.open_code('fonts.h')
            fonts_file.putln('#include "common.h"')
            fonts_file.start_guard('FONTS_H')
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
            fonts_file.close_guard('FONTS_H')
            fonts_file.putln('')
            fonts_file.close()
        
        # images

        if image_file is not None:
            image_data = open(self.get_filename(image_file), 'wb')
            images_file = self.open_code('images.cpp')
            images_file.putln('#include "image.h"')
            images_file.putln('')
            all_images = []
            if game.images:
                for image in game.images.items:
                    handle = image.handle
                    image_name = 'image%s' % handle
                    all_images.append(image_name)
                    pil_image = Image.fromstring('RGBA', (image.width, 
                        image.height), image.getImageData())
                    temp = StringIO()
                    pil_image.save(temp, 'PNG')
                    temp = temp.getvalue()
                    offset = image_data.tell()
                    image_data.write(temp)
                    images_file.putln(to_c(
                        'Image %s(%s, %s, %s, %s, %s);',
                        image_name, offset, image.xHotspot, 
                        image.yHotspot, image.actionX, image.actionY))
            image_data.close()
            images_file.putln('')
            images_file.close()
            images_header = self.open_code('images.h')
            images_header.start_guard('IMAGES_H')
            images_header.putln('extern Image %s;' % ', '.join(all_images))
            images_header.close_guard('IMAGES_H')
            images_header.close()
        
        # sounds
        if WRITE_SOUNDS:
            if game.sounds:
                for sound in game.sounds.items:
                    sound_type = sound.getType()
                    extension = {'OGG' : 'ogg', 'WAV' : 'wav'}[sound_type]
                    self.open('sounds', '%s.%s' % (sound.name, extension)
                        ).write(str(sound.data))
        
        # objects
        objects_file = self.open_code('objects.h')
        objects_file.start_guard('OBJECTS_H')
        objects_file.putln('#include "common.h"')
        objects_file.putln('#include "images.h"')
        objects_file.putln('#include "fonts.h"')
        # objects_file.putln('#include "sounds.h"')
        objects_file.putln('')
        
        self.object_names = {}
        self.all_objects = {}
        self.instance_names = {}
        self.name_to_item = {}
        self.object_types = {}

        type_id = itertools.count(1)
        
        for frameitem in game.frameItems.items:
            name = frameitem.name
            self.name_to_item[name] = frameitem
            handle = frameitem.handle
            if name is None:
                class_name = 'Object%s' % handle
            else:
                class_name = get_class_name(name) + '_' + str(handle)
            object_type = frameitem.properties.objectType
            try:
                object_writer = self.get_object_writer(object_type)(
                    self, frameitem)
            except (KeyError, AttributeError, NotImplementedError):
                print 'not implemented:', repr(frameitem.name), object_type, 
                print handle
                continue
            self.all_objects[handle] = object_writer
            self.object_types[handle] = object_type
            if object_writer.static or extra.is_special_object(name):
                continue
            common = object_writer.common
            subclass = object_writer.class_name
            self.object_names[handle] = class_name
            self.instance_names[handle] = get_method_name(class_name)
            object_writer.write_pre(objects_file)
            objects_file.putclass(class_name, subclass)
            objects_file.put_access('public')
            objects_file.putln('static const unsigned int type_id = %s;' % 
                type_id.next())
            object_writer.write_constants(objects_file)
            parameters = [to_c('%r', name), 'x', 'y', 'type_id']
            extra_parameters = [str(item) 
                for item in object_writer.get_parameters()]
            parameters = ', '.join(extra_parameters + parameters)
            init_list = [to_c('%s(%s)', subclass, parameters)]
            for name, value in object_writer.get_init_list():
                init_list.append(to_c('%s(%r)', name, value))
            init_list = ', '.join(init_list)
            objects_file.putln(to_c('%s(int x, int y) : %s', class_name, 
                init_list))
            objects_file.start_brace()

            if not object_writer.is_visible():
                objects_file.putln('set_visible(false);')

            object_writer.write_init(objects_file)
            
            shader_name = None
            if frameitem.shaderId is not None:
                # raise NotImplementedError('has shader')
                ink_effect = SHADER_EFFECT
            else:
                ink_effect = frameitem.inkEffect
            if ink_effect:
                if ink_effect & HWA_EFFECT or ink_effect == SHADER_EFFECT:
                    parameter = frameitem.inkEffectValue
                    b, g, r = get_color_tuple(parameter)
                    a = (parameter & 0xFF000000) >> 24
                    objects_file.putln('blend_color = %s;' % make_color(
                        (r, g, b, a)))
                    ink_effect &= ~HWA_EFFECT
                    if ink_effect == SHADER_EFFECT:
                        shader_data = game.shaders.items[frameitem.shaderId]
                        shader_name = shader_data.name
                elif ink_effect == ADD_EFFECT:
                    shader_name = 'Add'
                elif ink_effect == SEMITRANSPARENT_EFFECT:
                    objects_file.putln('blend_color = %s;' % make_color(
                        (255, 255, 255, frameitem.inkEffectValue)))
                else:
                    raise NotImplementedError(
                        'unknown inkeffect: %s' % ink_effect)
            if shader_name is not None:
                objects_file.putln('set_shader(&%s);' % shader.get_name(
                    shader_name))
            if ink_effect == SHADER_EFFECT:
                shader_data = game.shaders.items[frameitem.shaderId]
                parameters = shader_data.get_parameters()
                values = frameitem.items
                for index, parameter in enumerate(shader_data.parameters):
                    reader = values[index]
                    reader.seek(0)
                    if parameter.type == INT:
                        value = reader.readInt()
                    elif parameter.type == FLOAT:
                        value = reader.readFloat()
                    elif parameter.type == INT_FLOAT4:
                        value = make_color((reader.readByte(True), 
                            reader.readByte(True),
                            reader.readByte(True), 
                            reader.readByte(True)))
                    else:
                        raise NotImplementedError
                    parameters[parameter.name].value = value
                for name, value in parameters.iteritems():
                    objects_file.putln(to_c('set_shader_parameter(%r, %s);',
                        name, value.value))

            if hasattr(common, 'movements') and common.movements:
                movements = common.movements.items
                if len(movements) != 1:
                    raise NotImplementedError
                movement, = movements
                if movement.getName() != 'Static':
                    raise NotImplementedError
                start_direction = movement.directionAtStart
                if start_direction != 0:
                    objects_file.putln('set_direction(%s);' % parse_direction(
                        start_direction))
            if common and not common.isBackground():
                if common.values:
                    for index, value in enumerate(common.values.items):
                        objects_file.putln(to_c('values->set(%s, %s);', index, 
                            value))
                if common.strings:
                    for index, value in enumerate(common.strings.items):
                        objects_file.putln(to_c('strings->set(%s, %r);', index, 
                            value))
            objects_file.end_brace()
            object_writer.write_class(objects_file)
            
            objects_file.end_brace(True)
            objects_file.putln('static ObjectList %s_instances;' % 
                get_method_name(class_name))
            object_writer.write_post(objects_file)

            objects_file.putln('')
        objects_file.close_guard('OBJECTS_H')
        objects_file.close()
        
        processed_frames = []
        
        # frames
        for frame_index, frame in enumerate(game.frames):
            frame_class_name = self.frame_class = 'Frame%s' % (frame_index+1)
            frame.load()
            self.current_frame = frame
            processed_frames.append(frame_index + 1)
            frame_file = self.open_code('frame%s.h' % (frame_index + 1))
            frame_file.putln('#include "common.h"')
            frame_file.putln('#include "objects.h"')
            frame_file.putln('')
            
            startup_instances = []
            self.multiple_instances = set()
            startup_set = set()
            object_writers = []

            self.system_object = SystemObject(self)
            object_writers.append(self.system_object)

            for instance in getattr(frame.instances, 'items', ()):
                frameitem = instance.getObjectInfo(game.frameItems)
                try:
                    object_writer = self.all_objects[frameitem.handle]
                    object_writers.append(object_writer)
                except KeyError:
                    continue
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

            generated_groups = self.generated_groups = defaultdict(list)
            always_groups = []
            always_groups_dict = self.always_groups_dict = defaultdict(list)
            current_container = None
            self.containers = containers = {}
            changed_containers = set()
            for group_index, group in enumerate(events.items):
                first_condition = group.conditions[0]
                name = self.get_condition_name(first_condition)
                if name == 'NewGroup':
                    group_loader = first_condition.items[0].loader
                    current_container = EventContainer(group_loader.name,
                        group_loader.flags['Inactive'], current_container)
                    containers[group_loader.offset] = current_container
                    always_groups.append(ContainerMark(current_container, name))
                    continue
                elif name == 'GroupEnd':
                    always_groups.append(ContainerMark(current_container, name))
                    current_container = current_container.parent
                    continue

                # or groups
                condition_groups = []
                conditions = []
                not_always = False
                for condition in group.conditions:
                    name = condition.getName()
                    if name == 'OrLogical':
                        condition_groups.append(conditions)
                        conditions = []
                    else:
                        if name == 'NotAlways':
                            not_always = True
                            # not used yet, but just in case
                        condition_writer = self.get_condition_writer(condition)
                        condition_writer.container = current_container
                        conditions.append(condition_writer)
                condition_groups.append(conditions)

                action_groups = []

                for _ in xrange(len(condition_groups)):
                    actions = []
                    for action in group.actions:
                        action_writer = self.get_action_writer(action)
                        action_writer.container = current_container
                        actions.append(action_writer)

                        action_name = self.get_action_name(action)
                        if action_name in ('CreateObject', 'Shoot', 
                                           'DisplayText'):
                            loader = action.items[0].loader
                            try:
                                instances = self.current_frame.instances
                                create_info = instances.fromHandle(
                                    loader.objectInstance).objectInfo
                            except ValueError:
                                create_info = loader.objectInfo
                            if create_info == 0xFFFF:
                                # the case for DisplayText
                                create_info = action.objectInfo
                            loader.objectInfo = create_info
                            self.multiple_instances.add(create_info)
                        elif action_name in ('DeactivateGroup', 
                                             'ActivateGroup'):
                            pointer = action.items[0].loader.pointer
                            changed_containers.add(pointer)
                    action_groups.append(actions)

                all_groups = []
                has_generated = False
                for or_index, conditions in enumerate(condition_groups):
                    actions = action_groups[or_index]
                    new_group = EventGroup(conditions, actions, 
                        current_container, group_index+1, 
                        or_index, len(condition_groups), not_always)
                    all_groups.append(new_group)
                    first_writer = new_group.conditions[0]
                    first_condition = first_writer.data
                    name = self.get_condition_name(first_condition)
                    if first_writer.is_always is not None:
                        is_always = first_writer.is_always
                    else:
                        is_always = first_condition.flags['Always']
                    if is_always:
                        if current_container:
                            current_container.always_groups.append(new_group)
                        always_groups.append(new_group)
                        new_group.local_id = len(always_groups_dict[name])+1
                        always_groups_dict[name].append(new_group)
                    else:
                        has_generated = True
                        new_group.set_generated(True)
                        key = None
                        if first_condition.getType() == EXTENSION_BASE:
                            num = first_condition.getExtensionNum()
                            if num >= 0:
                                key = (first_condition.objectType, num)
                        if key is None:
                            key = name
                        new_group.local_id = len(generated_groups[key])+1
                        generated_groups[key].append(new_group)
                if has_generated:
                    for group in all_groups:
                        group.disable_or()

            for k, v in containers.iteritems():
                if k not in changed_containers:
                    if v.inactive:
                        print v.name, 'is never activated!'
                        continue
                    v.is_static = True

            self.qualifiers = {}
            self.qualifier_types = {}
            for qualifier in events.qualifiers.values():
                object_infos = qualifier.resolve_objects(self.game.frameItems)
                object_names = ['%s::type_id' % self.object_names[item] 
                    for item in object_infos] + ['0']
                class_name = 'qualifier_%s' % qualifier.qualifier
                frame_file.putln('static unsigned int %s[] = '
                    '{%s};' % (class_name, ', '.join(object_names)), 
                    wrap = True)
                frame_file.putln('static ObjectList %s_instances;' % (
                    class_name))
                frame_file.putln('')
                self.qualifiers[qualifier.qualifier] = object_infos
                self.qualifier_types[qualifier.qualifier] = qualifier.type
            
            frame_file.putclass(frame_class_name, 'Frame')
            frame_file.put_access('public')
            for container in containers.values():
                frame_file.putln('bool %s;' % container.code_name)
            frame_file.putln(to_c('%s(GameManager * manager) : '
                'Frame(%r, %s, %s, %s, %s, manager)',
                frame_class_name, frame.name, frame.width, frame.height, 
                make_color(frame.background), frame_index))
            frame_file.start_brace()
            frame_file.end_brace()

            startup_images = set()
            # object writer custom stuff
            for object_writer in object_writers:
                object_writer.write_frame(frame_file)
                startup_images.update(object_writer.get_images())

            frame_file.putmeth('void on_start')

            # load images on startup
            frame_file.putln('static bool images_initialized = false;')
            frame_file.putln('if (!images_initialized) {')
            frame_file.indent()
            frame_file.putln('images_initialized = true;')
            for image_handle in startup_images:
                frame_file.putln('%s.load();' % get_image_name(image_handle, 
                    False))
            frame_file.end_brace()

            for container in containers.values():
                if container.is_static:
                    continue
                frame_file.putln(to_c('%s = %s;', container.code_name,
                    not container.inactive))

            for layer in frame.layers.items:
                frame_file.putln(to_c('add_layer(%s, %s, %s);',
                    layer.xCoefficient, layer.yCoefficient, 
                    not layer.flags['ToHide']))

            for instance, frameitem in startup_instances:
                object_writer = self.all_objects[frameitem.handle]
                if object_writer.is_background():
                    method = 'add_background_object'
                else:
                    method = 'add_object'
                frame_file.putln('%s(new %s(%s, %s), %s);' %
                    (method, self.object_names[frameitem.handle], instance.x, 
                    instance.y, instance.layer))

            for object_writer in object_writers:
                object_writer.write_start(frame_file)

            self.begin_events()
            start_groups = generated_groups.pop('StartOfFrame', None)
            if start_groups:
                for group in start_groups:
                    self.write_event(frame_file, group, True)
            frame_file.end_brace()
            
            frame_file.putmeth('void handle_events')
            end_markers = [] # for debug
            self.begin_events()
            for group in always_groups:
                if group.is_container_mark:
                    container = group.container
                    if container.is_static:
                        continue
                    if group.mark == 'NewGroup':
                        # frame_file.putln('std::cout << "%s %s" << std::endl;' % (
                        #     container.code_name, group.mark))
                        frame_file.putln('if (!%s) goto %s;' % (
                            container.code_name, container.end_label))
                        end_markers.insert(0, container.end_label)
                        self.container_tree.insert(0, container)
                    elif group.mark == 'GroupEnd':
                        end_markers.remove(container.end_label)
                        frame_file.put_label(container.end_label)
                        self.container_tree.remove(container)
                        # frame_file.putln('std::cout << "%s %s" << std::endl;' % (
                        #     container.code_name, group.mark))
                    continue
                self.write_event(frame_file, group)

            for end_marker in end_markers:
                frame_file.put_label(end_marker)
            frame_file.end_brace()
            
            frame_file.end_brace(True) # end of frame
            frame_file.close()
                
            if generated_groups:
                print 'unimplemented generated groups in %r: %r' % (
                    frame.name, generated_groups)

        # general configuration
        header = game.header
        config_file = self.open_code('config.h')

        # small hack to make applications with Ultimate Fullscreen not open
        # a window before "Make Fullscreen" or "Make Windowed" are called
        # explicitly.
        extension_names = set([item.name for item in  game.extensions.items])
        if 'ultimatefullscreen' not in extension_names:
            config_file.putln('#define CHOWDREN_STARTUP_WINDOW')
            config_file.putln('')

        config_file.putdefine('NAME', game.name)
        config_file.putdefine('COPYRIGHT', game.copyright)
        config_file.putdefine('ABOUT', game.aboutText)
        config_file.putdefine('AUTHOR', game.author)
        config_file.putdefine('WINDOW_WIDTH', header.windowWidth)
        config_file.putdefine('WINDOW_HEIGHT', header.windowHeight)
        config_file.putdefine('RESIZABLE_WINDOW', 
            not header.newFlags['NoThickFrame'])

        config_file.putln('')
        config_file.putln('#include "common.h"')
        frame_classes = []
        for frame_index in processed_frames:
            frame_class_name = 'Frame%s' % frame_index
            frame_classes.append(frame_class_name)
            config_file.putln('#include "frame%s.h"' % frame_index)
        config_file.putln('')
        # config_file.putdef('BORDER_COLOR', header.borderColor)
        # config_file.putdefine('START_LIVES', header.initialLives)
        config_file.putln('')
        config_file.putln('static Frame ** frames = NULL;')
        config_file.putmeth('Frame ** get_frames', 'GameManager * manager')
        config_file.putln('if (frames) return frames;')

        config_file.putln('frames = new Frame*[%s];' % len(frame_classes))

        for i, frame in enumerate(frame_classes):
            config_file.putln('frames[%s] = new %s(manager);' % (i, frame))

        config_file.putln('return frames;')

        config_file.end_brace()

        config_file.putmeth('void setup_globals',
            'GlobalValues * values', 'GlobalStrings * strings')
        if game.globalValues:
            for index, value in enumerate(game.globalValues.items):
                config_file.putln('values->set(%s, %s);' % (index, value))
        if game.globalStrings:
            for index, value in enumerate(game.globalStrings.items):
                config_file.putln(to_c('strings->set(%s, %r);', index, 
                    value))
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

    @contextlib.contextmanager
    def iterate_object(self, object_info, writer, name = 'item'):
        selected_name = 'extra_' + self.get_list_name(
            self.get_object_name(object_info))
        make_dict = self.get_object(object_info, True)
        writer.putln('ObjectList %s = %s;' % (selected_name, make_dict))
        self.iterated_object = object_info
        writer.start_brace()
        writer.putln('ObjectList::const_iterator %s = %s.begin();' % (
            name, selected_name))
        writer.putln('while (%s != %s.end()) {' % 
            (name, selected_name))
        writer.indent()
        yield
        writer.putln('%s++;' % name)
        writer.dedent()
        writer.putln('}')
        writer.dedent()
        writer.putln('}')
        self.iterated_object = None

    def begin_events(self):
        pass

    def set_object(self, object_info, name):
        self.has_single_selection[object_info] = name

    def set_list(self, object_info, name):
        self.has_selection[object_info] = name

    def clear_selection(self):
        self.has_selection = {}
    
    def write_instance_check(self, object_info, writer):
        writer.putln('if (%s.size() == 0) %s' % (
            self.get_object(object_info, True), self.event_break))

    def write_container_check(self, group, writer):
        container = group.container
        if group.generated or not container:
            return
        for item in container.tree:
            if item.is_static:
                continue
            writer.putln('if (!%s) goto %s;' % (
                item.code_name, item.end_label))

    def write_event(self, outwriter, group, triggered = False):
        self.current_event_id = group.global_id
        actions, conditions = group.actions, group.conditions
        container = group.container
        has_container_check = False
        if container:
            is_static = all([item.is_static for item in container.tree])
            has_container_check = (triggered and not is_static)
        if triggered:
            conditions = conditions[1:]
        writer = CodeWriter()
        writer.putln('// event %s' % group.global_id)
        event_break = self.event_break = 'goto %s_end;' % group.name
        collisions = defaultdict(list)
        writer.start_brace() # new scope
        if conditions or has_container_check:
            if has_container_check:
                groups = []
                for item in container.tree:
                    if item.is_static:
                        continue
                    groups.append(item.code_name)
                condition = ' && '.join(groups)
                writer.putln('if (!(%s)) %s' % (condition, event_break))
            elif container:
                writer.putln('// group: %s' % container.name)
            for condition_index, condition_writer in enumerate(conditions):
                if condition_writer.custom:
                    condition_writer.write(writer)
                    continue
                negated = not condition_writer.is_negated()
                object_name = None
                has_multiple = False
                object_info, object_type = condition_writer.get_object()
                self.current_object = (object_info, object_type)
                if object_info is not None:
                    try:
                        object_name = self.get_object(object_info)
                    except KeyError:
                        pass
                if object_info in self.has_single_selection:
                    object_name = self.has_single_selection[object_info]
                    self.iterated_object = object_info
                elif object_name is not None and self.has_multiple_instances(
                        object_info):
                    selected_name = self.get_list_name(
                        self.get_object_name(object_info))
                    if object_info not in self.has_selection:
                        make_dict = self.get_object(object_info, True)
                        writer.putln('%s = %s;' % (selected_name, make_dict))
                        self.has_selection[object_info] = selected_name
                    if condition_writer.iterate_objects is not False:
                        has_multiple = True
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
                if object_name is None:
                    if condition_writer.static:
                        writer.put('%s::' % self.get_object_class(
                            object_type, star = False))
                elif condition_writer.iterate_objects is not False:
                    if condition_writer.negate:
                        writer.put('!')
                    obj = '((%s)%s)' % (self.get_object_class(
                        object_type), object_name)
                    if condition_writer.dereference:
                        obj += '->'
                    writer.put(obj)
                condition_writer.write(writer)
                if negated:
                    writer.put(')')
                if has_multiple:
                    writer.put(') item = %s.erase(item);\n' % selected_name)
                    writer.putln('else ++item;')
                    writer.end_brace()
                    writer.indented = False
                    writer.putln('if (%s.empty()) %s' % (selected_name, 
                        event_break))
                    self.iterated_object = None
                else:
                    writer.put(') %s\n' % event_break)

        if group.write_actions:
            if group.action_label:
                writer.put_label(group.action_label)
            for action_writer in actions:
                if action_writer.custom:
                    action_writer.write(writer)
                    continue
                has_multiple = False
                has_single = False
                object_info, object_type = action_writer.get_object()
                self.current_object = (object_info, object_type)
                if object_info is not None:
                    if object_info in self.has_single_selection:
                        has_single = True
                    elif self.has_multiple_instances(object_info):
                        has_multiple = True
                    elif object_info not in self.object_names:
                        object_info = None
                if action_writer.iterate_objects is False:
                    has_multiple = False
                    object_info = None
                object_name = None
                if has_single:
                    object_name = self.has_single_selection[object_info]
                elif has_multiple:
                    if object_info in self.has_selection:
                        list_name = self.has_selection[object_info]
                    else:
                        list_name = self.get_list_name(
                            self.get_object_name(object_info))
                        make_dict = self.get_object(object_info, True)
                        writer.putln('%s = %s;' % (list_name, make_dict))
                        self.has_selection[object_info] = list_name
                    writer.putln('for (item = %s.begin(); item != %s.end(); '
                                 'item++) {' % (list_name, list_name))
                    writer.indent()
                    self.iterated_object = object_info
                    object_name = '(*item)'
                elif object_info is not None:
                    writer.putindent()
                    writer.put('%s->' % self.get_object(object_info))
                elif action_writer.static:
                    writer.put('%s::' % self.get_object_class(object_type,
                        star = False))
                else:
                    writer.putindent()
                if object_name is not None:
                    writer.putindent()
                    writer.put('((%s)%s)->' % (self.get_object_class(
                        object_type), object_name))
                action_writer.write(writer)
                writer.put('\n')
                if has_multiple:
                    writer.end_brace()
                    self.iterated_object = None
            for action_writer in actions:
                action_writer.write_post(writer)
        else:
            writer.putln('goto %s;' % group.action_label)

        writer.putln('%s_end: ;' % group.name)
        writer.end_brace()
        outwriter.putcode(writer)
        self.has_single_selection = {}
        self.has_selection = {}
    
    def convert_parameter(self, container):
        loader = container.loader
        out = ''
        if loader.isExpression:
            self.expression_items = loader.items[:-1]
            self.item_index = 0
            while self.item_index < len(self.expression_items):
                item = self.expression_items[self.item_index]
                expression_writer = self.get_expression_writer(item)
                object_info, object_type = expression_writer.get_object()
                self.current_object = (object_info, object_type)
                if expression_writer.static:
                    out += '%s::' % self.get_object_class(object_type, 
                        star = False)
                elif object_info is not None:
                    try:
                        out += '%s->' % self.get_object(item.objectInfo)
                    except KeyError:
                        pass
                self.last_out = out
                out += expression_writer.get_string()
                self.item_index += 1
        else:
            parameter_name = type(container.loader).__name__
            parameter_type = container.getName()
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
                details['layer'] = position.layer
                details['x'] = position.x
                details['y'] = position.y
                return details
            elif parameter_name == 'KeyParameter':
                return convert_key(loader.key.getValue())
            elif parameter_name == 'Zone':
                return repr((loader.x1, loader.y1, loader.x2, loader.y2))
            elif parameter_name == 'Time':
                return repr(loader.timer / 1000.0)
            elif parameter_name == 'Every':
                return repr(loader.delay / 1000.0)
            elif parameter_name == 'Click':
                button = loader.getButton()
                if button == 'Left':
                    return convert_key(1)
                if button == 'Right':
                    return convert_key(2)
                elif button == 'Middle':
                    return convert_key(4)
            elif parameter_name in ('Int', 'Short', 'Colour'):
                if parameter_type == 'NEWDIRECTION':
                    return parse_direction(loader.value)
                elif parameter_type == 'TEXTNUMBER':
                    return loader.value + 1
                return loader.value
            elif parameter_name == 'Extension':
                return loader.get_reader()
            elif parameter_name in ('String', 'Filename'):
                return to_c('%r', loader.value)
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
        object_type = self.get_object_class(object_info = handle)
        if handle in self.has_single_selection:
            ret = self.has_single_selection[handle]
            if as_list:
                ret = 'make_single_list(%s)' % ret
            return ret
        # if is_qualifier(handle) and not handle in self.has_selection:
        #     resolved_qualifier = []
        #     for new_handle in self.resolve_qualifier(handle):
        #         if self.iterated_object == new_handle:
        #             return '((%s)*item)' % object_type
        #         elif new_handle in self.has_selection:
        #             resolved_qualifier.append(new_handle)
        #     if len(resolved_qualifier) == 1:
        #         handle = resolved_qualifier[0]
        if self.iterated_object == handle:
            return '((%s)*item)' % object_type
        elif handle in self.has_selection:
            ret = self.has_selection[handle]
            if not as_list:
                ret = '((%s)%s[0])' % (object_type, ret)
            return ret
        else:
            type_id, is_qual = self.get_object_handle(handle)
            object_index = ''
            getter_name = 'get_instances'
            if as_list:
                pass
            else:
                getter_name = 'get_instance'
            # elif (self.has_multiple_instances(handle) and
            #       self.iterated_object is not None):
            #     object_index = '[0]' # [index], but probably not needed
            # else:
            #     # getter_name = 'get_instance'
            #     object_index = '[0]'
            if as_list: # object_index:
                return '%s(%s)' % (getter_name, type_id)
            else:
                return '((%s)%s(%s)%s)' % (object_type, getter_name, type_id, 
                    object_index)

    def get_object_name(self, handle):
        if is_qualifier(handle):
            return 'qualifier_%s' % get_qualifier(handle)
        else:
            return '%s' % self.object_names[handle]

    def get_list_name(self, object_name):
        return '%s_instances' % get_method_name(object_name)

    def get_object_handle(self, handle):
        if is_qualifier(handle):
            return ('qualifier_%s' % get_qualifier(handle), True)
        else:
            return ('%s::type_id' % self.object_names[handle], False)

    def get_object_class(self, object_type = None, object_info = None, 
                         star = True):
        if object_info is not None:
            if object_info == self.current_object[0]:
                object_type = self.current_object[1]
            else:
                if is_qualifier(object_info):
                    object_type = self.qualifier_types[
                        get_qualifier(object_info)]
                else:
                    object_type = self.object_types[object_info]
        try:
            ret = self.get_object_writer(object_type).class_name
            if star:
                ret += '*'
            return ret
        except (KeyError, ValueError):
            return None

    def get_object_writer(self, object_type):
        if object_type >= EXTENSION_BASE:
            ext = self.game.extensions.fromHandle(object_type - EXTENSION_BASE)
            writer_module = load_extension_module(ext.name)
            return writer_module.get_object()
        else:
            return system_objects[object_type]

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
                # print 'getting %r %r %r' % (extension_name, key, num)
                native = load_native_extension(extension_name)
                if native is None:
                    menu_entry = []
                else:
                    menu_name = key[:-1] + 'Menu'
                    menu_dict = getattr(native, menu_name)
                    try:
                        menu_entry = menu_dict[num]
                    except KeyError, e:
                        print 'could not load menu', num, extension_name, key
                        menu_entry = []
                # print 'unnamed:', extension_name, num, menu_entry, key
                # print '%r' % menu_entry
                full_name = [extension_name] + menu_entry + [str(num)]
                return get_method_name('_'.join(full_name))
        ret = item.getName()
        if ret is None:
            print 'ret:', item.objectType, item.num
            import code
            code.interact(local = locals())
        return ret

    def get_action_writer(self, item, as_klass = False):
        return self.get_ace_writer(item, 'actions', as_klass)

    def get_condition_writer(self, item, as_klass = False):
        return self.get_ace_writer(item, 'conditions', as_klass)

    def get_expression_writer(self, item, as_klass = False):
        return self.get_ace_writer(item, 'expressions', as_klass)

    def get_ace_writer(self, item, ace_type, as_klass = False):
        writer_module = None
        if item.getType() == EXTENSION_BASE:
            num = item.getExtensionNum()
            if num >= 0:
                extension = self.get_extension(item)
                writer_module = load_extension_module(extension.name)
                key = num
        if writer_module is None:
            writer_module = system_writers
            key = item.getName()
        object_name = None
        if item.hasObjectInfo():
            object_writer = self.all_objects.get(item.objectInfo, None)
            if object_writer:
                object_name = object_writer.data.name
        else:
            object_name = None
        if extra.is_special_object(object_name):
            writer_module = extra
        try:
            klass = getattr(writer_module, ace_type)[key]
        except (KeyError, AttributeError):
            klass = default_writers[ace_type]
        if as_klass:
            return klass
        else:
            return klass(self, item)
    
    def get_extension(self, item):
        return item.getExtension(self.game.extensions)
    
    def open_code(self, *path):
        return CodeWriter(self.get_filename(*path))

    def open(self, *path):
        return open(self.get_filename(*path), 'wb')
    
    def get_filename(self, *path):
        return os.path.join(self.outdir, *path)