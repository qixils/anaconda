#ifndef CHOWDREN_DIRECTSHOW_H
#define CHOWDREN_DIRECTSHOW_H

#include "frameobject.h"
#include "color.h"
#include <string>

class DirectShow : public FrameObject
{
public:
    FRAMEOBJECT_HEAD(DirectShow)

    DirectShow(int x, int y, int type_id);
    ~DirectShow();
};

#endif // CHOWDREN_DIRECTSHOW_H
