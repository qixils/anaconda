from chowdren.writers.objects import ObjectWriter
import os

loaded_extensions = {}

names = {}
for name in os.listdir(os.path.dirname(__file__)):
    name, ext = os.path.splitext(name)
    names[name.lower()] = name

def load_extension_module(ext):
    try:
        return loaded_extensions[ext]
    except KeyError:
        pass
    mod = names[ext.lower()]
    module = __import__('chowdren.writers.extensions', locals(), globals(),
        [mod])
    extension = getattr(module, mod, None)
    if extension is None:
        class NewModule(object):
            @staticmethod
            def get_object():
                return ObjectWriter
        extension = NewModule
    loaded_extensions[ext] = extension
    return extension