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

enum ImageFlags
{
    IMAGE_USED = 1 << 0,
    IMAGE_FILE = 1 << 1,
    IMAGE_CACHED = 1 << 2,
    IMAGE_STATIC = 1 << 3
};

class Image
{
public:
    unsigned short handle;
    unsigned short flags;
    short hotspot_x, hotspot_y, action_x, action_y;
    short width, height;
    GLuint tex;
    unsigned char * image;
#ifndef CHOWDREN_IS_WIIU
    boost::dynamic_bitset<> alpha;
#endif

#ifdef CHOWDREN_NO_NPOT
    short pot_w, pot_h;
#endif

    Image();
    Image(int hot_x, int hot_y, int act_x, int act_y);
    Image(int handle);
    ~Image();
    void destroy();
    Image * copy();
    void replace(const Color & from, const Color & to);
    void load();
    void set_static();
    void upload_texture();
    void draw(int x, int y, float angle = 0.0f,
              float scale_x = 1.0f, float scale_y = 1.0f,
              bool flip_x = false, bool flip_y = false,
              GLuint back = 0, bool has_tex_param = false);
    void draw(int x, int y, int src_x, int src_y, int w, int h);
    bool is_valid();
    void unload();

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
    void load_file();
};

Image * get_internal_image(unsigned int i);
Image * get_image_cache(const std::string & filename, int hot_x, int hot_y,
                            int act_x, int act_y, TransparentColor color);
void reset_image_cache();
void flush_image_cache();
void preload_images();

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
        for (it = replacements.begin(); it != replacements.end(); ++it) {
            const Color & first = it->first;
            const Color & second = it->second;
            hash += first.r * hash_index;
            hash += first.g * hash_index;
            hash += first.b * hash_index;
            hash += second.r * hash_index;
            hash += second.g * hash_index;
            hash += second.b * hash_index;
            hash_index++;
        }

        vector<ReplacedImage>::const_iterator it2;
        for (it2 = images.begin(); it2 != images.end(); ++it2) {
            const ReplacedImage & img = *it2;
            if (img.hash == hash && img.src_image == image) {
                return img.image;
            }
        }

        std::cout << "Replacement with hash: " << hash << " "
            << replacements.size() << std::endl;

        Image * new_image = image->copy();
        for (it = replacements.begin(); it != replacements.end(); ++it) {
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
