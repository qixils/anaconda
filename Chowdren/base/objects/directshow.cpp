#include "objects/directshow.h"
#include "include_gl.h"
#include "collision.h"
#include <iostream>

DirectShow::DirectShow(int x, int y, int type_id)
: FrameObject(x, y, type_id)
{
    collision = new InstanceBox(this);
}

DirectShow::~DirectShow()
{
    delete collision;
}
