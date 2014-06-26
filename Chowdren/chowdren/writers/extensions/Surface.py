from chowdren.writers.objects import ObjectWriter
from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)
from chowdren.writers.events import (ComparisonWriter, ActionMethodWriter,
    ConditionMethodWriter, ExpressionMethodWriter, make_table)
from mmfparser.data.font import LogFont


class SurfaceObject(ObjectWriter):
    class_name = 'SurfaceObject'
    filename = 'surface'
    use_alterables = True

    def write_init(self, writer):
        data = self.get_data()
        width = data.readShort()
        height = data.readShort()
        writer.putlnc('width = %s;', width)
        writer.putlnc('height = %s;', height)
        width_def = data.readShort()
        height_def = data.readShort()

        images = []
        for _ in xrange(16):
            images.append(data.readShort())

        image_count = data.readShort()
        images = images[:image_count]

        load_first = data.readByte() != 0
        use_abs = data.readByte() != 0
        threaded_io = data.readByte() != 0
        keep_points = data.readByte() != 0
        multi_imgs = data.readByte() != 0
        disp_target = data.readByte() != 0
        select_last = data.readByte() != 0

        data.skipBytes(3) # what is this

        font = LogFont(data)
        color = data.readColor()
        flags = data.readInt()

actions = make_table(ActionMethodWriter, {
    13 : 'resize',
    15 : 'load',
    40 : 'set_effect', # by index
    41 : 'set_dest_pos',
    49 : 'blit_destination',
    66 : 'set_dest_size',
    78 : 'resize_canvas',
    93 : 'scroll',
    116 : 'set_stretch_mode'
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
})

def get_object():
    return SurfaceObject
