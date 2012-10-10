#include <string>
#include "image.h"
#include "SOIL.h"
#include "string.h"

void load_texture(const char *filename, int force_channels, 
                  unsigned int reuse_texture_ID, unsigned int flags,
                  GLuint * tex, int * width, int * height)
{
    unsigned char* img;
    int channels;
    img = SOIL_load_image(filename, width, height, &channels, force_channels);

    if( (force_channels >= 1) && (force_channels <= 4) )
    {
        channels = force_channels;
    }

    if(img == NULL)
    {
        *tex = 0;
        return;
    }

    *tex = SOIL_create_OGL_texture(img, *width, *height, channels,
        reuse_texture_ID, flags);

    SOIL_free_image_data(img);

    return;
}

Image::Image(int handle, int hot_x, int hot_y, int act_x, int act_y) 
: handle(handle), hotspot_x(hot_x), hotspot_y(hot_y), action_x(act_x), 
action_y(act_y), tex(0)
{
}

Image::Image(const std::string & filename, int hot_x, int hot_y, 
             int act_x, int act_y) 
: hotspot_x(hot_x), hotspot_y(hot_y), action_x(act_x), action_y(act_y), tex(0)
{
    load_filename(filename);
}

void Image::load()
{
    if (tex != NULL)
        return;
    load_filename("./images/" + number_to_string(handle) + ".png");
}

void Image::load_filename(const std::string & filename)
{
    if (tex != NULL)
        return;
    load_texture(filename.c_str(), 4, 0, SOIL_FLAG_POWER_OF_TWO,
        &tex, &width, &height);
    if (tex == 0) {
        printf("Could not load %s\n", filename.c_str());
    }
}

void Image::draw(double x, double y, double angle)
{
    load();

    glPushMatrix();
    glTranslated(x, y, 0.0);
    glRotated(angle, 0.0, 0.0, 1.0);
    x -= (double)hotspot_x;
    y -= (double)hotspot_y;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex2d(-hotspot_x, -hotspot_y);
    glTexCoord2f(1.0, 0.0);
    glVertex2d(-hotspot_x + width, -hotspot_y);
    glTexCoord2f(1.0, 1.0);
    glVertex2d(-hotspot_x + width, -hotspot_y + height);
    glTexCoord2f(0.0, 1.0);
    glVertex2d(-hotspot_x, -hotspot_y + height);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}