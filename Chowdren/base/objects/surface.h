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
    void set_stretch_mode(int mode);
    void set_dest_pos(int x, int y);
    void set_dest_size(int w, int h);
    void blit(Active * obj);
    void set_effect(int index);
};

#endif // CHOWDREN_SURFACE_H
