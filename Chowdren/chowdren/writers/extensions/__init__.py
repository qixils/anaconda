from chowdren.writers.objects import ObjectWriter
import os

loaded_extensions = {}

def filter_name(name):
    return name.replace(' ', '').replace('+', 'p').lower()

names = {}
for name in os.listdir(os.path.dirname(__file__)):
    name, ext = os.path.splitext(name)
    names[filter_name(name)] = name

def load_extension_module(ext):
    try:
        return loaded_extensions[ext]
    except KeyError:
        pass
    try:
        mod = names[filter_name(ext)]
        module = __import__('chowdren.writers.extensions', locals(), globals(),
            [mod])
        extension = getattr(module, mod, None)
    except KeyError:
        extension = None
    if extension is None:
        class NewModule(object):
            @staticmethod
            def get_object():
                return ObjectWriter
        extension = NewModule

    extension.extension_name = ext
    loaded_extensions[ext] = extension
    return extension