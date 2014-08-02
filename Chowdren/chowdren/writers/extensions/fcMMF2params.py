from chowdren.writers.objects import ObjectWriter

from chowdren.common import get_animation_name, to_c, make_color

from chowdren.writers.events import (StaticConditionWriter, StaticActionWriter,
                                     StaticExpressionWriter, ExpressionWriter,
                                     make_table)

class MMF2Params(ObjectWriter):
    class_name = 'MMFParams'
    static = True

    def write_init(self, writer):
        pass

class GetFrameName(ExpressionWriter):
    def get_string(self):
        return 'data->name'

class GetAboutText(ExpressionWriter):
    def get_string(self):
        return 'ABOUT'

actions = make_table(StaticActionWriter, {
})

conditions = make_table(StaticConditionWriter, {
})

expressions = make_table(StaticExpressionWriter, {
    2 : GetFrameName,
    1 : GetAboutText
})

def get_object():
    return MMF2Params
