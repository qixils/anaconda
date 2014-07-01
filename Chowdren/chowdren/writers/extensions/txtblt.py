from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

from chowdren.writers.events import (ComparisonWriter, ActionMethodWriter,
    ConditionMethodWriter, ExpressionMethodWriter, make_table)

def fix_string(v):
    new = ''
    for c in v:
        if ord(c) < 10:
            break
        new += c
    return new

ANIMATION_NAMES = {
    0 : 'None',
    1 : 'Marquee',
    2 : 'Sin Wave',
    3 : 'Sin Scroller',
    4 : 'Cos Scroller',
    5 : 'Tan Scroller'
}

ANIM_NONE = 0
ANIM_MARQUEE = 1
ANIM_FLAG = 2
ANIM_SINSCROLL = 3
ANIM_COSSCROLL = 4
ANIM_TANSCROLL = 5

FLAGS_CALLBACK = 2048
FLAGS_CALLBACK_ONBEGIN = 4096
FLAGS_CALLBACK_ONEND = 8192
FLAGS_CALLBACK_ONBEGINL = 16384
FLAGS_CALLBACK_ONENDL = 32768
FLAGS_CALLBACK_ONCHAR = 65536
FLAGS_BALLOON = 131072
FLAGS_BALLOON_HINVERT = 262144
FLAGS_BALLOON_KEEPSCREEN = 524288
FLAGS_ANIM_EDGEFADE = 1
FLAGS_TRANSPARENT = 1
FLAGS_USETEXTFUNCS = 2
FLAGS_DRAWVERTICLE = 4
FLAGS_DEBUGON = 8
FLAGS_DRAWCHARINTERUPT = 16
FLAGS_WORDWRAPPING = 32
FLAGS_MULTILINE = 64
FLAGS_NOREDRAW = 128
FLAGS_VARIABLEWIDTH = 256
FLAGS_EDGEFADE = 1024

class TextBlitter(ObjectWriter):
    class_name = 'TextBlitter'
    use_alterables = True
    update = True
    defines = ['CHOWDREN_USE_BLITTER']

    def write_init(self, writer):
        data = self.get_data()
        data.skipBytes(4)
        width = data.readShort()
        height = data.readShort()
        data.skipBytes(128)
        data.skipBytes(4)
        text = fix_string(data.read(1024))
        data.skipBytes(256)
        trans_color = data.readColor()
        char_size = (data.readInt(), data.readInt())
        char_spacing = (data.readInt(), data.readInt())
        char_offset = data.readInt() % 255
        image_size = (data.readInt(), data.readInt())
        image_offset = (data.readInt(), data.readInt())
        flags = data.readInt()
        tab_width = data.readInt()
        image = data.readShort()
        character_map = fix_string(data.read(256))
        data.skipBytes(256)
        data.skipBytes(4096)
        data.skipBytes(1024)
        data.skipBytes(2)
        left = data.readInt()
        top = data.readInt()
        right = data.readInt()
        bottom = data.readInt()
        horizontal_align = data.readInt()
        vertical_align = data.readInt()
        data.skipBytes(4)

        # read animation
        animation_type = data.readByte()
        type_name = ANIMATION_NAMES[animation_type]

        if type_name not in ('None', 'Sin Wave'):
            raise NotImplementedError('invalid blitter animation: %s'
                                      % type_name)
        data.skipBytes(3)
        animation_speed = data.readInt()
        speed_count = data.readInt()
        param = [data.readInt() for _ in xrange(16)]
        options = data.readInt()
        p1 = data.readInt()

        if type_name == 'Sin Wave':
            writer.putln('anim_type = BLITTER_ANIMATION_SINWAVE;')
            writer.putlnc('anim_frame = 0;')
            writer.putlnc('anim_speed = %s;', animation_speed)
            writer.putlnc('wave_freq = %s;', param[1])
            writer.putlnc('wave_height = %s;', param[2])

        ball_left = data.readInt()
        ball_top = data.readInt()
        ball_right = data.readInt()
        ball_bottom = data.readInt()
        ball_source = (data.readInt(), data.readInt())
        ball_min = (data.readInt(), data.readInt())
        ball_max = (data.readInt(), data.readInt())

        writer.putln('width = %s;' % width)
        writer.putln('height = %s;' % height)
        writer.putln('char_width = %s;' % char_size[0])
        writer.putln('char_height = %s;' % char_size[1])
        writer.putlnc('char_offset = %s;', char_offset)
        # writer.putlnc('off_x = %s;', image_offset[0])
        # writer.putlnc('off_y = %s;', image_offset[1])
        writer.putln('image = %s;' % get_image_name(image))
        align_flags = []
        if horizontal_align == 0:
            align_flags.append('ALIGN_LEFT')
        elif horizontal_align == 1:
            align_flags.append('ALIGN_HCENTER')
        elif horizontal_align == 2:
            align_flags.append('ALIGN_RIGHT')
        if vertical_align == 0:
            align_flags.append('ALIGN_TOP')
        elif vertical_align == 1:
            align_flags.append('ALIGN_VCENTER')
        elif vertical_align == 2:
            align_flags.append('ALIGN_BOTTOM')
        if not align_flags:
            align_flags.append('0')
        writer.putln('alignment = %s;' % ' | '.join(align_flags))
        writer.putln('static int charmap[256];')
        writer.putlnc('static std::string charmap_str = %r;', character_map)
        writer.putln('static bool initialized = false;')
        writer.putln('this->charmap = &charmap[0];')
        writer.putln('this->charmap_str = &charmap_str;')
        writer.putln('if (!initialized) {')
        writer.indent()
        writer.putln('initialized = true;')
        writer.putln('initialize(charmap_str);')
        writer.end_brace()
        # other data
        # data.openEditor()
        if flags & FLAGS_TRANSPARENT:
            writer.putln('has_transparent = true;')
            writer.putlnc('transparent_color = %s;', make_color(trans_color))
        writer.putlnc('set_text(%r);', text)

    def is_static_background(self):
        return False


class ASCIIValue(ExpressionMethodWriter):
    has_object = False
    method = 'get_ascii'


actions = make_table(ActionMethodWriter, {
    0 : 'set_text',
    4 : '.char_width = %s',
    5 : '.char_height = %s',
    6 : '.char_offset = %s',
    7 : 'set_charmap',
    13 : 'load',
    36 : 'set_x_align',
    37 : 'set_y_align',
    42 : 'set_x_scroll',
    43 : 'set_y_scroll',
    44 : 'set_x_spacing',
    45 : 'set_y_spacing',
    51 : 'set_animation_type',
    52 : 'set_animation_parameter',
    56 : 'set_transparent_color',
    57 : 'replace_color',
    58 : 'set_width',
    59 : 'set_height'
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
    0 : 'get_text()',
    4 : 'get_charmap()',
    9 : 'get_x_align()',
    10 : 'get_y_align()',
    16 : '.y_scroll',
    18 : '.y_spacing',
    21 : '.width',
    22 : '.height',
    23 : ASCIIValue,
    32 : 'get_line_count()',
    33 : 'get_line',
    42 : 'get_map_char',
    46 : '.callback_line_count'
})

def get_object():
    return TextBlitter
