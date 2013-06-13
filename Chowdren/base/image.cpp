#include <string>
#include "image.h"
#include "png.h"
#include "string.h"
#include "color.h"
#include <iostream>
#include <stdio.h>
#include <vector>

unsigned char * read_png(FILE *fp, int *w, int *h, bool * has_alpha)
{
    unsigned char *pixels;
    png_bytep *row_pointers;
    png_structp png_ptr;
    png_infop info_ptr, end_info;
    png_uint_32 width, height;
    int i, bit_depth, color_type, interlace_type;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, 
        NULL);
    if (png_ptr == NULL)
        return NULL;

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return NULL;
    }

    end_info = png_create_info_struct(png_ptr);
    if (end_info == NULL) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return NULL;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        return NULL;
    }

    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height,
     &bit_depth, &color_type, &interlace_type,
     NULL, NULL);
    pixels = new unsigned char[width * height * 4];
    row_pointers = new png_bytep[height];

    for (i = 0; i < (int)height; i++)
        row_pointers[i] = (pixels + width * 4 * i);

    *has_alpha = true; // default

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        *has_alpha = false;
        png_set_palette_to_rgb(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    if (bit_depth == 16)
        png_set_strip_16(png_ptr);

    if (bit_depth < 8)
        png_set_packing(png_ptr);

    if (color_type == PNG_COLOR_TYPE_RGB || 
        color_type == PNG_COLOR_TYPE_GRAY) {
        *has_alpha = false;
        png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY || 
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, end_info);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    delete[] row_pointers;
    *w = width;
    *h = height;
    return pixels;
}

unsigned char * read_png(const std::string & filename, int * w, int * h,
                         bool * has_alpha)
{
    FILE * fp = fopen(filename.c_str(), "rb");
    if (!fp)
        return NULL;
    unsigned char * pixels = read_png(fp, w, h, has_alpha);
    fclose(fp);
    return pixels;
}

typedef std::vector<Image*> ImageList;

static std::string image_path("./Sprites.dat");

void set_image_path(const std::string & filename)
{
    image_path = convert_path(filename);
    std::cout << "Setting image file to: " << image_path << std::endl;
}

static FILE * image_file = NULL;

void open_image_file()
{
    if (image_file == NULL)
        image_file = fopen(image_path.c_str(), "rb");
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
        delete[] image;
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
    bool has_alpha;
    image = read_png(filename, &width, &height, &has_alpha);

    if(image == NULL) {
        printf("Could not load image \"%s\"\n", filename.c_str());
        return;
    }

    if (!has_alpha && color != NULL) {
        int trans_color = color->get_int();
        int * data = (int*)image;
        for (int i = 0; i < width * height; i++)
            if (data[i] == trans_color)
                data[i] = data[i] & 0xFFFFFF;
    }
}

Image::Image(Image & img) 
: hotspot_x(img.hotspot_x), hotspot_y(img.hotspot_y), 
  action_x(img.action_x), action_y(img.action_y),
  tex(img.tex), image(img.image), ref(&img), width(img.width), 
  height(img.height), handle(-1)
{
}

void Image::load()
{
    if (image != NULL)
        return;
    open_image_file();
    fseek(image_file, 4 + handle * 4, SEEK_SET);
    unsigned int offset;
    fread(&offset, 4, 1, image_file);
    fseek(image_file, offset, SEEK_SET);
    fread(&hotspot_x, 4, 1, image_file);
    fread(&hotspot_y, 4, 1, image_file);
    fread(&action_x, 4, 1, image_file);
    fread(&action_y, 4, 1, image_file);
    bool has_alpha; // may be useful later
    image = read_png(image_file, &width, &height, &has_alpha);
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
}

unsigned int & Image::get(int x, int y)
{
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
        fseek(image_file, 0, SEEK_SET);
        fread(&image_count, 1, 4, image_file);
        internal_images = new Image*[image_count];
        for (unsigned int i2 = 0; i2 < image_count; i2++)
            internal_images[i2] = NULL;
    }

    if (internal_images[i] == NULL)
        internal_images[i] = new Image(i);

    return internal_images[i];
}