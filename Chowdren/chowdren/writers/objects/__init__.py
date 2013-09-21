from chowdren.writers import BaseWriter
from mmfparser.bytereader import ByteReader

class ObjectWriter(BaseWriter):
    common = None
    class_name = 'Undefined'
    static = False
    includes = []
    event_callbacks = None

    def __init__(self, *arg, **kw):
        self.event_callbacks = {}
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

    def add_event_callback(self, name):
        wrapper_name = '%s_%s' % (name, id(self))
        event_id = self.converter.event_callback_ids.next()
        self.event_callbacks[(name, wrapper_name)] = event_id
        return wrapper_name


