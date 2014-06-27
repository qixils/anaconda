#ifndef CHOWDREN_SURFACE_H
#define CHOWDREN_SURFACE_H

#include "frameobject.h"

class Active;

class SurfaceObject : public FrameObject
{
public:
    SurfaceObject(int x, int y, int type_id);
    ~SurfaceObject();
    void draw();
    void load(const std::string & filename, const std::string & ignore_ext);
    void resize(int w, int h);
    void resize_canvas(int x, int y, int w, int h);
    void set_stretch_mode(int mode);
    void set_dest_pos(int x, int y);
    void set_dest_size(int w, int h);
    void blit(Active * obj);
    void blit_background();
    void blit_alpha(int image);
    void set_effect(int index);
    void set_image(int index);
    void set_edit_image(int index);
    void set_transparent_color(const Color & color);
    void create_alpha(int index);
    void clear(const Color & color);
    void clear(int value);
    void apply_matrix(double div, double offset, double iterations,
                      double x1y1, double x1y2, double x1y3,
                      double x2y1, double x2y2, double x2y3,
                      double x3y1, double x3y2, double x3y3);
    void save(const std::string & filename, const std::string & ext);
    void reverse_x();
    void add_image(int w, int h);
    void set_transparent_color(const Color & color, bool replace);
    int get_edit_width();
    int get_image_width(int index);
    void scroll(int x, int y, int wrap);
};

#endif // CHOWDREN_SURFACE_H
