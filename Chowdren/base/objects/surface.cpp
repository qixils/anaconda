#include "objects/surface.h"
#include "include_gl.h"
#include "collision.h"
#include <iostream>
#include "shader.h"
#include "mathcommon.h"

SurfaceObject::SurfaceObject(int x, int y, int type_id)
: FrameObject(x, y, type_id), scroll_x(0), scroll_y(0), wrap(false)
{
    collision = new InstanceBox(this);
}

SurfaceObject::~SurfaceObject()
{
    delete collision;
}

void SurfaceObject::draw()
{
    if (image == NULL)
        return;

    // need to get light system working first
    if (shader == coldirblur_shader)
        return;

    double scale_x = width / double(image->width);
    double scale_y = height / double(image->height);

    blend_color.apply();

    glEnable(GL_SCISSOR_TEST);
    glc_scissor_world(x, y, canvas_width, canvas_height);

    if ((scroll_x == 0 && scroll_y == 0) || !wrap) {
        draw_image(image, x + scroll_x, y + scroll_y, 0.0, scale_x, scale_y,
                   has_reverse_x);
    } else {
        int start_x = x - (image->width - scroll_x);
        int start_y = y - (image->height - scroll_y);
        for (int xx = start_x; xx < x + width; xx += image->width)
        for (int yy = start_y; yy < y + height; yy += image->height) {
            draw_image(image, xx, yy, 0.0, 1.0, 1.0, has_reverse_x);
        }
    }

    glDisable(GL_SCISSOR_TEST);
}

void SurfaceObject::resize(int w, int h)
{
    width = w;
    height = h;
    collision->update_aabb();
    canvas_width = width;
    canvas_height = height;
}

void SurfaceObject::resize_canvas(int x1, int y1, int x2, int y2)
{
    canvas_width = int_min(canvas_width, x2 - x1);
    canvas_height = int_min(canvas_height, y2 - y1);
    collision->update_aabb();
}

void SurfaceObject::load(const std::string & filename,
                         const std::string & ignore_ext)
{
    scroll_x = scroll_y = 0;
    wrap = false;
    has_reverse_x = false;
    if (filename == this->filename)
        return;
    this->filename = filename;
    std::string path = convert_path(filename);

    Color * trans = NULL;
    if (has_transparent)
        trans = &transparent;

    image = get_image_cache(path, 0, 0, 0, 0, trans);

    if (!image)
        return;

    resize(image->width, image->height);
}

void SurfaceObject::set_stretch_mode(int v)
{
    // 0: no stretch, 1: no resampling, 2: resampling, 3: opaque resampling
    // std::cout << "Set stretch mode: " << v << std::endl;
}

void SurfaceObject::set_dest_pos(int x, int y)
{
    set_position(x, y);
}

void SurfaceObject::set_dest_size(int w, int h)
{
    width = w;
    height = h;
    collision->update_aabb();
}

void SurfaceObject::scroll(int x, int y, int wrap)
{
    if (image == NULL)
        return;
    scroll_x = (scroll_x + x) % image->width;
    scroll_y = (scroll_y + y) % image->height;
    this->wrap = wrap != 0;
}

void SurfaceObject::blit(Active * obj)
{
    // std::cout << "Blit onto surface" << std::endl;
}

void SurfaceObject::set_effect(int index)
{
    // std::cout << "Set Surface effect: " << index << std::endl;
}

void SurfaceObject::set_image(int index)
{
    if (index < 0 || index >= image_count)
        return;
    image = images[index];
    resize(image->width, image->height);
}

void SurfaceObject::set_edit_image(int index)
{
    // std::cout << "Set Surface edit image: " << index << std::endl;
}

void SurfaceObject::set_transparent_color(const Color & color)
{
    // std::cout << "Set Surface transparent color" << std::endl;
}

void SurfaceObject::create_alpha(int index)
{
    // std::cout << "Create Surface alpha: " << index << std::endl;
}

void SurfaceObject::clear(const Color & color)
{
    // std::cout << "Clear Surface: "
    //     << color.r << " " << color.g << " " << color.b << std::endl;
}

void SurfaceObject::clear(int value)
{
    clear(Color(value));
}

void SurfaceObject::blit_background()
{
    std::cout << "Blit Surface background" << std::endl;
}

void SurfaceObject::blit_alpha(int image)
{
    std::cout << "Blit onto image alpha channel: " << image << std::endl;
}

void SurfaceObject::apply_matrix(double div, double offset, double iterations,
                                 double x1y1, double x1y2, double x1y3,
                                 double x2y1, double x2y2, double x2y3,
                                 double x3y1, double x3y2, double x3y3)
{
    std::cout << "Apply matrix not implemented" << std::endl;
}

void SurfaceObject::save(const std::string & filename,
                         const std::string & ext)
{
    std::cout << "Surface save not implemented: " << filename << std::endl;
}

void SurfaceObject::reverse_x()
{
    has_reverse_x = !has_reverse_x;
}

void SurfaceObject::add_image(int w, int h)
{
    std::cout << "Surface add image not implemented" << std::endl;
}

void SurfaceObject::set_transparent_color(const Color & color, bool replace)
{
    has_transparent = true;
    transparent = color;
}

int SurfaceObject::get_edit_width()
{
    if (image == NULL)
        return 0;
    return image->width;
}

int SurfaceObject::get_image_width(int index)
{
    if (image == NULL)
        return 0;
    return image->width;
}
