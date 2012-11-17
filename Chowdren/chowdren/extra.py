# Extra features for Chowdren

from chowdren.writers.events import (ExpressionWriter, ActionWriter, 
    ConditionWriter)
from chowdren.common import to_c

SPRITES_STRING = 'Chowdren: Sprites'
PLATFORM_STRING = 'Chowdren: Platform'
FONT_STRING = 'Chowdren: Font'
RESIZE_STRING = 'Chowdren: Window Resize'
SHADERS_STRING = 'Chowdren: Shaders'

SPECIAL_OBJECTS = set([SPRITES_STRING, PLATFORM_STRING, FONT_STRING, 
    RESIZE_STRING, SHADERS_STRING])

def is_special_object(name):
    return name in SPECIAL_OBJECTS

def convert_repr_bool(value):
    value = eval(value)
    if value == 'Yes':
        return True
    elif value == 'No':
        return False
    else:
        raise NotImplementedError

class GetString(ExpressionWriter):
    has_object = False
    def get_string(self):
        name = self.converter.all_objects[self.data.objectInfo].data.name
        if name == PLATFORM_STRING:
            return 'get_platform()'

class SetString(ActionWriter):
    has_object = False
    def write(self, writer):
        name = self.converter.all_objects[self.data.objectInfo].data.name
        if name == SPRITES_STRING:
            writer.put('set_image_path(%s);' % self.convert_index(0))
        elif name == FONT_STRING:
            writer.put('set_font_path(%s);' % self.convert_index(0))
        elif name == RESIZE_STRING:
            writer.put(to_c('set_window_resize(%s);', convert_repr_bool(
                self.convert_index(0))))
        elif name == SHADERS_STRING:
            writer.put('set_shader_path(%s);' % self.convert_index(0))

actions = {
    'SetString' : SetString
}

conditions = {
}

expressions = {
    'CurrentText' : GetString
}