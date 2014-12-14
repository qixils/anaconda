"""
Assets file

IMAGE_COUNT uint16 handles, in the order of which to preload
IMAGE_COUNT offsets for each image
SOUND_COUNT offsets for each sound
FONT_COUNT offsets for each font
SHADER_COUNT offsets for each shader

Images:
    X, Y hotspot (short)
    X, Y action point (short)
    PNG image

Sounds:
    uint32 type
    uint32 size
    WAV/OGG data

Fonts:
    ... (see fontgen.py)

Shaders:
    data (could be uint32 vert + data, uint32 frag + data)
"""

NONE_TYPE, WAV_TYPE, OGG_TYPE, NATIVE_TYPE = xrange(4)

AUDIO_TYPES = {
    'wav': WAV_TYPE,
    'ogg': OGG_TYPE
}

import os
from chowdren.shader import get_shader_programs
from chowdren.common import get_method_name
from mmfparser.bytereader import ByteReader

def get_asset_name(typ, name, index=None):
    name = get_method_name(name).upper()
    if index is None:
        return '%s_%s' % (typ, name)
    return '%s_%s_%s' % (typ, name, index)

class Assets(object):
    def __init__(self, converter, skip=False):
        self.skip = skip
        if skip:
            return

        self.converter = converter
        self.header = converter.open_code('assets.h')
        self.header.start_guard('CHOWDREN_ASSETS_H')

        self.images = []
        self.sounds = []
        self.fonts = []
        self.shaders = []
        self.shader_names = set()

        self.sound_ids = {}

        self.path = self.converter.get_filename('Assets.dat')

        self.fp = open(self.path, 'wb')

    def write_data(self):
        header = ByteReader()
        data = ByteReader()
        header_size = (len(self.images) + len(self.sounds) + len(self.fonts) +
                       len(self.shaders)) * 4 + len(self.images) * 2

        # image preload
        self.use_count_offset = header.tell()
        for _ in xrange(len(self.images)):
            header.writeShort(0, True)

        for image in self.images:
            header.writeInt(data.tell() + header_size, True)
            data.write(image)

        for sound in self.sounds:
            header.writeInt(data.tell() + header_size, True)
            data.write(sound)

        for font in self.fonts:
            header.writeInt(data.tell() + header_size, True)
            data.write(font)

        for shader in self.shaders:
            header.writeInt(data.tell() + header_size, True)
            data.write(shader)

        self.fp.write(str(header))
        self.fp.write(str(data))

        self.image_count = len(self.images)
        self.sound_count = len(self.sounds)
        self.font_count = len(self.fonts)
        self.shader_count = len(self.shaders)

        self.sounds = self.images = self.fonts = self.shaders = None

    def write_cache(self, cache):
        cache['sound_ids'] = self.sound_ids

    def load_cache(self, cache):
        self.sound_ids = cache['sound_ids']

    def write_preload(self, images):
        self.fp.seek(self.use_count_offset)
        data = ByteReader()
        for handle in images:
            data.writeShort(handle, True)
        for handle in xrange(self.image_count):
            if handle in images:
                continue
            data.writeShort(handle, True)
        self.fp.write(str(data))

    def close(self):
        if self.skip:
            return
        self.fp.close()

    def write_header(self):
        if self.skip:
            return
        self.header.putln('')
        self.header.putdefine('IMAGE_COUNT', self.image_count)
        self.header.putdefine('SOUND_COUNT', self.sound_count)
        self.header.putdefine('FONT_COUNT', self.font_count)
        self.header.putdefine('SHADER_COUNT', self.shader_count)
        self.header.close_guard('CHOWDREN_ASSETS_H')
        self.header.close()

    def add_shader(self, name, data):
        if self.skip:
            return
        if name in self.shader_names:
            return
        self.shader_names.add(name)
        shader_id = get_asset_name('SHADER', name)
        if data is None:
            self.header.putlnc('#define %s %s', shader_id, 'INVALID_ASSET_ID')
            return
        index = len(self.shaders)
        self.header.putdefine(shader_id, index)
        self.shaders.append(data)

    def add_sound(self, name, ext=None, data=None):
        if ext is None:
            audio_type = NONE_TYPE
        else:
            audio_type = AUDIO_TYPES.get(ext, NATIVE_TYPE)

        index = len(self.sounds)
        sound_id = get_asset_name('SOUND', name, index)
        self.sound_ids[name.lower()] = sound_id
        self.header.putdefine(sound_id, index)

        writer = ByteReader()
        writer.writeInt(audio_type)

        if data:
            writer.writeIntString(data)

        self.sounds.append(str(writer))

    def add_font(self, name, data):
        self.fonts.append(data)

    def get_sound_id(self, name):
        return self.sound_ids.get(name.lower(), 'INVALID_ASSET_ID')

    def add_image(self, hot_x, hot_y, act_x, act_y, data):
        writer = ByteReader()
        writer.writeShort(hot_x)
        writer.writeShort(hot_y)
        writer.writeShort(act_x)
        writer.writeShort(act_y)
        writer.writeIntString(data)
        self.images.append(str(writer))
