# Extra features for Chowdren

from chowdren.writers.events import (ExpressionWriter, ActionWriter, 
    ConditionWriter)

SPRITES_STRING = 'Chowdren: Sprites'
PLATFORM_STRING = 'Chowdren: Platform'

SPECIAL_OBJECTS = set([SPRITES_STRING, PLATFORM_STRING])

def is_special_object(name):
    return name in SPECIAL_OBJECTS

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

actions = {
    'SetString' : SetString
}

conditions = {
}

expressions = {
    'CurrentText' : GetString
}