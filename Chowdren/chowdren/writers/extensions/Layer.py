from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color, get_method_name)

from chowdren.writers.events import (ConditionMethodWriter, 
    ExpressionMethodWriter, ActionMethodWriter, make_table)

class Layer(ObjectWriter):
    class_name = 'LayerObject'

    def write_init(self, writer):
        pass

    def write_class(self, writer):
        pass

actions = make_table(ActionMethodWriter, {
    38 : 'set_layer(%s-1)',
    27 : 'sort_alt_decreasing',
    31 : 'show_layer(%s-1)',
    32 : 'hide_layer(%s-1)'
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
})

def get_object():
    return Layer