from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

from chowdren.writers.events import (ActionMethodWriter, ConditionMethodWriter,
    ExpressionMethodWriter, make_table)

class UltimateFullcsreen(ObjectWriter):
    class_name = 'Fullscreen'
    static = True

actions = make_table(ActionMethodWriter, {

})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
})

def get_object():
    return UltimateFullcsreen