#ifndef CHOWDREN_ALPHAIMAGE_H
#define CHOWDREN_ALPHAIMAGE_H

#include "frameobject.h"

/*
This object is actually never rendered, but Heart Forth, Alicia depends on
the storage of some values.
*/

class AlphaImageObject : public FrameObject
{
public:
    int image;
    int anim_frame;
    int hotspot_x, hotspot_y;
    double angle;
    int start_width, start_height;

    AlphaImageObject(int x, int y, int type_id);
    void set_image(int index);
    void set_hotspot(int x, int y);
};

#endif // CHOWDREN_ALPHAIMAGE_H
