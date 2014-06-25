from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)

from chowdren.writers.events import (ActionMethodWriter, ConditionMethodWriter,
    ExpressionMethodWriter, make_table, ActionWriter)

class AdvancedDirection(ObjectWriter):
    class_name = 'AdvancedDirection'

    def write_init(self, writer):
        pass

class FindClosest(ActionWriter):
    custom = True
    def write(self, writer):
        writer.start_brace()
        object_info = (self.parameters[0].loader.objectInfo,
                       self.parameters[0].loader.objectType)
        instances = self.converter.create_list(object_info, writer)
        details = self.convert_index(1)
        x = str(details['x'])
        y = str(details['y'])
        parent = details.get('parent', None)
        if parent is not None:
            writer.putln('int parent_x, parent_y;')
            writer.putln('FrameObject * parent = %s;' % (
                self.converter.get_object(parent)))
            writer.putln('if (parent == NULL) parent_x = parent_y = 0;')
            writer.putln('else {')
            writer.indent()
            writer.putln('parent_x = parent->x;')
            writer.putln('parent_y = parent->y;')
            writer.end_brace()
            x = 'parent_x + %s' % x
            y = 'parent_y + %s' % y
        object_info = self.get_object()
        obj = self.converter.get_object(object_info)
        writer.put('%s->find_closest(%s, %s, %s);' % (obj, instances, x, y))
        writer.end_brace()

actions = make_table(ActionMethodWriter, {
    1 : FindClosest
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
    8 : 'get_closest'
})

def get_object():
    return AdvancedDirection