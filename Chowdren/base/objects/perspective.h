#ifndef CHOWDREN_PERSPECTIVE_H
#define CHOWDREN_PERSPECTIVE_H

#include "frameobject.h"

class PerspectiveObject : public FrameObject
{
public:
    double zoom;
    double offset;

    PerspectiveObject(int x, int y, int type_id);
    void draw();
    void set_waves(double value);
    void set_zoom(double value);
    void set_offset(double value);
};

#endif // CHOWDREN_PERSPECTIVE_H
