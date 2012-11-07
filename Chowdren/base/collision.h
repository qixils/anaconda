#include "frameobject.h"
#include <algorithm>
#include "mathcommon.h"

class CollisionBase
{
public:
    bool is_box;

    CollisionBase(bool is_box) : is_box(is_box)
    {

    }

    virtual void get_box(int v[4]) 
    {
        v[0] = v[1] = v[2] = v[3] = 0;
    }

    virtual bool get_bit(int x, int y)
    {
        return true;
    }
};

inline bool collides(int a_x1, int a_y1, int a_x2, int a_y2, 
                     int b_x1, int b_y1, int b_x2, int b_y2)
{
    if (a_x2 <= b_x1 || a_y2 <= b_y1 || a_x1 >= b_x2 || a_y1 >= b_y2)
        return false;
    return true;
}

inline void intersect(int a_x1, int a_y1, int a_x2, int a_y2, 
                      int b_x1, int b_y1, int b_x2, int b_y2,
                      int & r_x1, int & r_y1, int & r_x2, int & r_y2)
{
    r_x1 = std::max<int>(a_x1, b_x1);
    r_y1 = std::max<int>(a_y1, b_y1);
    r_x2 = std::min<int>(a_x2, b_x2);
    r_y2 = std::min<int>(a_y2, b_y2);
};

bool collide(CollisionBase * a, CollisionBase * b)
{
    if (a == NULL || b == NULL)
        return false;
    int v1[4];
    a->get_box(v1);
    int v2[4];
    b->get_box(v2);
    bool ret = collides(v1[0], v1[1], v1[2], v1[3], 
                        v2[0], v2[1], v2[2], v2[3]);
    if (!ret)
        return false;
    if (a->is_box && b->is_box)
        return true;

    // calculate the overlapping area
    int x1, y1, x2, y2;
    intersect(v1[0], v1[1], v1[2], v1[3], 
              v2[0], v2[1], v2[2], v2[3],
              x1, y1, x2, y2);

    // figure out the offsets of the overlapping area in each
    int offx1 = x1 - v1[0];
    int offy1 = y1 - v1[1];
    int offx2 = x1 - v2[0];
    int offy2 = y1 - v2[1];
    
    int x, y;
    bool c1, c2;
    for (x = 0; x < x2 - x1; x++) {
        for (y = 0; y < y2 - y1; y++) {
            c1 = a->get_bit(offx1 + x, y + offy1);
            c2 = b->get_bit(offx2 + x, y + offy2);
            if (c1 && c2)
                return true;
        }
    }
    return false;
}

class InstanceBox : public CollisionBase
{
public:
    FrameObject * instance;

    InstanceBox(FrameObject * instance)
    : CollisionBase(true), instance(instance)
    {
    }

    void get_box(int v[4])
    {
        v[0] = instance->x;
        v[1] = instance->y;
        v[2] = instance->x + instance->width;
        v[3] = instance->y + instance->height;
    }
};

inline int int_min_4(int a, int b, int c, int d)
{
    return std::min<int>(
        a, std::min<int>(
            b, std::min<int>(c, d)
        )
    );
}

inline int int_max_4(int a, int b, int c, int d)
{
    return std::max<int>(
        a, std::max<int>(
            b, std::max<int>(c, d)
        )
    );
}

inline void transform_rect(int width, int height, double co, double si, 
                           double x_scale, double y_scale, 
                           int & x1, int & y1, int & x2, int & y2)
{
    int top_right_x = int(width * x_scale * co);
    int top_right_y = int(-width * x_scale * si);
    int bottom_left_x = int(height * y_scale * si);
    int bottom_left_y = int(height * y_scale * co);
    int bottom_right_x = int(width * x_scale * co + height * y_scale * si);
    int bottom_right_y = int(height * y_scale * co - width * x_scale * si);
    x1 = int_min_4(0, top_right_x, bottom_left_x, bottom_right_x);
    y1 = int_min_4(0, top_right_y, bottom_left_y, bottom_right_y);
    x2 = int_max_4(0, top_right_x, bottom_left_x, bottom_right_x);
    y2 = int_max_4(0, top_right_y, bottom_left_y, bottom_right_y);
};

class SpriteCollision : public CollisionBase
{
public:
    FrameObject * instance;
    Image * image;
    double angle;
    double x_scale, y_scale;
    // transformed variables
    bool transform;
    double co, si; // optimization
    int x1, y1, x2, y2; // transformed bounding box
    int width, height;
    int hotspot_x, hotspot_y;

    SpriteCollision(FrameObject * instance, Image * image)
    : CollisionBase(false), instance(instance), image(image), transform(false),
      angle(0.0), x_scale(1.0), y_scale(1.0)
    {
    }

    void set_image(Image * image)
    {
        this->image = image;
        update_transform();
    }

    void get_box(int v[4])
    {
        int h_x, h_y;
        if (transform) {
            h_x = hotspot_x;
            h_y = hotspot_y;
        } else {
            h_x = image->hotspot_x;
            h_y = image->hotspot_y;
        }

        v[0] = instance->x - h_x;
        v[1] = instance->y - h_y;
        v[2] = v[0] + width;
        v[3] = v[1] + height;
    }

    void set_angle(double value)
    {
        angle = value;
        update_transform();
    }
    
    void set_scale(double value)
    {
        x_scale = y_scale = value;
        update_transform();
    }

    void set_x_scale(double x)
    {
        x_scale = x;
        update_transform();
    }

    void set_y_scale(double y)
    {
        y_scale = y;
        update_transform();
    }

    void update_transform()
    {
        if (x_scale == 1.0 && y_scale == 1.0 && angle == 0.0) {
            width = image->width;
            height = image->height;
            hotspot_x = image->hotspot_x;
            hotspot_y = image->hotspot_y;
            transform = false;
            return;
        }
        transform = true;
        co = cos_deg(angle);
        si = sin_deg(angle);
        transform_rect(image->width, image->height, co, si, x_scale, y_scale, 
                       x1, y1, x2, y2);
        width = x2 - x1;
        height = y2 - y1;
        get_transform(image->hotspot_x, image->hotspot_y, hotspot_x, hotspot_y);
    }

    void get_transform(int & x, int & y, int & r_x, int & r_y)
    {
        if (!transform) {
            r_x = x;
            r_y = y;
            return;
        }
        int new_x = int(x * x_scale * co + y * y_scale * si);
        int new_y = int(y * y_scale * co - x * x_scale * si);
        r_x = -(x1 - x);
        r_y = -(y1 - y);
    }

    bool get_bit(int x, int y)
    {
        if (transform) {
            double x2 = x + x1;
            double y2 = y + y1;
            x = int((x2 * co - y2 * si) / x_scale);
            y = int((y2 * co + x2 * si) / y_scale);
            if (x < 0 || x >= image->width || y < 0 || y >= image->height)
                return false;
        }
        if (is_box)
            return true;
        return ((unsigned char*)&image->get(x, y))[3] != 0;
        
    }
};

class PointCollision : public CollisionBase
{
public:
    int x, y;

    PointCollision(int x, int y)
    : CollisionBase(true), x(x), y(y)
    {
    }

    void get_box(int v[4])
    {
        v[0] = x;
        v[1] = y;
        v[2] = x + 1;
        v[3] = y + 1;
    }
};

class BoundingBox : public CollisionBase
{
public:
    int x1, y1, x2, y2;

    BoundingBox(int x1, int y1, int x2, int y2)
    : CollisionBase(true), x1(x1), y1(y1), x2(y2), y2(y2)
    {
    }

    void get_box(int v[4])
    {
        v[0] = x1;
        v[1] = y1;
        v[2] = x2;
        v[3] = y2;
    }
};

class MaskCollision : public CollisionBase
{
public:
    unsigned char * mask;
    int x1, y1, x2, y2;

    MaskCollision(unsigned char * mask, int x1, int y1, int x2, int y2)
    : CollisionBase(false), x1(x1), y1(y1), x2(x2), y2(y2), mask(mask)
    {
    }

    void get_box(int v[4])
    {
        v[0] = x1;
        v[1] = y1;
        v[2] = x2;
        v[3] = y2;
    }

    bool get_bit(int x, int y)
    {
        return mask[x + y * (x2 - x1)] != 0;
    }
};