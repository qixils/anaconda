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

        writer.putlnc('image_count = %s;', image_count)

        if image_count:
            image_names = [get_image_name(image) for image in images]
            image_names = ', '.join(image_names)
            writer.putlnc('static Image* static_images[%s] = {%s};',
                          image_count, image_names)
            writer.putlnc('images = (Image**)static_images;')
            writer.putlnc('set_image(0);')
        else:
            writer.putlnc('image = NULL;')

class ReverseColor(ExpressionMethodWriter):
    has_object = False
    method = 'reverse_color'

actions = make_table(ActionMethodWriter, {
    1 : 'set_image',
    3 : 'clear',
    4 : 'create_alpha',
    13 : 'resize',
    15 : 'load',
    14 : 'save',
    19 : 'blit_alpha',
    21 : 'reverse_x',
    24 : 'set_transparent_color',
    29 : 'set_edit_image',
    39 : 'blend_color.set_semi_transparency(%s)',
    40 : 'set_effect', # by index
    41 : 'set_dest_pos',
    49 : 'blit',
    61 : 'apply_matrix',
    62 : 'blit_background',
    66 : 'set_dest_size',
    78 : 'resize_canvas',
    93 : 'scroll',
    116 : 'set_stretch_mode',
    17 : 'add_image'
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
    5 : 'get_edit_width()',
    15 : 'get_image_width',
    25 : ReverseColor
})

def get_object():
    return SurfaceObject
