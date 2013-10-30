#include "frameobject.h"
#include <algorithm>
#include "mathcommon.h"
#include "coltree.h"

enum CollisionType
{
    INSTANCE_BOX,
    SPRITE_COLLISION,
    POINT_COLLISION,
    BOUNDING_BOX,
    BACKGROUND_ITEM
};

class CollisionBase
{
public:
    CollisionType type;
    bool is_box;

    CollisionBase(CollisionType type, bool is_box) 
    : is_box(is_box), type(type)
    {
    }

    virtual ~CollisionBase()
    {

    }

    virtual void get_box(int v[4]) = 0;
};

inline bool collide_line(int x1, int y1, int x2, int y2,
                         int line_x1, int line_y1, int line_x2, int line_y2)
{
    float delta;
    if (line_x2 - line_x1 > line_y2 - line_y1) {
        delta = float(line_y2 - line_y1) / (line_x2 - line_x1);
        if (line_x2 > line_x1) {
            if (x2 < line_x1 || x1 >= line_x2)
                return false;
        } else {
            if (x2 < line_x2 || x1 >= line_x1)
                return false;
        }
        int y = delta * (x1 - line_x1) + line_y1;
        if (y >= y1 && y < y2)
            return true;
        y = delta * (x2 - line_x1) + line_y1;
        if (y >= y1 && y < y2)
            return true;
        return false;
    } else {
        delta = float(line_x2 - line_x1) / (line_y2 - line_y1);
        if (line_y2 > line_y1) {
            if (y2 < line_y1 || y2 >= line_y2)
                return false;
        } else {
            if (y2 < line_y2 || y1 >= line_y1)
                return false;
        }
        int x = delta * (y1 - line_y1) + x1;
        if (x >= x1 && x < x2)
            return true;
        x = delta * (y2 - line_y1) + x1;
        if (x >= x1 && x < x2)
            return true;
        return false;
    }
}

class InstanceBox : public CollisionBase
{
public:
    FrameObject * instance;

    InstanceBox(FrameObject * instance)
    : CollisionBase(INSTANCE_BOX, true), instance(instance)
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
}

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
    float co_divx, si_divx;
    float co_divy, si_divy;
    int x1, y1, x2, y2; // transformed bounding box
    int width, height;
    int hotspot_x, hotspot_y;

    SpriteCollision(FrameObject * instance, Image * image = NULL)
    : CollisionBase(SPRITE_COLLISION, false), instance(instance), 
      image(image), transform(false), angle(0.0), x_scale(1.0), y_scale(1.0)
    {
        if (image == NULL)
            return;
        width = image->width;
        height = image->height;
        hotspot_x = image->hotspot_x;
        hotspot_y = image->hotspot_y;
    }

    void set_image(Image * image)
    {
        this->image = image;
        update_transform();
    }

    void get_box(int v[4])
    {
        if (image == NULL) {
            v[0] = v[2] = instance->x;
            v[1] = v[3] = instance->y;
            return;
        }

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
        co_divx = float(co / x_scale);
        co_divy = float(co / y_scale);
        si = sin_deg(angle);
        si_divx = float(si / x_scale);
        si_divy = float(si / y_scale);
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
        r_x = -(x1 - new_x);
        r_y = -(y1 - new_y);
    }

    inline bool get_bit(int x, int y)
    {
        if (transform) {
            int x2 = x + x1;
            int y2 = y + y1;
            x = int(x2 * co_divx - y2 * si_divx);
            y = int(y2 * co_divy + x2 * si_divy);
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
    : CollisionBase(POINT_COLLISION, true), x(x), y(y)
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
    : CollisionBase(BOUNDING_BOX, true), x1(x1), y1(y1), x2(y2), y2(y2)
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

class BackgroundItem : public CollisionBase
{
public:
    int dest_x, dest_y, src_x, src_y, src_width, src_height;
    Image * image;
    int collision_type;

#ifdef USE_COL_TREE
    TreeItem tree_item;
#endif

    BackgroundItem(Image * img, int dest_x, int dest_y, int src_x, int src_y,
                   int src_width, int src_height, int type)
    : dest_x(dest_x), dest_y(dest_y), src_x(src_x), src_y(src_y),
      src_width(src_width), src_height(src_height), collision_type(type),
      image(img), CollisionBase(BACKGROUND_ITEM, false)
#ifdef USE_COL_TREE
      , tree_item(this)
#endif
    {
    }

    void get_box(int v[4])
    {
        v[0] = dest_x;
        v[1] = dest_y;
        v[2] = dest_x + src_width;
        v[3] = dest_y + src_height;
    }

    inline bool get_bit(int x, int y)
    {
        x += src_x;
        y += src_y;
        return ((unsigned char*)&image->get(x, y))[3] != 0;
    }
};

inline bool collide_sprite_background(CollisionBase * a, CollisionBase * b,
                                      int w, int h, int offx1, int offy1,
                                      int offx2, int offy2)
{
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            bool c1 = ((SpriteCollision*)a)->get_bit(offx1 + x, offy1 + y);
            bool c2 = ((BackgroundItem*)b)->get_bit(offx2 + x, offy2 + y);
            if (c1 && c2)
                return true;
        }
    }
    return false;
}

inline bool collide_background_box(CollisionBase * a, int w, int h,
                                   int offx, int offy)
{
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            if (((BackgroundItem*)a)->get_bit(offx + x, offy + y))
                return true;
        }
    }
    return false;
}

inline bool collide_sprite_box(CollisionBase * a, int w, int h,
                               int offx, int offy)
{
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            if (((SpriteCollision*)a)->get_bit(offx + x, offy + y))
                return true;
        }
    }
    return false;
}

inline bool collide_sprite_sprite(CollisionBase * a, CollisionBase * b,
                                  int w, int h, int offx1, int offy1,
                                  int offx2, int offy2)
{
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            bool c1 = ((SpriteCollision*)a)->get_bit(offx1 + x, offy1 + y);
            bool c2 = ((SpriteCollision*)b)->get_bit(offx2 + x, offy2 + y);
            if (c1 && c2)
                return true;
        }
    }
    return false;
}

inline bool collide(CollisionBase * a, CollisionBase * b)
{
    if (a == NULL || b == NULL)
        return false;
    int v1[4];
    a->get_box(v1);
    int v2[4];
    b->get_box(v2);

    if (!collides(v1[0], v1[1], v1[2], v1[3], 
                  v2[0], v2[1], v2[2], v2[3]))
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

    int w = x2 - x1;
    int h = y2 - y1;
    
    switch (a->type) {
        case SPRITE_COLLISION:
            switch (b->type) {
                case SPRITE_COLLISION:
                    return collide_sprite_sprite(a, b, w, h, offx1, offy1,
                                                 offx2, offy2);
                case BACKGROUND_ITEM:
                    return collide_sprite_background(a, b, w, h, offx1, offy1,
                                                     offx2, offy2);
                default:
                    return collide_sprite_box(a, w, h, offx1, offy1);
            }
        case BACKGROUND_ITEM:
            switch (b->type) {
                case SPRITE_COLLISION:
                    return collide_sprite_background(b, a, w, h, offx2, offy2,
                                                     offx1, offy1);
                default:
                    return collide_background_box(a, w, h, offx1, offy1);
            }
        default:
            switch (b->type) {
                case SPRITE_COLLISION:
                    return collide_sprite_box(b, w, h, offx2, offy2);
                case BACKGROUND_ITEM:
                    return collide_background_box(b, w, h, offx2, offy2);
                default:
                    return true;
            }
    }
}
