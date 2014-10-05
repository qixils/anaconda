#include <string>
#include "image.h"
#include "string.h"
#include "color.h"
#include <iostream>
#include <stdio.h>
#include "datastream.h"
#include "chowconfig.h"
#include "types.h"
#include "mathcommon.h"

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

// dummy constructor
Image::Image()
: handle(0), flags(0), tex(0), image(NULL), width(0), height(0),
  hotspot_x(0), hotspot_y(0), action_x(0), action_y(0)
{
}

Image::Image(int hot_x, int hot_y, int act_x, int act_y)
: handle(0), flags(0), tex(0), image(NULL), width(0), height(0),
  hotspot_x(hot_x), hotspot_y(hot_y), action_x(act_x), action_y(act_y)
{
}

Image::Image(int handle)
: handle(handle), tex(0), image(NULL), flags(0)
{
}

Image::~Image()
{
    unload();
}

void Image::destroy()
{
    if (flags & IMAGE_CACHED)
        return;
    delete this;
}

Image * Image::copy()
{
    Image * new_image;
    if (image == NULL) {
        if (flags & IMAGE_USED) {
            FileImage * image = (FileImage*)this;
            new_image = new FileImage(image->filename,
                                      image->hotspot_x, image->hotspot_y,
                                      image->action_x, image->action_y,
                                      image->transparent);
        } else {
            new_image = new Image(handle);
        }
        new_image->load();
        return new_image;
    }
    new_image = new Image();
    new_image->width = width;
    new_image->height = height;
    new_image->handle = handle;
    int size = width*height*4;
    new_image->image = (unsigned char*)malloc(size);
    memcpy(new_image->image, image, size);
    return new_image;
}

void Image::load()
{
    flags |= IMAGE_USED;

    if (tex != 0 || image != NULL)
        return;

    if (flags & IMAGE_FILE) {
        ((FileImage*)this)->load_file();
        return;
    }

    open_image_file();
    FileStream stream(image_file);
    stream.seek(4 + handle * 4);
    unsigned int offset;
    stream >> offset;
    image_file.seek(offset);

    int hot_x, hot_y, act_x, act_y;
    stream >> hot_x;
    stream >> hot_y;
    stream >> act_x;
    stream >> act_y;

    hotspot_x = hot_x;
    hotspot_y = hot_y;
    action_x = act_x;
    action_y = act_y;

    int w, h, channels;
    image = stbi_load_from_callbacks(&fsfile_callbacks, &image_file,
        &w, &h, &channels, 4);

    width = w;
    height = h;

    if (image == NULL) {
        std::cout << "Could not load image " << handle << std::endl;
        std::cout << stbi_failure_reason() << std::endl;
        return;
    }
}

void Image::unload()
{
    if (image != NULL)
        stbi_image_free(image);
    if (tex != 0)
        glDeleteTextures(1, &tex);
    image = NULL;
    tex = 0;

#ifndef CHOWDREN_IS_WIIU
    boost::dynamic_bitset<>().swap(alpha);
#endif
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

#ifndef CHOWDREN_IS_WIIU
    // create alpha mask
    alpha.resize(width * height);
    for (int i = 0; i < width * height; i++) {
        unsigned char c = ((unsigned char*)(((unsigned int*)image) + i))[3];
        alpha.set(i, c != 0);
    }
#endif

    int gl_width, gl_height;

#ifdef CHOWDREN_NO_NPOT
    pot_w = std::max(8, round_pow2(width));
    pot_h = std::max(8, round_pow2(height));

    if (pot_w >= 1024 || pot_h >= 1024) {
        std::cout << "Image size too large" << std::endl;
        pot_w = std::min<short>(1024, pot_w);
        pot_h = std::min<short>(1024, pot_h);
    }

    gl_width = pot_w;
    gl_height = pot_h;

    if (pot_w != width || pot_h != height) {
        unsigned int * old_image =(unsigned int*)image;
        image = (unsigned char*)malloc(pot_w * pot_h * 4);
        unsigned int * image_arr = (unsigned int*)image;

        // in case the image is being cut off due to dimension restrictions
        int ww = std::min(width, pot_w);
        int hh = std::min(height, pot_h);

        for (int x = 0; x < ww; x++)
        for (int y = 0; y < hh; y++) {
            image_arr[x + y * pot_w] = old_image[x + y * width];
        }

        stbi_image_free(old_image);
    }
#else
    gl_width = width;
    gl_height = height;
#endif

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gl_width, gl_height, 0, GL_RGBA,
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

        if (tex == 0)
            return;
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

#ifdef CHOWDREN_NO_NPOT
    float tex_coords[8];
    if (flip_x) {
        memcpy(tex_coords, flipped_texcoords, sizeof(tex_coords));
    } else {
        memcpy(tex_coords, normal_texcoords, sizeof(tex_coords));
    }
    float u = float(width) / float(pot_w);
    float v = float(height) / float(pot_h);
    for (int i = 0; i < 8; i += 2) {
        tex_coords[i] *= u;
        tex_coords[i+1] *= v;
    }
#else
    const float * tex_coords;
    if (flip_x) {
        tex_coords = flipped_texcoords;
    } else {
        tex_coords = normal_texcoords;
    }
#endif

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
    flags |= IMAGE_FILE;
}

void FileImage::load_file()
{
    int channels;
    FSFile fp(filename.c_str(), "r");

    if (!fp.is_open()) {
        printf("Could not open image \"%s\"\n", filename.c_str());
        return;
    }

    int w, h;
    image = stbi_load_from_callbacks(&fsfile_callbacks, &fp,
        &w, &h, &channels, 4);

    width = w;
    height = h;

    fp.close();

    if (image == NULL) {
        printf("Could not load image \"%s\"\n", filename.c_str());
        return;
    }

    if (!transparent.is_enabled())
        return;

#ifndef CHOWDREN_FORCE_TRANSPARENT
    if (channels != 1 && channels != 3)
        return;
#endif

    for (int i = 0; i < width * height; i++) {
        unsigned char * c = &image[i*4];
        if (c[0] != transparent.r || c[1] != transparent.g ||
            c[2] != transparent.b)
            continue;
        c[3] = 0;
    }
}

static Image * internal_images[IMAGE_COUNT];

typedef hash_map<std::string, FileImage*> ImageCache;
static ImageCache image_cache;

Image * get_internal_image(unsigned int i)
{
    if (internal_images[i] == NULL) {
        internal_images[i] = new Image(i);
        internal_images[i]->flags |= IMAGE_CACHED;
    }

    Image * image = internal_images[i];
    image->load();

    return internal_images[i];
}

Image * get_image_cache(const std::string & filename, int hot_x, int hot_y,
                            int act_x, int act_y, TransparentColor color)
{
    FileImage * image;
    ImageCache::const_iterator it = image_cache.find(filename);
    if (it == image_cache.end()) {
        image = new FileImage(filename, 0, 0, 0, 0, color);
        image->flags |= IMAGE_USED;
        image->load();
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

void reset_image_cache()
{
#ifdef CHOWDREN_TEXTURE_GC
    for (int i = 0; i < IMAGE_COUNT; i++) {
        Image * image = internal_images[i];
        if (image == NULL)
            continue;
        image->flags &= ~IMAGE_USED;
    }
#endif
}

void flush_image_cache()
{
#ifdef CHOWDREN_TEXTURE_GC
    for (int i = 0; i < IMAGE_COUNT; i++) {
        Image * image = internal_images[i];
        if (image == NULL || image->flags & IMAGE_USED)
            continue;
        image->unload();
    }
#endif
}

Image dummy_image;

void ReplacedImages::replace(const Color & from, const Color & to)
{
    Replacements::iterator it = replacements.begin();

    while (it != replacements.end()) {
        const Color & first = it->first;
        const Color & second = it->second;
        if (first.r == from.r &&
            first.g == from.g &&
            first.b == from.b &&
            second.r == to.r &&
            second.g == to.g &&
            second.b == to.b)
        {
            it = replacements.erase(it);
            continue;
        }
        if (second.r == from.r &&
            second.g == from.g &&
            second.b == from.b &&
            first.r == to.r &&
            first.g == to.g &&
            first.b == to.b)
        {
            it = replacements.erase(it);
            return;
        }
        ++it;
    }
    replacements.push_back(Replacement(from, to));
}

vector<ReplacedImage> ReplacedImages::images;
