from mmfparser.data.chunkloaders.objectinfo import (NONE_EFFECT,
    INVERTED_EFFECT, XOR_EFFECT, AND_EFFECT,
    OR_EFFECT, MONOCHROME_EFFECT, ADD_EFFECT, SUBTRACT_EFFECT)
from chowdren.common import get_base_path, makedirs
import os

INK_EFFECTS = {
    NONE_EFFECT : None,
    ADD_EFFECT : 'Add',
    SUBTRACT_EFFECT : 'Subtract',
    MONOCHROME_EFFECT : 'Monochrome',
    INVERTED_EFFECT : 'Invert',
    XOR_EFFECT : 'XOR',
    AND_EFFECT : 'AND',
    OR_EFFECT : 'OR'
}

NATIVE_SHADERS = {
    None : 'NULL',
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
    'Lens.fx' : 'lens_shader',
    'LinearDodge.fx' : 'dummy_shader',
    'SoftLight.fx' : 'dummy_shader',
    'PinLight.fx' : 'dummy_shader',
    'Invert' : 'invert_shader',
    'GrainPS2.fx' : 'grain_shader',
    'Multiply.fx' : 'multiply_shader',
    'HardLight.fx' : 'hardlight_shader',
    'CS_Tint.fx' : 'tint_shader',
    'ChannelBlur.fx' : 'channelblur_shader',
    'BgBloom.fx' : 'bgbloom_shader',
    'CS_UnderWater.fx' : 'underwater_shader',
    'RotateSub.fx' : 'rotatesub_shader',
    'SimpleMask.fx' : 'simplemask_shader',
    'Offsetstationary.fx' : 'offsetstationary_shader',
    'Pattern Overlay alonso.fx' : 'patternoverlay_shader',
    'SubPx.fx' : 'subpx_shader',
    'ColDirBlur alonso.fx' : 'coldirblur_shader',
    'OverlayAlpha.fx' : 'overlayalpha_shader',
    'Gradient.fx' : 'gradient_shader',
    'CS_ZoomOffset.fx' : 'zoomoffset_shader',

    # missing effects
    'CRT.fx' : 'dummy_shader',
    'FlipY.fx' : 'dummy_shader',

    # unsupported in HWA
    'XOR' : 'dummy_shader',
    'AND' : 'dummy_shader',
    'OR' : 'dummy_shader'
}

def get_name(name):
    return NATIVE_SHADERS[name]

VERTEX_REPLACEMENTS = {
    'gl_MultiTexCoord0': 'in_tex_coord1',
    'gl_MultiTexCoord1': 'in_tex_coord2',
    'gl_Vertex': 'in_pos',
    'gl_Color': 'in_blend_color',
    'gl_FrontColor': 'blend_color',
    'gl_ModelViewProjectionMatrix': '1.0'
}

FRAGMENT_REPLACEMENTS = {
    'gl_Color': 'blend_color'
}

REPLACEMENTS = {
    'vertex': VERTEX_REPLACEMENTS,
    'fragment': FRAGMENT_REPLACEMENTS,
}

SHADER_TYPES = {
    'blend_color': 'varying vec4 blend_color',
    'in_tex_coord1': 'attribute vec2 in_tex_coord1',
    'in_tex_coord2': 'attribute vec2 in_tex_coord2',
    'in_pos': 'attribute vec4 in_pos',
    'in_blend_color': 'attribute vec4 in_blend_color'
}

def translate_shader_data(data, typ, profile):
    # translates GLSL shaders to GLES-compatible ones
    features = set()
    for k, v in REPLACEMENTS[typ].iteritems():
        if not k in data:
            continue
        data = data.replace(k, v)
        features.add(v)

    lines = data.splitlines()
    if lines[0].strip() != '#version 120':
        raise NotImplementedError()
    if profile == 'gles':
        lines.pop(0)
        lines.insert(0, 'precision mediump float;')
    for feature in features:
        new_typ = SHADER_TYPES.get(feature, None)
        if not new_typ:
            continue
        new_line = '%s;' % (new_typ)
        lines.insert(1, new_line)
    return '\n'.join(lines)

def translate_shader_path(path, typ, out_path, profile):
    with open(path, 'rU') as fp:
        data = fp.read()
    data = translate_shader_data(data, typ, profile)
    open(out_path, 'wb').write(data)

def translate_program(name, out_dir, profile):
    makedirs(out_dir)
    shader_path = os.path.join(get_base_path(), 'shaders')
    vert_path = os.path.join(shader_path, '%s.vert' % name)
    frag_path = os.path.join(shader_path, '%s.frag' % name)
    new_vert_path = os.path.join(out_dir, '%s.vert' % name)
    new_frag_path = os.path.join(out_dir, '%s.frag' % name)
    translate_shader_path(vert_path, 'vertex', new_vert_path, profile)
    translate_shader_path(frag_path, 'fragment', new_frag_path, profile)
    return new_vert_path, new_frag_path

def get_shader_programs():
    shaders = set()
    for path in os.listdir(os.path.join(get_base_path(), 'shaders')):
        shaders.add(os.path.splitext(os.path.basename(path))[0])
    return shaders

def main():
    out_dir = os.path.join(os.getcwd(), 'glesshaders')
    for name in get_shader_programs():
        translate_program(name, out_dir, 'gles')
        print 'Translated shader', name

if __name__ == '__main__':
    main()
