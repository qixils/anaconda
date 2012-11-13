#ifndef FRAMEOBJECT_H
#define FRAMEOBJECT_H

#include "alterables.h"
#include "color.h"
#include <map>
#include <string>

class CollisionBase;
class Frame;
class Shader;
class Image;

typedef std::map<std::string, double> ShaderParameters;

class FrameObject
{
public:
    std::string name;
    int x, y;
    int width, height;
    int direction;
    int id;
    AlterableValues * values;
    AlterableStrings * strings;
    Color blend_color;
    int layer_index;
    bool visible;
    Frame * frame;
    Shader * shader;
    ShaderParameters * shader_parameters;
    bool destroying;

    FrameObject(std::string name, int x, int y, int type_id);
    virtual ~FrameObject();
    void set_position(int x, int y);
    void set_x(int x);
    void set_y(int y);
    void create_alterables();
    void set_visible(bool value);
    void set_blend_color(int color);
    virtual void draw();
    void draw_image(Image * img, double x, double y, double angle = 0.0, 
        double scale_x = 1.0, double scale_y = 1.0, 
        bool flip_x = false, bool flip_y = false);
    virtual void update(float dt);
    virtual void set_direction(int value);
    virtual int get_direction();
    virtual CollisionBase * get_collision();
    bool mouse_over();
    bool overlaps(FrameObject * other);
    void set_layer(int layer);
    void set_shader(Shader * shader);
    void set_shader_parameter(const std::string & name, double value);
    void set_shader_parameter(const std::string & name, const Color & color);
    double get_shader_parameter(const std::string & name);
    void set_level(int index);
    int get_level();
    void move_back();
    void move_front();
    void move_front(FrameObject * other);
    void destroy();
    double get_fixed();
    bool outside_playfield();
    void get_box(int box[4]);
    int get_box_index(int index);
    bool overlaps_background();
};

#endif // FRAMEOBJECT_H