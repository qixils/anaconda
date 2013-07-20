NATIVE_SHADERS = {
    'Sub' : 'subtract_shader',
    'Add' : 'additive_shader',
    'ColorMixer.fx' : 'mixer_shader',
    'Looki Offset.fx' : 'offset_shader',
    'CS_Hue.fx' : 'hue_shader',
    'DodgeBlur.fx' : 'dodgeblur_shader',
    'Mono' : 'monochrome_shader',
    'Monochrome' : 'monochrome_shader',
    'Blend' : 'blend_shader',
    'Subtract' : 'subtract_shader',
    'MonoExample.fx' : 'monochrome_shader',
    'HardMix.fx' : 'dummy_shader',
    'Overlay.fx' : 'dummy_shader',
    'Lens.fx' : 'dummy_shader',
    'LinearDodge.fx' : 'dummy_shader',
    'SoftLight.fx' : 'dummy_shader',
    'PinLight.fx' : 'dummy_shader'
}

def get_name(name):
    return NATIVE_SHADERS.get(name, name)