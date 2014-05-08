#ifndef CHOWDREN_COLLISION_H
#define CHOWDREN_COLLISION_H

#include "frame.h"
#include <algorithm>
#include "mathcommon.h"
#include "broadphase.h"

bool collide(CollisionBase * a, CollisionBase * b);

enum CollisionType
{
    INSTANCE_BOX,
    OFFSET_INSTANCE_BOX,
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
    int aabb[4];

    CollisionBase(CollisionType type, bool is_box)
    : is_box(is_box), type(type)
    {
    }

    virtual ~CollisionBase()
    {

    }
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

struct TypeOverlapCallback
{
public:
    CollisionBase * collision;
    int type;

    TypeOverlapCallback(CollisionBase * collision, int type)
    : collision(collision), type(type)
    {
    }

    bool on_callback(void * data)
    {
        FrameObject * obj = (FrameObject*)data;
        if (obj->id != type)
            return true;
        if (obj->collision == NULL)
            return true;
        if (!collide(collision, (CollisionBase*)obj->collision))
            return true;
        return false;
    }
};

class InstanceCollision : public CollisionBase
{
public:
    FrameObject * instance;
    int proxy;

    InstanceCollision(FrameObject * instance, CollisionType type, bool is_box)
    : instance(instance), CollisionBase(type, is_box), proxy(-1)
    {
    }

    ~InstanceCollision()
    {
        remove_proxy();
    }

    void remove_proxy()
    {
        if (proxy == -1)
            return;
        instance->layer->broadphase.remove(proxy);
        proxy = -1;
    }

    void create_proxy()
    {
        if (proxy != -1)
            return;
        proxy = instance->layer->broadphase.add(instance, aabb);
    }

    virtual void update_aabb()
    {
        if (proxy == -1)
            return;
        instance->layer->broadphase.move(proxy, aabb);
    }

    bool overlaps_type(int type)
    {
        TypeOverlapCallback callback(this, type);
        if (!instance->layer->broadphase.query(proxy, callback))
            return true;
        return false;
    }
};

class InstanceBox : public InstanceCollision
{
public:
    InstanceBox(FrameObject * instance)
    : InstanceCollision(instance, INSTANCE_BOX, true)
    {
        update_aabb();
    }

    void update_aabb()
    {
        aabb[0] = instance->x;
        aabb[1] = instance->y;
        aabb[2] = aabb[0] + instance->width;
        aabb[3] = aabb[1] + instance->height;
        InstanceCollision::update_aabb();
    }
};

class OffsetInstanceBox : public InstanceCollision
{
public:
    int off_x, off_y;

    OffsetInstanceBox(FrameObject * instance)
    : InstanceCollision(instance, OFFSET_INSTANCE_BOX, true),
      off_x(0), off_y(0)
    {
        update_aabb();
    }

    void update_aabb()
    {
        aabb[0] = instance->x + off_x;
        aabb[1] = instance->y + off_y;
        aabb[2] = aabb[0] + instance->width;
        aabb[3] = aabb[1] + instance->height;
        InstanceCollision::update_aabb();
    }

    void set_offset(int x, int y)
    {
        off_x = x;
        off_y = y;
        update_aabb();
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

inline void transform_rect(int width, int height, float co, float si,
                           float x_scale, float y_scale,
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

class SpriteCollision : public InstanceCollision
{
public:
    Image * image;
    float angle;
    float x_scale, y_scale;
    // transformed variables
    bool transform;
    float co, si;
    float co_divx, si_divx;
    float co_divy, si_divy;
    int x1, y1, x2, y2; // transformed bounding box
    int width, height;
    int hotspot_x, hotspot_y;

    SpriteCollision(FrameObject * instance = NULL, Image * image = NULL)
    : InstanceCollision(instance, SPRITE_COLLISION, false), image(image),
      transform(false), angle(0.0f), x_scale(1.0f), y_scale(1.0f), co(1.0f),
      si(0.0f)
    {
        if (image == NULL)
            return;
        width = image->width;
        height = image->height;
        hotspot_x = image->hotspot_x;
        hotspot_y = image->hotspot_y;
        update_aabb();
    }

    void set_image(Image * image)
    {
        this->image = image;
        update_transform();
    }

    void set_angle(float value)
    {
        angle = value;
        co = cos_deg(angle);
        si = sin_deg(angle);
        update_transform();
    }

    void set_scale(float value)
    {
        x_scale = y_scale = value;
        update_transform();
    }

    void set_x_scale(float x)
    {
        x_scale = x;
        update_transform();
    }

    void set_y_scale(float y)
    {
        y_scale = y;
        update_transform();
    }

    void update_transform()
    {
        if (x_scale == 1.0f && y_scale == 1.0f && angle == 0.0f) {
            width = image->width;
            height = image->height;
            hotspot_x = image->hotspot_x;
            hotspot_y = image->hotspot_y;
            transform = false;
            update_aabb();
            return;
        }

        transform = true;

        float x_scale_inv = 1.0f / x_scale;
        float y_scale_inv = 1.0f / y_scale;

        if (angle == 0.0f) {
            co_divx = x_scale_inv;
            co_divy = y_scale_inv;
            si_divx = si_divy = 0.0f;
            width = int(image->width * x_scale);
            height = int(image->height * y_scale);
            x1 = 0;
            y1 = 0;
            x2 = width;
            y2 = height;
            hotspot_x = int(image->hotspot_x * x_scale);
            hotspot_y = int(image->hotspot_y * y_scale);
            update_aabb();
            return;
        }

        co_divx = co * x_scale_inv;
        co_divy = co * y_scale_inv;
        si_divx = si * x_scale_inv;
        si_divy = si * y_scale_inv;
        transform_rect(image->width, image->height, co, si, x_scale, y_scale,
                       x1, y1, x2, y2);
        width = x2 - x1;
        height = y2 - y1;
        get_transform(image->hotspot_x, image->hotspot_y,
                      hotspot_x, hotspot_y);
        update_aabb();
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
        r_x = new_x - x1;
        r_y = new_y - y1;
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
        return image->get_alpha(x, y);
    }

    void update_aabb()
    {
        aabb[0] = instance->x - hotspot_x;
        aabb[1] = instance->y - hotspot_y;
        aabb[2] = aabb[0] + width;
        aabb[3] = aabb[1] + height;
        InstanceCollision::update_aabb();
    }
};

class PointCollision : public CollisionBase
{
public:
    int x, y;

    PointCollision(int x, int y)
    : CollisionBase(POINT_COLLISION, true), x(x), y(y)
    {
        aabb[0] = x;
        aabb[1] = y;
        aabb[2] = x + 1;
        aabb[3] = y + 1;
    }
};

// class BoundingBox : public CollisionBase
// {
// public:
//     int x1, y1, x2, y2;

//     BoundingBox(int x1, int y1, int x2, int y2)
//     : CollisionBase(BOUNDING_BOX, true), x1(x1), y1(y1), x2(y2), y2(y2)
//     {
//         aabb[0] = x1;
//         aabb[1] = y1;
//         aabb[2] = x2;
//         aabb[3] = y2;
//     }
// };

class BackgroundItem : public CollisionBase
{
public:
    int dest_x, dest_y, src_x, src_y, src_width, src_height;
    Image * image;
    int collision_type;

    unsigned int col;

    BackgroundItem(Image * img, int dest_x, int dest_y, int src_x, int src_y,
                   int src_width, int src_height, int type)
    : dest_x(dest_x), dest_y(dest_y), src_x(src_x), src_y(src_y),
      src_width(src_width), src_height(src_height), collision_type(type),
      image(img), CollisionBase(BACKGROUND_ITEM, false)
    {
        aabb[0] = dest_x;
        aabb[1] = dest_y;
        aabb[2] = dest_x + src_width;
        aabb[3] = dest_y + src_height;
    }

    inline bool get_bit(int x, int y)
    {
        x += src_x;
        y += src_y;
        return image->get_alpha(x, y);
    }

    void draw()
    {
        image->draw(dest_x, dest_y, src_x, src_y, src_width, src_height);
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
    if (!collides(a->aabb, b->aabb))
        return false;

    if (a->is_box && b->is_box)
        return true;

    // calculate the overlapping area
    int x1, y1, x2, y2;
    intersect(a->aabb[0], a->aabb[1], a->aabb[2], a->aabb[3],
              b->aabb[0], b->aabb[1], b->aabb[2], b->aabb[3],
              x1, y1, x2, y2);

    // figure out the offsets of the overlapping area in each
    int offx1 = x1 - a->aabb[0];
    int offy1 = y1 - a->aabb[1];
    int offx2 = x1 - b->aabb[0];
    int offy2 = y1 - b->aabb[1];

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
            // case box
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

inline bool collide_box(FrameObject * a, int v[4])
{
    CollisionBase * col = a->collision;
    if (col == NULL) {
        int xx1 = a->x;
        int yy1 = a->y;
        int xx2 = xx1 + a->width;
        int yy2 = yy1 + a->height;
        return collides(xx1, yy1, xx2, yy2, v[0], v[1], v[2], v[3]);
    }
    return collides(col->aabb, v);
}

#endif // CHOWDREN_COLLISION_H
