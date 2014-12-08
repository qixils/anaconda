from chowdren.writers.objects import ObjectWriter

from chowdren.common import get_animation_name, to_c, make_color

from chowdren.writers.events import (StaticConditionWriter,
    StaticActionWriter, StaticExpressionWriter, make_table)

class AdvancedDirection(ObjectWriter):
    class_name = 'AdvancedDirection'
    filename = 'advdir'

actions = make_table(StaticActionWriter, {
})

conditions = make_table(StaticConditionWriter, {
})

expressions = make_table(StaticExpressionWriter, {
    57 : 'get_object_angle'
})

def get_object():
    return AdvancedDirection
