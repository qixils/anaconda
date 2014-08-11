from chowdren.writers.objects import ObjectWriter

from chowdren.common import get_animation_name, to_c, make_color

from chowdren.writers.events import (StaticConditionWriter, StaticActionWriter,
                                     StaticExpressionWriter, ExpressionWriter,
                                     make_table, EmptyAction)

class ClockObject(ObjectWriter):
    class_name = 'ClockObject'
    static = True

    def write_init(self, writer):
        pass

class NullExpression(ExpressionWriter):
    def get_string(self):
        return '0'

actions = make_table(StaticActionWriter, {
})

conditions = make_table(StaticConditionWriter, {
})

expressions = make_table(StaticExpressionWriter, {
    5 : NullExpression, # day of month
    6 : NullExpression, # month
    7 : NullExpression # retrieve year
})

def get_object():
    return ClockObject
