from chowdren.writers import BaseWriter
from mmfparser.bytereader import ByteReader
from chowdren.idpool import get_id

class ObjectWriter(BaseWriter):
    common = None
    class_name = 'Undefined'
    static = False
    includes = []
    event_callbacks = None
    use_alterables = False
    has_color = False

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

    def is_global(self):
        return False
        return self.data.flags['Global']

    def get_images(self):
        return []

    def add_event_callback(self, name):
        wrapper_name = '%s_%s' % (name, get_id(self))
        event_id = self.converter.event_callback_ids.next()
        self.event_callbacks[(name, wrapper_name)] = event_id
        return wrapper_name

    def write_internal_class(self, writer):
        if not self.is_global():
            return
        writer.putln('static bool has_saved_alterables = false;')
        writer.putln('static AlterableValues saved_values;')
        writer.putln('static AlterableStrings saved_strings;')

    def has_dtor(self):
        return self.use_alterables

    def write_dtor(self, writer):
        if self.is_global():
            writer.putln('has_saved_alterables = true;')
            writer.putln('saved_values.set(*values);')
            writer.putln('saved_strings.set(*strings);')
        writer.putln('delete values;')
        writer.putln('delete strings;')

    def load_alterables(self, writer):
        if not self.use_alterables:
            return

        is_global = self.is_global()
        writer.putln('create_alterables();')

        if is_global:
            writer.putln('if (has_saved_alterables) {')
            writer.indent()
            writer.putln('values->set(saved_values);')
            writer.putln('strings->set(saved_strings);')
            writer.dedent()
            writer.putln('} else {')
            writer.indent()

        common = self.common
        if common.values:
            for index, value in enumerate(common.values.items):
                if value == 0:
                    continue
                writer.putlnc('values->set(%s, %s);', index, value)
        if common.strings:
            for index, value in enumerate(common.strings.items):
                if value == '':
                    continue
                writer.putlnc('strings->set(%s, %r);', index, value)

        if is_global:
            writer.end_brace()

