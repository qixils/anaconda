#include "include_gl.h"

void Render::set_view(int x, int y, int w, int h)
{
    glViewport(x, y, w, h);
}

void Render::clear(Color color)
{
    glClearColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f,
                 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Render::set_filter(Texture tex, bool linear)
{
    glBindTexture(GL_TEXTURE_2D, tex);
    if (linear) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
}

Texture Render::create_tex(void * pixels, Format f, int width, int height)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    GLenum format;
    switch (f) {
        case Format::RGBA:
            format = GL_RGBA;
            break;
        case Format::L:
            format = GL_ALPHA;
            break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return tex;
}
