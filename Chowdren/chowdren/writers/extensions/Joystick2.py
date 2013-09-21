from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

from chowdren.writers.events import (StaticConditionWriter, 
    StaticActionWriter, StaticExpressionWriter, make_table,
    ConditionMethodWriter, ExpressionMethodWriter, EmptyAction,
    StaticConditionWriter)

class Joystick2(ObjectWriter):
    class_name = 'Joystick'
    static = True

    def write_init(self, writer):
        pass

actions = make_table(StaticActionWriter, {
    0 : EmptyAction, # ignore control
    1 : EmptyAction, # restore control,
    35 : EmptyAction, # poll for devices, not necessary on GLFW
})

conditions = make_table(ConditionMethodWriter, {
    0 : 'is_joystick_pressed', # repeat while
    2 : 'is_joystick_attached',
    4 : 'is_joystick_pressed',
    5 : 'is_joystick_pressed', # once
    6 : 'is_joystick_released',
    7 : 'any_joystick_pressed',
    17 : 'compare_joystick_direction',
    27 : 'is_joystick_direction_changed'
})

expressions = make_table(ExpressionMethodWriter, {
})

def get_object():
    return Joystick2