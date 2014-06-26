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

void SurfaceObject::set_dest_pos(int w, int h)
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
