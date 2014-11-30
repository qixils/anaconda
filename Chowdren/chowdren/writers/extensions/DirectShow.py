from chowdren.writers.objects import ObjectWriter

from chowdren.common import get_animation_name, to_c, make_color

from chowdren.writers.events import (ConditionMethodWriter,
    ActionMethodWriter, ExpressionMethodWriter, make_table, EmptyAction)

class DirectShow(ObjectWriter):
    class_name = 'DirectShow'
    filename = 'directshow'
    use_alterables = True

    def write_init(self, writer):
        pass

actions = make_table(ActionMethodWriter, {
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
})

def get_object():
    return DirectShow
