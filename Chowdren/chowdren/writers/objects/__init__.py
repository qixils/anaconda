from chowdren.writers import BaseWriter

class ObjectWriter(BaseWriter):
    common = None
    class_name = 'Undefined'

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

    def write_init(self, writer):
        pass

    def write_post(self, writer):
        pass