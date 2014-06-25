from chowdren.writers.objects import ObjectWriter
from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)
from chowdren.writers.events import (ComparisonWriter, ActionMethodWriter,
    ConditionMethodWriter, ExpressionMethodWriter, make_table)

class StringParser(ObjectWriter):
    class_name = 'StringParser'
    filename = 'stringparser'

    def write_init(self, writer):
        pass

actions = make_table(ActionMethodWriter, {
    0 : 'set',
    6 : 'add_delimiter'
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
    29 : '.get_element(-1 + '
})

def get_object():
    return StringParser
