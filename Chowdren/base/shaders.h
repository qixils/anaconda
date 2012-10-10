#include "include_gl.h"

class Shader
{
public:
    virtual void begin() {}
    virtual void end() {}
};

enum ParameterType
{

};

class GLSLShader : public Shader
{
public:
    std::map<std::string, int> parameter_types;

    void begin() 
    {

    }

    void end() 
    {

    }
};

class SubtractShader : public Shader
{
public:
    void begin()
    {
        glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
        glBlendFunc(GL_DST_COLOR, GL_ONE);
    }

    void end()
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
    }
};

class AdditiveShader : public Shader
{
public:
    void begin()
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }

    void end()
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
    }
};

static SubtractShader subtract_shader;
static AdditiveShader additive_shader;
static Shader dummy_shader;