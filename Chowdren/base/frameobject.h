#ifndef CHOWDREN_FRAMEOBJECT_H
#define CHOWDREN_FRAMEOBJECT_H

#include "alterables.h"
#include "color.h"
#include <string>
#include <vector>
#include <boost/unordered_map.hpp>

class CollisionBase;
class Frame;
class Shader;
class Image;

typedef boost::unordered_map<std::string, double> ShaderParameters;

class FrameObject;
class Movement;

class FixedValue
{
public:
    FrameObject * object;

    FixedValue(FrameObject * object);
    operator double() const;
    operator std::string() const;
    operator FrameObject*() const;
};

class FrameObject
{
public:
#ifndef NDEBUG
    std::string name;
#endif
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
    bool scroll;
    int movement_count;
    Movement ** movements;
    Movement * movement;
#ifdef CHOWDREN_USE_BOX2D
    int body;
#endif

    FrameObject(int x, int y, int type_id);
    virtual ~FrameObject();
    void set_position(int x, int y);
    void set_global_position(int x, int y);
    int get_x();
    void set_x(int x);
    int get_y();
    void set_y(int y);
    virtual int get_action_x();
    virtual int get_action_y();
    virtual double get_angle();
    virtual void set_angle(double angle, int quality = 0);
    void create_alterables();
    void set_visible(bool value);
    void set_blend_color(int color);
    virtual void draw();
    void draw_image(Image * img, double x, double y, double angle = 0.0,
        double scale_x = 1.0, double scale_y = 1.0,
        bool flip_x = false, bool flip_y = false);
    virtual void update(float dt);
    virtual void set_direction(int value, bool set_movement = true);
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
    void move_back(FrameObject * other);
    void move_front();
    void move_front(FrameObject * other);
    virtual void destroy();
    FixedValue get_fixed();
    bool outside_playfield();
    void get_box(int box[4]);
    int get_box_index(int index);
    bool overlaps_background();
    bool overlaps_background_save();
    void clear_movements();
    void set_movement(int i);
    Movement * get_movement();
    void shoot(FrameObject * other, int speed, int direction);
    const std::string & get_name();
    void look_at(int x, int y);
    void update_flash(float dt, float interval, float & time);
    virtual void flash(float value);
    virtual void set_animation(int value);
};

typedef std::vector<FrameObject*> ObjectList;

#endif // CHOWDREN_FRAMEOBJECT_H
