#ifndef CHOWDREN_FBO_H
#define CHOWDREN_FBO_H

#include "include_gl.h"

class Framebuffer
{
public:
    GLuint tex;
    GLuint fbo;
    Framebuffer * old_fbo;

    Framebuffer(int w, int h);
    Framebuffer();
    ~Framebuffer();
    void init(int w, int h);
    void bind();
    void unbind();
    GLuint get_tex();
};

#endif // CHOWDREN_FBO_H
