from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

from chowdren.writers.events import (ActionMethodWriter, ConditionMethodWriter,
    ExpressionMethodWriter, make_table)

class UltimateFullcsreen(ObjectWriter):
    class_name = 'Fullscreen'
    static = True

actions = make_table(ActionMethodWriter, {
    1 : 'set_fullscreen(false)',
    0 : 'set_fullscreen(true)'
})

conditions = make_table(ConditionMethodWriter, {
    1 : '!is_fullscreen()'
})

expressions = make_table(ExpressionMethodWriter, {
})

def get_object():
    return UltimateFullcsreen