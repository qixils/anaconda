from chowdren.writers.objects import ObjectWriter
from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)
from chowdren.writers.events import (ComparisonWriter, StaticActionWriter,
    StaticConditionWriter, StaticExpressionWriter, make_table)

class StringReplace(ObjectWriter):
    class_name = 'StringReplace'
    includes = ['objects/stringreplace.h']
    static = True

    def write_init(self, writer):
        pass

actions = make_table(StaticActionWriter, {
})

conditions = make_table(StaticConditionWriter, {
})

expressions = make_table(StaticExpressionWriter, {
    1 : 'replace'
})

def get_object():
    return StringReplace