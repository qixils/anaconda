#ifndef COMMON_H
#define COMMON_H

#define _USE_MATH_DEFINES
#include <math.h>

#include "include_gl.h"
#include <string>
#include <list>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include "font.cpp"
#include <algorithm>
#include <fstream>
#include "utility.h"
#include "media.h"
#include "string.h"
#include "shaders.h"
#include "datastream.h"
#include <cctype>

int randrange(int range)
{
    return (rand() * range) / RAND_MAX;
}

std::string get_app_path()
{
    return "./";
}

std::string convert_path(std::string value)
{
    std::string new_str = value;
    std::replace(new_str.begin(), new_str.end(), '\\', '/');
    return new_str;
}

int pick_random(int count, ...)
{
    va_list ap;
    va_start(ap, count);
    int picked_index = randrange(count);
    int value;
    for(int i = 0; i < count; i++) {
        if (i != picked_index)
            va_arg(ap, int);
        else
            value = va_arg(ap, int);
    }
    va_end(ap);
    return value;
}

// key helpers

bool is_mouse_pressed(int button)
{
    return glfwGetMouseButton(button) == GLFW_PRESS;
}

bool is_key_pressed(int button)
{
    return glfwGetKey(button) == GLFW_PRESS;
}

// math helpers

template <class T>
T rad(T x)
{
    return x * (M_PI/180);
}

double sin_deg(double x)
{
    return sin(rad(x));
}

// string helpers

inline size_t string_find(const std::string & a, const std::string & b, 
                          size_t pos)
{
    return a.find(b, pos);
}

inline size_t string_size(const std::string & a)
{
    return a.size();
}

inline std::string lowercase_string(std::string v)
{
    std::transform(v.begin(), v.end(), v.begin(), tolower);
    return v;
}

inline std::string mid_string(const std::string & v, size_t index, size_t count)
{
    return v.substr(index, count);
}

class Color
{
public:
    float r, g, b, a;

    Color()
    {
        set(1.0f, 1.0f, 1.0f, 1.0f);
    }

    Color(int r, int g, int b, int a = 255)
    {
        set(r, g, b, a);
    }

    Color(float r, float g, float b, float a = 1.0)
    {
        set(r, g, b, a);
    }

    Color(int color)
    {
        set(color);
    }

    void set(float r, float g, float b, float a = 1.0)
    {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    void set(int r, int g, int b, int a = 255)
    {
        set(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

    void set_alpha(float a)
    {
        a = a;
    }

    void set(int color)
    {
        set(color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF);
    }

    void apply()
    {
        glColor4f(r, g, b, a);
    }
};

int make_color_int(unsigned char r, unsigned char g, unsigned char b)
{
    return (r) | (g << 8) | (b << 16);
}

template <class T, size_t B>
class Alterables
{
public:
    T values[B];

    Alterables()
    {

    }

    T get(size_t index)
    {
        if (index < 0 || index >= B)
            return 0;
        return values[index];
    }

    void set(size_t index, T value)
    {
        if (index < 0 || index >= B)
            return;
        values[index] = value;
    }
};

typedef Alterables<int, 26> AlterableValues;
typedef Alterables<std::string, 10> AlterableStrings;

class Font
{
    char * face;
    int size;
    bool bold;
    bool italic;
    bool underline;
public:
    Font(char * face, int size, bool bold, bool italic, bool underline) :
        face(face), 
        size(size), 
        bold(bold), 
        italic(italic), 
        underline(underline) {}
};

class Sound
{
    char * name;
public:
    Sound(char * name) : name(name) {}
};

#include "image.h"
#include "frameobject.h"

typedef std::vector<FrameObject*> ObjectList;

class Layer
{
public:
    ObjectList instances;
    ObjectList backgrounds;
    bool visible;
    double scroll_x, scroll_y;

    Layer(double scroll_x, double scroll_y) 
    : visible(true), scroll_x(scroll_x), scroll_y(scroll_y)
    {

    }

    void add_object(FrameObject * instance)
    {
        instances.push_back(instance);
    }

    void remove_object(FrameObject * instance)
    {
        instances.erase(
            std::remove(instances.begin(), instances.end(), instance), 
            instances.end());
    }

    void draw()
    {
        if (!visible)
            return;

        for (ObjectList::const_iterator iter = instances.begin(); 
             iter != instances.end(); iter++) {
            FrameObject * item = (*iter);
            if (!item->visible)
                continue;
            if (item->shader != NULL)
                item->shader->begin();
            item->draw();
            if (item->shader != NULL)
                item->shader->end();
        }
    }
};

template <class T>
class Globals
{
public:
    std::vector<T> values;

    Globals()
    {

    }

    T get(size_t index)
    {
        if (index < 0 || index >= values.size())
            return T();
        return values[index];
    }

    void set(size_t index, T value)
    {
        if (index < 0)
            return;
        if (index >= values.size())
            values.resize(index + 1);
        values[index] = value;
    }

    void add(size_t index, T value)
    {
        set(index, get(index) + value);
    }
};

typedef Globals<double> GlobalValues;
typedef Globals<std::string> GlobalStrings;

class Frame
{
public:
    std::string name;
    int width, height;
    int index;
    GameManager * manager;
    ObjectList instances;
    std::vector<Layer*> layers;
    std::map<int, ObjectList> instance_classes;
    std::map<std::string, int> loop_indexes;
    std::map<std::string, bool> running_loops;
    Color background_color;
    GlobalValues * global_values;
    GlobalStrings * global_strings;
    Media * media;
    bool has_quit;
    int off_x, off_y;
    int last_key;
    bool key_presses[GLFW_KEY_LAST + 1];
    bool mouse_presses[GLFW_MOUSE_BUTTON_LAST + 1];
    int next_frame;

    Frame(std::string name, int width, int height, Color background_color,
          int index, GameManager * manager)
    : name(name), width(width), height(height), index(index), 
      background_color(background_color), manager(manager),
      off_x(0), off_y(0), has_quit(false), last_key(-1), next_frame(-1)
    {}

    ~Frame()
    {
        ObjectList::const_iterator it;
        for (it = instances.begin(); it != instances.end(); it++) {
            delete (*it);
        }
    }

    virtual void on_start() {}
    virtual void on_end() {}
    virtual void handle_events() {}

    bool update(float dt)
    {
        for (ObjectList::const_iterator iter = instances.begin(); 
             iter != instances.end(); iter++) {
            (*iter)->update(dt);
        }

        handle_events();

        last_key = -1;

        std::fill_n(key_presses, GLFW_KEY_LAST, false);
        std::fill_n(mouse_presses, GLFW_MOUSE_BUTTON_LAST, false);

        return !has_quit;
    }

    void on_key(int key, int state)
    {
        if (state == GLFW_PRESS) {
            last_key = key;
            key_presses[key] = true;
        }
    }

    void on_mouse(int key, int state)
    {
        if (state == GLFW_PRESS) {
            mouse_presses[key] = false;
        }
    }

    void draw() 
    {
        int window_width, window_height;
        glfwGetWindowSize(&window_width, &window_height);
        glViewport(0, 0, window_width, window_height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, window_width, window_height, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glClearColor(background_color.r, background_color.g, background_color.b,
                     background_color.a);
        glClear(GL_COLOR_BUFFER_BIT);

        for (std::vector<Layer*>::const_iterator iter = layers.begin(); 
             iter != layers.end(); iter++) {
            glLoadIdentity();
            glTranslated(-off_x * (*iter)->scroll_x, 
                         -off_y * (*iter)->scroll_y, 0.0);
            (*iter)->draw();
        }

    }

    ObjectList & get_instances(int object_id)
    {
        return instance_classes[object_id];
    }

    ObjectList get_instances(unsigned int qualifier[])
    {
        ObjectList list;
        int index = 0;
        while (qualifier[index] != NULL) {
            ObjectList & b = get_instances(qualifier[index]);
            list.insert(list.end(), b.begin(), b.end());
            index++;
        }
        return list;
    }

    FrameObject & get_instance(int object_id)
    {
        return *instance_classes[object_id][0];
    }

    void add_layer(double scroll_x, double scroll_y)
    {
        layers.push_back(new Layer(scroll_x, scroll_y));
    }

    void add_object(FrameObject * object, int layer_index)
    {
        object->frame = this;
        object->layer_index = layer_index;
        instances.push_back(object);
        instance_classes[object->id].push_back(object);
        layers[layer_index]->add_object(object);
    }

    ObjectList create_object(FrameObject * object, int layer_index)
    {
        add_object(object, layer_index);
        ObjectList new_list;
        new_list.push_back(object);
        return new_list;
    }

    void set_object_layer(FrameObject * object, int new_layer)
    {
        layers[object->layer_index]->remove_object(object);
        layers[new_layer]->add_object(object);
    }

    unsigned int get_loop_index(std::string name)
    {
        return loop_indexes[name];
    }

    void set_display_center(int x = -1, int y = -1)
    {
        if (x != -1) {
            off_x = x - WINDOW_WIDTH / 2;
        }
        if (y != -1) {
            off_y = std::min<int>(
                std::max<int>(0, y - WINDOW_HEIGHT / 2),
                height - WINDOW_HEIGHT);
        }
    }

    void get_mouse_pos(int * x, int * y)
    {
        glfwGetMousePos(x, y);
        (*x) += off_x;
        (*y) += off_y;
    }

    int get_mouse_x()
    {
        int x, y;
        get_mouse_pos(&x, &y);
        return x;
    }

    int get_mouse_y()
    {
        int x, y;
        get_mouse_pos(&x, &y);
        return y;
    }

    bool is_mouse_pressed_once(int key)
    {
        return (bool)mouse_presses[key];
    }

    bool is_key_pressed_once(int key)
    {
        return (bool)key_presses[key];
    }

};

// object types

#include "collision.h"  

FrameObject::FrameObject(std::string name, int x, int y, int type_id) 
: name(name), x(x), y(y), id(type_id), visible(true), shader(NULL)
{
}

void FrameObject::set_position(int x, int y)
{
    this->x = x;
    this->y = y;
}

void FrameObject::set_x(int x)
{
    this->x = x;
}

void FrameObject::set_y(int y)
{
    this->y = y;
}

void FrameObject::create_alterables()
{
    values = new AlterableValues;
    strings = new AlterableStrings;
}

void FrameObject::set_visible(bool value)
{
    visible = value;
}

void FrameObject::set_blend_color(int color)
{
    blend_color = Color(color);
}

void FrameObject::set_layer(int index)
{
    frame->set_object_layer(this, index);
}

void FrameObject::draw() {}
void FrameObject::update(float dt) {}
void FrameObject::set_direction(int value) {}

CollisionBase * FrameObject::get_collision()
{
    return NULL;
}

bool FrameObject::mouse_over()
{
    int x, y;
    frame->get_mouse_pos(&x, &y);
    PointCollision col1(x, y);
    CollisionBase * col2 = get_collision();
    if (col2 == NULL)
        return false;
    return col1.collide(col2);
}

bool FrameObject::overlaps(FrameObject * other)
{
    return other->get_collision()->collide(get_collision());
}

void FrameObject::set_shader(Shader * value)
{
    shader = value;
}

enum AnimationIndex {
    STOPPED = 0,
    WALKING = 1,
    RUNNING = 2,
    APPEARING = 3,
    DISAPPEARING = 4,
    BOUNCING = 5,
    SHOOTING = 6,
    JUMPING = 7,
    FALLING = 8,
    CLIMBING = 9,
    CROUCH = 10,
    STAND = 11,
    USER_DEFINED_1 = 12,
    USER_DEFINED_2 = 13,
    USER_DEFINED_3 = 14,
    USER_DEFINED_4 = 15
};

class Direction
{
public:
    int index, min_speed, max_speed, back_to;
    bool repeat;
    std::vector<Image*> frames;

    Direction(int index, int min_speed, int max_speed, bool repeat, int back_to)
    : index(index), min_speed(min_speed), max_speed(max_speed), repeat(repeat),
      back_to(back_to)
    {

    }
};

class Active : public FrameObject
{
public:
    std::map<int, std::map<int, Direction*>> animations;

    int animation;
    unsigned int direction, frame;
    unsigned int counter;
    double angle;
    CollisionBase * collision;

    Active(std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id), animation(STOPPED), direction(0),
      frame(0), counter(0), angle(0.0)
    {
        create_alterables();
        collision = new BoundingBox(this);
    }

    void force_animation(int value)
    {
        animation = value;
    }

    void add_direction(int animation, int direction,
                       int min_speed, int max_speed, bool repeat,
                       int back_to)
    {
        animations[animation][direction] = new Direction(direction, min_speed,
            max_speed, repeat, back_to);
    }

    void add_image(int animation, int direction, Image * image)
    {
        animations[animation][direction]->frames.push_back(image);
    }

    void update_frame()
    {
        Image * img = get_image();
        img->load();
        width = img->width;
        height = img->height;
    }

    void update(float dt)
    {
        Direction * dir = animations[animation][direction];
        counter += dir->max_speed;
        while (counter > 100) {
            frame++;
            if (frame >= dir->frames.size())
                frame = 0;
            counter -= 100;
        }
        update_frame();
    }

    void draw()
    {
        blend_color.apply();
        animations[animation][direction]->frames[frame]->draw(x, y, angle);
    }

    Image * get_image()
    {
        return animations[animation][direction]->frames[frame];
    }

    void set_angle(double angle, int quality)
    {
        this->angle = angle;
    }

    double get_angle()
    {
        return angle;
    }

    CollisionBase * get_collision()
    {
        return collision;
    }

    void set_direction(int value)
    {
        if (animations[animation].count(value) == 0)
            return;
        direction = value;
    }

    void set_x_scale(double x_scale)
    {

    }

    void set_y_scale(double y_scale)
    {

    }
};

static FTTextureFont default_font("Arial.ttf", false);

class Text : public FrameObject
{
public:
    std::vector<std::string> paragraphs;
    std::string text;
    unsigned int current_paragraph;
    bool initialized;
    Color color;
    int alignment;
    CollisionBase * collision;

    Text(std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id), initialized(false), current_paragraph(0)
    {
        create_alterables();
        collision = new BoundingBox(this);
    }

    void add_line(std::string text)
    {
        paragraphs.push_back(text);
        if (!initialized) {
            initialized = true;
            this->text = text;
        }
    }

    void draw()
    {
        static bool init = false;
        if (!init) {
            init = true;
            default_font.FaceSize(12, 96);
        }
        color.apply();
        glPushMatrix();
        FTBBox box = default_font.BBox(text.c_str(), text.size(), FTPoint());
        double box_w = box.Upper().X() - box.Lower().X();
        double box_h = box.Upper().Y() - box.Lower().Y();
        double off_x = x;
        double off_y = y + default_font.Ascender();

        if (alignment & ALIGN_HCENTER)
            off_x += 0.5 * (width - box_w);
        else if (alignment & ALIGN_RIGHT) 
            off_x += width - box_w;

        if (alignment & ALIGN_VCENTER)
            off_y += height * 0.5 - default_font.LineHeight() * 0.5;
        else if (alignment & ALIGN_BOTTOM)
            off_y += default_font.LineHeight();

        glTranslated((int)off_x, (int)off_y, 0.0);
        glScalef(1, -1, 1);
        default_font.Render(text.c_str(), text.size(), FTPoint(),
            FTPoint(), RENDER_ALL);
        glPopMatrix();
    }

    void set_string(std::string value)
    {
        text = value;
    }

    void set_paragraph(unsigned int index)
    {
        current_paragraph = index;
        set_string(get_paragraph(index));
    }

    int get_index()
    {
        return current_paragraph;
    }

    std::string get_paragraph(int index)
    {
        if (index < 0)
            index = 0;
        else if (index >= (int)paragraphs.size())
            index = paragraphs.size() - 1;
        return paragraphs[index];
    }

    CollisionBase * get_collision()
    {
        return collision;
    }
};

class Backdrop : public FrameObject
{
public:
    Image * image;

    Backdrop(Image * image, std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id), image(image)
    {
    }

    void draw()
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        image->draw(x, y);
    }
};

class Counter : public FrameObject
{
public:
    Image * images[14];
    double value;
    double minimum, maximum;
    std::string cached_string;

    Counter(int init, int min, int max, std::string name, 
            int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id), minimum(min), maximum(max)
    {
        set(init);
    }

    Image * get_image(char c)
    {
        switch (c) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                return images[c - '0'];
            case '-':
                return images[10];
            case '+':
                return images[11];
            case '.':
                return images[12];
            case 'e':
                return images[13];
            default:
                return NULL;
        }
    }

    void add(double value) {
        set(this->value + value);
    }

    void set(double value) {
        value = std::max<double>(std::min<double>(value, maximum), minimum);
        this->value = value;

        std::ostringstream str;
        str << value;
        cached_string = str.str();
    }

    void draw()
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        double current_x = x;
        for (std::string::const_reverse_iterator it = cached_string.rbegin(); 
             it < cached_string.rend(); it++) {
            Image * image = get_image(it[0]);
            if (image == NULL)
                continue;
            image->draw(current_x + image->hotspot_x - image->width, 
                        y + image->hotspot_y - image->height);
            current_x -= image->width;
        }
    }
};

#include "ini.c"

typedef std::map<std::string, std::string> OptionMap;
typedef std::map<std::string, OptionMap> SectionMap;

inline bool match_wildcard(const std::string & pattern, 
                           const std::string & value)
{
    if (pattern == "*")
        return true;
    else if (std::count(pattern.begin(), pattern.end(), '*') > 0) {
        std::cout << "Wildcards not implemented yet: " << pattern << std::endl;
        return false;
    }
    return value == pattern;
}

class INI : public FrameObject
{
public:
    std::string current_group;
    SectionMap data;
    std::vector<std::pair<std::string,std::string>> search_results;
    bool overwrite;
    bool auto_save;

    INI(std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id), overwrite(false), auto_save(false)
    {
    }

    static int _parse_handler(void* user, const char* section, const char* name,
                             const char* value)
    {
        INI * reader = (INI*)user;
        reader->parse_handler(section, name, value);
        return 1;
    }

    void parse_handler(const std::string & section, const std::string & name,
                       const std::string & value)
    {
        if (!overwrite && has_item(section, name))
            return;
        data[section][name] = value;
    }

    void set_group(std::string name, bool new_group)
    {
        current_group = name;
    }

    std::string get_string(const std::string & group, const std::string & item, 
                           const std::string & def)
    {
        SectionMap::const_iterator it = data.find(group);
        if (it == data.end())
            return def;
        OptionMap::const_iterator new_it = (*it).second.find(item);
        if (new_it == (*it).second.end())
            return def;
        return (*new_it).second;
    }

    std::string get_string(const std::string & item, const std::string & def)
    {
        return get_string(current_group, item, def);
    }

    std::string get_string_index(const std::string & group, unsigned int index)
    {
        SectionMap::const_iterator it = data.find(group);
        if (it == data.end())
            return "";
        OptionMap::const_iterator new_it = (*it).second.begin();
        int current_index = 0;
        while (new_it != (*it).second.end()) {
            if (current_index == index)
                return (*new_it).second;
            new_it++;
            current_index++;
        }
        return "";
    }

    std::string get_string_index(unsigned int index)
    {
        return get_string_index(current_group, index);
    }

    std::string get_item_name(const std::string & group, unsigned int index)
    {
        SectionMap::const_iterator it = data.find(group);
        if (it == data.end())
            return "";
        OptionMap::const_iterator new_it = (*it).second.begin();
        int current_index = 0;
        while (new_it != (*it).second.end()) {
            if (current_index == index)
                return (*new_it).first;
            new_it++;
            current_index++;
        }
        return "";
    }

    std::string get_item_name(unsigned int index)
    {
        return get_item_name(current_group, index);
    }

    std::string get_group_name(unsigned int index)
    {
        SectionMap::const_iterator it = data.begin();
        int current_index = 0;
        while (it != data.end()) {
            if (current_index == index)
                return (*it).first;
            it++;
            current_index++;
        }
        return "";
    }

    double get_value(const std::string & group, const std::string & item, 
                     double def)
    {
        SectionMap::const_iterator it = data.find(current_group);
        if (it == data.end())
            return def;
        OptionMap::const_iterator new_it = (*it).second.find(item);
        if (new_it == (*it).second.end())
            return def;
        return string_to_double((*new_it).second, def);
    }

    double get_value(const std::string & item, double def)
    {
        return get_value(current_group, item, def);
    }

    void set_value(const std::string & group, const std::string & item, 
                   int pad, double value)
    {
        data[group][item] = number_to_string(value);
    }

    void set_value(const std::string & item, int pad, double value)
    {
        data[current_group][item] = number_to_string(value);
    }

    void set_string(const std::string & item, const std::string & value)
    {
        data[current_group][item] = value;
    }

    void load_file(const std::string & fn, bool read_only = false, 
                   bool merge = false, bool overwrite = false)
    {
        if (!merge)
            reset();
        std::string filename = convert_path(fn);
        std::cout << "Loading " << filename << std::endl;
        int e = ini_parse(filename.c_str(), _parse_handler, this);
        if (e != 0) {
            std::cout << "INI load failed (" << filename << ") with code " << e
            << std::endl;
        }
    }

    void merge_file(const std::string & fn, bool overwrite)
    {
        load_file(fn, false, true, overwrite);
    }

    void get_data(std::stringstream & out)
    {
        SectionMap::const_iterator it1;
        OptionMap::const_iterator it2;
        for (it1 = data.begin(); it1 != data.end(); it1++) {
            out << "[" << (*it1).first << "]" << std::endl;
            for (it2 = (*it1).second.begin(); it2 != (*it1).second.end(); 
                 it2++) {
                out << (*it2).first << " = " << (*it2).second << std::endl;
            }
            out << std::endl;
        }
    }

    void save_file(const std::string & filename)
    {
        std::stringstream out;
        get_data(out);
        std::ofstream fp;
        fp.open(filename.c_str());
        fp << out.rdbuf();
        fp.close();
    }

    int get_item_count(const std::string & section)
    {
        return data[section].size();
    }

    int get_item_count()
    {
        return get_item_count(current_group);
    }

    int get_group_count()
    {
        return data.size();
    }

    bool has_item(const std::string & group, const std::string & option)
    {
        SectionMap::const_iterator it = data.find(group);
        if (it == data.end())
            return false;
        OptionMap::const_iterator new_it = (*it).second.find(option);
        if (new_it == (*it).second.end())
            return false;
        return true;
    }

    bool has_item(const std::string & option)
    {
        return has_item(current_group, option);
    }

    void search(const std::string & group, const std::string & item, 
                const std::string & value)
    {
        search_results.clear();
        SectionMap::const_iterator it1;
        OptionMap::const_iterator it2;
        for (it1 = data.begin(); it1 != data.end(); it1++) {
            if (!match_wildcard(group, (*it1).first))
                continue;
            for (it2 = (*it1).second.begin(); it2 != (*it1).second.end(); 
                 it2++) {
                if (!match_wildcard(item, (*it2).first))
                    continue;
                if (!match_wildcard(value, (*it2).second))
                    continue;
                search_results.push_back(
                    std::pair<std::string, std::string>(group, item));
            }
        }
    }

    void reset()
    {
        data.clear();
    }

    void set_global_data(const std::string & key)
    {

    }

    void merge_object(INI * other, bool overwrite)
    {
        merge_map(other->data, overwrite);
    }

    void merge_map(const SectionMap & data2, bool overwrite)
    {
        SectionMap::const_iterator it1;
        OptionMap::const_iterator it2;
        for (it1 = data2.begin(); it1 != data2.end(); it1++) {
            for (it2 = (*it1).second.begin(); it2 != (*it1).second.end(); 
                 it2++) {
                if (!overwrite && has_item((*it1).first, (*it2).first))
                    continue;
                data[(*it1).first][(*it2).first] = (*it2).second;
            }
        }
    }

    size_t get_search_count()
    {
        return search_results.size();
    }

    std::string get_search_result_group(int index)
    {
        return search_results[index].first;
    }
};

class File
{
public:
    static std::string get_appdata_directory()
    {
        return get_app_path();
    }

    static bool file_exists(std::string path)
    {
        std::ifstream ifile(path.c_str());
        if (ifile) {
            return true;
        } else {
            return false;
        }
    }
};

class Workspace
{
public:
    std::string name;
    std::string data;

    Workspace(DataStream & stream)
    {
        stream >> name;
        unsigned int len;
        stream >> len;
        stream.read(data, len);
    }
};

class BinaryArray : public FrameObject
{
public:
    std::map<std::string, Workspace*> workspaces;

    BinaryArray(std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id)
    {

    }

    void load(const std::string & filename)
    {
        std::ifstream fp(filename.c_str(), std::ios::binary);
        DataStream stream(fp);
        while (!stream.eof()) {
            Workspace * workspace = new Workspace(stream);
            workspaces[workspace->name] = workspace;
        }
    }
};

class ArrayObject : public FrameObject
{
public:
    ArrayObject(std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id)
    {

    }
};

class ActivePicture : public FrameObject
{
public:
    Image * image;

    ActivePicture(std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id), image(NULL)
    {
        
    }

    void load(const std::string & filename)
    {
        image = new Image(filename, 0, 0, 0, 0);
    }

    void set_hotspot(int x, int y)
    {
        image->hotspot_x = x;
        image->hotspot_y = y;
    }

    void draw()
    {
        if (image == NULL)
            return;
        image->draw(x, y);
    }
};

#define CHOWDREN_PYTHON
#ifdef CHOWDREN_PYTHON
#include "python.h"
#endif

// collision helpers

bool check_overlap(ObjectList in_a, ObjectList in_b, 
                   ObjectList & out_a, ObjectList & out_b,
                   bool negated)
{
    out_a.clear();
    out_b.clear();
    ObjectList::const_iterator item1, item2;
    bool added;
    bool ret = false;
    for (item1 = in_a.begin(); item1 != in_a.end(); item1++) {
        added = false;
        for (item2 = in_b.begin(); item2 != in_b.end(); item2++) {
            FrameObject * f1 = (*item1);
            FrameObject * f2 = (*item2);
            if ((*item1)->overlaps((*item2)) == negated)
                continue;
            ret = true;
            if (!added) {
                added = true;
                out_a.push_back((*item1));
            }
            out_b.push_back((*item2));
        }
    }
    return ret;
}

void open_process(std::string exe, std::string cmd, int pad)
{

};

void set_cursor_visible(bool value)
{
    if (value)
        glfwEnable(GLFW_MOUSE_CURSOR);
    else
        glfwDisable(GLFW_MOUSE_CURSOR);
}

void set_fullscreen(bool value)
{
    std::cout << "Set fullscreen: " << value << std::endl;
}

static ObjectList::const_iterator item;

#endif /* COMMON_H */