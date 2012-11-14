#ifndef CHOWDREN_IMAGE_H
#define CHOWDREN_IMAGE_H

#include <string>
#include "include_gl.h"
#include "color.h"

const std::string & get_image_path();
void set_image_path(const std::string & path);

class Image
{
public:
    int offset;
    int hotspot_x, hotspot_y, action_x, action_y;
    GLuint tex;
    unsigned char * image;
    int width, height;
    Image * ref;

    Image(int offset, int hot_x, int hot_y, int act_x, int act_y);
    Image(const std::string & filename, int hot_x, int hot_y, 
          int act_x, int act_y, Color * color = NULL);
    Image(Image & image);
    ~Image();
    void load();
    void load_filename(const std::string & filename, Color * color = NULL);
    void upload_texture();
    void draw(double x, double y, double angle = 0.0, 
              double scale_x = 1.0, double scale_y = 1.0,
              bool flip_x = false, bool flip_y = false, GLuint back = 0);
    unsigned int & get(int x, int y);
};

#endif // CHOWDREN_IMAGE_H