#ifndef CHOWDREN_IMAGE_H
#define CHOWDREN_IMAGE_H

#include "path.h"
#include <string>
#include "include_gl.h"
#include "color.h"
#include "types.h"
#include "platform.h"
#include "chowconfig.h"
#include <boost/dynamic_bitset.hpp>

const std::string & get_image_path();
void set_image_path(const std::string & path);
void initialize_images();

extern const float normal_texcoords[8];
extern const float back_texcoords[8];

class Image
{
public:
    int handle;
    int hotspot_x, hotspot_y, action_x, action_y;
    GLuint tex;
    unsigned char * image;
#ifndef CHOWDREN_IS_WIIU
    boost::dynamic_bitset<> alpha;
#endif
    int width, height;

    Image();
    Image(int hot_x, int hot_y, int act_x, int act_y);
    Image(int handle);
    ~Image();
    Image * copy();
    void replace(const Color & from, const Color & to);
    void load(bool upload = false);
    void upload_texture();
    void draw(double x, double y, double angle = 0.0,
              double scale_x = 1.0, double scale_y = 1.0,
              bool flip_x = false, bool flip_y = false, GLuint back = 0,
              bool has_tex_param = false);
    void draw(double x, double y, int src_x, int src_y, int w, int h);
    bool is_valid();

    // inline methods

    bool get_alpha(int x, int y)
    {
    #ifdef CHOWDREN_IS_WIIU
        if (tex != 0) {
            unsigned int & v = platform_get_texture_pixel(tex, x, y);
            unsigned char c = ((unsigned char*)&v)[3];
            return c != 0;
        }
    #else
        if (!alpha.empty())
            return alpha.test(y * width + x);
    #endif
        unsigned int * v = (unsigned int*)image + y * width + x;
        unsigned char c = ((unsigned char*)v)[3];
        return c != 0;
    }
};

class FileImage : public Image
{
public:
    std::string filename;
    TransparentColor transparent;

    FileImage(const std::string & filename, int hot_x, int hot_y,
              int act_x, int act_y, TransparentColor transparent);
};

Image * get_internal_image(unsigned int i);
Image * get_image_cache(const std::string & filename, int hot_x, int hot_y,
                        int act_x, int act_y, TransparentColor color);

extern Image dummy_image;

// image replacer

struct ReplacedImage
{
    Image * src_image;
    Image * image;
    int hash;

    ReplacedImage(Image * src_image, Image * image, int hash)
    : src_image(src_image), image(image), hash(hash)
    {
    }
};

typedef std::pair<Color, Color> Replacement;
typedef vector<Replacement> Replacements;

class ReplacedImages
{
public:
    Replacements replacements;
    static vector<ReplacedImage> images;

    ReplacedImages()
    {
    }

    void replace(const Color & from, const Color & to);

    Image * apply(Image * image)
    {
        int hash = 0;
        vector<Replacement>::const_iterator it;
        int hash_index = 1;
        for (it = replacements.begin(); it != replacements.end(); it++) {
            hash += it->first.r * hash_index;
            hash += it->first.g * hash_index;
            hash += it->first.b * hash_index;
            hash += it->second.r * hash_index;
            hash += it->second.g * hash_index;
            hash += it->second.b * hash_index;
            hash_index++;
        }

        vector<ReplacedImage>::const_iterator it2;
        for (it2 = images.begin(); it2 != images.end(); it2++) {
            const ReplacedImage & img = *it2;
            if (img.hash == hash && img.src_image == image) {
                return img.image;
            }
        }

        std::cout << "Replacement with hash: " << hash << " "
            << replacements.size() << std::endl;

        Image * new_image = image->copy();
        for (it = replacements.begin(); it != replacements.end(); it++) {
            std::cout << "Replace: " <<
                int(it->first.r) << " " << int(it->first.g) << " " << int(it->first.b)
                << " -> " <<
                int(it->second.r) << " " << int(it->second.g) << " " << int(it->second.b)
                << std::endl;
            new_image->replace(it->first, it->second);
        }

        images.push_back(ReplacedImage(image, new_image, hash));
        return new_image;
    }

    bool empty()
    {
        return replacements.empty();
    }
};

#endif // CHOWDREN_IMAGE_H
