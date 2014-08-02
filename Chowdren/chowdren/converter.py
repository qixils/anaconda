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
    NONE_EFFECT, SEMITRANSPARENT_EFFECT, INVERTED_EFFECT,
    XOR_EFFECT, AND_EFFECT, OR_EFFECT, MONOCHROME_EFFECT, ADD_EFFECT,
    SUBTRACT_EFFECT, HWA_EFFECT, SHADER_EFFECT, getObjectType)
from mmfparser.data.chunkloaders.shaders import INT, FLOAT, INT_FLOAT4, IMAGE
from mmfparser.data.chunkloaders.frame import NONE_PARENT
from mmfparser.bitdict import BitDict
from cStringIO import StringIO
import textwrap
from PIL import Image
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
    to_c, make_color, parse_direction, get_base_path, makedirs,
    is_qualifier, get_qualifier, get_iter_type, get_list_type,
    TEMPORARY_GROUP_ID, TEMPORARY_HASH_ID)
from chowdren.writers.extensions import load_extension_module
from chowdren.key import VK_TO_SDL, VK_TO_NAME, convert_key, KEY_TO_NAME
from chowdren import extra
from chowdren import shader
from chowdren.shader import INK_EFFECTS
from chowdren import hacks
from chowdren.idpool import get_id
from chowdren.codewriter import CodeWriter
import platform
import math
import wave
import audioop
import struct

WRITE_FONTS = True
WRITE_SOUNDS = True
PROFILE = False

# enabled for porting
NATIVE_EXTENSIONS = True

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

def is_file_changed(src, dst):
    return True
    if not os.path.exists(dst):
        return True
    diff = os.stat(src).st_mtime - os.stat(dst).st_mtime
    return math.fabs(diff) > 0.01

def copytree(src, dst, excludes=[]):
    try:
        os.makedirs(dst)
    except OSError:
        pass
    errors = []
    for name in os.listdir(src):
        if name in excludes:
            continue
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)
        if os.path.isdir(srcname):
            copytree(srcname, dstname, excludes)
        elif is_file_changed(srcname, dstname):
            shutil.copy2(srcname, dstname)
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

native_extension_cache = {}

MMF_BASE = ''

if sys.platform == 'win32':
    try:
        import _winreg
        reg_name = ('Software\\Clickteam\\Multimedia Fusion Developer 2\\'
                    'Settings')
        reg_key = _winreg.OpenKey(_winreg.HKEY_CURRENT_USER, reg_name)
        MMF_BASE = _winreg.QueryValueEx(reg_key, 'ProPath')[0]
    except (ImportError, WindowsError):
        pass

MMF_PATH = os.path.join(MMF_BASE, 'Extensions')
MMF_EXT = '.mfx'

EXTENSION_ALIAS = {
    'INI++15' : 'INI++'
}

IGNORE_EXTENSIONS = set([
    'kcwctrl', 'SteamChowdren', 'ChowdrenFont', 'INI++'
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

DEFAULT_CHARMAP = ''.join([chr(item) for item in xrange(32, 256)])

class EventContainer(object):
    is_static = False
    def __init__(self, name, inactive, parent, converter):
        self.name = name
        if parent is None:
            self.tree = [self]
        else:
            self.tree = parent.tree + [self]
        names = [item.name for item in self.tree]
        names = '_'.join(names)
        converter.container_ids[names] += 1
        container_id = converter.container_ids[names]

        self.code_name = 'group_%s_%s' % (get_method_name(names), container_id)
        self.inactive = inactive
        self.parent = parent
        self.children = []
        self.end_label = '%s_end' % self.code_name
        self.check_ids = itertools.count()
        hacks.init_container(parent, self)

    def add_child(self, child):
        self.children.append(child)

    def get_all_children(self):
        children = set()
        for child in self.children:
            children.add(child)
            children.update(child.get_all_children())
        return list(children)

class ContainerMark(object):
    is_container_mark = True
    conditions = ()

    def __init__(self, container, mark):
        self.container = container
        self.mark = mark

def fix_conditions(conditions):
    return conditions
    # this is a nasty hack based on very odd MMF2 behaviour
    names = [c.data.getName() for c in conditions]
    if names == ['Never', 'OnCollision']:
        conditions.pop(0)
    return conditions

class EventGroup(object):
    global_id = None
    local_id = None
    generated = False
    or_generated = False
    or_exit = None
    is_container_mark = False
    or_first = False
    or_save = False
    or_final = False
    or_type = None
    event_name = None
    ids = None
    written = False
    write_callbacks = None
    data_hash = None

    def __init__(self, conditions, actions, container, global_id,
                 or_index, not_always, or_type):
        self.conditions = fix_conditions(conditions)
        self.actions = actions
        for ace_list in (conditions, actions):
            for ace in ace_list:
                ace.group = self
        self.container = container
        self.config = {}
        self.global_id = global_id
        self.not_always = not_always
        self.or_index = or_index
        self.or_type = or_type
        self.force_multiple = set()
        self.ids = {}

    def get_id(self, obj):
        key = hash(obj)
        try:
            return self.ids[key]
        except KeyError:
            pass
        new_id = '%s_%s' % (TEMPORARY_GROUP_ID, len(self.ids))
        self.ids[key] = new_id
        return new_id

    def set_groups(self, converter, groups):
        is_simple = hacks.use_simple_or(converter)
        if len(groups) == 1:
            self.name = 'event_%s' % TEMPORARY_GROUP_ID
            self.unique_id = TEMPORARY_GROUP_ID
            return
        self.unique_id = '%s_%s' % (TEMPORARY_GROUP_ID, self.or_index)
        if is_simple:
            self.name = 'event_or_%s' % self.unique_id
            return
        first_group = None
        last_group = None
        for group in groups:
            if group.or_generated == self.or_generated:
                if first_group is None:
                    first_group = group
                last_group = group
        if last_group is first_group:
            self.name = 'event_or_%s' % self.unique_id
            return
        # use first group as data store
        if first_group is self:
            self.or_first = True
            self.or_selected = {}
            self.or_instance_groups = defaultdict(list)
        else:
            self.or_selected = first_group.or_selected
            self.or_instance_groups = first_group.or_instance_groups

        if last_group is self:
            self.or_final = True
        else:
            self.or_save = True
            self.actions = []
        self.name = self.get_or_name()
        # self.or_exit = 'goto %s_end;' % last_group.get_or_name()
        self.or_result = 'or_result_%s' % TEMPORARY_GROUP_ID
        self.or_temp_result = 'or_temp_result_%s_%s' % (TEMPORARY_GROUP_ID,
                                                        self.or_index)

    def get_or_name(self):
        return 'or_event_%s' % self.unique_id

    def set_generated(self, value):
        self.generated = value

    def set_or_generated(self, value):
        self.or_generated = value

    def add_write_callback(self, f, *arg, **kw):
        if self.written:
            f(*arg, **kw)
            return
        if self.write_callbacks is None:
            self.write_callbacks = []
        self.write_callbacks.append((f, arg, kw))

    def add_member(self, writer, name, default=None):
        self.add_write_callback(self.on_add_member, writer, name, default)

    def on_add_member(self, writer, name, default):
        name = self.filter(name)
        writer.add_member(name, default)

    def filter(self, data):
        if self.data_hash is None:
            raise NotImplementedError()
        data = data.replace(TEMPORARY_GROUP_ID, str(self.global_id))
        hash_str = str(self.data_hash & 0xFFFFFFFF)
        data = data.replace(TEMPORARY_HASH_ID, hash_str)
        return data

    def fire_write_callbacks(self):
        self.written = True
        if not self.write_callbacks:
            return
        for (f, arg, kw) in self.write_callbacks:
            f(*arg, **kw)
        self.write_callbacks = None

class ObjectFileWriter(CodeWriter):
    index = 1
    object_count = 0

    def __init__(self, converter, max_count=100):
        self.max_count = max_count
        self.converter = converter
        self.open_next()

    def open_next(self):
        filename = 'objects%s.cpp' % self.index
        filename = self.converter.get_filename(filename)
        if self.fp:
            CodeWriter.close(self)
        self.open(filename)
        self.putln('#include "objects.h"')
        self.putln('#include "common.h"')
        self.putln('#include "fonts.h"')
        self.putln('#include "font.h"')
        self.putln('#include "lists.h"')
        self.putln('')
        self.index += 1

    def next(self):
        self.object_count += 1
        if self.object_count >= self.max_count:
            self.object_count = 0
            self.open_next()


class MultiFileWriter(CodeWriter):
    index = 1
    function_count = 0

    def __init__(self, converter, max_count=100):
        self.max_count = max_count
        self.converter = converter
        self.members = {}

        self.class_name = self.get_class()

        header_path = '%s.h' % self.get_filename()
        self.header = converter.open_code(header_path)

        self.header.start_guard(self.get_guard())
        self.write_header_includes()
        self.header.putln('')
        self.header.putclass(self.class_name, self.base_class)
        self.header.put_access('public')

        self.open_next()

    def get_guard(self):
        return 'CHOWDREN_%s_H' % self.get_filename().upper()

    def close(self):
        CodeWriter.close(self)
        self.header.end_brace(True)
        self.header.close_guard(self.get_guard())
        self.header.close()

    def add_member(self, name, default=None):
        if name in self.members:
            return
        self.members[name] = default
        self.header.putlnc('%s;', name)

    def open_next(self):
        filename = '%s_%s.cpp' % (self.get_filename(), self.index)
        filename = self.converter.get_filename(filename)
        if self.fp:
            CodeWriter.close(self)
        self.open(filename)
        self.write_includes()
        self.putln('')
        self.index += 1

    def putmeth(self, name, *arg, **kw):
        self.function_count += 1
        if self.function_count >= self.max_count:
            self.open_next()
            self.function_count = 0
        fullarg = list(arg)
        prototype = '%s(%s)' % (name, ', '.join(fullarg))
        self.add_member(prototype)
        # add FrameXX::method qualifier
        name = name.split(' ')
        name[-1] = '%s::%s' % (self.class_name, name[-1])
        try:
            name.remove('static')
        except ValueError:
            pass
        name = ' '.join(name)
        CodeWriter.putmeth(self, name, *arg, **kw)

class EventFileWriter(MultiFileWriter):
    base_class = 'Frame'

    def get_filename(self):
        return 'events'

    def get_class(self):
        return 'Frames'

    def write_header_includes(self):
        self.header.putln('#include "frame.h"')

    def write_includes(self):
        self.putln('#include "events.h"')
        self.putln('#include "framedata.h"')
        self.putln('#include "lists.h"')
        self.putln('#include "media.h"')
        self.putln('#include "objects.h"')

class FrameDataWriter(MultiFileWriter):
    base_class = 'FrameData'

    def __init__(self, frame_index, *arg, **kw):
        self.frame_index = frame_index
        MultiFileWriter.__init__(self, *arg, **kw)

    def get_filename(self):
        return 'frame%s' % self.frame_index

    def get_class(self):
        return 'Frame%s' % self.frame_index

    def write_header_includes(self):
        self.header.putln('#include "frame.h"')

    def write_includes(self):
        self.putln('#include "events.h"')
        self.putlnc('#include "%s.h"', self.get_filename())

def fix_sound(data, extension):
    data = str(data)
    if extension != 'wav':
        return data
    wav = wave.open(StringIO(data), 'rb')
    channels, width, rate, frame_count, comptype, compname = wav.getparams()
    if width == 2:
        return data
    frames = wav.readframes(frame_count)
    wav.close()
    if width == 3:
        new_frames = ''
        for i in xrange(frame_count*channels):
            new_frames += frames[i*3+1:i*3+3]
        frames = new_frames
    else:
        if width == 1:
            frames = audioop.bias(frames, 1, 128)
        frames = audioop.lin2lin(frames, width, 2)
    fp = StringIO()
    wav = wave.open(fp, 'wb')
    wav.setparams((channels, 2, rate, frame_count, comptype, compname))
    wav.writeframesraw(frames)
    wav.close()
    return fp.getvalue()


class Converter(object):
    debug = False
    def __init__(self, filename, outdir, image_file='Sprites.dat',
                 win_ico=None, mac_icns=None, company=None,
                 version=None, copyright=None, base_only=False,
                 copy_shaders=False):
        self.filename = filename
        self.outdir = outdir

        # copy base
        if base_only:
            return

        self.has_single_selection = {}
        self.has_selection = {}
        self.container_tree = []
        self.collision_objects = set()
        self.current_object = None
        self.iterated_index = self.iterated_object = self.iterated_name = None
        self.in_actions = False
        self.strings = {}
        self.event_functions = {}
        self.event_wrappers = {}
        self.objs_to_qualifier = {}

        fp = open(filename, 'rb')
        if filename.endswith('.exe'):
            exe = ExecutableData(ByteReader(fp), loadFrames = False)
            game = exe.gameData
        elif filename.endswith('.ccn'):
            game = GameData(ByteReader(fp))
        elif filename.endswith('.ccj'):
            game = GameData(ByteReader(fp), java = True)
        else:
            raise NotImplementedError('invalid extension')

        self.game = game

        if win_ico is not None:
            shutil.copy(win_ico, self.get_filename('icon.ico'))
        if mac_icns is not None:
            shutil.copy(mac_icns, self.get_filename('icon.icns'))

        # set up directory structure
        makedirs(outdir)
        makedirs(os.path.join(outdir, 'build'))

        # application info
        company = company or game.author
        version = version or '1.0.0.0'
        version_number = ', '.join(version.split('.'))
        copyright = copyright or game.author
        self.base_path = get_base_path()
        self.info_dict = dict(company = company, version = version,
            copyright = copyright, description = game.name,
            version_number = version_number, name = game.name,
            base_path = self.base_path.replace('\\', '/'))
        hacks.init(self)

        if copy_shaders:
            src = os.path.join(self.base_path, 'shaders')
            dst = self.get_filename('shaders')
            copytree(src, dst)

        # format input
        self.format_file('resource.rc')
        self.format_file('Application.cmake', 'CMakeLists.txt')
        self.copy_file('icon.icns', overwrite=False)

        self.write_config(self.info_dict, 'config.py')

        # fonts
        if WRITE_FONTS:
            fonts_header = self.open_code('fonts.h')
            fonts_header.putln('#include "common.h"')
            fonts_header.start_guard('CHOWDREN_FONTS_H')

            fonts_file = self.open_code('fonts.cpp')
            fonts_file.putln('#include "fonts.h"')
            all_fonts = []
            if game.fonts:
                for font in game.fonts.items:
                    logfont = font.value
                    font_name = 'font%s' % font.handle
                    all_fonts.append(font_name)
                    fonts_file.putlnc('Font %s = Font(%r, %s, %s, %s, %s);',
                                      font_name, logfont.faceName,
                                      logfont.getSize(),
                                      logfont.isBold(), bool(logfont.italic),
                                      bool(logfont.underline), cpp=False)
                    fonts_header.putln('extern Font %s;' % font_name)
            fonts_header.close_guard('FONTS_H')
            fonts_file.close()
            fonts_header.close()

        # images

        # print 'Image count:', len(game.images.items)

        if image_file is None:
            config = self.read_config('images.py')
            self.solid_images = config['solid']
            self.image_indexes = config['indexes']
        else:
            self.solid_images = {}
            self.image_indexes = {}
            image_hashes = {}
            new_entries = []
            image_fp = open(self.get_filename(image_file), 'wb')
            image_data = ByteReader()
            image_header = ByteReader(image_fp)
            if game.images:
                image_index = 0
                for image in game.images.itemDict.itervalues():
                    pil_image = Image.fromstring('RGBA', (image.width,
                        image.height), image.getImageData())

                    handle = image.handle
                    colors = pil_image.getcolors(1)
                    if colors is not None:
                        color, = colors
                        self.solid_images[handle] = color[1]

                    image_hash = hash(pil_image.tobytes())

                    try:
                        alias = image_hashes[image_hash]
                        self.image_indexes[handle] = alias
                        continue
                    except KeyError:
                        pass

                    self.image_indexes[handle] = image_index
                    image_hashes[image_hash] = image_index

                    temp = StringIO()
                    pil_image.save(temp, 'PNG')
                    temp = temp.getvalue()
                    new_entries.append(temp)

                    image_index += 1

                print 'image count:', len(new_entries)

                image_header.writeInt(len(new_entries), True)
                header_size = 4 * (len(new_entries) + 1)

                for i, data in enumerate(new_entries):
                    image_header.writeInt(image_data.tell() + header_size,
                                          True)
                    image_data.writeInt(image.xHotspot)
                    image_data.writeInt(image.yHotspot)
                    image_data.writeInt(image.actionX)
                    image_data.writeInt(image.actionY)
                    image_data.write(data)

            image_header.writeReader(image_data)
            image_fp.close()

            config = {'solid': self.solid_images,
                      'indexes': self.image_indexes}
            self.write_config(config, 'images.py')

        # sounds
        if WRITE_SOUNDS:
            dir_created = False
            sounds_file = self.open_code('sounds.h')
            sounds_file.putln('#include "common.h"')
            sounds_file.start_guard('SOUNDS_H')
            sounds_file.putmeth('void setup_sounds', 'Media * media')

            if game.sounds:
                for sound in game.sounds.items:
                    sound_type = sound.getType()
                    extension = {'OGG' : 'ogg', 'WAV' : 'wav', 'MOD' : 'mp3'}[
                        sound_type]
                    filename = '%s.%s' % (sound.name, extension)
                    data = fix_sound(sound.data, extension)
                    if not dir_created:
                        makedirs(self.get_filename('sounds'))
                        dir_created = True
                    self.open('sounds', filename).write(data)
                    sounds_file.putln(to_c('media->add_cache(%r, %r);',
                        sound.name, filename))
            sounds_file.end_brace()
            sounds_file.close_guard('SOUNDS_H')
            sounds_file.putln('')
            sounds_file.close()

        objects_header = self.open_code('objects.h')
        objects_header.start_guard('CHOWDREN_OBJECTS_H')
        objects_header.putln('#include "frameobject.h"')
        objects_header.putln('')

        objects_file = ObjectFileWriter(self)

        lists_header = self.open_code('lists.h')
        lists_header.start_guard('CHOWDREN_LISTS_H')
        lists_header.putln('#include "objects.h"')
        lists_header.putln('void global_object_update(float dt);')

        lists_file = self.open_code('lists.cpp')
        lists_file.putln('#include "lists.h"')
        lists_file.putln('#include "manager.h"')
        lists_file.putln('#include "frameobject.h"')
        lists_file.putln('#include "common.h"')

        self.object_names = {}
        self.all_objects = {}
        self.object_cache = {}
        self.instance_names = {}
        self.name_to_item = {}
        self.object_types = {}
        extension_includes = set()
        extension_sources = set()
        self.event_callback_ids = itertools.count()
        application_writers = set()

        type_id = itertools.count(2)
        qualifier_id = itertools.count(1)

        for frameitem in game.frameItems.items:
            name = frameitem.name
            self.name_to_item[name] = frameitem
            handle = (frameitem.handle, frameitem.objectType)
            if name is None:
                class_name = 'Object%s' % handle[0]
            else:
                class_name = get_class_name(name) + '_' + str(handle[0])
            object_type = frameitem.properties.objectType
            try:
                object_writer = self.get_object_writer(object_type)(
                    self, frameitem)
            except (KeyError, AttributeError, NotImplementedError):
                print 'not implemented:', repr(frameitem.name), object_type,
                print handle
                continue
            object_writer.new_class_name = class_name
            object_writer.object_type = object_type
            for define in object_writer.defines:
                extra.add_define(define)
            application_writers.add(object_writer.write_application)
            self.all_objects[handle] = object_writer
            self.object_types[handle[0]] = object_type
            extension_includes.update(object_writer.get_includes())
            extension_sources.update(object_writer.get_sources())
            if (object_writer.static or extra.is_special_object(name)
                or object_writer.class_name == 'Undefined'):
                continue

            if object_writer.is_static_background():
                object_type_id = 'BACKGROUND_TYPE'
            else:
                object_type_id = '%s_type' % class_name

            object_writer.type_id = object_type_id

            object_code = self.write_object(object_writer).get_data()
            object_hash = hash(object_code.replace(class_name, ''))

            try:
                other = self.object_cache[object_hash]
                class_name = other.new_class_name
                self.all_objects[handle] = other
                self.object_types[handle[0]] = other.object_type
                self.object_names[handle] = class_name
                self.instance_names[handle] = get_method_name(class_name)
                continue
            except KeyError:
                pass

            self.object_cache[object_hash] = object_writer

            for line in object_code.splitlines():
                objects_file.putln(line)

            self.object_names[handle] = class_name
            self.instance_names[handle] = get_method_name(class_name)

            if not object_writer.is_static_background():
                objects_header.putln('#define %s %s' %
                    (object_type_id, type_id.next()))
                list_name = self.get_object_list(handle)
                list_ref = 'GameManager::instances.items[%s]' % object_type_id
                lists_header.putlnc('#define %s %s', list_name, list_ref)

            object_func = 'create_%s' % get_method_name(class_name)

            objects_header.putln('FrameObject * %s(int x, int y);'
                                 % object_func)

            objects_file.putmeth('FrameObject * %s' % object_func,
                                 'int x', 'int y')
            objects_file.putln('return new %s(x, y);' % class_name)
            objects_file.end_brace()

            object_writer.write_post(objects_file)

            objects_file.next()

        self.max_type_id = type_id.next()
        self.max_qualifier_id = qualifier_id.next()

        objects_header.close_guard('CHOWDREN_OBJECTS_H')
        objects_file.close()
        objects_header.close()

        extensions_header = self.open_code('extensions.h')
        extensions_header.start_guard('CHOWDREN_EXTENSIONS_H')
        for include in extension_includes:
            extensions_header.putln('#include "%s"' % include)
        extensions_header.close_guard('CHOWDREN_EXTENSIONS_H')
        extensions_header.close()

        extensions_file = self.open_code('extensions.cpp')
        extensions_file.putln('#include "extensions.h"')
        for include in extension_sources:
            extensions_file.putln('#include "%s"' % include)
        extensions_file.close()

        # write object updates
        update_calls = []
        for handle in self.object_names.keys():
            writer = self.all_objects[handle]
            has_updates = writer.has_updates()
            has_movements = writer.has_movements()
            if not has_updates and not has_movements:
                continue
            func_name = 'update_%s' % handle[0]
            update_calls.append(func_name)
            lists_file.putmeth('inline void %s' % func_name, 'float dt')
            lists_file.start_brace()
            lists_file.putln('ObjectList::iterator it;')
            list_name = self.get_object_list(handle)
            lists_file.putlnc('for (it = %s.begin(); it != %s.end(); it++) {',
                              list_name, list_name)
            lists_file.indent()
            lists_file.putln('FrameObject * instance = it->obj;')
            lists_file.putln('if (instance->flags & DESTROYING)')
            lists_file.indent()
            lists_file.putln('continue;')
            lists_file.dedent()
            if has_updates:
                lists_file.putlnc('((%s*)instance)->update(dt);',
                                  writer.class_name)
            if has_movements:
                lists_file.putln('if (instance->movement)')
                lists_file.indent()
                lists_file.putln('instance->movement->update(dt);')
                lists_file.dedent()
            lists_file.end_brace()
            lists_file.end_brace()
            lists_file.end_brace()

        lists_file.putmeth('void global_object_update', 'float dt')
        for call in update_calls:
            lists_file.putlnc('%s(dt);', call)
        lists_file.end_brace()

        processed_frames = []

        frame_dict = dict(enumerate(game.frames))
        frame_dict = hacks.get_frames(self, frame_dict)

        event_file = EventFileWriter(self)
        event_file.putmeth('Frames', 'GameManager * manager',
                           init_list=['Frame(manager)'])
        event_file.end_brace()
        events_ref = '((Frames*)frame)->'

        # frames
        for frame_index, frame in frame_dict.iteritems():
            self.event_callbacks = {}
            self.container_ids = Counter()
            frame_file = FrameDataWriter(frame_index+1, self)
            frame_class_name = self.frame_class = frame_file.class_name
            frame.load()
            self.current_frame = frame
            self.current_frame_index = frame_index
            processed_frames.append(frame_index + 1)

            startup_instances = []
            self.multiple_instances = set()
            startup_set = set()
            object_writers = []

            self.system_object = SystemObject(self)
            object_writers.append(self.system_object)

            for instance in getattr(frame.instances, 'items', ()):
                frameitem = instance.getObjectInfo(game.frameItems)
                obj = (frameitem.handle, frameitem.objectType)
                try:
                    object_writer = self.all_objects[obj]
                    if object_writer not in object_writers:
                        object_writers.append(object_writer)
                except KeyError:
                    continue
                if object_writer.static:
                    continue
                if obj not in self.object_names:
                    continue
                common = frameitem.properties.loader
                try:
                    create_startup = not common.flags['DoNotCreateAtStart']
                except AttributeError:
                    create_startup = True
                if create_startup and instance.parentType == NONE_PARENT:
                    if frameitem in startup_set:
                        self.multiple_instances.add(obj)
                    startup_set.add(frameitem)
                    startup_instances.append((instance, frameitem))
                if not create_startup:
                    self.multiple_instances.add(obj)

            events = frame.events

            self.qualifier_names = {}
            self.qualifiers = {}
            self.qualifier_types = {}
            for qualifier in events.qualifier_list:
                qual_obj = (qualifier.objectInfo, qualifier.type)
                object_infos = qualifier.resolve_objects(self.game.frameItems)
                object_infos = tuple([(info, qualifier.type)
                                      for info in object_infos])
                self.qualifiers[qual_obj] = object_infos
                self.qualifier_types[qual_obj] = qualifier.type
                if object_infos in self.objs_to_qualifier:
                    name = self.objs_to_qualifier[object_infos]
                else:
                    name = 'qualifier_%s' % len(self.objs_to_qualifier)
                    self.objs_to_qualifier[object_infos] = name
                    lists_header.putlnc('extern QualifierList %s;', name)
                    instances = []
                    for obj in object_infos:
                        instances.append('&' + self.get_object_list(obj))
                    instances = ', '.join(instances)
                    lists_file.putlnc('// qualifier %s %s',
                                      qualifier.objectInfo,
                                      qualifier.qualifier)
                    lists_file.putlnc('QualifierList %s(%s, %s);', name,
                                      len(object_infos), instances)

                self.qualifier_names[qual_obj] = name

            generated_groups = self.generated_groups = defaultdict(list)
            always_groups = []
            always_groups_dict = self.always_groups_dict = defaultdict(list)
            action_dict = self.action_dict = defaultdict(list)
            current_container = None
            self.containers = containers = {}
            changed_containers = set()

            event_items = events.items

            group_index = 1
            for group in event_items:
                first_condition = group.conditions[0]
                name = self.get_condition_name(first_condition)
                if name == 'NewGroup':
                    group_loader = first_condition.items[0].loader
                    new_container = EventContainer(group_loader.name,
                        group_loader.flags['Inactive'], current_container,
                        self)
                    if current_container:
                        current_container.add_child(new_container)
                    current_container = new_container
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
                not_always = None
                or_type = None
                for condition in (group.conditions + [None]):
                    if condition is None:
                        name = 'End'
                    else:
                        name = condition.getName()
                    if name in ('OrFiltered', 'OrLogical', 'End'):
                        if name != 'End':
                            or_type = name
                        if not_always:
                            conditions.append(not_always)
                            not_always = None
                        condition_groups.append(conditions)
                        conditions = []
                    else:
                        condition_writer = self.get_condition_writer(condition)
                        condition_writer.container = current_container
                        condition_writer.group = group
                        if name == 'NotAlways':
                            # move NotAlways to end of condition list
                            not_always = condition_writer
                        else:
                            conditions.append(condition_writer)

                actions = []
                for action in group.actions:
                    action_writer = self.get_action_writer(action)
                    action_writer.container = current_container
                    actions.append(action_writer)

                    action_name = self.get_action_name(action)
                    action_dict[action_name].append(action_writer)
                    if action_name in ('CreateObject', 'Shoot',
                                       'DisplayText', 'Destroy'):
                        if action_name == 'Destroy':
                            create_info = (action.objectInfo,
                                           action.objectType)
                        else:
                            info = action.items[0].loader.objectInfo

                            create_info = (info, self.object_types[info])
                        if create_info[0] == 0xFFFF:
                            # the case for DisplayText
                            create_info = (action.objectInfo,
                                           action.objectType)
                        create_infos = self.resolve_qualifier(create_info)
                        for create_info in create_infos:
                            if self.all_objects[create_info].static:
                                continue
                            self.multiple_instances.add(create_info)
                    elif action_name in ('DeactivateGroup',
                                         'ActivateGroup'):
                        pointer = action.items[0].loader.pointer
                        changed_containers.add(pointer)

                always_group_index = None
                generated_group_index = None
                new_always_groups = []
                all_groups = []
                for or_index, conditions in enumerate(condition_groups):
                    first_writer = conditions[0]
                    first_condition = first_writer.data

                    is_always = first_condition.flags['Always']
                    force_always = (first_writer.is_always is not None and
                                    first_writer.is_always)

                    if is_always:
                        if always_group_index is None:
                            always_group_index = group_index
                            group_index += 1
                        new_group_index = always_group_index
                    else:
                        if generated_group_index is None:
                            generated_group_index = group_index
                            group_index += 1
                        new_group_index = generated_group_index

                    new_group = EventGroup(conditions, actions,
                        current_container, new_group_index,
                        or_index, not_always, or_type)
                    all_groups.append(new_group)
                    name = self.get_condition_name(first_condition)

                    new_group.set_or_generated(not is_always)

                    if is_always or force_always:
                        new_always_groups.append(new_group)
                        new_group.local_id = len(always_groups_dict[name])+1

                        for cond in new_group.conditions:
                            name = self.get_condition_name(cond.data)
                            always_groups_dict[name].append(new_group)
                    else:
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

                new_always_groups.sort(key=lambda x: x.global_id)
                always_groups.extend(new_always_groups)

                for group in all_groups:
                    group.set_groups(self, all_groups)

            for k, v in containers.iteritems():
                if k not in changed_containers:
                    if v.inactive:
                        # print v.name, 'is never activated!'
                        continue
                    v.is_static = True

            for container in containers.values():
                if container.is_static:
                    continue
                event_file.add_member('bool %s' % container.code_name,
                                      not container.inactive)

            # since fire_write_callbacks can be called whenever, we need to
            # keep an initialization writer handy
            start_name = 'on_frame_%s_start' % (frame_index + 1)
            start_writer = self.start_writer = CodeWriter()
            start_writer.add_member = event_file.add_member

            frame_file.putmeth(frame_class_name, 'Frame * frame',
                               init_list=['FrameData(frame)'])
            frame_file.putlnc('name = %r;', frame.name)
            frame_file.end_brace()

            startup_images = set()
            # object writer custom stuff
            for object_writer in object_writers:
                object_writer.write_frame(event_file)
                startup_images.update(object_writer.get_images())

            # print 'Startup images:', len(startup_images)

            frame_file.putmeth('void on_start')
            frame_file.putlnc('%s%s();', events_ref, start_name)
            frame_file.end_brace()

            # frame setup
            frame_width = frame.width
            frame_height = frame.height
            virtual_right = frame.right
            if virtual_right == -1:
                virtual_right = 0x7FFFF000
            virtual_bottom = frame.bottom
            if virtual_bottom == -1:
                virtual_bottom = 0x7FFFF000
            virtual_width = virtual_right - frame.left
            virtual_height = virtual_bottom - frame.top
            if virtual_width not in (0x7FFFF000, frame_width):
                raise NotImplementedError()
            if virtual_height not in (0x7FFFF000, frame_height):
                raise NotImplementedError()
            start_writer.putlnc('index = %s;', frame_index)
            start_writer.putlnc('width = %s;', frame_width)
            start_writer.putlnc('height = %s;', frame_height)
            start_writer.putlnc('virtual_width = %s;', virtual_width)
            start_writer.putlnc('virtual_height = %s;', virtual_height)
            start_writer.putlnc('background_color = %s;',
                                make_color(frame.background))
            if frame.flags['TimedMovements']:
                timer_base = frame.movementTimer
            else:
                timer_base = 0
            start_writer.putln('timer_base = %s;' % timer_base)

            # load images on startup
            if False:
                start_writer.putln('static bool images_initialized = false;')
                start_writer.putln('if (!images_initialized) {')
                start_writer.indent()
                start_writer.putln('images_initialized = true;')
                for image_handle in startup_images:
                    img = game.images.itemDict[image_handle]
                    start_writer.putln('%s->load(true);' %
                                       self.get_image(image_handle))
                start_writer.end_brace()


            start_writer.putlnc('layers.resize(%s);', len(frame.layers.items))

            for layer_index, layer in enumerate(frame.layers.items):
                visible = not layer.flags['ToHide']
                wrap_horizontal = layer.flags['WrapHorizontally']
                wrap_vertical = layer.flags['WrapVertically']
                start_writer.putlnc('layers[%s].init(%s, %s, %s, %s, %s, %s);',
                                    layer_index, layer_index,
                                    layer.xCoefficient, layer.yCoefficient,
                                    visible, wrap_horizontal, wrap_vertical)

            start_writer.putraw('#if defined(CHOWDREN_IS_WIIU) || '
                                'defined(CHOWDREN_EMULATE_WIIU)')
            for layer_index, layer in enumerate(frame.layers.items):
                if layer.name.endswith('(DRC)'):
                    remote_type = 'CHOWDREN_REMOTE_TARGET'
                elif layer.name.endswith('(Hybrid)'):
                    remote_type = 'CHOWDREN_HYBRID_TARGET'
                else:
                    continue
                start_writer.putlnc('layers[%s]->set_remote(%s);', layer_index,
                                    remote_type)
            start_writer.putraw('#endif')

            startup_instances = hacks.get_startup_instances(self,
                                                            startup_instances)

            for instance, frameitem in startup_instances:
                obj = (frameitem.handle, frameitem.objectType)
                object_writer = self.all_objects[obj]
                if object_writer.is_background():
                    method = 'add_background_object'
                    object_func = 'create_%s' % get_method_name(
                        self.object_names[obj])
                    start_writer.putlnc('%s(%s(%s, %s), %s);',
                                        method, object_func, instance.x,
                                        instance.y, instance.layer)
                    continue
                self.create_object(obj, instance.x, instance.y,
                                   instance.layer, None, start_writer)

            for object_writer in object_writers:
                object_writer.write_start(start_writer)

            frame_end_groups = generated_groups.pop('EndOfFrame', None)

            if frame_end_groups:
                end_name = 'on_frame_%s_end' % (frame_index + 1)
                end_name = self.write_generated(end_name, event_file,
                                                frame_end_groups)

                frame_file.putmeth('void on_end')
                frame_file.putlnc('%s%s();', events_ref, end_name)
                frame_file.putlnc('%sreset();', events_ref)
                frame_file.end_brace()

            # write event callbacks
            frame_file.putmeth('void event_callback', 'int id')
            if self.event_callbacks:
                frame_file.putln('switch (id) {')
                frame_file.indent()
                for event_id, name in self.event_callbacks.iteritems():
                    frame_file.putln('case %s:' % event_id)
                    frame_file.indent()
                    frame_file.putlnc('%s%s();', events_ref, name)
                    frame_file.putln('break;')
                    frame_file.dedent()
                frame_file.end_brace()
            frame_file.end_brace()

            # write 'always' event subfunctions
            group_index = 0
            call_groups = []
            first_groups = []

            while True:
                try:
                    group = always_groups[group_index]
                except IndexError:
                    break

                call_groups.append(group)

                if group.is_container_mark:
                    group_index += 1
                    continue

                or_groups = []
                while True:
                    try:
                        new_group = always_groups[group_index]
                    except IndexError:
                        break
                    if (new_group.is_container_mark or
                            new_group.global_id != group.global_id):
                        break

                    or_groups.append(new_group)
                    group_index += 1

                self.write_event_function(event_file, or_groups)

            # write main 'always' handler
            handle_name = 'handle_frame_%s_events' % (frame_index+1)
            frame_file.putmeth('void handle_events')
            frame_file.putlnc('%s%s();', events_ref, handle_name)
            frame_file.end_brace()

            event_file.putmeth('void %s' % handle_name)

            end_markers = []

            call_groups = first_groups + call_groups

            for group in call_groups:
                if group.is_container_mark:
                    container = group.container
                    if container.is_static:
                        continue
                    if group.mark == 'NewGroup':
                        event_file.putln('if (!%s) goto %s;' % (
                            container.code_name, container.end_label))
                        end_markers.insert(0, container.end_label)
                        self.container_tree.insert(0, container)
                    elif group.mark == 'GroupEnd':
                        end_markers.remove(container.end_label)
                        event_file.put_label(container.end_label)
                        self.container_tree.remove(container)
                    continue
                event_file.putlnc('%s();', group.event_name)
            for end_marker in end_markers:
                event_file.put_label(end_marker)

            event_file.end_brace()

            # write any added defaults
            for member_name, member_default in event_file.members.iteritems():
                if member_default is None:
                    continue
                member_name = member_name.rsplit(' ', 1)[-1]
                start_writer.putlnc('%s = %s;', member_name, member_default)

            event_start_name = None
            start_groups = generated_groups.pop('StartOfFrame', None)
            if start_groups:
                event_start_name = 'on_frame_%s_start_events'
                event_start_name = event_start_name % (frame_index + 1)
                event_start_name = self.write_generated(event_start_name,
                                                        event_file,
                                                        start_groups)
                start_writer.putlnc('%s();', event_start_name)
            fade = frame.fadeIn
            if fade:
                if fade.duration == 0:
                    print 'invalid fade duration:', fade.duration
                    fade.duration = 0.00001
                start_writer.putlnc('manager->set_fade(%s, %s);',
                    make_color(fade.color), -1.0 / (fade.duration / 1000.0))

            event_file.putmeth('void %s' % start_name)
            event_file.putcode(start_writer)
            event_file.end_brace()

            frame_file.close()
            frame.close()

            if generated_groups:
                missing_groups = []
                for k, v in generated_groups.iteritems():
                    if isinstance(k, str):
                        missing_groups.append(k)
                        continue
                    object_type, num = k
                    object_class = self.get_object_class(object_type)
                    missing_groups.append((object_class, num))
                print 'unimplemented generated groups in %r: %r' % (
                    frame.name, missing_groups)

        event_file.putmeth('void set_index', 'int index')
        for frame_index in processed_frames:
            event_file.putlnc('static Frame%s frame%s(this);', frame_index,
                              frame_index)
        event_file.putln('switch (index) {')
        event_file.indent()
        for frame_index in processed_frames:
            event_file.putlnc('case %s:', frame_index - 1)
            event_file.indent()
            event_file.putlnc('data = &frame%s;', frame_index)
            event_file.putln('break;')
            event_file.dedent()
        event_file.putln('default:')
        event_file.indent()
        event_file.putln('data = NULL;')
        event_file.putln('break;')
        event_file.dedent()
        event_file.end_brace()
        event_file.end_brace()

        event_file.close()

        lists_header.close_guard('CHOWDREN_LISTS_H')
        lists_header.close()

        lists_file.close()

        header = game.header

        frames_file = self.open_code('framedata.h')
        frames_file.putln('#include "common.h"')
        for frame_index in processed_frames:
            frames_file.putlnc('#include "frame%s.h"', frame_index)
        frames_file.putln('')

        frames_file.putmeth('inline void setup_globals',
            'GlobalValues * values', 'GlobalStrings * strings')
        if game.globalValues:
            for index, value in enumerate(game.globalValues.items):
                frames_file.putln('values->set(%s, %s);' % (index, value))
        if game.globalStrings:
            for index, value in enumerate(game.globalStrings.items):
                frames_file.putln(to_c('strings->set(%s, %r);', index,
                    value))
        frames_file.end_brace()

        frames_file.putmeth('inline void setup_keys', 'GameManager * manager')
        controls = self.game.header.controls.items[0]
        is_keyboard = controls.getControlType() == 'Keyboard'

        if is_keyboard:
            frames_file.putln('manager->control_type = CONTROL_KEYBOARD;')
        else:
            frames_file.putln('manager->control_type = CONTROL_JOYSTICK1;')

        for name in ('up', 'down', 'left', 'right', 'button1', 'button2',
                     'button3', 'button4'):
            key = getattr(controls.keys, name)
            key = convert_key(key.getValue())
            frames_file.putlnc('manager->%s = %s;', name, key)

        frames_file.end_brace()

        frames_file.close()

        strings_file = self.open_code('strings.cpp')
        strings_header = self.open_code('strings.h')
        strings_header.start_guard('CHOWDREN_STRINGS_H')
        strings_header.putln('#include <string>')
        for value, name in self.strings.iteritems():
            strings_file.putlnc('const std::string %s(%r, %s);', name, value,
                                len(value), cpp=False)
            strings_header.putlnc('extern const std::string %s;', name)
        strings_header.close_guard('CHOWDREN_STRINGS_H')
        strings_file.close()
        strings_header.close()

        for writer in application_writers:
            writer(self)

        # general configuration
        # this is the last thing we should do
        config_file = self.open_code('chowconfig.h')
        config_file.start_guard("CHOWDREN_CONFIG_H")

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
        config_file.putdefine('FRAMERATE', header.frameRate)
        config_file.putdefine('MAX_OBJECT_ID', self.max_type_id)
        if header.newFlags['SamplesOverFrames']:
            config_file.putln('#define CHOWDREN_SAMPLES_OVER_FRAMES')
        if header.newFlags['VSync']:
            config_file.putln('#define CHOWDREN_VSYNC')
        if PROFILE:
            config_file.putdefine('CHOWDREN_USE_PROFILER', '')
        hacks.write_defines(self, config_file)
        extra.write_defines(self, config_file)

        config_file.close_guard("CHOWDREN_CONFIG_H")
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

    def write_object(self, object_writer):
        class_name = object_writer.new_class_name
        object_type_id = object_writer.type_id
        frameitem = object_writer.data
        common = object_writer.common
        subclass = object_writer.class_name
        objects_file = CodeWriter()
        object_writer.write_pre(objects_file)
        objects_file.putclass(class_name, subclass)
        objects_file.put_access('public')
        object_writer.write_constants(objects_file)
        parameters = ['x', 'y', object_type_id]
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
        objects_file.putraw('#ifndef NDEBUG')
        objects_file.putln(to_c('name = %r;', frameitem.name))
        objects_file.putraw('#endif')

        if object_writer.is_background():
            objects_file.putln('flags |= BACKGROUND;')

        if not object_writer.is_visible():
            objects_file.putln('set_visible(false);')

        if not object_writer.is_scrolling():
            objects_file.putln('flags &= ~SCROLL;')

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
                if object_writer.has_color:
                    objects_file.putlnc('blend_color.a = %s;', a)
                else:
                    objects_file.putln('blend_color = %s;' % make_color(
                        (r, g, b, a)))
                ink_effect &= ~HWA_EFFECT
            if ink_effect == NONE_EFFECT:
                pass
            elif ink_effect == SHADER_EFFECT:
                shader_data = self.game.shaders.items[frameitem.shaderId]
                shader_name = shader_data.name
            elif ink_effect == SEMITRANSPARENT_EFFECT:
                a = min(255, (128 - frameitem.inkEffectValue) * 2)
                if object_writer.has_color:
                    objects_file.putlnc('blend_color.a = %s;', a)
                else:
                    objects_file.putlnc('blend_color = %s;', make_color(
                        (255, 255, 255, a)))
            elif ink_effect in INK_EFFECTS:
                shader_name = INK_EFFECTS[ink_effect]
            elif shader_name is None:
                raise NotImplementedError(
                    'unknown inkeffect: %s' % ink_effect)
        if shader_name is not None:
            objects_file.putln('set_shader(%s);' % shader.get_name(
                shader_name))
        if ink_effect == SHADER_EFFECT:
            shader_data = self.game.shaders.items[frameitem.shaderId]
            parameters = shader_data.get_parameters()
            values = frameitem.items
            if shader_data.parameters is not None:
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
                    elif parameter.type == IMAGE:
                        value = self.get_image(reader.readShort(), False)
                    else:
                        print 'shader parameter type not supported:',
                        print parameter.type
                        continue
                        raise NotImplementedError
                    parameters[parameter.name].value = value
            for name, value in parameters.iteritems():
                objects_file.putln(to_c('set_shader_parameter(%r, %s);',
                    name, value.value))

        if hasattr(common, 'movements') and common.movements:
            movements = common.movements.items
            self.write_movements(objects_file, object_writer, movements)

        object_writer.load_alterables(objects_file)
        objects_file.end_brace()

        object_writer.write_class(objects_file)
        object_writer.write_internal_class(objects_file)
        # write event callbacks
        if object_writer.event_callbacks:
            for v in object_writer.event_callbacks.iteritems():
                name, event_id = v
                objects_file.putmeth('void %s' % name)
                objects_file.putln('frame->event_callback(%s);' % event_id)
                objects_file.end_brace()

        if object_writer.has_dtor():
            objects_file.putmeth('~%s' % class_name)
            object_writer.write_dtor(objects_file)
            objects_file.end_brace()

        if PROFILE:
            if object_writer.update:
                objects_file.putmeth('void update', 'float dt')
                objects_file.putlnc('PROFILE_BLOCK(%s_update);',
                                    class_name)
                objects_file.putlnc('%s::update(dt);', subclass)
                objects_file.end_brace()
            objects_file.putmeth('void draw')
            objects_file.putlnc('PROFILE_BLOCK(%s_draw);', class_name)
            objects_file.putlnc('%s::draw();', subclass)
            objects_file.end_brace()

        objects_file.end_brace(True)
        return objects_file

    def get_image(self, value, pointer=True):
        value = self.image_indexes[value]
        ret = 'get_internal_image(%s)' % value
        if not pointer:
            ret = '(*%s)' % ret
        return ret

    def iterate_groups(self, groups):
        index = 0

        while True:
            try:
                group = groups[index]
            except IndexError:
                break

            if group.is_container_mark:
                index += 1
                continue

            or_groups = []
            while True:
                try:
                    new_group = groups[index]
                except IndexError:
                    break
                if (new_group.is_container_mark or
                        new_group.global_id != group.global_id):
                    break

                or_groups.append(new_group)
                index += 1

            yield or_groups

    @contextlib.contextmanager
    def iterate_object(self, *arg, **kw):
        self.start_object_iteration(*arg, **kw)
        yield
        self.end_object_iteration(*arg, **kw)

    def start_object_iteration(self, object_info, writer, name='it',
                               copy=True):
        iter_type = get_iter_type(object_info)
        getter = self.create_list(object_info, writer)
        if copy:
            list_name = 'extra_%s' % self.get_object_list(object_info)
            list_type = get_list_type(object_info)
            writer.putlnc('static %s %s;', list_type, list_name)
            writer.putlnc('%s.copy(%s);', list_name, getter)
        else:
            list_name = getter
        self.set_iterator(object_info, list_name, '*' + name)
        writer.putlnc('for (%s %s(%s); !%s.end(); %s++) {',
                      iter_type, name, list_name, name, name)
        writer.indent()

    def end_object_iteration(self, object_info, writer, name = 'it',
                             copy=True):
        writer.end_brace()
        self.set_iterator(None)

    def set_iterator(self, object_info, object_list=None, name='*it'):
        self.iterated_object = object_info
        # self.iterated_list = object_list
        if object_info is None:
            self.iterated_name = None
            self.iterated_index = None
            return
        self.iterated_name = name
        if object_list is None:
            self.iterated_index = '0'
        else:
            it = name.replace('*', '')
            self.iterated_index = '%s.current_index' % it

    def create_object(self, obj, x, y, layer, object_name, writer):
        name = self.get_object_name(obj)
        obj_create_func = 'create_%s' % get_method_name(name)
        arguments = [str(x), str(y)]
        if object_name:
            prefix = '%s = ' % object_name
        else:
            prefix = ''
        writer.putlnc('%sadd_object(%s(%s), %s);', prefix,
                      obj_create_func, ', '.join(arguments), layer)

    def begin_events(self):
        pass

    def set_object(self, obj, name):
        self.has_single_selection[obj] = name

    def set_list(self, obj, name):
        self.has_selection[obj] = name

    def clear_selection(self):
        self.has_selection = {}

    def write_container_check(self, group, writer):
        container = group.container
        if group.generated or not container:
            return
        for item in container.tree:
            if item.is_static:
                continue
            writer.putln('if (!%s) goto %s;' % (
                item.code_name, item.end_label))

    def get_container_check(self, container):
        groups = []
        for item in container.tree:
            if item.is_static:
                continue
            groups.append(item.code_name)
        if not groups:
            return 'true'
        return ' && '.join(groups)

    def write_movements(self, writer, obj, movements):
        count = len(movements)
        has_list = count > 1
        if has_list:
            writer.putlnc('movement_count = %s;', count)
            writer.putlnc('movements = new Movement*[%s];', count)
        has_init = False
        for movement_index, movement in enumerate(movements):
            def set_start_dir(direction):
                if movement_index != 0 or not direction:
                    return
                writer.putlnc('set_direction(%s);',
                              parse_direction(direction))

            movement_name = movement.getName()
            if movement_name == 'Extension':
                movement_name = movement.loader.name
            set_start_dir(movement.directionAtStart)
            sets = {}
            extra = []
            if movement_name == 'Ball':
                movement_class = 'BallMovement'
                sets['max_speed'] = movement.loader.speed
            elif movement_name == 'Path':
                movement_class = 'PathMovement'
                path = movement.loader
                loop = bool(path.loop)
                reposition = bool(path.repositionAtEnd)
                reverse = bool(path.reverseAtEnd)
                end_x = end_y = 0
                for i, step in enumerate(path.steps):
                    if reposition:
                        end_x += step.destinationX
                        end_y += step.destinationY
                    args = []
                    args.append(step.speed)
                    args.append(step.destinationX)
                    args.append(step.destinationY)
                    args.append(step.cosinus)
                    args.append(step.sinus)
                    args.append(step.length)
                    args.append(step.direction)
                    args.append(step.pause)
                    args = [to_c('%r', arg) for arg in args]
                    extra.append('add_node(%s)' % ', '.join(args))
                    if step.name:
                        extra.append(to_c('add_named_node(%s, %r)', i,
                                          step.name))
                sets['path'] = to_c('%s, %s, %s, %s', loop, reverse,
                                    end_x, end_y)
            elif movement_name == 'EightDirections':
                movement_class = 'EightDirections'
                sets['max_speed'] = movement.loader.speed
                sets['deceleration'] = movement.loader.deceleration
                sets['acceleration'] = movement.loader.acceleration
            elif movement_name == 'pinball':
                movement_class = 'PinballMovement'
                data = movement.loader.data
                data.skipBytes(1)
                speed = data.readInt()
                deceleration = data.readInt()
                gravity = data.readInt()
                set_start_dir(data.readInt())
                flags = data.readInt()
                sets['max_speed'] = speed
                sets['deceleration'] = deceleration
                sets['gravity'] = gravity
            else:
                if movement_name != 'Static':
                    print 'movement', movement_name, 'not implemented'
                if not has_list:
                    break
                movement_class = 'StaticMovement'
            if has_list:
                name = 'movements[%s]' % movement_index
            else:
                name = 'movement'
            has_init = True
            writer.putlnc('%s = new %s(this);', name, movement_class)
            name = '((%s*)%s)' % (movement_class, name)
            for k, v in sets.iteritems():
                writer.putlnc('%s->set_%s(%s);', name, k, v)
            for line in extra:
                writer.putlnc('%s->%s;', name, line)
        if has_init:
            if has_list:
                writer.putlnc('set_movement(0);')
            else:
                writer.putlnc('movement->start();')
            obj.movement_count = count
        else:
            obj.movement_count = 0

    def write_event(self, writer, group, triggered=False):
        data = self.get_event_code(group, triggered)
        group.data_hash = hash(data)
        group.fire_write_callbacks()
        data = group.filter(data)

        for line in data.splitlines():
            writer.putln(line)

    def write_event_function(self, writer, groups, triggered=False):
        data = ''
        for group in groups:
            data += self.get_event_code(group, triggered)
        event_hash = hash(data)
        for group in groups:
            group.data_hash = event_hash

        try:
            event_name = self.event_functions[event_hash]
            groups[0].event_name = event_name
            return event_name
        except KeyError:
            pass

        for group in groups:
            group.fire_write_callbacks()

        data = group.filter(data)

        event_name = 'event_func_%s' % (len(self.event_functions) + 1)
        groups[0].event_name = event_name
        self.event_functions[event_hash] = event_name

        writer.putmeth('void %s' % event_name)
        for line in data.splitlines():
            writer.putln(line)
        writer.end_brace()
        return event_name

    def write_events(self, name, writer, groups, triggered=False):
        names = []

        for new_groups in self.iterate_groups(groups):
            names.append(self.write_event_function(writer, new_groups,
                                                   triggered))

        wrapper_hash = hash(tuple(names))
        try:
            return self.event_wrappers[wrapper_hash]
        except KeyError:
            pass

        self.event_wrappers[wrapper_hash] = name

        writer.putmeth('void %s' % name)
        for event_name in names:
            writer.putlnc('%s();', event_name)
        writer.end_brace()
        return name

    def write_generated(self, name, writer, groups):
        return self.write_events(name, writer, groups, True)

    def get_event_code(self, group, triggered=False):
        self.current_group = group
        self.current_event_id = group.global_id
        self.event_settings = {}
        actions = group.actions
        conditions = group.conditions
        container = group.container
        has_container_check = False
        if container:
            is_static = all([item.is_static for item in container.tree])
            has_container_check = not is_static
        if triggered:
            conditions = conditions[1:]
        writer = CodeWriter()
        writer.putln('// event %s' % TEMPORARY_GROUP_ID)
        event_break = self.event_break = 'goto %s_end;' % group.name

        if group.or_first:
            writer.putln('bool %s = false;' % group.or_result)
        if group.or_first or group.or_save or group.or_final:
            writer.putln('bool %s = false;' % group.or_temp_result)

        writer.start_brace() # new scope

        if PROFILE:
            writer.putlnc('PROFILE_BLOCK(event_%s);', group.global_id)

        hacks.write_pre(self, writer, group)
        if conditions or has_container_check:
            if has_container_check:
                condition = self.get_container_check(container)
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
                obj = condition_writer.get_object()
                object_info, object_type = obj
                self.current_object = obj
                if object_info is not None:
                    try:
                        object_name = self.get_object(obj)
                    except KeyError:
                        pass
                if obj in self.has_single_selection:
                    object_name = self.has_single_selection[obj]
                    self.set_iterator(obj, None, object_name)
                elif object_name is not None and self.has_multiple_instances(
                        obj):
                    selected_name = self.create_list(obj, writer)
                    if condition_writer.iterate_objects is not False:
                        has_multiple = True
                        self.set_iterator(obj, selected_name)
                        iter_type = get_iter_type(obj)
                        writer.putlnc('for (%s it(%s); !it.end(); '
                                      'it++) {', iter_type, selected_name)
                        writer.indent()
                        object_name = '(*it)'
                writer.putindent()
                if negated:
                    writer.put('if (!(')
                else:
                    writer.put('if (')
                writer.put(condition_writer.prefix)
                if object_name is None:
                    if condition_writer.static:
                        writer.put('%s::' % self.get_object_class(
                            object_type, star = False))
                elif condition_writer.iterate_objects is not False:
                    if condition_writer.negate:
                        writer.put('!')
                    if has_multiple:
                        obj = '((%s)%s)' % (self.get_object_class(
                            object_type), object_name)
                    else:
                        obj = object_name
                    if condition_writer.dereference:
                        obj += '->'
                    writer.put(obj)
                condition_writer.write(writer)
                if negated:
                    writer.put(')')
                if has_multiple:
                    writer.put(') it.deselect();\n')
                    writer.end_brace()
                    writer.indented = False
                    writer.putln('if (!%s.has_selection()) %s' % (selected_name,
                        event_break))
                    self.set_iterator(None)
                else:
                    writer.put(') %s\n' % event_break)

        if group.or_final:
            writer.putlnc('%s = true;', group.or_result)
            writer.putlnc('%s = true;', group.or_temp_result)
            writer.putlnc('%s_end: ;', group.name)
            writer.end_brace()
            self.write_instance_save(group, writer)
            writer.putlnc('if (!%s) goto or_final_%s_end;', group.or_result,
                          group.name)
            self.has_selection = {}
            new_selection = {}

            is_logical = group.or_type == 'OrLogical'

            for obj, or_bools in group.or_instance_groups.iteritems():
                list_name = self.get_object_list(obj)
                new_selection[obj] = list_name
                or_list = group.or_selected[obj]
                check = '%s' % (' || '.join(or_bools))
                writer.putlnc('if (%s) {', check)
                writer.indent()
                writer.putlnc('%s.restore(%s);', or_list, list_name)
                # blah blah copy over
                writer.dedent()
                writer.putlnc('} else {')
                writer.indent()
                instance_list = self.get_object(obj, True)
                if is_logical:
                    writer.putlnc('%s.clear_selection();', list_name)
                else:
                    writer.putlnc('%s.empty_selection();', list_name)
                writer.end_brace()
            writer.start_brace()
            self.has_selection = new_selection # group.or_selected

        self.in_actions = True

        for action_writer in actions:
            if action_writer.custom:
                action_writer.write(writer)
                continue
            has_multiple = False
            has_single = False
            obj = action_writer.get_object()
            object_info, object_type = obj
            if object_info is not None and not is_qualifier(object_info):
                is_static = self.all_objects[obj].static
                if is_static and action_writer.ignore_static:
                    continue
            self.current_object = obj
            if object_info is not None:
                if obj in self.has_single_selection:
                    has_single = True
                elif self.has_multiple_instances(obj):
                    has_multiple = True
                elif obj not in self.object_names:
                    object_info = None
            if action_writer.iterate_objects is False:
                has_multiple = False
                object_info = None
            object_name = None
            if has_single:
                object_name = self.has_single_selection[obj]
            elif has_multiple:
                list_name = self.create_list(obj, writer)
                iter_type = get_iter_type(obj)
                writer.putlnc('for (%s it(%s); !it.end(); '
                              'it++) {', iter_type, list_name)
                writer.indent()
                self.set_iterator(obj, list_name)
                object_name = '*it'
            elif object_info is not None:
                writer.putindent()
                writer.put('%s->' % self.get_object(obj))
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
                self.set_iterator(None)
        for action_writer in actions:
            action_writer.write_post(writer)
        # if group.or_exit:
        #     writer.putln(group.or_exit)

        self.in_actions = False

        if group.or_save:
            writer.putlnc('%s = true;', group.or_result)
            writer.putlnc('%s = true;', group.or_temp_result)

        writer.end_brace()
        if group.or_final:
            writer.putln('or_final_%s_end: ;' % group.name)
        else:
            writer.putln('%s_end: ;' % group.name)

        if group.or_save:
            self.write_instance_save(group, writer)

        self.has_single_selection = {}
        self.has_selection = {}
        self.collision_objects = set()
        self.current_object = None
        self.current_group = None
        self.current_event_id = None

        return writer.get_data()

    def write_instance_save(self, group, writer):
        if not self.has_selection:
            return
        if_first = set()
        for obj, list_name in self.has_selection.iteritems():
            group.or_instance_groups[obj].append(group.or_temp_result)
            new_list = group.or_selected.get(obj, None)
            if new_list is not None:
                continue
            new_list = 'or_%s_%s' % (list_name, TEMPORARY_GROUP_ID)
            group.or_selected[obj] = new_list
            if is_qualifier(obj[0]):
                class_name = 'SavedQualifierSelection'
            else:
                class_name = 'SavedSelection'
            writer.putlnc('static %s %s;', class_name, new_list)
            writer.putlnc('%s.clear();', new_list)

        writer.putlnc('if (%s) {', group.or_temp_result)
        writer.indent()
        for obj, list_name in self.has_selection.iteritems():
            new_list = group.or_selected[obj]
            writer.putlnc('%s.add(%s);', new_list,
                          self.get_object_list(obj))

        writer.end_brace()

    def convert_static_expression(self, items, start=0, end=-1):
        expressions = items[start:end]
        out = ''
        for item in expressions:
            name = item.getName()
            if name == 'String':
                out += item.loader.value
            elif name == 'Long':
                out += str(item.loader.value)
            elif name == 'Plus':
                continue
            elif name == 'AlterableString':
                object_info = (item.objectInfo, item.objectType)
                index = item.loader.value
                writer = self.all_objects[object_info]
                value = writer.common.strings.items[index]
                out += value
            else:
                return None
        return out

    def get_string_expressions(self, parameter):
        loader = parameter.loader
        strings = []
        for item in loader.items:
            if item.getName() != 'String':
                continue
            strings.append(item.loader.value)
        return strings

    def convert_parameter(self, container):
        loader = container.loader
        out = ''
        self.start_clauses = self.end_clauses = 0
        parameter_type = container.getName()

        if loader.isExpression:
            self.expression_items = loader.items[:-1]
            self.item_index = 0
            while self.item_index < len(self.expression_items):
                item = self.expression_items[self.item_index]
                expression_writer = self.get_expression_writer(item)
                obj = expression_writer.get_object()
                object_info, object_type = obj
                self.current_object = obj
                if expression_writer.static:
                    out += '%s::' % self.get_object_class(object_type,
                        star = False)
                elif object_info is not None:
                    try:
                        out += '%s->' % self.get_object(obj, use_default=True)
                    except KeyError:
                        pass
                self.last_out = out
                out += expression_writer.get_string()
                self.item_index += 1

            if parameter_type == 'VARGLOBAL_EXP':
                out = '-1 + ' + out
        else:
            parameter_name = type(container.loader).__name__
            if parameter_name == 'Object':
                return self.get_object((loader.objectInfo, loader.objectType))
            elif parameter_name == 'Sample':
                return loader.name
            elif parameter_name in ('Position', 'Shoot', 'Create'):
                if parameter_name != 'Position':
                    obj = (loader.objectInfo,
                           self.object_types[loader.objectInfo])
                    if self.all_objects[obj].static:
                        return None
                details = {}
                if parameter_name == 'Position':
                    position = loader
                elif parameter_name == 'Shoot':
                    details['shoot_speed'] = loader.shootSpeed
                    details['shoot_object'] = (loader.objectInfo,
                                               loader.objectType)
                elif parameter_name == 'Create':
                    create_info = (loader.objectInfo,
                                   self.object_types[loader.objectInfo])
                    details['create_object'] = create_info
                if parameter_name in ('Shoot', 'Create'):
                    position = loader.position
                flags = position.flags
                details['direction'] = position.direction
                if flags['Action']:
                    details['use_action_point'] = True
                if flags['Direction']:
                    details['transform_position_direction'] = True
                if flags['InitialDirection']:
                    details['use_direction'] = True
                if flags['DefaultDirection']:
                    details['use_default_direction'] = True
                if position.objectInfoParent != 0xFFFF:
                    details['parent'] = (position.objectInfoParent,
                                         position.typeParent)
                details['layer'] = position.layer
                details['x'] = position.x
                details['y'] = position.y
                return details
            elif parameter_name == 'KeyParameter':
                return convert_key(loader.key.getValue())
            elif parameter_name == 'Zone':
                return repr((loader.x1, loader.y1, loader.x2, loader.y2))
            elif parameter_name == 'Time':
                return repr(loader.timer)
            elif parameter_name == 'Every':
                return repr(loader.delay)
            elif parameter_name == 'Click':
                button = loader.getButton()
                if button == 'Left':
                    return convert_key(1)
                if button == 'Right':
                    return convert_key(2)
                elif button == 'Middle':
                    return convert_key(4)
            elif parameter_name in ('Colour'):
                return make_color(loader.value)
            elif parameter_name in ('Int', 'Short'):
                if parameter_type == 'NEWDIRECTION':
                    return parse_direction(loader.value)
                elif parameter_type == 'TEXTNUMBER':
                    return loader.value + 1
                return loader.value
            elif parameter_name == 'Extension':
                return loader.get_reader()
            elif parameter_name in ('String', 'Filename'):
                # self.start_clauses -= loader.value.count('(')
                # self.end_clauses -= loader.value.count(')')
                return self.intern_string(loader.value)
            elif parameter_name in ('TwoShorts'):
                return to_c('%r, %r', loader.value1, loader.value2)
            else:
                raise NotImplementedError('parameter: %s' % parameter_name)
        self.start_clauses += out.count('(')
        self.end_clauses += out.count(')')
        for _ in xrange(self.start_clauses - self.end_clauses):
            out += ')'

        if self.end_clauses > self.start_clauses:
            # MMF expression bug
            remove = self.end_clauses - self.start_clauses

            if remove != 1:
                print out
                raise NotImplementedError()

            end = len(out)

            while True:
                end = out.rindex('),', 0, end)
                if out[end-1] != '(':
                    break

            filtered = out[end+1:]
            out = out[:end+1]
            # print 'too many end-clauses, filtered', filtered
        return out

    def intern_string(self, value):
        if value == '':
            return 'empty_string'
        try:
            return self.strings[value]
        except KeyError:
            pass
        name = 'str_%s_%s' % (get_method_name(value), len(self.strings))
        self.strings[value] = name
        return name

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
        if is_qualifier(handle[0]):
            return True
        if handle in self.multiple_instances:
            return True
        if handle in self.current_group.force_multiple:
            return True
        if self.current_group.or_type == 'OrFiltered':
            return True
        return False

    def create_list(self, object_info, writer):
        single = self.get_single(object_info)
        if single is not None:
            return single
        list_name = self.get_object_list(object_info)
        if object_info in self.has_selection:
            return list_name
        writer.putlnc('%s.clear_selection();', list_name)
        self.set_list(object_info, list_name)
        return list_name

    def get_single(self, obj):
        if obj in self.has_single_selection:
            return self.has_single_selection[obj]
        elif obj == self.iterated_object:
            return self.iterated_name
        return None

    def get_object(self, obj, as_list=False, use_default=False, index=None):
        handle = obj[0]
        object_type = self.get_object_class(obj[1])
        use_index = (hacks.use_iteration_index(self) and self.iterated_index
                     and self.in_actions) or index is not None
        index = index or self.iterated_index
        if obj in self.has_single_selection:
            return self.has_single_selection[obj]
        if self.iterated_object == obj:
            return '((%s)%s)' % (object_type, self.iterated_name)
        elif obj in self.has_selection:
            ret = self.has_selection[obj]
            if not as_list:
                args = [ret]
                if use_index:
                    args.append(index)
                args = ', '.join(args)
                ret = '((%s)get_single(%s))' % (object_type, args)
            return ret
        else:
            name = self.get_object_list(obj)
            if as_list:
                self.set_list(obj, name)
                return '%s.clear_selection()' % name
            type_id, is_qual = self.get_object_handle(obj)
            args = [name]
            if use_index:
                args.append(index)

            if use_default:
                default_instance = self.get_default_instance(obj[1])
                if default_instance:
                    args.append(default_instance)

            if is_qual:
                getter_name = 'get_qualifier'
            else:
                getter_name = 'get_instance'

            args = ', '.join(args)
            return '((%s)%s(%s))' % (object_type, getter_name, args)

    def get_default_instance(self, object_type):
        return self.get_object_writer(object_type).default_instance

    def get_object_list(self, obj, allow_single=False):
        if self.get_single(obj) is not None and not allow_single:
            raise NotImplementedError()
        type_id, is_qual = self.get_object_handle(obj)
        if is_qual:
            return type_id
        return self.get_list_name(self.get_object_name(obj))

    def filter_object_type(self, obj):
        if is_qualifier(obj[0]):
            return obj
        if obj in self.all_objects:
            return obj
        print 'getting real object type for obj', obj
        obj_type = self.object_types[obj[0]]
        new_obj = (obj[0], obj_type)
        return new_obj

    def get_object_name(self, obj):
        if is_qualifier(obj[0]):
            return self.qualifier_names[obj]
        obj = self.filter_object_type(obj)
        return self.object_names[obj]

    def get_list_name(self, object_name):
        return '%s_instances' % get_method_name(object_name, True)

    def get_type_name(self, object_type):
        name = getObjectType(object_type)
        if name == 'Extension':
            ext_index = object_type - EXTENSION_BASE
            ext = self.game.extensions.fromHandle(ext_index)
            name = ext.name
        return name

    def get_object_handle(self, obj):
        if is_qualifier(obj[0]):
            return (self.qualifier_names[obj], True)
        obj = self.filter_object_type(obj)
        try:
            return ('%s_type' % self.object_names[obj], False)
        except Exception, e:
            data = self.all_objects[obj].data
            name = self.get_type_name(data.objectType)
            raise e

    def get_object_class(self, object_type, star=True):
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

    def resolve_qualifier(self, obj):
        if not is_qualifier(obj[0]):
            return [obj]
        return self.qualifiers[obj]

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
            obj = (item.objectInfo, item.objectType)
            object_writer = self.all_objects.get(obj, None)
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

    def write_config(self, data, *path):
        with open(self.get_filename(*path), 'wb') as fp:
            fp.write(repr(data))

    def read_config(self, *path):
        with open(self.get_filename(*path), 'rb') as fp:
            data = fp.read()
        return eval(data)

    def get_filename(self, *path):
        return os.path.join(self.outdir, *path)

    def copy_file(self, src, dst=None, overwrite=True):
        dst = self.get_filename(dst or src)
        src = os.path.join(self.base_path, src)
        if not overwrite and os.path.isfile(dst):
            return
        shutil.copy(src, dst)

    def format_file(self, src, dst=None):
        dst = self.get_filename(dst or src)
        src = os.path.join(self.base_path, src)
        fp = open(src, 'rb')
        data = fp.read()
        fp.close()
        data = data % self.info_dict
        fp = open(dst, 'wb')
        fp.write(data)
        fp.close()
