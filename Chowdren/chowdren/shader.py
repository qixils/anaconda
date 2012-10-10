NATIVE_SHADERS = {
    'Sub' : 'subtract_shader',
    'Add' : 'additive_shader',
    'ColorMixer.fx' : 'subtract_shader'
}

def get_name(name):
    return NATIVE_SHADERS[name]