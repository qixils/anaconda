#include "objects/alphaimage.h"

AlphaImageObject::AlphaImageObject(int x, int y, int type_id)
: FrameObject(x, y, type_id)
{
}

AlphaImageObject::set_image(int index)
{
    image = index;
}