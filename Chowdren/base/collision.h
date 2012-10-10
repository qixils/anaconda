bool collides(int a_x1, int a_y1, int a_x2, int a_y2, 
              int b_x1, int b_y1, int b_x2, int b_y2)
{
    if (a_x2 <= b_x1 || a_y2 <= b_y1 || a_x1 >= b_x2 || a_y1 >= b_y2)
        return false;
    return true;
}

class CollisionBase
{
public:
    virtual void get_box(int v[4]) 
    {
        v[0] = v[1] = v[2] = v[3] = 0;
    }

    bool collide(int x1, int y1, int x2, int y2)
    {
        int v[4];
        get_box(v);
        return ::collides(x1, y1, x2, y2, v[0], v[1], v[2], v[3]);
    }

    bool collide(CollisionBase * c)
    {
        int v[4];
        c->get_box(v);
        return collide(v[0], v[1], v[2], v[3]);
    }
};

class BoundingBox : public CollisionBase
{
public:
    FrameObject * instance;

    BoundingBox(FrameObject * instance)
    : instance(instance)
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

class SpriteCollision : public CollisionBase
{
public:
    FrameObject * instance;

    SpriteCollision(FrameObject * instance)
    : instance(instance)
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

class PointCollision : public CollisionBase
{
public:
    int x, y;

    PointCollision(int x, int y)
    : x(x), y(y)
    {
    }

    void get_box(int v[4])
    {
        v[0] = x;
        v[1] = y;
        v[2] = x;
        v[3] = y;
    }
};