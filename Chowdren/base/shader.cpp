#include "shader.h"
#include "include_gl.h"
#include "platform.h"

std::string shader_path = "./shaders";

void set_shader_path(const std::string & path)
{
    shader_path = path;
}

void convert_vec4(int val, float & a, float & b, float & c, float & d)
{
    a = (val & 0xFF) / 255.0f;
    b = ((val >> 8) & 0xFF) / 255.0f;
    c = ((val >> 16) & 0xFF) / 255.0f;
    d = ((val >> 24) & 0xFF) / 255.0f;
}

class SubtractShader : public Shader
{
public:
    void begin(FrameObject * instance, Image * image)
    {
        glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
        glBlendFunc(GL_DST_COLOR, GL_ONE);
    }

    void end(FrameObject * instance)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
    }
};

class AdditiveShader : public Shader
{
public:
    void begin(FrameObject * instance, Image * image)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }

    void end(FrameObject * instance)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
};

#include "glslshader.h"

class MonochromeShader : public GLSLShader
{
public:
    MonochromeShader()
    : GLSLShader("monochrome")
    {

    }
};

class MixerShader : public GLSLShader
{
public:
    int r;
    int g;
    int b;

    MixerShader()
    : GLSLShader("colormixer")
    {

    }

    void initialize_parameters()
    {
        r = get_uniform("r");
        g = get_uniform("g");
        b = get_uniform("b");
    }

    void set_parameters(FrameObject * instance)
    {
        set_vec4(instance, "r", r);
        set_vec4(instance, "g", g);
        set_vec4(instance, "b", b);
    }
};

class HueShader : public GLSLShader
{
public:
    int hue;

    HueShader()
    : GLSLShader("hue")
    {

    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, "fHue", hue);
    }

    void initialize_parameters()
    {
        hue = get_uniform("fHue");
    }
};

class OffsetShader : public GLSLShader
{
public:
    int width;
    int height;

    OffsetShader()
    : GLSLShader("offset", GLSL_HAS_BACK | GLSL_HAS_TEX_SIZE)
    {

    }

    void initialize_parameters()
    {
        width = get_uniform("width");
        height = get_uniform("height");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, "width", width);
        set_float(instance, "height", height);
    }
};

class InvertShader : public GLSLShader
{
public:
    InvertShader()
    : GLSLShader("invert")
    {

    }
};

class DodgeBlurShader : public GLSLShader
{
public:
    int vertical;
    int radius;

    DodgeBlurShader()
    : GLSLShader("dodgeblur", GLSL_HAS_BACK | GLSL_HAS_TEX_SIZE)
    {

    }

    void initialize_parameters()
    {
        vertical = get_uniform("vertical");
        radius = get_uniform("radius");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, "vertical", vertical);
        set_float(instance, "radius", radius);
    }
};

class GrainShader : public GLSLShader
{
public:
    int strength, seed;
    int invert;
    int r, g, b, a;

    GrainShader()
    : GLSLShader("grain")
    {

    }

    void initialize_parameters()
    {
        strength = get_uniform("fStrength");
        seed = get_uniform("fSeed");
        invert = get_uniform("iInvert");
        r = get_uniform("iR");
        g = get_uniform("iG");
        b = get_uniform("iB");
        a = get_uniform("iA");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, "fStrength", strength);
        set_float(instance, "fSeed", seed);
        set_int(instance, "iInvert", invert);
        set_int(instance, "iR", r);
        set_int(instance, "iG", g);
        set_int(instance, "iB", b);
        set_int(instance, "iA", a);
    }
};

class MultiplyShader : public GLSLShader
{
public:
    MultiplyShader()
    : GLSLShader("multiply", GLSL_HAS_BACK | GLSL_HAS_TEX_SIZE)
    {
    }
};

class HardLightShader : public GLSLShader
{
public:
    HardLightShader()
    : GLSLShader("hardlight", GLSL_HAS_BACK | GLSL_HAS_TEX_SIZE)
    {
    }
};

class TintShader : public GLSLShader
{
public:
    int tint_color;
    int tint_power;
    int original_power;

    TintShader()
    : GLSLShader("tint")
    {

    }

    void initialize_parameters()
    {
        tint_color = get_uniform("fTintColor");
        tint_power = get_uniform("fTintPower");
        original_power = get_uniform("fOriginalPower");
    }

    void set_parameters(FrameObject * instance)
    {
        set_vec4(instance, "fTintColor", tint_color);
        set_float(instance, "fTintPower", tint_power);
        set_float(instance, "fOriginalPower", original_power);
    }
};

class ChannelBlurShader : public GLSLShader
{
public:
    int coeff;
    int r, g, b, a;

    ChannelBlurShader()
    : GLSLShader("channelblur")
    {
    }

    void initialize_parameters()
    {
        coeff = get_uniform("fCoeff");
        r = get_uniform("iR");
        g = get_uniform("iG");
        b = get_uniform("iB");
        a = get_uniform("iA");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, "fCoeff", coeff);
        set_int(instance, "iR", r);
        set_int(instance, "iG", g);
        set_int(instance, "iB", b);
        set_int(instance, "iA", a);
    }
};

class BgBloomShader : public GLSLShader
{
public:
    int radius;
    int exponent;
    int coeff;

    BgBloomShader()
    : GLSLShader("bgbloom", GLSL_HAS_BACK | GLSL_HAS_TEX_SIZE)
    {
    }

    void initialize_parameters()
    {
        coeff = get_uniform("coeff");
        radius = get_uniform("radius");
        exponent = get_uniform("exponent");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, "coeff", coeff);
        set_float(instance, "exponent", exponent);
        set_float(instance, "radius", radius);
    }
};

class UnderwaterShader : public GLSLShader
{
public:
    int blur;
    int amplitudeX;
    int periodsX;
    int freqX;
    int amplitudeY;
    int periodsY;
    int freqY;

    UnderwaterShader()
    : GLSLShader("underwater")
    {
    }

    void initialize_parameters()
    {
        blur = get_uniform("fBlur");
        amplitudeX = get_uniform("fAmplitudeX");
        periodsX = get_uniform("fPeriodsX");
        freqX = get_uniform("fFreqX");
        amplitudeY = get_uniform("fAmplitudeY");
        periodsY = get_uniform("fPeriodsY");
        freqY = get_uniform("fFreqY");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, "fBlur", blur);
        set_float(instance, "fAmplitudeX", amplitudeX);
        set_float(instance, "fPeriodsX", periodsX);
        set_float(instance, "fFreqX", freqX);
        set_float(instance, "fAmplitudeY", amplitudeY);
        set_float(instance, "fPeriodsY", periodsY);
        set_float(instance, "fFreqY", freqY);
    }
};

class RotateSubShader : public GLSLShader
{
public:
    int angle, x, y, shift_x, shift_y;

    RotateSubShader()
    : GLSLShader("rotatesub")
    {
    }

    void initialize_parameters()
    {
        angle = get_uniform("fA");
        x = get_uniform("fX");
        y = get_uniform("fY");
        shift_x = get_uniform("fSx");
        shift_y = get_uniform("fSy");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, "fA", angle);
        set_float(instance, "fX", x);
        set_float(instance, "fY", y);
        set_float(instance, "fSx", shift_x);
        set_float(instance, "fSy", shift_y);
    }
};

class SimpleMaskShader : public GLSLShader
{
public:
    int fade, color;

    SimpleMaskShader()
    : GLSLShader("simplemask")
    {
    }

    void initialize_parameters()
    {
        color = get_uniform("fC");
        fade = get_uniform("fFade");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, "fFade", fade);
        set_vec4(instance, "fC", color);
    }
};

class OffsetStationaryShader : public GLSLShader
{
public:

    int width, height, x_offset, y_offset;

    OffsetStationaryShader()
    : GLSLShader("offsetstationary")
    {
    }

    void initialize_parameters()
    {
        width = get_uniform("width");
        height = get_uniform("height");
        x_offset = get_uniform("xoff");
        y_offset = get_uniform("yoff");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, "width", width);
        set_float(instance, "height", height);
        set_float(instance, "xoff", x_offset);
        set_float(instance, "yoff", y_offset);
    }
};

class PatternOverlayShader : public GLSLShader
{
public:
    int x, y, width, height;
    int alpha;

    PatternOverlayShader()
    : GLSLShader("patternoverlay", GLSL_HAS_BACK | GLSL_HAS_TEX_SIZE,
        "pattern")
    {
    }

    void initialize_parameters()
    {
        x = get_uniform("x");
        y = get_uniform("y");
        width = get_uniform("width");
        height = get_uniform("height");
        alpha = get_uniform("alpha");
    }

    void set_parameters(FrameObject * instance)
    {
        set_image(instance, "pattern");
        set_float(instance, "width", width);
        set_float(instance, "height", height);
        set_float(instance, "x", x);
        set_float(instance, "y", y);
        set_float(instance, "alpha", alpha);
    }
};

class SubPxShader : public GLSLShader
{
public:
    int x, y, limit;

    SubPxShader()
    : GLSLShader("subpx", GLSL_HAS_TEX_SIZE)
    {
    }

    void initialize_parameters()
    {
        x = get_uniform("x");
        y = get_uniform("y");
        limit = get_uniform("limit");
    }

    void set_parameters(FrameObject * instance)
    {
        set_int(instance, "limit", limit);
        set_float(instance, "x", x);
        set_float(instance, "y", y);
    }
};

#ifndef CHOWDREN_USE_GL
class BasicShader : public GLSLShader
{
public:
    BasicShader()
    : GLSLShader("basic", false)
    {
    }
};

class TextureShader : public GLSLShader
{
public:
    TextureShader()
    : GLSLShader("texture", false)
    {
    }
};
#endif

Shader * subtract_shader;
Shader * additive_shader;
Shader * monochrome_shader;
Shader * mixer_shader;
Shader * hue_shader;
Shader * dummy_shader;
Shader * blend_shader;
Shader * offset_shader;
Shader * dodgeblur_shader;
Shader * invert_shader;
Shader * grain_shader;
Shader * multiply_shader;
Shader * hardlight_shader;
Shader * tint_shader;
Shader * channelblur_shader;
Shader * bgbloom_shader;
Shader * underwater_shader;
Shader * rotatesub_shader;
Shader * simplemask_shader;
Shader * offsetstationary_shader;
Shader * patternoverlay_shader;
Shader * subpx_shader;
#ifndef CHOWDREN_USE_GL
Shader * basic_shader;
Shader * texture_shader;
#endif

void init_shaders()
{
    subtract_shader = new SubtractShader;
    additive_shader = new AdditiveShader;
    monochrome_shader = new MonochromeShader;
    mixer_shader = new MixerShader;
    hue_shader = new HueShader;
    dummy_shader = new Shader;
    blend_shader = new Shader;
    offset_shader = new OffsetShader;
    dodgeblur_shader = new DodgeBlurShader;
    invert_shader = new InvertShader;
    grain_shader = new GrainShader;
    multiply_shader = new MultiplyShader;
    hardlight_shader = new HardLightShader;
    tint_shader = new TintShader;
    channelblur_shader = new ChannelBlurShader;
    bgbloom_shader = new BgBloomShader;
    underwater_shader = new UnderwaterShader;
    rotatesub_shader = new RotateSubShader;
    simplemask_shader = new SimpleMaskShader;
    offsetstationary_shader = new OffsetStationaryShader;
    patternoverlay_shader = new PatternOverlayShader;
    subpx_shader = new SubPxShader;
#ifndef CHOWDREN_USE_GL
    basic_shader = new BasicShader;
    texture_shader = new TextureShader;
#endif
    glc_init_shaders();
}
