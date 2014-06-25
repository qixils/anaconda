from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

from chowdren.writers.events import (StaticConditionWriter,
    StaticActionWriter, StaticExpressionWriter, make_table,
    ConditionMethodWriter, ExpressionMethodWriter, EmptyAction,
    StaticConditionWriter, TrueCondition, FalseCondition)

class ControlsObject(ObjectWriter):
    class_name = 'ControlsObject'
    static = True

    def write_init(self, writer):
        pass

actions = make_table(StaticActionWriter, {
    7 : EmptyAction, # set player up
    8 : EmptyAction, # set player down
    9 : EmptyAction, # set player left
    10 : EmptyAction, # set player right
    11 : EmptyAction, # set player fire 1
    12 : EmptyAction, # set player fire 2
    33 : EmptyAction, # simulate key value down
    38 : EmptyAction, # set player fire 3
    39 : EmptyAction, # set player fire 4
    15 : EmptyAction, # disable_alt_tab_ctrl_esc_and_ctrl_alt_del
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
})

def get_object():
    return ControlsObject
