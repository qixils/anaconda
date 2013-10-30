#include <string>
#include "image.h"
#include "stb_image.c"
#include "string.h"
#include "color.h"
#include <iostream>
#include <stdio.h>
#include <vector>
#include "platform.h"
#include "datastream.h"

static int read_func(void *user, char *data, int size)
{
   return ((FSFile*)user)->read(data, size);
}

static void skip_func(void *user, unsigned n)
{
   ((FSFile*)user)->seek(n, SEEK_CUR);
}

static int eof_func(void *user)
{
   return ((FSFile*)user)->at_end();
}

static stbi_io_callbacks fsfile_callbacks =
{
   read_func,
   skip_func,
   eof_func,
};

typedef std::vector<Image*> ImageList;

static std::string image_path("./Sprites.dat");

void set_image_path(const std::string & filename)
{
    image_path = convert_path(filename);
    std::cout << "Setting image file to: " << image_path << std::endl;
}

static FSFile image_file;

void open_image_file()
{
    if (!image_file.is_open())
        image_file.open(image_path.c_str(), "r");
}

Image::Image(int handle) 
: handle(handle), tex(0), image(NULL), ref(NULL)
{
    load();
}

Image::~Image()
{
    if (ref)
        return;
    if (image != NULL)
        stbi_image_free(image);
    if (tex != 0)
        glDeleteTextures(1, &tex);
    image = NULL;
    tex = 0;
}

Image::Image(const std::string & filename, int hot_x, int hot_y, 
             int act_x, int act_y, Color * color) 
: hotspot_x(hot_x), hotspot_y(hot_y), action_x(act_x), action_y(act_y),
  tex(0), image(NULL), ref(NULL), handle(-1)
{
    int channels;
    FSFile fp(filename.c_str(), "r");

    if (!fp.is_open()) {
        printf("Could not open image \"%s\"\n", filename.c_str());
        return;
    }

    image = stbi_load_from_callbacks(&fsfile_callbacks, &fp, 
        &width, &height, &channels, 4);

    fp.close();

    if(image == NULL) {
        printf("Could not load image \"%s\"\n", filename.c_str());
        return;
    }

    if ((channels == 1 || channels == 3) && color != NULL) {
        for (int i = 0; i < width * height; i++) {
            unsigned char * c = &image[i*4];
            if (c[0] == color->r && c[1] == color->g && c[2] == color->b)
                c[3] = 0;
        }
    }
}

Image::Image(Image & img) 
: hotspot_x(img.hotspot_x), hotspot_y(img.hotspot_y), 
  action_x(img.action_x), action_y(img.action_y),
  tex(img.tex), image(img.image), ref(&img), width(img.width), 
  height(img.height), handle(-1)
{
}

void Image::load(bool upload)
{
#ifdef CHOWDREN_IS_WIIU
    if (tex != 0)
        return;
#endif
    if (image != NULL)
        return;
    open_image_file();
    FileStream stream(image_file);
    stream.seek(4 + handle * 4);
    unsigned int offset;
    stream >> offset;
    image_file.seek(offset);
    stream >> hotspot_x;
    stream >> hotspot_y;
    stream >> action_x;
    stream >> action_y;
    int channels;
    image = stbi_load_from_callbacks(&fsfile_callbacks, &image_file, 
        &width, &height, &channels, 4);
    if (upload)
        upload_texture();
}

void Image::upload_texture()
{
    if (tex != 0)
        return;
    else if (image == NULL)
        return;
    if (ref != NULL) {
        ref->upload_texture();
        tex = ref->tex;
        return;
    }

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, 
                 GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#ifdef CHOWDREN_IS_WIIU
    // for memory reasons, we delete the image and access the texture directly
    delete image;
    image = NULL;
#endif
}

unsigned int & Image::get(int x, int y)
{
#ifdef CHOWDREN_IS_WIIU
    if (tex != 0)
        return platform_get_texture_pixel(tex, x, y);
#endif
    return ((unsigned int*)image)[y * width + x];
}

void Image::draw(double x, double y, double angle, 
                 double scale_x, double scale_y,
                 bool flip_x, bool flip_y, GLuint background)
{
    if (tex == 0) {
        upload_texture();

        if (tex == 0)
            return;
    }

    glPushMatrix();
    glTranslated(x, y, 0.0);
    glRotated(-angle, 0.0, 0.0, 1.0);
    glScaled(scale_x, scale_y, 1.0);
    x -= (double)hotspot_x;
    y -= (double)hotspot_y;
    if (background != 0)
        glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    if (background != 0) {
        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, background);
    }
    glBegin(GL_QUADS);
    float tex_coords[8];
    if (flip_x) {
         tex_coords[0] = 1.0; tex_coords[1] = 0.0;
         tex_coords[2] = 0.0; tex_coords[3] = 0.0;
         tex_coords[4] = 0.0; tex_coords[5] = 1.0;
         tex_coords[6] = 1.0; tex_coords[7] = 1.0;
    } else {
         tex_coords[0] = 0.0; tex_coords[1] = 0.0;
         tex_coords[2] = 1.0; tex_coords[3] = 0.0;
         tex_coords[4] = 1.0; tex_coords[5] = 1.0;
         tex_coords[6] = 0.0; tex_coords[7] = 1.0;
    }
    glTexCoord2f(tex_coords[0], tex_coords[1]);
    if (background != 0)
        glMultiTexCoord2f(GL_TEXTURE1, tex_coords[0], tex_coords[1]);
    glVertex2d(-hotspot_x, -hotspot_y);
    glTexCoord2f(tex_coords[2], tex_coords[3]);
    if (background != 0)
        glMultiTexCoord2f(GL_TEXTURE1, tex_coords[2], tex_coords[3]);
    glVertex2d(-hotspot_x + width, -hotspot_y);
    glTexCoord2f(tex_coords[4], tex_coords[5]);
    if (background != 0)
        glMultiTexCoord2f(GL_TEXTURE1, tex_coords[4], tex_coords[5]);
    glVertex2d(-hotspot_x + width, -hotspot_y + height);
    glTexCoord2f(tex_coords[6], tex_coords[7]);
    if (background != 0)
        glMultiTexCoord2f(GL_TEXTURE1, tex_coords[6], tex_coords[7]);
    glVertex2d(-hotspot_x, -hotspot_y + height);
    glEnd();
    if (background != 0) {
        glDisable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
    }
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

static Image ** internal_images = NULL;

Image * get_internal_image(unsigned int i)
{
    if (internal_images == NULL) {
        open_image_file();
        unsigned int image_count;
        FileStream stream(image_file);
        stream.seek(0);
        stream >> image_count;
        internal_images = new Image*[image_count];
        for (unsigned int i2 = 0; i2 < image_count; i2++)
            internal_images[i2] = NULL;
    }

    if (internal_images[i] == NULL)
        internal_images[i] = new Image(i);

    return internal_images[i];
}
