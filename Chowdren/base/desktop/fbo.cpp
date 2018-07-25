#include "fbo.h"
#include "chowconfig.h"

static Framebuffer * current_fbo = NULL;

Framebuffer::Framebuffer(int w, int h)
{
    init(w, h);
}

Framebuffer::Framebuffer()
{
}

Framebuffer::~Framebuffer()
{
}

void Framebuffer::init(int w, int h)
{
    // for fullscreen or window resize
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 NULL);
#ifdef CHOWDREN_POINT_FILTER
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenFramebuffers(1, &fbo);
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, tex, 0);
    unbind();
}

void Framebuffer::bind()
{
    old_fbo = current_fbo;
    current_fbo = this;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Framebuffer::unbind()
{
    if (old_fbo == NULL)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    else
        glBindFramebuffer(GL_FRAMEBUFFER, old_fbo->fbo);
    current_fbo = old_fbo;
}

GLuint Framebuffer::get_tex()
{
    return tex;
}
