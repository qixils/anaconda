from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

from chowdren.writers.events import (StaticConditionWriter, 
    StaticActionWriter, StaticExpressionWriter, make_table,
    ConditionWriter, ExpressionMethodWriter)

class Keyboard(ObjectWriter):
    class_name = 'Keyboard'
    static = True

    def write_init(self, writer):
        pass

class OnAnyDown(ConditionWriter):
    is_always = True
    def write(self, writer):
        writer.put('last_key != -1')

actions = make_table(StaticActionWriter, {
})

conditions = make_table(ConditionWriter, {
    3 : OnAnyDown
})

expressions = make_table(ExpressionMethodWriter, {
    0 : '.last_key'
})

def get_object():
    return Keyboard