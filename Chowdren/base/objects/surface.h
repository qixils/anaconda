#ifndef CHOWDREN_SURFACE_H
#define CHOWDREN_SURFACE_H

#include "frameobject.h"

class SurfaceObject : public FrameObject
{
public:
    SurfaceObject(int x, int y, int type_id);
    void draw();
    void load(const std::string & filename, const std::string & ignore_ext);
    void resize(int w, int h);
    void resize_canvas(int x, int y, int w, int h);
};

#endif // CHOWDREN_SURFACE_H
