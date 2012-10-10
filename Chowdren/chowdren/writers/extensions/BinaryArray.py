from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

from chowdren.writers.events import (ComparisonWriter, ActionMethodWriter, 
    ConditionMethodWriter, ExpressionMethodWriter, make_table)

class BinaryArray(ObjectWriter):
    class_name = 'BinaryArray'

    def write_init(self, writer):
        pass

actions = make_table(ActionMethodWriter, {
    2 : 'load'
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
})

def get_object():
    return BinaryArray