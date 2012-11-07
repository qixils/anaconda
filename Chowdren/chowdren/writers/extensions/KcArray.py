from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

from chowdren.writers.events import (ComparisonWriter, ActionMethodWriter, 
    ConditionMethodWriter, ExpressionMethodWriter, make_table)

class KcArray(ObjectWriter):
    class_name = 'ArrayObject'

    def write_init(self, writer):
        data = self.get_data()
        x_size = data.readInt()
        y_size = data.readInt()
        z_size = data.readInt()
        writer.putln('initialize(%s, %s, %s);' % (x_size, y_size, z_size))

actions = make_table(ActionMethodWriter, {
    14 : 'set_value'
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
    6 : 'get_value'
})

def get_object():
    return KcArray