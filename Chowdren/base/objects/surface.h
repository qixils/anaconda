#ifndef CHOWDREN_SURFACE_H
#define CHOWDREN_SURFACE_H

#include "frameobject.h"
#include "image.h"
#include <string>
#include "color.h"

class Active;

class SurfaceImage
{
public:
    Image * handle;
    Color transparent;
    bool has_transparent;

    // Simulated - actually modifies bitmap
    int width, height; // Resize
    int canvas_width, canvas_height; // Resize (canvas)
    int scroll_x, scroll_y; // Scroll, resize canvas
    bool wrap; // Scroll
    bool has_reverse_x; // Reverse X

    SurfaceImage()
    {
    }

    // Create blank image with specified size
    void reset(int w = 0, int h = 0);
    void set_image(Image * image);
    void draw(FrameObject * instance, int x, int y);

    int get_display_width()
    {
        if (handle == NULL)
            return 0;
        return canvas_width * width / double(handle->width);
    }
    int get_display_height()
    {
        if (handle == NULL)
            return 0;
        return canvas_height * height / double(handle->height);
    }
};

class SurfaceObject : public FrameObject
{
public:
    // Edit-time flags
    bool display_selected;
    bool use_abs_coords;

    // Runtime stuff
    int dest_width, dest_height;
    int dest_x, dest_y;
    int stretch_mode;
    int effect;

    // Image array
    vector<SurfaceImage> images;

    // Current images
    SurfaceImage * displayed_image;
    int displayed_index;
    SurfaceImage * selected_image;
    int selected_index;

    // File loading
    std::string filename;
    bool load_failed;

    SurfaceObject(int x, int y, int type_id);
    ~SurfaceObject();
    void update(float dt);
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
    void blit_image(int image);
    void set_alpha_mode(int mode);
    void set_effect(int index);
    void set_display_image(int index);
    void set_edit_image(int index, bool display = false);
    void create_alpha(int index);
    void clear_alpha(int index);
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
    void resize_source(int w, int h);
};

#endif // CHOWDREN_SURFACE_H
