#ifndef CHOWDREN_IMAGE_H
#define CHOWDREN_IMAGE_H

#include "path.h"
#include <string>
#include "include_gl.h"
#include "color.h"
#include <vector>

const std::string & get_image_path();
void set_image_path(const std::string & path);
void initialize_images();

class Image
{
public:
    int handle;
    int hotspot_x, hotspot_y, action_x, action_y;
    GLuint tex;
    unsigned char * image;
#ifndef CHOWDREN_IS_WIIU
    bool * alpha;
#endif
    int width, height;
    Image * ref;

    Image(int handle);
    Image(const std::string & filename, int hot_x, int hot_y, 
          int act_x, int act_y, Color * color = NULL);
    Image(Image & image);
    ~Image();
    void load(bool upload = false);
    void upload_texture();
    void draw(double x, double y, double angle = 0.0,
              double scale_x = 1.0, double scale_y = 1.0,
              bool flip_x = false, bool flip_y = false, GLuint back = 0);
    void draw(double x, double y, int src_x, int src_y, int w, int h);
    bool get_alpha(int x, int y);
    bool is_valid();
};

Image * get_internal_image(unsigned int i);

#endif // CHOWDREN_IMAGE_H
