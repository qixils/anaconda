#include "colorizer.h"

ColorizerObject::ColorizerObject(int x, int y, int id)
: FrameObject(x, y, id), r(1.0f), g(1.0f), b(1.0f)
{

}

void ColorizerObject::set_red(float v)
{
    r = v;
}

void ColorizerObject::set_green(float v)
{
    g = v;
}

void ColorizerObject::set_blue(float v)
{
    b = v;
}