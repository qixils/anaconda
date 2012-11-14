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

class PerformSearch(ActionMethodWriter):
    def write(self, writer):
        writer.put('search(%s, %s, %s);' % (
            self.convert_index(0),
            self.convert_index(1),
            self.convert_index(2)))

class DeletePattern(ActionMethodWriter):
    def write(self, writer):
        writer.put('delete_pattern(%s, %s, %s);' % (
            self.convert_index(0),
            self.convert_index(1),
            self.convert_index(2)))

class MergeObject(ActionMethodWriter):
    def write(self, writer):
        name = self.parameters[0].loader.data.readString()
        handle = self.converter.name_to_item[name].handle
        overwrite = self.convert_index(1)
        writer.put('merge_object(%s, %s);' % (self.converter.get_object(handle),
            overwrite))

class MergeGroupObject(ActionMethodWriter):
    def write(self, writer):
        name = self.parameters[0].loader.data.readString()
        handle = self.converter.name_to_item[name].handle
        src_group = self.convert_index(1)
        dst_group = self.convert_index(2)
        overwrite = self.convert_index(3)
        writer.put('merge_group(%s, %s, %s, %s);' % (
            self.converter.get_object(handle), src_group, dst_group, overwrite))    

actions = make_table(ActionMethodWriter, {
    0 : 'set_group',
    14 : 'set_value', # specified group
    15 : 'set_string', # specified group
    1 : 'set_value', # current group
    2 : 'set_string', # current group
    28 : 'delete_group',
    9 : 'delete_group',
    10 : 'delete_item',
    29 : 'delete_item',
    43 : 'load_file',
    48 : 'load_string',
    32 : 'reset',
    38 : 'merge_file',
    45 : 'save_file',
    31 : DeletePattern,
    33 : PerformSearch,
    40 : MergeObject,
    41 : MergeGroupObject

})

conditions = make_table(ConditionMethodWriter, {
    1 : 'has_item',
    4 : 'has_item',
    3 : 'has_group'
})

expressions = make_table(ExpressionMethodWriter, {
    17 : 'get_item_count',
    7 : 'get_item_count()',
    16 : 'get_group_count()',
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
    19 : 'get_search_count',
    37 : 'get_item_part'
})

def get_object():
    return INI