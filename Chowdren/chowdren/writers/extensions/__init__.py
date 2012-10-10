loaded_extensions = {}

def load_extension_module(ext):
    try:
        return loaded_extensions[ext]
    except KeyError:
        pass
    module = __import__('chowdren.writers.extensions', locals(), globals(), 
        [ext])
    extension = getattr(module, ext)
    loaded_extensions[ext] = extension
    return extension