from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color, get_method_name)

from chowdren.writers.events import (StaticConditionWriter, 
    StaticActionWriter, StaticExpressionWriter, make_table,
    ActionWriter)

from collections import defaultdict

LOOP_FOR_OBJECT = 4

class StartForObject(ActionWriter):
    def write(self, writer):
        exp, = self.parameters[0].loader.items[:-1]
        real_name = exp.loader.value
        name = get_method_name(real_name)
        with self.converter.iterate_object(
                self.parameters[1].loader.objectInfo, writer,
                'selected'):
            writer.putln('if (!foreach_%s((*selected))) break;' % name)

class ForEach(ObjectWriter):
    class_name = 'ForEach'
    static = True

    def write_init(self, writer):
        pass

    def write_class(self, writer):
        pass

    def write_frame(self, writer):
        if self.converter.debug:
            return
        loops = defaultdict(list)
        for loop in self.get_conditions(LOOP_FOR_OBJECT):
            parameters = loop.conditions[0].data.items
            exp, = parameters[0].loader.items[:-1]
            real_name = exp.loader.value
            loops[real_name].append(loop)

        for real_name, groups in loops.iteritems():
            name = get_method_name(real_name)
            writer.putmeth('bool foreach_%s' % name, 'FrameObject * selected')
            self.converter.set_object(parameters[1].loader.objectInfo, 
                'selected')
            for group in groups:
                self.converter.write_event(writer, group, True)
            writer.putln('return true;')
            writer.end_brace()

actions = make_table(StaticActionWriter, {
    0 : StartForObject
})

conditions = make_table(StaticConditionWriter, {
})

expressions = make_table(StaticExpressionWriter, {
})

def get_object():
    return ForEach