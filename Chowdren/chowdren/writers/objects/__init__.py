from chowdren.writers import BaseWriter
from mmfparser.bytereader import ByteReader

class ObjectWriter(BaseWriter):
    common = None
    class_name = 'Undefined'
    static = False

    def __init__(self, *arg, **kw):
        BaseWriter.__init__(self, *arg, **kw)
        self.common = self.data.properties.loader

    def write_pre(self, writer):
        pass

    def write_constants(self, writer):
        pass

    def get_parameters(self):
        return []

    def get_init_list(self):
        return {}

    def write_class(self, writer):
        pass

    def write_frame(self, writer):
        pass    

    def write_init(self, writer):
        pass

    def write_post(self, writer):
        pass

    def get_data(self):
        return ByteReader(self.common.extensionData)

    def get_conditions(self, value):
        if self.data is None:
            key = value
        else:
            key = (self.data.properties.objectType, value)
        return self.converter.generated_groups.pop(key, [])

    def is_visible(self):
        try:
            return self.common.newFlags['VisibleAtStart']
        except (AttributeError, KeyError):
            return True