#ifndef CHOWDREN_SYSTEMBOX_H
#define CHOWDREN_SYSTEMBOX_H

#include "frameobject.h"

#define PATTERN_IMAGE 0
#define CENTER_IMAGE 1
#define TOPLEFT_IMAGE 2

class SystemBox : public FrameObject
{
public:
    Image * image;
    int type;

    SystemBox(int x, int y, int type_id);
    void draw();
};

#endif // CHOWDREN_SYSTEMBOX_H
