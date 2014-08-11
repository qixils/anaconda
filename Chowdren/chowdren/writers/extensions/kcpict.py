from kcpica import ActivePicture
from chowdren.writers.events import (ComparisonWriter, ActionMethodWriter,
    ConditionMethodWriter, ExpressionMethodWriter, make_table)

class BackgroundPicture(ActivePicture):
    def is_static_background(self):
        return False

actions = make_table(ActionMethodWriter, {
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
})

def get_object():
    return BackgroundPicture
