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

void PerspectiveObject::draw()
{
    begin_draw();

    glc_copy_color_buffer_rect(texture, x, y, x + width, y + height);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(back_texcoords[0], back_texcoords[1]);
    glVertex2d(x, y);
    glTexCoord2f(back_texcoords[2], back_texcoords[3]);
    glVertex2d(x + width, y);
    glTexCoord2f(back_texcoords[4], back_texcoords[5]);
    glVertex2d(x + width, y + height);
    glTexCoord2f(back_texcoords[6], back_texcoords[7]);
    glVertex2d(x, y + height);
    glEnd();
    glDisable(GL_TEXTURE_2D);

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
