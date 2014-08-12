from chowdren.writers.objects import ObjectWriter
from mmfparser.bitdict import BitDict

from chowdren.common import get_animation_name, to_c, make_color

from chowdren.writers.events import (ComparisonWriter, ActionMethodWriter,
    ConditionMethodWriter, ExpressionMethodWriter, make_table)

class KcArray(ObjectWriter):
    class_name = 'ArrayObject'

    def write_init(self, writer):
        data = self.get_data()
        x_size = max(1, data.readInt())
        y_size = max(1, data.readInt())
        z_size = max(1, data.readInt())
        flags = BitDict('Numeric', 'Text', 'Base1', 'Global')
        flags.setFlags(data.readInt())
        is_numeric = flags['Numeric']
        offset = int(flags['Base1'])
        writer.putln(to_c('initialize(%s, %s, %s, %s, %s);', is_numeric,
                          offset, x_size, y_size, z_size))
        # if flags['Global']:
        #     raise NotImplementedError()

actions = make_table(ActionMethodWriter, {
    0 : '.x_pos = %s',
    1 : '.y_pos = %s',
    2 : '.z_pos = %s',
    3 : '.x_pos++',
    4 : '.y_pos++',
    7 : 'set_string',
    14 : 'set_value',
    16 : 'set_string',
    18 : 'set_string', # with xyz
    8 : 'clear',
    9 : 'load'
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
    0 : '.x_pos',
    1 : '.y_pos',
    4 : 'get_string', # current pos
    6 : 'get_value',
    8 : 'get_string',
    10 : 'get_string' # with xyz
})

def get_object():
    return KcArray