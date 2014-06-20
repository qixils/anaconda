from chowdren.writers.objects import ObjectWriter

from chowdren.common import (get_image_name, get_animation_name, to_c,
    make_color, get_method_name)

from chowdren.writers.events import (ConditionMethodWriter,
    ExpressionMethodWriter, ActionMethodWriter, make_table, ExpressionWriter)

class Layer(ObjectWriter):
    class_name = 'LayerObject'

def get_layer(converter, name):
    frame = converter.current_frame
    for index, layer in enumerate(frame.layers.items):
        if layer.name != name:
            continue
        break
    else:
        raise NotImplementedError()
    return index, layer

class SetByName(ActionMethodWriter):
    def write(self, writer):
        name = self.converter.convert_static_expression(
            self.parameters[0].loader.items)
        index, layer = get_layer(self.converter, name)
        self.write_layer(index, writer)

class SetXByName(SetByName):
    def write_layer(self, layer, writer):
        writer.put('set_x(%s, %s);' % (layer, self.convert_index(1)))

class SetYByName(SetByName):
    def write_layer(self, layer, writer):
        writer.put('set_y(%s, %s);' % (layer, self.convert_index(1)))

class SetAlphaByName(SetByName):
    def write_layer(self, layer, writer):
        writer.put('set_alpha_coefficient(%s, %s);' % (layer,
                                                       self.convert_index(1)))

class HideByName(SetByName):
    def write_layer(self, layer, writer):
        writer.put('hide_layer(%s);' % layer)

class GetLayerCount(ExpressionWriter):
    has_object = False

    def get_string(self):
        return str(len(self.converter.current_frame.layers.items))

class GetByName(ExpressionMethodWriter):
    def get_string(self):
        converter = self.converter
        items = converter.expression_items
        last_exp = items[converter.item_index + 2]
        if last_exp.getName() != 'EndParenthesis':
            raise NotImplementedError()
        next_exp = items[converter.item_index + 1]
        name = next_exp.loader.value
        converter.item_index += 2
        index, layer = get_layer(self.converter, name)
        return self.get_string_layer(index, layer)

class GetIndexByName(GetByName):
    has_object = False

    def get_string_layer(self, index, layer):
        return str(index+1)

class GetLayerCount(ExpressionWriter):
    has_object = False

    def get_string(self):
        return str(len(self.converter.current_frame.layers.items))

class GetXByName(GetByName):
    has_object = False

    def get_string_layer(self, index, layer):
        return 'layers[%s]->x' % index

class GetYByName(GetByName):
    has_object = False

    def get_string_layer(self, index, layer):
        return 'layers[%s]->y' % index

class IsVisible(ConditionMethodWriter):
    has_object = False
    method = 'layers[%s+1]->visible'

actions = make_table(ActionMethodWriter, {
    30 : 'set_position(%s-1, %s, %s)',
    33 : SetXByName,
    34 : SetYByName,
    37 : HideByName,
    38 : 'set_layer(%s-1)',
    27 : 'sort_alt_decreasing',
    31 : 'show_layer(%s-1)',
    32 : 'hide_layer(%s-1)',
    50 : SetAlphaByName,
})

conditions = make_table(ConditionMethodWriter, {
    10 : IsVisible
})

expressions = make_table(ExpressionMethodWriter, {
    12 : GetLayerCount,
    14 : GetIndexByName,
    10 : GetXByName,
    11 : GetYByName,
})

def get_object():
    return Layer