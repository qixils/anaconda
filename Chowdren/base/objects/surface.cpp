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
