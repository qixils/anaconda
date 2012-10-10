from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color, get_method_name)

from chowdren.writers.events import (StaticConditionWriter, 
    StaticActionWriter, StaticExpressionWriter, make_table,
    ActionWriter)

class Layer(ObjectWriter):
    class_name = 'Layer'
    static = True

    def write_init(self, writer):
        pass

    def write_class(self, writer):
        pass

actions = make_table(StaticActionWriter, {
})

conditions = make_table(StaticConditionWriter, {
})

expressions = make_table(StaticExpressionWriter, {
})

def get_object():
    return Layer