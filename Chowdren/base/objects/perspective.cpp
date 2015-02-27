#include "objects/perspective.h"
#include "include_gl.h"
#include "collision.h"
#include "shader.h"
#include <iostream>

PerspectiveObject::PerspectiveObject(int x, int y, int type_id)
: FrameObject(x, y, type_id)
{
    collision = new InstanceBox(this);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    set_shader(perspective_shader);
}

PerspectiveObject::~PerspectiveObject()
{
    glDeleteTextures(1, &texture);
    delete collision;
}

void PerspectiveObject::set_width(int width)
{
    this->width = width;
    collision->update_aabb();
}

void PerspectiveObject::draw()
{
    int box[4];
    get_screen_aabb(box);
    glc_copy_color_buffer_rect(texture, box[0], box[1],
                               box[2], box[3]);

    begin_draw();
    Render::disable_blend();
    Render::draw_tex(x, y, x + width, y + height, Color(255, 255, 255, 255),
                     texture,
                     back_texcoords[0], back_texcoords[1],
                     back_texcoords[4], back_texcoords[5]);
    Render::enable_blend();
    end_draw();
}

void PerspectiveObject::set_waves(double value)
{
    set_shader_parameter("sine_waves", value);
}

void PerspectiveObject::set_zoom(double value)
{
    set_shader_parameter("zoom", value);
}

void PerspectiveObject::set_offset(double value)
{
    set_shader_parameter("offset", value);
}
