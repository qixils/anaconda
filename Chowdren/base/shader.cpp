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
    : GLSLShader("offset", true)
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
    : GLSLShader("dodgeblur", true)
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
    int fStrength, fSeed;
    int iInvert;
    int iR, iG, iB, iA;

    GrainShader()
    : GLSLShader("grain", false)
    {

    }

    void initialize_parameters()
    {
        fStrength = get_uniform("fStrength");
        fSeed = get_uniform("fSeed");
        iInvert = get_uniform("iInvert");
        iR = get_uniform("iR");
        iG = get_uniform("iG");
        iB = get_uniform("iB");
        iA = get_uniform("iA");
    }

    void set_parameters(FrameObject * instance)
    {
        set_float(instance, "fStrength", fStrength);
        set_float(instance, "fSeed", fSeed);
        set_int(instance, "iInvert", iInvert);
        set_int(instance, "iR", iR);
        set_int(instance, "iG", iG);
        set_int(instance, "iB", iB);
        set_int(instance, "iA", iA);
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
#ifndef CHOWDREN_USE_GL
    basic_shader = new BasicShader;
    texture_shader = new TextureShader;
#endif
    glc_init_shaders();
}
