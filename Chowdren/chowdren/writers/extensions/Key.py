from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

from chowdren.writers.events import (StaticConditionWriter, 
    StaticActionWriter, StaticExpressionWriter, make_table,
    ConditionMethodWriter, ExpressionMethodWriter, EmptyAction)

class KeyObject(ObjectWriter):
    class_name = 'Keyboard'
    static = True

    def write_init(self, writer):
        pass

actions = make_table(StaticActionWriter, {
    1 : EmptyAction,
    0 : EmptyAction
})

conditions = make_table(ConditionMethodWriter, {
    1 : 'is_key_pressed_once(translate_key(%s))',
    6 : 'is_key_released_once(translate_key(%s))',
    7 : 'is_key_released_once(translate_key(%s))'
})

expressions = make_table(ExpressionMethodWriter, {
})

def get_object():
    return KeyObject