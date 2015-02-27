#include "include_gl.h"

// XXX consider merging drawcalls
#define RENDER_BUFFER 1

struct RenderData
{
    Render::Effect effect;
    Render::Effect last_effect;
    bool tex1, tex2, tex3;
    float trans_x, trans_y;
    float positions[(RENDER_BUFFER * 3) * 6];
    unsigned int colors[RENDER_BUFFER * 6];
    float texcoord1[(RENDER_BUFFER * 2) * 6];
    float texcoord2[(RENDER_BUFFER * 2) * 6];
};

extern RenderData render_data;

inline void Render::set_offset(int x, int y)
{
    offset[0] = x;
    offset[1] = y;
}

inline void Render::set_view(int x, int y, int w, int h)
{
    glViewport(x, y, w, h);

    render_data.trans_x = 2.0f / w;
    render_data.trans_y = 2.0f / h;
}

inline void Render::clear(Color color)
{
    glClearColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f,
                 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

inline void Render::set_filter(Texture tex, bool linear)
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

inline Texture Render::create_tex(void * pixels, Format f,
                                  int width, int height)
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

inline void insert_quad(int x1, int y1, int x2, int y2)
{
    x1 -= Render::offset[0];
    y1 -= Render::offset[1];
    x2 -= Render::offset[0];
    y2 -= Render::offset[1];
    float fx1 = x1 * render_data.trans_x - 1.0f;
    float fy1 = y1 * render_data.trans_y - 1.0f;
    float fx2 = x2 * render_data.trans_x - 1.0f;
    float fy2 = y2 * render_data.trans_y - 1.0f;

    float * p = &render_data.positions[0];

    // 1
    *p++ = fx1; *p++ = fy1;
    *p++ = fx2; *p++ = fy1;
    *p++ = fx2; *p++ = fy2;

    // 2
    *p++ = fx2; *p++ = fy2;
    *p++ = fx1; *p++ = fy2;
    *p++ = fx1; *p++ = fy1;
}

inline void insert_color(Color c)
{
    unsigned int cc;
    // rely on endianness
    memcpy(&cc, &c, sizeof(Color));

    unsigned int * p = &render_data.colors[0];
    for (int i = 0; i < 6; ++i)
        *p++ = cc;
}

const float render_texcoords[12] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f
};

inline void insert_texcoord1()
{
    memcpy(&render_data.texcoord1[0], &render_texcoords[0],
           sizeof(render_texcoords));
}

inline void insert_texcoord2()
{
    memcpy(&render_data.texcoord2[0], &render_texcoords2[0],
           sizeof(render_texcoords2));
}

inline void begin_draw()
{
    if (render_data.effect == render_data.last_effect)
        return;
    shader_set_effect(render_data.effect);
    render_data.last_effect = render_data.effect;
}

inline void Render::draw_quad(int x1, int y1, int x2, int y2, Color c)
{
    begin_draw();

    insert_quad(x1, y1, x2, y2);
    insert_color(c);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

inline void Render::draw_tex(int x1, int y1, int x2, int y2, Color c,
                             Texture t)
{
    begin_draw();

    insert_quad(x1, y1, x2, y2);
    insert_color(c);
    insert_texcoord1();
    glBindTexture(GL_TEXTURE_2D, t);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    render_data.last_effect = render_data.effect;
}

inline void Render::enable_blend()
{
    glEnable(GL_BLEND);
}

inline void Render::disable_blend()
{
    glDisable(GL_BLEND);
}
