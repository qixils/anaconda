#include "include_gl.h"
#include "filecommon.h"
#include "image.h"
#include <algorithm>

class FrameObject;

void set_shader_path(const std::string & path);
void init_shaders();

class Shader
{
public:
    virtual void begin(FrameObject * instance, Image * image) {}
    virtual void end(FrameObject * instance) {}
    virtual GLuint get_background_texture()
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
