#include "objects/perspective.h"
#include "include_gl.h"
#include "collision.h"
#include <iostream>

PerspectiveObject::PerspectiveObject(int x, int y, int type_id)
: FrameObject(x, y, type_id), zoom(0), offset(0)
{
    collision = new InstanceBox(this);
}

void PerspectiveObject::draw()
{
}

void PerspectiveObject::set_waves(double value)
{
    std::cout << "Set waves: " << value << std::endl;
}

void PerspectiveObject::set_zoom(double value)
{
    zoom = value;
    std::cout << "Set Perspective zoom: " << value << std::endl;
}

void PerspectiveObject::set_offset(double value)
{
    offset = value;
    std::cout << "Set Perspective offset: " << value << std::endl;
}
