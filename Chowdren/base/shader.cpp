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
        glBlendEquation(GL_FUNC_ADD);
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
    void * r;
    void * g;
    void * b;

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
    void * hue;

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
    void * width;
    void * height;

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

class DodgeBlurShader : public GLSLShader
{
public:
    void * vertical;
    void * radius;

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

Shader * subtract_shader;
Shader * additive_shader;
Shader * monochrome_shader;
Shader * mixer_shader;
Shader * hue_shader;
Shader * dummy_shader;
Shader * blend_shader;
Shader * offset_shader;
Shader * dodgeblur_shader;

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
    init_shaders_platform();
}