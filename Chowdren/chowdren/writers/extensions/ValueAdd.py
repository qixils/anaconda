from chowdren.writers.objects import ObjectWriter
from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color)
from chowdren.writers.events import (ComparisonWriter, ActionMethodWriter,
    ConditionMethodWriter, ExpressionMethodWriter, make_table)

class ValueAdd(ObjectWriter):
    class_name = 'ValueAdd'
    defines = ['CHOWDREN_USE_VALUEADD']
    static = True

    def write_init(self, writer):
        pass

hashed_keys = {}

def hash_key(value):
    value = value.lower()
    try:
        return hashed_keys[value]
    except KeyError:
        pass
    new_hash = len(hashed_keys)
    hashed_keys[value] = new_hash
    return new_hash

class SetValue(ActionMethodWriter):
    def write(self, writer):
        key = self.parameters[1].loader.items
        key = hash_key(self.converter.convert_static_expression(key))
        value = self.convert_index(2)
        writer.put('get_extra_alterables().set_value(%s, %s);' % (key,
                                                                  value))

    def get_object(self):
        parameter = self.parameters[0].loader
        return parameter.objectInfo, parameter.objectType

class GetValue(ExpressionMethodWriter):
    def get_string(self):
        converter = self.converter
        items = converter.expression_items

        last_exp = items[converter.item_index + 2]
        if last_exp.getName() != 'Virgule':
            raise NotImplementedError()
        next_exp = items[converter.item_index + 1]
        obj = (next_exp.objectInfo, next_exp.objectType)
        converter.item_index += 2

        last_exp = items[converter.item_index + 2]
        if last_exp.getName() != 'EndParenthesis':
            raise NotImplementedError()
        next_exp = items[converter.item_index + 1]
        name = hash_key(next_exp.loader.value)
        converter.item_index += 2

        obj = self.converter.get_object(obj)
        return '%s->get_extra_alterables().get_value(%s)' % (obj, name)

actions = make_table(ActionMethodWriter, {
    0 : SetValue
})

conditions = make_table(ConditionMethodWriter, {
})

expressions = make_table(ExpressionMethodWriter, {
    0 : GetValue
})

def get_object():
    return ValueAdd
