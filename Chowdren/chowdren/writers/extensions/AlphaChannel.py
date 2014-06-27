from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

from chowdren.writers.events import (ComparisonWriter, ActionMethodWriter,
    ConditionMethodWriter, ExpressionMethodWriter, make_table, EmptyAction,
    make_comparison)

class AlphaImageObject(ObjectWriter):
    class_name = 'AlphaImageObject'
    filename = 'alphaimage'
    use_alterables = True

    def write_init(self, writer):
        pass

actions = make_table(ActionMethodWriter, {
    0 : 'set_image',
    8 : '.anim_frame = %s',
    14 : '.angle = %s',
    32 : 'blend_color.set_alpha',
    33 : EmptyAction, # load image from png
    15 : '.width = %s',
    16 : '.height = %s',
    17 : '.width *= %s / 100.0',
    18 : '.height *= %s / 100.0',
    20 : 'set_hotspot'
})

conditions = make_table(ConditionMethodWriter, {
    5 : make_comparison('image')
})

expressions = make_table(ExpressionMethodWriter, {
    0 : '.image',
    8 : '.start_width',
    9 : '.start_height',
    12 : '.angle',
    14 : '.width',
    15 : '.blend_color.a'
})

def get_object():
    return AlphaImageObject
