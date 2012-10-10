from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color, get_method_name)

from chowdren.writers.events import (StaticConditionWriter, 
    StaticActionWriter, StaticExpressionWriter, make_table,
    ActionWriter, ActionMethodWriter, ConditionMethodWriter,
    ExpressionMethodWriter, EmptyAction)

class Console(ObjectWriter):
    class_name = 'Console'
    static = True

    def write_init(self, writer):
        pass

actions = make_table(ActionMethodWriter, {
    0 : EmptyAction
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
})

def get_object():
    return Console