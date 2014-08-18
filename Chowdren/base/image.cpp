#include <string>
#include "image.h"
#include "string.h"
#include "color.h"
#include <iostream>
#include <stdio.h>
#include "datastream.h"
#include "chowconfig.h"
#include "types.h"

#define STBI_NO_STDIO
#define STBI_NO_HDR
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static int read_func(void *user, char *data, int size)
{
   return ((FSFile*)user)->read(data, size);
}

static void skip_func(void *user, int n)
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

typedef vector<Image*> ImageList;

static std::string image_path("./Sprites.dat");

void set_image_path(const std::string & filename)
{
    image_path = convert_path(filename);
    std::cout << "Setting image file to: " << image_path << std::endl;
}

static FSFile image_file;

void open_image_file()
{
    if (image_file.is_open())
        return;
    image_file.open(image_path.c_str(), "r");
    if (!image_file.is_open())
        std::cout << "Could not open image file " << image_path << std::endl;
}

Image::Image()
// handle needs to be 0 so it is considered cached
: handle(0), tex(0), image(NULL), width(0), height(0),
  hotspot_x(0), hotspot_y(0), action_x(0), action_y(0)
{
}

Image::Image(int hot_x, int hot_y, int act_x, int act_y)
: handle(-1), tex(0), image(NULL), width(0), height(0),
  hotspot_x(hot_x), hotspot_y(hot_y), action_x(act_x), action_y(act_y)
{
}

Image::Image(int handle)
: handle(handle), tex(0), image(NULL)
{
    load();
}

Image::~Image()
{
    if (image != NULL)
        stbi_image_free(image);
    if (tex != 0)
        glDeleteTextures(1, &tex);
    image = NULL;
    tex = 0;
}

Image * Image::copy()
{
    if (image == NULL) {
        if (handle != -1) {
            return new Image(handle);
        } else {
            FileImage * image = (FileImage*)this;
            return new FileImage(image->filename,
                                 image->hotspot_x, image->hotspot_y,
                                 image->action_x, image->action_y,
                                 image->transparent);
        }
    }
    Image * new_image = new Image();
    new_image->width = width;
    new_image->height = height;
    new_image->handle = handle;
    int size = width*height*4;
    new_image->image = (unsigned char*)malloc(size);
    memcpy(new_image->image, image, size);
    return new_image;
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
    if (image == NULL) {
        std::cout << "Could not load image " << handle << std::endl;
        std::cout << stbi_failure_reason() << std::endl;
        return;
    }
    if (upload)
        upload_texture();
}

void Image::replace(const Color & from, const Color & to)
{
    if (image == NULL) {
        std::cout << "Could not replace color in unloaded image" << std::endl;
        return;
    }
    for (int i = 0; i < width * height; i++) {
        unsigned char * c = &image[i*4];
        if (c[0] != from.r || c[1] != from.g || c[2] != from.b)
            continue;
        c[0] = to.r;
        c[1] = to.g;
        c[2] = to.b;
    }
}

void Image::upload_texture()
{
    if (tex != 0 || image == NULL)
        return;

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
    alpha.resize(width * height);
    for (int i = 0; i < width * height; i++) {
        unsigned char c = ((unsigned char*)(((unsigned int*)image) + i))[3];
        alpha.set(i, c != 0);
    }
#endif
    // for memory reasons, we delete the image and access the alpha or
    // the texture directly
    stbi_image_free(image);
    image = NULL;
}

const float flipped_texcoords[8] = {
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f
};

const float normal_texcoords[8] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
};

#ifndef CHOWDREN_IS_WIIU
// XXX change glc_copy_color_buffer_rect so this isn't necessary

const float back_texcoords[8] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f
};
#else
const float back_texcoords[8] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
};
#endif

void Image::draw(double x, double y, double angle,
                 double scale_x, double scale_y,
                 bool flip_x, bool flip_y, GLuint background,
                 bool has_tex_param)
{
    if (tex == 0) {
        upload_texture();

        if (tex == 0) {
            return;
        }
    }

    glPushMatrix();
    glTranslated(x, y, 0.0);
    glRotated(-angle, 0.0, 0.0, 1.0);
    glScaled(scale_x, scale_y, 1.0);

    if (background != 0)
        glActiveTexture(GL_TEXTURE0);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    if (background != 0) {
        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, background);
    }
    if (has_tex_param) {
        glActiveTexture(GL_TEXTURE2);
        glEnable(GL_TEXTURE_2D);
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
        glMultiTexCoord2f(GL_TEXTURE1, back_texcoords[0], back_texcoords[1]);
    glVertex2d(-hotspot_x, -hotspot_y);

    glTexCoord2f(tex_coords[2], tex_coords[3]);
    if (background != 0)
        glMultiTexCoord2f(GL_TEXTURE1, back_texcoords[2], back_texcoords[3]);
    glVertex2d(-hotspot_x + width, -hotspot_y);

    glTexCoord2f(tex_coords[4], tex_coords[5]);
    if (background != 0)
        glMultiTexCoord2f(GL_TEXTURE1, back_texcoords[4], back_texcoords[5]);
    glVertex2d(-hotspot_x + width, -hotspot_y + height);

    glTexCoord2f(tex_coords[6], tex_coords[7]);
    if (background != 0)
        glMultiTexCoord2f(GL_TEXTURE1, back_texcoords[6], back_texcoords[7]);
    glVertex2d(-hotspot_x, -hotspot_y + height);
    glEnd();

    if (has_tex_param) {
        glDisable(GL_TEXTURE_2D);
        if (background != 0)
            glActiveTexture(GL_TEXTURE1);
        else
            glActiveTexture(GL_TEXTURE0);
    }
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

// FileImage

FileImage::FileImage(const std::string & filename, int hot_x, int hot_y,
                     int act_x, int act_y, TransparentColor color)
: filename(filename), transparent(color), Image(hot_x, hot_y, act_x, act_y)
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

    if (image == NULL) {
        printf("Could not load image \"%s\"\n", filename.c_str());
        return;
    }

    if (!color.is_enabled())
        return;

#ifndef CHOWDREN_FORCE_TRANSPARENT
    if (channels != 1 && channels != 3)
        return;
#endif

    for (int i = 0; i < width * height; i++) {
        unsigned char * c = &image[i*4];
        if (c[0] != color.r || c[1] != color.g || c[2] != color.b)
            continue;
        c[3] = 0;
    }
}

static Image * internal_images[IMAGE_COUNT];

typedef hash_map<std::string, FileImage*> ImageCache;
static ImageCache image_cache;

Image * get_internal_image(unsigned int i)
{
    if (internal_images[i] == NULL)
        internal_images[i] = new Image(i);

    return internal_images[i];
}

Image * get_image_cache(const std::string & filename, int hot_x, int hot_y,
                        int act_x, int act_y, TransparentColor color)
{
    FileImage * image;
    ImageCache::const_iterator it = image_cache.find(filename);
    if (it == image_cache.end()) {
        image = new FileImage(filename, 0, 0, 0, 0, color);
        if (!image->is_valid()) {
            delete image;
            image = NULL;
        }
        image_cache[filename] = image;
    } else {
        image = it->second;
    }
    return image;
}

Image dummy_image;

void ReplacedImages::replace(const Color & from, const Color & to)
{
    Replacements::iterator it = replacements.begin();

    while (it != replacements.end()) {
        if (it->first.r == from.r &&
            it->first.g == from.g &&
            it->first.b == from.b &&
            it->second.r == to.r &&
            it->second.g == to.g &&
            it->second.b == to.b)
        {
            it = replacements.erase(it);
            continue;
        }
        if (it->second.r == from.r &&
            it->second.g == from.g &&
            it->second.b == from.b &&
            it->first.r == to.r &&
            it->first.g == to.g &&
            it->first.b == to.b)
        {
            it = replacements.erase(it);
            return;
        }
        it++;
    }
    replacements.push_back(Replacement(from, to));
}

vector<ReplacedImage> ReplacedImages::images;