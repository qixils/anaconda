loaded_extensions = {}

from chowdren.writers.objects import ObjectWriter

def load_extension_module(ext):
    try:
        return loaded_extensions[ext]
    except KeyError:
        pass
    module = __import__('chowdren.writers.extensions', locals(), globals(), 
        [ext])
    extension = getattr(module, ext, None)
    if extension is None:
        class NewModule(object):
            @staticmethod
            def get_object():
                return ObjectWriter
        extension = NewModule
    loaded_extensions[ext] = extension
    return extension