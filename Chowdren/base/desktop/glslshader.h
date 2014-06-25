#include "../shader.h"

class GLSLShader : public Shader
{
public:
    static GLSLShader * current;
#ifndef CHOWDREN_USE_GL
    GLint blend_color;
#endif
    GLuint program;
    GLint size_uniform;
    bool initialized;
    std::string name;
    bool has_background;
    bool has_tex_param;
    bool param_tex_set; // whether the uniform storing the texture slot index was set yet

    GLSLShader(const std::string & name, bool has_back = false, bool has_tex_param = false);
    void initialize();
    GLuint attach_source(const std::string & ext, GLenum type);
    GLuint get_background_texture();
    bool has_texture_param();
    int get_uniform(const char * value);
    virtual void initialize_parameters();
    void begin(FrameObject * instance, Image * image);
    virtual void set_parameters(FrameObject * instance);
    void end(FrameObject * instance);
    void set_int(FrameObject * instance, const std::string & name,
                 int uniform);
    void set_float(FrameObject * instance, const std::string & name,
                   int uniform);
    void set_vec4(FrameObject * instance, const std::string & name,
                  int uniform);
    void set_image(FrameObject * instance, const std::string & name,
                   int uniform);
};