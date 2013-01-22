from chowdren.writers import BaseWriter
from mmfparser.bytereader import ByteReader

class ObjectWriter(BaseWriter):
    common = None
    class_name = 'Undefined'
    static = False

    def __init__(self, *arg, **kw):
        BaseWriter.__init__(self, *arg, **kw)
        self.common = self.data.properties.loader
        self.initialize()

    def initialize(self):
        pass

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

    def write_start(self, writer):
        pass

    def write_frame(self, writer):
        pass    

    def write_init(self, writer):
        pass

    def write_post(self, writer):
        pass

    def get_data(self):
        return ByteReader(self.common.extensionData)

    def get_conditions(self, *values):
        groups = []
        for value in values:
            if self.data is None:
                key = value
            else:
                key = (self.data.properties.objectType, value)
            groups.extend(self.converter.generated_groups.pop(key, []))
        groups.sort(key = lambda x: x.global_id)
        return groups

    def is_visible(self):
        try:
            return self.common.newFlags['VisibleAtStart']
        except (AttributeError, KeyError):
            return True

    def is_scrolling(self):
        try:
            return not self.common.flags['ScrollingIndependant']
        except AttributeError:
            return True

    def is_background(self):
        return self.common.isBackground()

    def get_images(self):
        return []