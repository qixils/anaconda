from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

from chowdren.writers.events import (ComparisonWriter, ActionMethodWriter,
    ConditionMethodWriter, ExpressionMethodWriter, make_table)

class Perspective(ObjectWriter):
    class_name = 'PerspectiveObject'
    filename = 'perspective'

    def write_init(self, writer):
        pass

actions = make_table(ActionMethodWriter, {
    0 : 'set_zoom',
    5 : 'set_waves',
    6 : 'set_offset'
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
    0 : '.zoom',
    1 : '.offset'
})

def get_object():
    return Perspective
