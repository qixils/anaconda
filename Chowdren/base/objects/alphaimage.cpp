#include "objects/alphaimage.h"

AlphaImageObject::AlphaImageObject(int x, int y, int type_id)
: FrameObject(x, y, type_id), anim_frame(0)
{
    start_width = width;
    start_height = height;
}

void AlphaImageObject::set_image(int index)
{
    image = index;
}

void AlphaImageObject::set_hotspot(int x, int y)
{
    hotspot_x = x;
    hotspot_y = y;
}