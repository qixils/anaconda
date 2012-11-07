# Extra features for Chowdren

from chowdren.writers.events import (ExpressionWriter, ActionWriter, 
    ConditionWriter)

ASSETS_STRING = 'Chowdren: Assets Folder'
PLATFORM_STRING = 'Chowdren: Platform'

SPECIAL_OBJECTS = set([ASSETS_STRING, PLATFORM_STRING])

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
        if name == ASSETS_STRING:
            writer.put('set_assets_folder(%s);' % self.convert_index(0))

actions = {
    'SetString' : SetString
}

conditions = {
}

expressions = {
    'CurrentText' : GetString
}