from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

from chowdren.writers.events import (ComparisonWriter, ActionMethodWriter, 
    ConditionMethodWriter, ExpressionMethodWriter, make_table)

class ActivePicture(ObjectWriter):
    class_name = 'ActivePicture'

    def write_init(self, writer):
        pass
        # data = self.get_data()
        # objects_file.putdef('width', data.readInt())
        # objects_file.putdef('height', data.readInt())
        # active_picture_flags.setFlags(data.readInt(True))
        # visible = not active_picture_flags['HideOnStart']
        # transparent_color = data.readColor()
        # image = data.readString(260) or None
        # objects_file.putdef('filename', image)

actions = make_table(ActionMethodWriter, {
    0 : 'load',
    2 : 'set_hotspot'
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
})

def get_object():
    return ActivePicture