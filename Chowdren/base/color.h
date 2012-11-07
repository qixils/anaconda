#ifndef CHOWDREN_COLOR_H
#define CHOWDREN_COLOR_H

#include <algorithm>

inline unsigned char clamp_color_component(int v)
{
    return (unsigned char)std::min<int>(std::max<int>(v, 0), 255);   
}

inline int make_color_int(unsigned char r, unsigned char g, unsigned char b,
                          unsigned char a = 0)
{
    return r | (g << 8) | (b << 16) | (a << 24);
}

class Color
{
public:
    unsigned char r, g, b, a;

    Color()
    {
        set(255, 255, 255, 255);
    }

    Color(int r, int g, int b, int a = 255)
    {
        set(r, g, b, a);
    }

    Color(int color)
    {
        set(color);
    }

    void set(int r, int g, int b, int a = 255)
    {
        this->r = clamp_color_component(r);
        this->g = clamp_color_component(g);
        this->b = clamp_color_component(b);
        set_alpha(a);
    }

    void set_alpha(int a)
    {
        a = clamp_color_component(a);
        this->a = a;
    }

    void set(int color)
    {
        set(color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF);
    }

    void apply()
    {
        glColor4ub(r, g, b, a);
    }

    void set_alpha_coefficient(int a)
    {
        set_alpha(255 - a);
    }

    int get_alpha_coefficient()
    {
        return 255 - a;
    }

    int get_int()
    {
        return make_color_int(r, g, b, a);
    }
};

#endif // CHOWDREN_COLOR_H