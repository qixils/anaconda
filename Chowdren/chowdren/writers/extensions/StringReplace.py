from chowdren.writers.objects import ObjectWriter
from chowdren.common import get_animation_name, to_c, make_color
from chowdren.writers.events import (ComparisonWriter, StaticActionWriter,
    StaticConditionWriter, StaticExpressionWriter, make_table,
    ActionMethodWriter, ExpressionMethodWriter)

class StringReplace(ObjectWriter):
    class_name = 'StringReplace'
    filename = 'stringreplace'

    def write_init(self, writer):
        pass

class ReplaceExpression(StaticExpressionWriter):
    method = 'replace'

actions = make_table(ActionMethodWriter, {
    1 : 'add_replacement'
})

conditions = make_table(StaticConditionWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
    0 : 'replace',
    1 : ReplaceExpression
})

def get_object():
    return StringReplace