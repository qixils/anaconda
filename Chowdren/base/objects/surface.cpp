#include "objects/surface.h"
#include "include_gl.h"
#include "collision.h"
#include <iostream>

SurfaceObject::SurfaceObject(int x, int y, int type_id)
: FrameObject(x, y, type_id)
{
    collision = new InstanceBox(this);
}

void SurfaceObject::draw()
{
}

void SurfaceObject::resize(int w, int h)
{
    width = w;
    height = h;
    collision->update_aabb();
}

void SurfaceObject::resize_canvas(int x, int y, int w, int h)
{
    std::cout << "Resize canvas: " << x << " " << y << " "
        << w << " " << h << std::endl;
}

void SurfaceObject::load(const std::string & filename,
                         const std::string & ignore_ext)
{
    std::cout << "Load surface image: " << filename << std::endl;
}

void SurfaceObject::set_stretch_mode(int v)
{
    // 0: no stretch, 1: no resampling, 2: resampling, 3: opaque resampling
    std::cout << "Set stretch mode: " << v << std::endl;
}

void SurfaceObject::set_dest_pos(int x, int y)
{
    std::cout << "Set blit dest pos: " << x << " " << y << std::endl;
}

void SurfaceObject::set_dest_size(int w, int h)
{
    std::cout << "Set blit dest size: " << w << " " << h << std::endl;
}

void SurfaceObject::scroll(int x, int y, int wrap)
{
    std::cout << "Scroll surface: " << x << " " << y << " " << wrap
        << std::endl;
}

void SurfaceObject::blit(Active * obj)
{
    std::cout << "Blit onto surface" << std::endl;
}

void SurfaceObject::set_effect(int index)
{
    std::cout << "Set Surface effect: " << index << std::endl;
}

void SurfaceObject::set_image(int index)
{
    std::cout << "Set Surface image: " << index << std::endl;
}

void SurfaceObject::set_edit_image(int index)
{
    std::cout << "Set Surface edit image: " << index << std::endl;
}

void SurfaceObject::set_transparent_color(const Color & color)
{
    std::cout << "Set Surface transparent color" << std::endl;
}

void SurfaceObject::create_alpha(int index)
{
    std::cout << "Create Surface alpha: " << index << std::endl;
}

void SurfaceObject::clear(const Color & color)
{
    std::cout << "Clear Surface: "
        << color.r << " " << color.g << " " << color.b << std::endl;
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
    std::cout << "Surface reverse X not implemented" << std::endl;
}

void SurfaceObject::add_image(int w, int h)
{
    std::cout << "Surface add image not implemented" << std::endl;
}

void SurfaceObject::set_transparent_color(const Color & color, bool replace)
{
    std::cout << "Surface set transparent color not implemented" << std::endl;
}

int SurfaceObject::get_edit_width()
{
    std::cout << "Surface get editing image width not implemented"
        << std::endl;
    return 0;
}

int SurfaceObject::get_image_width(int index)
{
    std::cout << "Surface get image width not implemented: " << index
        << std::endl;
    return 0;
}
