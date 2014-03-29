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
#include "chowconfig.h"

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
:
#ifndef CHOWDREN_IS_WIIU
  alpha(NULL),
#endif
  handle(handle), tex(0), image(NULL), ref(NULL)
{
    load();
}

Image::~Image()
{
    if (ref)
        return;
    if (image != NULL)
        stbi_image_free(image);
#ifndef CHOWDREN_IS_WIIU
    if (alpha != NULL)
        delete[] alpha;
#endif
    if (tex != 0)
        glDeleteTextures(1, &tex);
    image = NULL;
    tex = 0;
}

Image::Image(const std::string & filename, int hot_x, int hot_y, 
             int act_x, int act_y, Color * color) 
: 
#ifndef CHOWDREN_IS_WIIU
  alpha(NULL),
#endif
  hotspot_x(hot_x), hotspot_y(hot_y), action_x(act_x), action_y(act_y),
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
:
#ifndef CHOWDREN_IS_WIIU
  alpha(img.alpha),
#endif
  hotspot_x(img.hotspot_x), hotspot_y(img.hotspot_y), 
  action_x(img.action_x), action_y(img.action_y),
  tex(img.tex), image(img.image), ref(&img), width(img.width), 
  height(img.height), handle(img.handle)
{
}

void Image::load(bool upload)
{
    if (tex != 0)
        return;
    if (image != NULL) {
        if (upload)
            upload_texture();
        return;
    }
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
    if (tex != 0 || image == NULL)
        return;
    if (ref != NULL) {
        ref->upload_texture();
        tex = ref->tex;
        image = ref->image;
#ifndef CHOWDREN_IS_WIIU
        alpha = ref->alpha;
#endif
        return;
    }

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, 
                     GL_UNSIGNED_BYTE, image);
#ifdef CHOWDREN_QUICK_SCALE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#ifndef CHOWDREN_IS_WIIU
    // create alpha mask
    alpha = new bool[width * height];
    for (int i = 0; i < width * height; i++) {
        unsigned char c = ((unsigned char*)(((unsigned int*)image) + i))[3];
        alpha[i] = c != 0;
    }
#endif
    // for memory reasons, we delete the image and access the alpha or
    // the texture directly
    stbi_image_free(image);
    image = NULL;
}

bool Image::get_alpha(int x, int y)
{
#ifdef CHOWDREN_IS_WIIU
    if (tex != 0) {
        unsigned int & v = platform_get_texture_pixel(tex, x, y);
        unsigned char c = ((unsigned char*)&v)[3];
        return c != 0;
    }
#else
    if (alpha != NULL)
        return alpha[y * width + x];
#endif
    unsigned int * v = (unsigned int*)image + y * width + x;
    unsigned char c = ((unsigned char*)v)[3];
    return c != 0;
}

const float flipped_texcoords[] = {
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f
};

const float normal_texcoords[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
};

#ifdef CHOWDREN_IS_DESKTOP
const float back_texcoords[] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f
};
#else
const float back_texcoords[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
};
#endif

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
    const float * tex_coords;
    if (flip_x) {
        tex_coords = flipped_texcoords;
    } else {
        tex_coords = normal_texcoords;
    }
    glTexCoord2f(tex_coords[0], tex_coords[1]);
    if (background != 0)
        glMultiTexCoord2f(GL_TEXTURE1,
                              back_texcoords[0], back_texcoords[1]);
    glVertex2d(-hotspot_x, -hotspot_y);
    glTexCoord2f(tex_coords[2], tex_coords[3]);
    if (background != 0)
        glMultiTexCoord2f(GL_TEXTURE1,
                              back_texcoords[2], back_texcoords[3]);
    glVertex2d(-hotspot_x + width, -hotspot_y);
    glTexCoord2f(tex_coords[4], tex_coords[5]);
    if (background != 0)
        glMultiTexCoord2f(GL_TEXTURE1,
                              back_texcoords[4], back_texcoords[5]);
    glVertex2d(-hotspot_x + width, -hotspot_y + height);
    glTexCoord2f(tex_coords[6], tex_coords[7]);
    if (background != 0)
        glMultiTexCoord2f(GL_TEXTURE1,
                              back_texcoords[6], back_texcoords[7]);
    glVertex2d(-hotspot_x, -hotspot_y + height);
    glEnd();
    if (background != 0) {
        glDisable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
    }
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void Image::draw(double x, double y, int src_x, int src_y, int w, int h)
{
    if (tex == 0) {
        upload_texture();

        if (tex == 0)
            return;
    }

    double x2 = x + double(w);
    double y2 = y + double(h);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBegin(GL_QUADS);
    float t_x1 = float(src_x) / float(width);
    float t_x2 = t_x1 + float(w) / float(width);
    float t_y1 = float(src_y) / float(height);
    float t_y2 = t_y1 + float(h) / float(height);
    glTexCoord2f(t_x1, t_y1);
    glVertex2d(x, y);
    glTexCoord2f(t_x2, t_y1);
    glVertex2d(x2, y);
    glTexCoord2f(t_x2, t_y2);
    glVertex2d(x2, y2);
    glTexCoord2f(t_x1, t_y2);
    glVertex2d(x, y2);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

bool Image::is_valid()
{
    return image != NULL || tex != 0;
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
