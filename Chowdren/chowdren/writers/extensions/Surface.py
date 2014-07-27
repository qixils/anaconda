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
    update = True

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

        load_first = data.readByte() != 0 # always true!
        use_abs = data.readByte() != 0
        threaded_io = data.readByte() != 0 # unused (I bet)
        keep_points = data.readByte() != 0 # unused
        multi_imgs = data.readByte() != 0
        disp_target = data.readByte() != 0
        select_last = data.readByte() != 0 # always false!

        data.skipBytes(3) # what is this

        font = LogFont(data)
        color = data.readColor()
        flags = data.readInt()

        writer.putlnc('display_selected = %s;', disp_target)
        writer.putlnc('use_abs_coords = %s;', use_abs)

        image_names = [get_image_name(image) for image in images]

        writer.putln('SurfaceImage tmp;')
        if multi_imgs:
            # blank first image
            if image_count == 0 or images[0] == -1:
                writer.putln('tmp.reset(width, height);')
                writer.putln('images.push_back(tmp);')
            for i in range(0, image_count):
                writer.putlnc('// Image %d', i)
                # non-blank images
                if images[i] != -1:
                    writer.putlnc('tmp.set_image(%s);', image_names[i])
                    writer.putln('images.push_back(tmp);')
                # blank images (NOT first)
                elif i > 0 and image_count > 1:
                    writer.putln('tmp.reset();')
                    writer.putln('images.push_back(tmp);')
        else:
            # single image
            if image_count > 0 and images[0] != -1:
                writer.putlnc('tmp.set_image(%s);', image_names[0])
            else:
                writer.putlnc('tmp.reset(width, height);')
            writer.putln('images.push_back(tmp);')
        # load_first always true -> there will always be an image 0
        writer.putln('set_edit_image(0, true);')


class ReverseColor(ExpressionMethodWriter):
    has_object = False
    method = 'reverse_color'

class LoadFailed(ConditionMethodWriter):
    is_always = True
    method = '.load_failed'

actions = make_table(ActionMethodWriter, {
    1 : 'set_display_image',
    3 : 'clear',
    4 : 'create_alpha',
    13 : 'resize',
    15 : 'load',
    14 : 'save',
    19 : 'blit_alpha',
    21 : 'reverse_x',
    24 : 'set_transparent_color',
    29 : 'set_edit_image',
    38 : 'set_alpha_mode',
    39 : 'blend_color.set_semi_transparency(%s)',
    40 : 'set_effect', # by index
    41 : 'set_dest_pos',
    49 : 'blit',
    61 : 'apply_matrix',
    62 : 'blit_background',
    63 : 'blit_image',
    66 : 'set_dest_size',
    78 : 'resize_canvas',
    93 : 'scroll',
    115 : 'resize_source',
    116 : 'set_stretch_mode',
    17 : 'add_image'
})

conditions = make_table(ConditionMethodWriter, {
    2 : LoadFailed
})

expressions = make_table(ExpressionMethodWriter, {
    5 : 'get_edit_width()',
    15 : 'get_image_width',
    25 : ReverseColor
})

def get_object():
    return SurfaceObject
