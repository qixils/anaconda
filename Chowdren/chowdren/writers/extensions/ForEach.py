from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color, get_method_name)

from chowdren.writers.events import (StaticConditionWriter,
    StaticActionWriter, ExpressionMethodWriter, make_table,
    ActionWriter, make_comparison)

from collections import defaultdict

LOOP_FOR_OBJECT = 4
ON_LOOP = 0

class StartForObject(ActionWriter):
    custom = True
    def write(self, writer):
        writer.start_brace()
        eval_name = self.convert_index(0)
        object_info = self.parameters[1].loader.objectInfo
        object_class = self.converter.get_object_class(
            object_info = object_info)
        name = None
        try:
            exp, = self.parameters[0].loader.items[:-1]
            real_name = exp.loader.value
            name = get_method_name(real_name)
            func_call = 'foreach_%s(' % name
        except ValueError:
            writer.putln('std::string name = %s;' % eval_name)
            func_call = 'call_dynamic_foreach(name, '
            object_class = 'FrameObject*'
        with self.converter.iterate_object(
                self.parameters[1].loader.objectInfo, writer, 'selected'):
            if name is not None:
                writer.putln('foreach_instance_%s = *selected;' % name)
            writer.putln('if (!%s((%s)*selected))) break;' % (func_call,
                object_class))
        # self.converter.write_container_check(self.group, writer)
        writer.end_brace()

# PROFILE_FUNCTIONS = set(['sensor_engines_arms'])
PROFILE_FUNCTIONS = set([])

class ForEach(ObjectWriter):
    class_name = 'ForEach'
    static = True

    def write_frame(self, writer):
        # writer.putln('std::map<std::string, FrameObject*> loop_instances;')
        # writer.putln('FrameObject * current_foreach_instance;')
        # writer.putmeth('double get_foreach_fixed', 'const std::string & name')
        # writer.putln('return loop_instances[name]->get_fixed();')
        # writer.end_brace()

        loops = defaultdict(list)
        loop_objects = {}
        for loop in self.get_conditions(LOOP_FOR_OBJECT, ON_LOOP):
            parameters = loop.conditions[0].data.items
            exp, = parameters[0].loader.items[:-1]
            real_name = exp.loader.value
            try:
                object_info = parameters[1].loader.objectInfo
                loop_objects[real_name] = object_info
            except IndexError:
                # for ON_LOOP
                pass
            loops[real_name].append(loop)

        for real_name in loops.keys():
            name = 'foreach_instance_' + get_method_name(real_name)
            writer.add_member('FrameObject * %s' % name)

        self.converter.begin_events()
        for real_name, groups in loops.iteritems():
            object_info = loop_objects[real_name]
            name = get_method_name(real_name)
            object_class = self.converter.get_object_class(
                object_info = object_info)
            writer.putmeth('bool foreach_%s' % name, '%s selected' %
                object_class)
            for group in groups:
                self.converter.set_object(object_info, 'selected')
                self.converter.write_event(writer, group, True)
            writer.putln('return true;')
            writer.end_brace()

        writer.putmeth('bool call_dynamic_foreach', 'std::string name',
            'FrameObject * selected')
        for name in loops.keys():
            object_info = loop_objects[name]
            object_class = self.converter.get_object_class(
                object_info = object_info)
            writer.putln(to_c('if (name == %r) return foreach_%s((%s)selected);',
                name, get_method_name(name), object_class))
        writer.putln('return false;')
        writer.end_brace()

class GetForeachFixed(ExpressionMethodWriter):
    def get_string(self):
        converter = self.converter
        name = converter.expression_items[converter.item_index+1].loader.value
        foreach_instance = 'foreach_instance_' + get_method_name(name)
        fixed = foreach_instance + '->get_fixed()'
        converter.item_index += 2
        return fixed

actions = make_table(StaticActionWriter, {
    0 : StartForObject
})

conditions = make_table(StaticConditionWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
    0 : GetForeachFixed
})

def get_object():
    return ForEach