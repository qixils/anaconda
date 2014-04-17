from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

from chowdren.writers.events import (ComparisonWriter, ActionMethodWriter,
    ConditionMethodWriter, ExpressionMethodWriter, make_table)

from mmfparser.bitdict import BitDict
from mmfparser.data.font import LogFont

class KcList(ObjectWriter):
    class_name = 'ListObject'

    def write_init(self, writer):
        data = self.get_data()
        flags = BitDict(
            'FreeFlag',
            'VerticalScrollbar',
            'Sort',
            'Border',
            'HideOnStart',
            'SystemColor',
            '3DLook',
            'ScrollToNewline'
        )
        width = data.readShort()
        height = data.readShort()
        # XXX support unicode
        font = LogFont(data, old = True)
        font_color = data.readColor()
        data.readString(40)
        data.skipBytes(16 * 4)
        back_color = data.readColor()
        flags.setFlags(data.readInt())
        line_count = data.readShort(True)
        index_offset = -1 if data.readInt() == 1 else 0
        data.skipBytes(4 * 3)
        lines = []
        for _ in xrange(line_count):
            line = data.readString()
            writer.putln(to_c('add_line(%r);', line))

        if flags['HideOnStart']:
            writer.putln('set_visible(false);')

actions = make_table(ActionMethodWriter, {
    0 : 'load_file'
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
    4 : 'get_line',
    7 : 'get_count'
})

def get_object():
    return KcList