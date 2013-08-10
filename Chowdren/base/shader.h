#ifndef CHOWDREN_SHADER_H
#define CHOWDREN_SHADER_H

#include <string>

class FrameObject;
class Image;

extern std::string shader_path;
void set_shader_path(const std::string & path);
void init_shaders();
void convert_vec4(int value, float & a, float & b, float & c, float & d);

class Shader
{
public:
    virtual void begin(FrameObject * instance, Image * image) {}
    virtual void end(FrameObject * instance) {}
    virtual unsigned int get_background_texture()
    {
        return 0;
    }
};

extern Shader * subtract_shader;
extern Shader * additive_shader;
extern Shader * monochrome_shader;
extern Shader * mixer_shader;
extern Shader * hue_shader;
extern Shader * dummy_shader;
extern Shader * blend_shader;
extern Shader * offset_shader;
extern Shader * dodgeblur_shader;
#ifndef CHOWDREN_IS_DESKTOP
extern Shader * basic_shader;
extern Shader * texture_shader;
#endif

#endif // CHOWDREN_SHADER_H
