from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

from chowdren.writers.events import (StaticConditionWriter, 
    StaticActionWriter, StaticExpressionWriter, make_table)

class File(ObjectWriter):
    class_name = 'File'
    static = True

    def write_init(self, writer):
        pass

actions = make_table(StaticActionWriter, {
})

conditions = make_table(StaticConditionWriter, {
    4 : 'file_exists'
})

expressions = make_table(StaticExpressionWriter, {
    28 : 'get_appdata_directory()'
})

def get_object():
    return File