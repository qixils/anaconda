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

class TextBlitter(ObjectWriter):
    class_name = 'TextBlitter'

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
        something2 = data.readByte()
        data.skipBytes(3)
        writer.putln('width = %s;' % width)
        writer.putln('height = %s;' % height)
        writer.putln('char_width = %s;' % char_size[0])
        writer.putln('char_height = %s;' % char_size[1])
        writer.putln(to_c('text = %r;', text))
        writer.putln('image = %s;' % get_image_name(image))
        writer.putln('static int * charmap = new int[256];')
        writer.putln('static bool initialized = false;')
        writer.putln('this->charmap = charmap;')
        writer.putln('if (!initialized) {')
        writer.indent()
        writer.putln('initialized = true;')
        writer.putln(to_c('initialize(%r);', character_map))
        writer.end_brace()
        # other data
        # data.openEditor()


actions = make_table(ActionMethodWriter, {
    0 : 'set_text'
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
})

def get_object():
    return TextBlitter
