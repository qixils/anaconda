from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

from chowdren.writers.events import (ComparisonWriter, ActionMethodWriter, 
    ConditionMethodWriter, ExpressionMethodWriter, make_table)

class INI(ObjectWriter):
    class_name = 'INI'

    def write_init(self, writer):
        data = self.get_data()
        data.skipBytes(2)
        filename = data.readString()
        data.seek(3269)
        auto_save = data.readByte() == 1
        data.seek(3429)
        is_global = data.readInt() == 1
        data.skipBytes(1)
        global_key = data.readString()
        if auto_save:
            writer.putln('auto_save = true;')
        if is_global:
            writer.putln(to_c('set_global_data(%r);', global_key))
        elif filename:
            writer.putln(to_c('load_file(%r);', filename))

class PerformSearch(ConditionMethodWriter):
    def write(self, writer):
        writer.put('search(%s, %s, %s);' % (
            self.convert_index(0),
            self.convert_index(1),
            self.convert_index(2)))

class MergeObject(ConditionMethodWriter):
    def write(self, writer):
        name = self.parameters[0].loader.data.readString()
        handle = self.converter.name_to_item[name].handle
        overwrite = self.convert_index(1)
        writer.put('merge_object(%s, %s);' % (self.converter.get_object(handle),
            overwrite))

actions = make_table(ActionMethodWriter, {
    0 : 'set_group',
    14 : 'set_value', # specified group
    1 : 'set_value', # current group
    2 : 'set_string', # current group
    43 : 'load_file',
    38 : 'merge_file',
    45 : 'save_file',
    33 : PerformSearch,
    40 : MergeObject
})

conditions = make_table(ConditionMethodWriter, {
    1 : 'has_item'
})

expressions = make_table(ExpressionMethodWriter, {
    17 : 'get_item_count',
    7 : 'get_item_count',
    16 : 'get_group_count',
    14 : 'get_string_index',
    6 : 'get_string_index',
    1 : 'get_string',
    9 : 'get_string',
    0 : 'get_value',
    13 : 'get_item_name',
    4 : 'get_item_name',
    12 : 'get_group_name',
    8 : 'get_value',
    20 : 'get_search_result_group',
    19 : 'get_search_count'
})

def get_object():
    return INI