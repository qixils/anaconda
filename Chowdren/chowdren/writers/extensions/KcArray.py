from chowdren.writers.objects import ObjectWriter
from mmfparser.bitdict import BitDict

from chowdren.common import get_animation_name, to_c, make_color

from chowdren.writers.events import (ComparisonWriter, ActionMethodWriter,
    ConditionMethodWriter, ExpressionMethodWriter, make_table)

class KcArray(ObjectWriter):
    class_name = 'ArrayObject'

    def write_init(self, writer):
        data = self.get_data()
        x_size = data.readInt()
        y_size = data.readInt()
        z_size = data.readInt()
        flags = BitDict('Numeric', 'Text', 'Base1', 'Global')
        flags.setFlags(data.readInt())
        is_numeric = flags['Numeric']
        offset = int(flags['Base1'])
        writer.putln(to_c('initialize(%s, %s, %s, %s, %s);', is_numeric,
                          offset, x_size, y_size, z_size))
        # if flags['Global']:
        #     raise NotImplementedError()

actions = make_table(ActionMethodWriter, {
    14 : 'set_value',
    16 : 'set_string',
    8 : 'clear',
    9 : 'load'
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
    6 : 'get_value',
    8 : 'get_string'
})

def get_object():
    return KcArray