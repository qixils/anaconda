NATIVE_SHADERS = {
    'Sub' : 'subtract_shader',
    'Add' : 'additive_shader',
    'ColorMixer.fx' : 'mixer_shader',
    'Looki Offset.fx' : 'offset_shader',
    'CS_Hue.fx' : 'hue_shader',
    'DodgeBlur.fx' : 'dodgeblur_shader',
    'Mono' : 'monochrome_shader',
    'Blend' : 'blend_shader'
}

def get_name(name):
    return NATIVE_SHADERS.get(name, name.split('.')[0])