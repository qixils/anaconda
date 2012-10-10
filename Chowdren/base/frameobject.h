class CollisionBase;
class Frame;
class Shader;

class FrameObject
{
public:
    std::string name;
    int x, y;
    int width, height;
    int id;
    AlterableValues * values;
    AlterableStrings * strings;
    Color blend_color;
    int layer_index;
    bool visible;
    Frame * frame;
    Shader * shader;

    FrameObject(std::string name, int x, int y, int type_id);
    void set_position(int x, int y);
    void set_x(int x);
    void set_y(int y);
    void create_alterables();
    void set_visible(bool value);
    void set_blend_color(int color);
    virtual void draw();
    virtual void update(float dt);
    virtual void set_direction(int value);
    virtual CollisionBase * get_collision();
    bool mouse_over();
    bool overlaps(FrameObject * other);
    void set_layer(int layer);
    void set_shader(Shader * shader);
};