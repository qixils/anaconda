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
#include "globals.h"
#include "image.h"
#include "frameobject.h"
#include "collision.h"
#include "alterables.h"
#include "color.h"
#include "mathcommon.h"
#include "assets.h"
#include "path.h"

std::string newline_character("\r\n");

int randrange(int range)
{
    if (range == 0)
        return 0;
    return rand() / (RAND_MAX / range + 1);
}

bool random_chance(int a, int b)
{
    return randrange(b) < a;
}

int pick_random(int count, ...)
{
    if (count == 0)
        std::cout << "Invalid pick_random count!" << std::endl;
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

std::vector<std::string> & split_string(const std::string &s, char delim, 
                                        std::vector<std::string> &elems) 
{
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

// key helpers

bool is_mouse_pressed(int button)
{
    if (button < 0)
        return false;
    return glfwGetMouseButton(button) == GLFW_PRESS;
}

bool is_key_pressed(int button)
{
    if (button < 0)
        return false;
    return glfwGetKey(button) == GLFW_PRESS;
}

// string helpers

inline int string_find(const std::string & a, const std::string & b, 
                          size_t pos)
{
    size_t ret = a.find(b, pos);
    if (ret == std::string::npos)
        return -1;
    return ret;
}

inline int string_rfind(const std::string & a, const std::string & b, 
                           size_t pos)
{
    size_t ret = a.rfind(b, pos);
    if (ret == std::string::npos)
        return -1;
    return ret;
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
    if (index > v.size())
        return "";
    return v.substr(index, count);
}

inline std::string left_string(const std::string & v, size_t count)
{
    return v.substr(0, count);
}

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

typedef std::vector<FrameObject*> ObjectList;

inline unsigned int nearest_pot(unsigned int v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

#define BACK_WIDTH WINDOW_WIDTH
#define BACK_HEIGHT WINDOW_HEIGHT
#define BACK_X_OFFSET 0
#define BACK_Y_OFFSET 0
#define BACK_WIDTH_POT nearest_pot(BACK_WIDTH)
#define BACK_HEIGHT_POT nearest_pot(BACK_HEIGHT)

class BackgroundItem
{
public:
    int dest_x, dest_y, src_x, src_y, src_width, src_height;
    Image * image;
    int collision_type;

    BackgroundItem(Image * img, int dest_x, int dest_y, int src_x, int src_y, 
                   int src_width, int src_height, int collision_type)
    : dest_x(dest_x), dest_y(dest_y), src_x(src_x), src_y(src_y),
      src_width(src_width), src_height(src_height), 
      collision_type(collision_type), image(img)
    {

    }
};

class Background
{
public:
    unsigned char * mask;
    unsigned char * image;
    bool image_changed;
    bool items_changed;
    GLuint tex;
    CollisionBase * collision;
    std::vector<BackgroundItem> items;

    Background()
    : mask(NULL), image(NULL), image_changed(true)
    {
        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, BACK_WIDTH_POT,
            BACK_HEIGHT_POT, 0, GL_RGBA, GL_UNSIGNED_BYTE,
            0);
        glDisable(GL_TEXTURE_2D);
        reset();
    }

    void reset(bool clear_items = true)
    {
        if (mask != NULL) {
            delete mask;
            delete image;
            delete collision;
        }
        mask = new unsigned char[BACK_WIDTH * BACK_HEIGHT]();
        image = new unsigned char[BACK_WIDTH * BACK_HEIGHT * 4]();
        collision = new MaskCollision(mask, 0, 0, BACK_WIDTH, BACK_HEIGHT);
        image_changed = true;
        items_changed = false;
        if (clear_items)
            items.clear();
    }

    void destroy_at(int x, int y)
    {
        std::vector<BackgroundItem>::iterator it = items.begin();
        while (it != items.end())
        {
            BackgroundItem & item = (*it);
            if (collides(item.dest_x, item.dest_y, 
                         item.dest_x + item.src_width, 
                         item.dest_y + item.src_height,
                         x, y, x, y)) {
                // std::cout << "Destroying: " << item.collision_type << std::endl;
                it = items.erase(it);
                items_changed = true;
            } else
                ++it;
        }
    }

    inline unsigned int & get(int x, int y)
    {
        return ((unsigned int*)image)[y * BACK_WIDTH + x];
    }

    inline unsigned char & get_mask(int x, int y)
    {
        return mask[y * BACK_WIDTH + x];
    }

    void update()
    {
        if (!items_changed)
            return;
        reset(false);
        std::vector<BackgroundItem>::const_iterator it;
        for (it = items.begin(); it != items.end(); it++) {
            const BackgroundItem & item = (*it);
            paste(item.image, item.dest_x, item.dest_y, 
                  item.src_x, item.src_y, item.src_width, item.src_height,
                  item.collision_type, false);
        }
    }

    void paste(Image * img, int dest_x, int dest_y, 
               int src_x, int src_y, int src_width, int src_height, 
               int collision_type, bool save = true)
    {
        if (save) {
            items.push_back(BackgroundItem(
                img, dest_x, dest_y, src_x, src_y, src_width, src_height, 
                collision_type));
            items_changed = true;
            return;
        }
        int x, y, dest_x2, dest_y2, src_x2, src_y2;
        for (x = 0; x < src_width; x++) {
            src_x2 = src_x + x;
            if (src_x2 < 0 || src_x2 >= img->width)
                continue;
            dest_x2 = dest_x + x;
            if (dest_x2 < 0 || dest_x2 >= BACK_WIDTH)
                continue;
            for (y = 0; y < src_height; y++) {
                src_y2 = src_y + y;
                if (src_y2 < 0 || src_y2 >= img->height)
                    continue;
                dest_y2 = dest_y + y;
                if (dest_y2 < 0 || dest_y2 >= BACK_HEIGHT)
                    continue;
                unsigned char * src_c = (unsigned char*)&img->get(
                    src_x2, src_y2);
                unsigned char & src_a = src_c[3];
                if (collision_type == 1) {
                    unsigned char & m = get_mask(dest_x2, dest_y2);
                    m = src_a | m;
                } else {
                    unsigned char & src_r = src_c[0];
                    unsigned char & src_g = src_c[1];
                    unsigned char & src_b = src_c[2];
                    unsigned char * dst_c = (unsigned char*)&get(dest_x2, 
                        dest_y2);
                    unsigned char & dst_r = dst_c[0];
                    unsigned char & dst_g = dst_c[1];
                    unsigned char & dst_b = dst_c[2];
                    unsigned char & dst_a = dst_c[3];
                    float srcf_a = src_a / 255.0f;
                    float one_minus_src = 1.0f - srcf_a;
                    dst_r = srcf_a * src_r + one_minus_src * dst_r;
                    dst_g = srcf_a * src_g + one_minus_src * dst_g;
                    dst_b = srcf_a * src_b + one_minus_src * dst_b;
                    dst_a = (srcf_a + (dst_a / 255.0f) * one_minus_src) * 255;
                }
/*                unsigned char & m = get_mask(dest_x2, dest_y2);
                m = src_a | m;
                unsigned char * dst_c = (unsigned char*)&get(dest_x2, 
                    dest_y2);
                dst_c[0] = dst_c[1] = dst_c[2] = 0;
                dst_c[3] = m;*/
            }
        }
        image_changed = true;
    }

    void draw()
    {
        update();
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
        glColor4f(1.0, 1.0, 1.0, 1.0);
        if (image_changed) {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, BACK_WIDTH,
                BACK_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE,
                image);
            image_changed = false;
        }
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex2d(0, 0);
        glTexCoord2f(1.0, 0.0);
        glVertex2d(BACK_WIDTH_POT, 0.0);
        glTexCoord2f(1.0, 1.0);
        glVertex2d(BACK_WIDTH_POT, BACK_HEIGHT_POT);
        glTexCoord2f(0.0, 1.0);
        glVertex2d(0, BACK_HEIGHT_POT);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    CollisionBase * get_collision()
    {
        return collision;
    }
};

class Layer
{
public:
    ObjectList instances;
    ObjectList background_instances;
    bool visible;
    double scroll_x, scroll_y;
    Background * back;
    int index;

    Layer(double scroll_x, double scroll_y, bool visible, int index) 
    : visible(visible), scroll_x(scroll_x), scroll_y(scroll_y), back(NULL),
      index(index)
    {

    }

    ~Layer()
    {
        delete back;

        // layers are in charge of deleting background instances
        for (ObjectList::const_iterator iter = background_instances.begin(); 
             iter != background_instances.end(); iter++) {
            delete (*iter);
        }
    }

    void add_background_object(FrameObject * instance)
    {
        background_instances.push_back(instance);
    }

    void add_object(FrameObject * instance)
    {
        instances.push_back(instance);
    }

    void insert_object(FrameObject * instance, int index)
    {
        instances.insert(instances.begin() + index, instance);
    }

    void remove_object(FrameObject * instance)
    {
        instances.erase(
            std::remove(instances.begin(), instances.end(), instance), 
            instances.end());
    }

    void set_level(FrameObject * instance, int index)
    {
        remove_object(instance);
        if (index == -1)
            add_object(instance);
        else
            insert_object(instance, index);
    }

    int get_level(FrameObject * instance)
    {
        return std::find(instances.begin(), instances.end(), 
            instance) - instances.begin();
    }

    void create_background()
    {
        if (back == NULL)
            back = new Background;
    }

    void destroy_backgrounds()
    {
        create_background();
        back->reset();
    }

    void destroy_backgrounds(int x, int y, bool fine)
    {
        if (fine)
            std::cout << "Destroy backgrounds at " << x << ", " << y <<
                " (" << fine << ") not implemented" << std::endl;
        back->destroy_at(x, y);
    }

    bool test_background_collision(int x, int y)
    {
        if (back == NULL)
            return false;
        return back->get_mask(x, y) != 0;
    }

    void paste(Image * img, int dest_x, int dest_y, 
               int src_x, int src_y, int src_width, int src_height, 
               int collision_type)
    {
        create_background();
        back->paste(img, dest_x, dest_y, src_x, src_y, 
            src_width, src_height, collision_type);
    }

    void draw()
    { 
        if (!visible)
            return;

        if (back != NULL)
            back->draw();

        // draw backgrounds
        for (ObjectList::const_iterator iter = background_instances.begin(); 
             iter != background_instances.end(); iter++) {
            FrameObject * item = (*iter);
            if (!item->visible)
                continue;
            item->draw();
        }

        // draw active instances
        for (ObjectList::const_iterator iter = instances.begin(); 
             iter != instances.end(); iter++) {
            FrameObject * item = (*iter);
            if (!item->visible)
                continue;
            item->draw();
        }
    }
};

class Frame
{
public:
    std::string name;
    int width, height;
    int index;
    GameManager * manager;
    ObjectList instances;
    ObjectList destroyed_instances;
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
    unsigned int loop_count;
    double frame_time;

    Frame(std::string name, int width, int height, Color background_color,
          int index, GameManager * manager)
    : name(name), width(width), height(height), index(index), 
      background_color(background_color), manager(manager),
      off_x(0), off_y(0), has_quit(false), last_key(-1), next_frame(-1),
      loop_count(0), frame_time(0.0)
    {
        std::fill_n(key_presses, GLFW_KEY_LAST, false);
        std::fill_n(mouse_presses, GLFW_MOUSE_BUTTON_LAST, false);
    }

    virtual void on_start() {}

    void on_end() 
    {
        ObjectList::const_iterator it;
        for (it = instances.begin(); it != instances.end(); it++) {
            delete (*it);
        }
        std::vector<Layer*>::const_iterator layer_it;
        for (layer_it = layers.begin(); layer_it != layers.end(); layer_it++) {
            delete (*layer_it);
        }
        instances.clear();
        layers.clear();
        instance_classes.clear();
        next_frame = -1;
        loop_count = 0;
        off_x = 0;
        off_y = 0;
        frame_time = 0.0;
    }

    virtual void handle_events() {}

    bool update(float dt)
    {
        frame_time += dt;

        for (ObjectList::const_iterator iter = instances.begin(); 
             iter != instances.end(); iter++) {
            (*iter)->update(dt);
        }

        handle_events();

        for (ObjectList::const_iterator iter = destroyed_instances.begin(); 
             iter != destroyed_instances.end(); iter++) {
            delete (*iter);
        }

        destroyed_instances.clear();

        last_key = -1;

        std::fill_n(key_presses, GLFW_KEY_LAST, false);
        std::fill_n(mouse_presses, GLFW_MOUSE_BUTTON_LAST, false);

        loop_count++;

        return !has_quit;
    }

    void pause()
    {

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
            mouse_presses[key] = true;
        }
    }

    void draw() 
    {
        // first, draw the actual window contents
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glClearColor(background_color.r / 255.0f, background_color.g / 255.0f, 
                     background_color.b / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        int index = 0;
        for (std::vector<Layer*>::const_iterator iter = layers.begin(); 
             iter != layers.end(); iter++) {
            glLoadIdentity();
            glTranslated(-off_x * (*iter)->scroll_x, 
                         -off_y * (*iter)->scroll_y, 0.0);
            (*iter)->draw();
            index++;
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
        while (qualifier[index] != 0) {
            ObjectList & b = get_instances(qualifier[index]);
            list.insert(list.end(), b.begin(), b.end());
            index++;
        }
        return list;
    }

    FrameObject * get_instance(int object_id)
    {
        ObjectList & instances = instance_classes[object_id];
        if (instances.size() == 0)
            std::cout << "Fatal error: invalid instance count" << std::endl;
        return instances[0];
    }

    FrameObject * get_instance(unsigned int qualifier[])
    {
        ObjectList list = get_instances(qualifier);
        if (list.size() == 0)
            std::cout << "Fatal error: invalid instance count" << std::endl;
        return list[0];
    }

    void add_layer(double scroll_x, double scroll_y, bool visible)
    {
        layers.push_back(new Layer(scroll_x, scroll_y, visible, layers.size()));
    }

    void add_object(FrameObject * object, int layer_index)
    {
        object->frame = this;
        object->layer_index = layer_index;
        instances.push_back(object);
        instance_classes[object->id].push_back(object);
        layers[layer_index]->add_object(object);
    }

    void add_background_object(FrameObject * object, int layer_index)
    {
        object->frame = this;
        object->layer_index = layer_index;
        layers[layer_index]->add_background_object(object);
    }

    ObjectList create_object(FrameObject * object, int layer_index)
    {
        add_object(object, layer_index);
        ObjectList new_list;
        new_list.push_back(object);
        return new_list;
    }

    void destroy_object(FrameObject * object)
    {
        instances.erase(
            std::remove(instances.begin(), instances.end(), object), 
            instances.end());
        ObjectList & class_instances = instance_classes[object->id];
        class_instances.erase(
            std::remove(class_instances.begin(), class_instances.end(), object), 
            class_instances.end());
        layers[object->layer_index]->remove_object(object);
        destroyed_instances.push_back(object);
    }

    void set_object_layer(FrameObject * object, int new_layer)
    {
        layers[object->layer_index]->remove_object(object);
        layers[new_layer]->add_object(object);
        object->layer_index = new_layer;
    }

    int get_loop_index(std::string name)
    {
        return loop_indexes[name];
    }

    void set_display_center(int x = -1, int y = -1)
    {
        if (x != -1) {
            off_x = x - WINDOW_WIDTH / 2;
        }
        if (y != -1) {
            off_y = int_min(
                int_max(0, y - WINDOW_HEIGHT / 2),
                height - WINDOW_HEIGHT);
        }
    }

    void set_background_color(int color)
    {
        background_color = Color(color);
    }

    void get_mouse_pos(int * x, int * y)
    {
        (*x) = manager->mouse_x + off_x;
        (*y) = manager->mouse_y + off_y;
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
        if (key < 0)
            return false;
        return (bool)mouse_presses[key];
    }

    bool is_key_pressed_once(int key)
    {
        if (key < 0)
            return false;
        return (bool)key_presses[key];
    }

    bool test_background_collision(int x, int y)
    {
        if (x < 0 || y < 0 || x > width || y > width)
            return false;
        std::vector<Layer*>::const_iterator it;
        for (it = layers.begin(); it != layers.end(); it++) {
            if ((*it)->test_background_collision(x, y))
                return true;
        }
        return false;
    }

};

// object types

FrameObject::FrameObject(std::string name, int x, int y, int type_id) 
: name(name), x(x), y(y), id(type_id), visible(true), shader(NULL), 
values(NULL), strings(NULL), shader_parameters(NULL), direction(0)
{
}

FrameObject::~FrameObject()
{
    delete values;
    delete strings;
    delete shader_parameters;
}

void FrameObject::draw_image(Image * img, double x, double y, double angle,
                             double x_scale, double y_scale, bool flip_x,
                             bool flip_y)
{
    GLuint back_tex = 0;
    if (shader != NULL) {
        shader->begin(this, img);
        back_tex = shader->get_background_texture();
    }
    img->draw(x, y, angle, x_scale, y_scale, flip_x, flip_y, back_tex);
    if (shader != NULL)
        shader->end(this);
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
    int a = blend_color.a;
    blend_color = Color(color);
    blend_color.a = a;
}

void FrameObject::set_layer(int index)
{
    frame->set_object_layer(this, index);
}

void FrameObject::draw() {}
void FrameObject::update(float dt) {}
void FrameObject::set_direction(int value)
{
    direction = value & 31;
}

int FrameObject::get_direction() 
{
    return direction;
}

CollisionBase * FrameObject::get_collision()
{
    return NULL;
}

bool FrameObject::mouse_over()
{
    int x, y;
    frame->get_mouse_pos(&x, &y);
    PointCollision col1(x, y);
    return collide(&col1, get_collision());
}

bool FrameObject::overlaps(FrameObject * other)
{
    return collide(other->get_collision(), get_collision());
}

bool FrameObject::overlaps_background()
{
    Background * back = frame->layers[layer_index]->back;
    back->update();
    return collide(get_collision(), back->get_collision());
}

bool FrameObject::outside_playfield()
{
    int box[4];
    get_collision()->get_box(box);
    return !collides(box[0], box[1], box[2], box[3],
        0, 0, frame->width, frame->height);
}

int FrameObject::get_box_index(int index)
{
    int box[4];
    get_collision()->get_box(box);
    return box[index];
}

void FrameObject::get_box(int box[4])
{
    get_collision()->get_box(box);
}

void FrameObject::set_shader(Shader * value)
{
    if (shader_parameters == NULL)
        shader_parameters = new ShaderParameters;
    shader = value;
}

void FrameObject::set_shader_parameter(const std::string & name, double value)
{
    if (shader_parameters == NULL)
        shader_parameters = new ShaderParameters;
    (*shader_parameters)[name] = value;
}

void FrameObject::set_shader_parameter(const std::string & name,
                                       const Color & color)
{
    set_shader_parameter(name, (double)color.get_int());
}

double FrameObject::get_shader_parameter(const std::string & name)
{
    if (shader_parameters == NULL)
        return 0.0;
    return (*shader_parameters)[name];
}

void FrameObject::destroy()
{
    frame->destroy_object(this);
}

void FrameObject::set_level(int index)
{
    frame->layers[layer_index]->set_level(this, index);
}

int FrameObject::get_level()
{
    return frame->layers[layer_index]->get_level(this);
}

void FrameObject::move_back()
{
    set_level(0);
}

void FrameObject::move_front()
{
    set_level(-1);
}

void FrameObject::move_front(FrameObject * other)
{
    set_level(other->get_level());
}

double FrameObject::get_fixed()
{
    FrameObject * v = this;
    return *((double*)&v);
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

    Direction()
    {
        
    }
};

typedef Direction* DirectionArray[32];

struct DirectionArrayStruct
{
    DirectionArray dirs;

    DirectionArrayStruct()
    {
        for (int i = 0; i < 32; i++) {
            dirs[i] = NULL;
        }
    }
};

typedef std::map<int, DirectionArrayStruct> AnimationMap;

inline int direction_diff(int dir1, int dir2)
{
    return (((dir1 - dir2 + 540) % 360) - 180);
}

inline Direction * find_nearest_direction(int dir, DirectionArray & dirs)
{
    int i = 0;
    Direction * check_dir;
    while (true) {
        i++;
        check_dir = dirs[(dir + i) & 31];
        if (check_dir != NULL)
            return check_dir;
        check_dir = dirs[(dir - i) & 31];
        if (check_dir != NULL)
            return check_dir;
    }
    std::cout << "Could not find direction (fatal error)" << std::endl;
    return NULL;
}

class Active : public FrameObject
{
public:
    AnimationMap * animations;

    int animation;
    int animation_direction, animation_frame;
    int forced_frame, forced_speed, forced_direction;
    unsigned int counter;
    double angle;
    SpriteCollision * collision;
    double x_scale, y_scale;
    int action_x, action_y;
    bool collision_box;
    bool stopped;

    Active(std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id), animation(),
      animation_frame(0), counter(0), angle(0.0), forced_frame(-1), 
      forced_speed(-1), forced_direction(-1), x_scale(1.0), y_scale(1.0),
      animation_direction(0), stopped(false)
    {
        create_alterables();
    }

    void initialize_active()
    {
        collision = new SpriteCollision(this, NULL);
        collision->is_box = collision_box;
        update_frame();
    }

    ~Active()
    {
        delete collision;
    }

    void initialize_animations()
    {
        AnimationMap::iterator it;
        for (it = animations->begin(); it != animations->end(); it++) {
            DirectionArray & current_dirs = it->second.dirs;
            DirectionArray check_dirs;
            memcpy(check_dirs, current_dirs, sizeof(DirectionArray));
            for (int i = 0; i < 32; i++) {
                if (current_dirs[i] == NULL)
                    current_dirs[i] = find_nearest_direction(i, check_dirs);
            }
        }
    }

    void force_animation(int value)
    {
        if (value == animation)
            return;
        if (animations->find(value) == animations->end())
            std::cout << "Invalid animation: " << value 
                << " (" << name << ")" << std::endl;
        animation = value;
        if (forced_frame == -1)
            animation_frame = 0;
        update_frame();
    }

    void force_frame(int value)
    {
        forced_frame = value;
        update_frame();
    }

    void force_speed(int value)
    {
        forced_speed = value;
    }

    void force_direction(int value)
    {
        forced_direction = value & 31;
        update_frame();
    }

    void restore_frame()
    {
        animation_frame = forced_frame;
        forced_frame = -1;
        update_frame();
    }

    void add_direction(int animation, int direction,
                       int min_speed, int max_speed, bool repeat,
                       int back_to)
    {
        (*animations)[animation].dirs[direction] = new Direction(direction, 
            min_speed, max_speed, repeat, back_to);
    }

    void add_image(int animation, int direction, Image * image)
    {
        (*animations)[animation].dirs[direction]->frames.push_back(image);
    }

    void update_frame()
    {
        int frame_count = get_direction_data()->frames.size();
        int & current_frame = get_frame();
        current_frame = int_max(0, int_min(current_frame, frame_count - 1));

        Image * img = get_image();
        if (img == NULL)
            return;

        img->load();
        collision->set_image(img);
        update_action_point();
    }

    void update_action_point()
    {
        Image * img = get_image();
        collision->get_transform(img->action_x, img->action_y, 
                                 action_x, action_y);
        action_x -= collision->hotspot_x;
        action_y -= collision->hotspot_y;
    }

    void update(float dt)
    {
        if (forced_frame != -1 || stopped)
            return;
        Direction * dir = get_direction_data();
        counter += get_speed();
        int old_frame = animation_frame;
        while (counter > 100) {
            animation_frame++;
            if (animation_frame >= (int)dir->frames.size()) {
                if (!dir->repeat)
                    animation_frame--;
                else
                    animation_frame = dir->back_to;
            }
            counter -= 100;
        }
        if (animation_frame != old_frame)
            update_frame();
    }

    void draw()
    {
        Image * img = get_image();
        if (img == NULL) {
            std::cout << "Invalid image draw (" << name << ")" << std::endl;
            return;
        }
        blend_color.apply();
        draw_image(img, x, y, angle, x_scale, y_scale, false, false);
    }

    inline Image * get_image()
    {
        AnimationMap::const_iterator it = animations->find(animation);
        if (it == animations->end()) {
            std::cout << "Invalid animation: " << animation << std::endl;
            return NULL;
        }
        const DirectionArray & dirs = it->second.dirs;
        const Direction * dir = dirs[get_animation_direction()];
        if (get_frame() >= (int)dir->frames.size()) {
            std::cout << "Invalid frame: " << get_frame() << " " <<
                dir->frames.size() << " " <<
                "(" << name << ")" << std::endl;
            return NULL;
        }
        return dir->frames[get_frame()];
    }

    int get_action_x()
    {
        return x + action_x;
    }

    int get_action_y()
    {
        return y + action_y;
    }

    void set_angle(double angle, int quality)
    {
        this->angle = angle;
        collision->set_angle(angle);
        update_action_point();
    }

    double get_angle()
    {
        return angle;
    }

    int & get_frame()
    {
        if (forced_frame != -1)
            return forced_frame;
        return animation_frame;
    }

    int get_speed()
    {
        if (forced_speed != -1)
            return forced_speed;
        return get_direction_data()->max_speed;
    }

    Direction * get_direction_data(int & dir)
    {
        AnimationMap & ref = *animations;
        DirectionArray & array = ref[get_animation()].dirs;
        return array[dir];
    }

    Direction * get_direction_data()
    {
        return get_direction_data(get_animation_direction());
    }

    int get_animation()
    {
        return animation;
    }

    CollisionBase * get_collision()
    {
        return collision;
    }

    void set_direction(int value)
    {
        FrameObject::set_direction(value);
        animation_direction = direction;
        update_frame();
    }

    int & get_animation_direction()
    {
        if (forced_direction != -1)
            return forced_direction;
        return animation_direction;
    }

    void set_scale(double scale)
    {
        x_scale = y_scale = scale;
        collision->set_scale(scale);
        update_action_point();
    }

    void set_x_scale(double value)
    {
        x_scale = value;
        collision->set_x_scale(value);
        update_action_point();
    }

    void set_y_scale(double value)
    {
        y_scale = value;
        collision->set_y_scale(value);
        update_action_point();
    }

    void paste(int collision_type)
    {
        Image * img = get_image();
        frame->layers[layer_index]->paste(img, 
            x-img->hotspot_x, y-img->hotspot_y, 0, 0, 
            img->width, img->height, collision_type);
    }

    bool test_direction(int value)
    {
        return get_direction() == value;
    }

    bool test_directions(int value)
    {
        int direction = get_direction();
        return ((value >> direction) & 1) != 0;
    }

    bool test_animation(int value)
    {
        return value == animation;
    }

    void stop_animation()
    {
        stopped = true;
    }

    void start_animation()
    {
        stopped = false;
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
        collision = new InstanceBox(this);
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

    void next_paragraph()
    {
        set_paragraph(current_paragraph + 1);
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

class QuickBackdrop : public FrameObject
{
public:
    Color color;

    QuickBackdrop(const Color & color, int width, int height, 
                  std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id), color(color)
    {
        this->width = width;
        this->height = height;
    }

    void draw()
    {
        glColor4ub(color.r, color.g, color.b, blend_color.a);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
        glEnd();
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
            image->load(); // need to know metrics beforehand
            image->draw(current_x + image->hotspot_x - image->width, 
                        y + image->hotspot_y - image->height);
            current_x -= image->width;
        }
    }
};

#include "ini.cpp"

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
    static std::map<std::string, SectionMap> global_data;
    std::string current_group;
    SectionMap data;
    std::vector<std::pair<std::string, std::string> > search_results;
    bool overwrite;
    bool auto_save;
    std::string filename;
    std::string global_key;

    INI(std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id), overwrite(false), auto_save(false)
    {
        create_alterables();
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
        SectionMap::const_iterator it = data.find(group);
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
        set_string(group, item, number_to_string(value));
    }

    void set_value(const std::string & item, int pad, double value)
    {
        set_value(current_group, item, pad, value);
    }

    void set_string(const std::string & group, const std::string & item, 
                    const std::string & value)
    {
        data[group][item] = value;
        if (auto_save)
            save_file();
    }

    void set_string(const std::string & item, const std::string & value)
    {
        set_string(current_group, item, value);
    }

    void load_file(const std::string & fn, bool read_only = false, 
                   bool merge = false, bool overwrite = false)
    {
        if (!merge)
            reset();
        filename = convert_path(fn);
        std::cout << "Loading " << filename << " (" << name << ")" << std::endl;
        int e = ini_parse_file(filename.c_str(), _parse_handler, this);
        if (e != 0) {
            std::cout << "INI load failed (" << filename << ") with code " << e
            << std::endl;
        }
    }

    void load_string(const std::string & data, bool merge)
    {
        if (!merge)
            reset();
        int e = ini_parse_string(data, _parse_handler, this);
        if (e != 0) {
            std::cout << "INI load failed with code " << e << std::endl;
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

    void save_file(const std::string & fn)
    {
        filename = convert_path(fn);
        std::stringstream out;
        get_data(out);
        std::ofstream fp;
        fp.open(filename.c_str());
        fp << out.rdbuf();
        fp.close();
    }

    void save_file()
    {
        save_file(filename);
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

    bool has_group(const std::string & group)
    {
        SectionMap::const_iterator it = data.find(group);
        if (it == data.end())
            return false;
        return true;
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
                    std::pair<std::string, std::string>(
                        (*it1).first,
                        (*it2).first
                    ));
            }
        }
    }

    void delete_pattern(const std::string & group, const std::string & item, 
                        const std::string & value)
    {
        SectionMap::iterator it1;
        OptionMap::iterator it2;
        for (it1 = data.begin(); it1 != data.end(); it1++) {
            if (!match_wildcard(group, (*it1).first))
                continue;
            OptionMap & option_map = (*it1).second;
            it2 = option_map.begin();
            while (it2 != option_map.end()) {
                if (!match_wildcard(item, (*it2).first) || 
                    !match_wildcard(value, (*it2).second)) {
                    it2++;
                    continue;
                }
                option_map.erase(it2++);
            }
        }
        if (auto_save)
            save_file();
    }

    void reset()
    {
        data.clear();
        if (auto_save)
            save_file();
    }

    void delete_group(const std::string & group)
    {
        data.erase(group);
        if (auto_save)
            save_file();
    }

    void delete_group()
    {
        delete_group(current_group);
    }

    void delete_item(const std::string & group, const std::string & item)
    {
        data[group].erase(item);
        if (auto_save)
            save_file();
    }

    void delete_item(const std::string & item)
    {
        delete_item(current_group, item);
    }

    void set_global_data(const std::string & key)
    {
        data = global_data[key];
        global_key = key;
    }

    void merge_object(INI * other, bool overwrite)
    {
        merge_map(other->data, overwrite);
    }

    void merge_group(INI * other, const std::string & src_group, 
                     const std::string & dst_group, bool overwrite)
    {
        merge_map(other->data, src_group, dst_group, overwrite);
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
        if (auto_save)
            save_file();
    }

    void merge_map(SectionMap & data2, const std::string & src_group, 
                   const std::string & dst_group, bool overwrite)
    {
        OptionMap & items = data2[src_group];
        OptionMap::const_iterator it;
        for (it = items.begin(); it != items.end(); it++) {
            if (!overwrite && has_item(dst_group, (*it).first))
                continue;
            data[dst_group][(*it).first] = (*it).second;
        }
        if (auto_save)
            save_file();
    }

    size_t get_search_count()
    {
        return search_results.size();
    }

    std::string get_search_result_group(int index)
    {
        return search_results[index].first;
    }

    std::string get_item_part(const std::string & group, 
                              const std::string & item, int index,
                              const std::string & def)
    {
        if (index < 0)
            return def;
        std::string value = get_string(group, item, "");
        std::vector<std::string> elem;
        split_string(value, ',', elem);
        if (index >= (int)elem.size())
            return def;
        return elem[index];

    }

    ~INI()
    {
        if (global_key.empty())
            return;
        global_data[global_key] = data;
    }
};

std::map<std::string, SectionMap> INI::global_data;

class File
{
public:
    static std::string get_appdata_directory()
    {
        return get_app_path();
    }

    static bool file_exists(const std::string & path)
    {
        std::ifstream ifile(convert_path(path).c_str());
        if (ifile) {
            return true;
        } else {
            return false;
        }
    }

    static void delete_file(const std::string & path)
    {
        if (remove(convert_path(path).c_str()) != 0)
            std::cout << "Could not remove " << path << std::endl;
    }
};

class WindowControl
{
public:
    static bool has_focus()
    {
        return glfwGetWindowParam(GLFW_ACTIVE) == GL_TRUE;
    }

    static void set_focus(bool value)
    {
        if (value)
            glfwRestoreWindow();
        else
            glfwIconifyWindow();
    }
};

class Workspace
{
public:
    std::string name;
    std::stringstream data;

    Workspace(DataStream & stream)
    {
        stream >> name;
        unsigned int len;
        stream >> len;
        stream.read(data, len);
    }

    Workspace(const std::string & name)
    : name(name)
    {
    }
};

typedef std::map<std::string, Workspace*> WorkspaceMap;

class BinaryArray : public FrameObject
{
public:
    WorkspaceMap workspaces;
    Workspace * current;
    DataStream stream;

    BinaryArray(std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id)
    {

    }

    void load_workspaces(const std::string & filename)
    {
        std::ifstream fp(convert_path(filename).c_str(), std::ios::binary);
        stream.set_stream(fp);
        Workspace * workspace;
        while (!stream.at_end()) {
            workspace = new Workspace(stream);
            workspaces[workspace->name] = workspace;
        }
        fp.close();
        switch_workspace(current);
    }

    void create_workspace(const std::string & name)
    {
        if (workspaces.find(name) != workspaces.end())
            return;
        Workspace * workspace = new Workspace(name);
        workspaces[name] = workspace;
    }

    void switch_workspace(const std::string & name)
    {
        WorkspaceMap::const_iterator it = workspaces.find(name);
        if (it == workspaces.end())
            return;
        switch_workspace((*it).second);
    }

    void switch_workspace(Workspace * workspace)
    {
        current = workspace;
        stream.set_stream(current->data);
    }

    bool has_workspace(const std::string & name)
    {
        return workspaces.count(name) > 0;
    }

    void load_file(const std::string & filename)
    {
        std::ifstream fp(convert_path(filename).c_str(), std::ios::binary);
        current->data << fp.rdbuf();
        fp.close();
    }

    std::string read_string(int pos, size_t size)
    {
        stream.seekg(pos);
        std::string v;
        stream.read(v, size);
        return v;
    }

    size_t get_size()
    {
        std::stringstream & oss = current->data;
        std::stringstream::pos_type current = oss.tellg();
        oss.seekg(0, std::ios::end);
        std::stringstream::pos_type offset = oss.tellg();
        oss.seekg(current);
        return (size_t)offset;
    }
};

class ArrayObject : public FrameObject
{
public:
    int * array;
    int x_size, y_size, z_size;

    ArrayObject(std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id), array(NULL)
    {

    }

    void initialize(int x, int y, int z)
    {
        x_size = x;
        y_size = y;
        z_size = z;
        clear();
    }

    void clear()
    {
        if (array != NULL)
            delete array;
        array = new int[x_size * y_size * z_size]();
    }

    int & get_value(int x, int y)
    {
        return array[x + y * x_size];
    }

    void set_value(int value, int x, int y)
    {
        get_value(x, y) = value;
    }
};

class LayerObject : public FrameObject
{
public:
    int current_layer;
    static int sort_index;
    static bool sort_reverse;
    static double def;

    LayerObject(std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id), current_layer(0)
    {

    }

    void set_layer(int value)
    {
        current_layer = value;
    }

    void hide_layer(int index)
    {
        frame->layers[index]->visible = false;
    }

    void show_layer(int index)
    {
        frame->layers[index]->visible = true;
    }

    static inline double get_alterable(FrameObject * instance)
    {
        if (instance->values == NULL)
            return def;
        return instance->values->get(sort_index);
    } 

    static bool sort_func(FrameObject * a, FrameObject * b)
    {
        double value1 = get_alterable(a);
        double value2 = get_alterable(b);
        if (sort_reverse)
            return value1 < value2;
        else
            return value1 > value2;
    }

    void sort_alt_decreasing(int index, double def)
    {
        sort_index = index;
        sort_reverse = true;
        this->def = def;
        ObjectList & instances = frame->layers[current_layer]->instances;
        std::sort(instances.begin(), instances.end(), sort_func);
    }
};

int LayerObject::sort_index;
bool LayerObject::sort_reverse;
double LayerObject::def;

typedef std::map<std::string, Image*> ImageCache;

class ActivePicture : public FrameObject
{
public:
    Image * image;
    bool horizontal_flip;
    std::string filename;
    Color transparent_color;
    bool has_transparent_color;
    double scale_x, scale_y;
    double angle;
    SpriteCollision * collision;
    static ImageCache image_cache;

    ActivePicture(std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id), image(NULL), horizontal_flip(false),
      scale_x(1.0), scale_y(1.0), angle(0.0), has_transparent_color(false)
    {
        create_alterables();
        collision = new SpriteCollision(this, NULL);
    }

    void load(const std::string & fn)
    {
        filename = convert_path(fn);
        ImageCache::const_iterator it = image_cache.find(filename);
        if (it == image_cache.end()) {
            Color * transparent = NULL;
            if (has_transparent_color)
                transparent = &transparent_color;
            image = new Image(filename, 0, 0, 0, 0, transparent);
            if (image->image == NULL) {
                delete image;
                image = NULL;
            } else {
                std::cout << "Cached new image: " << filename << " (" <<
                    name << ")" << std::endl;
                image_cache[filename] = image;
            }
        } else {
            image = it->second;
        }
        if (image != NULL)
            collision->set_image(image);
    }

    void set_transparent_color(const Color & color)
    {
        transparent_color = color;
        has_transparent_color = true;
    }

    void set_hotspot(int x, int y)
    {
        if (image == NULL)
            return;
        this->x += x - image->hotspot_x;
        this->y += y - image->hotspot_y;
        image->hotspot_x = x;
        image->hotspot_y = y;
    }

    void set_hotspot_mul(float x, float y)
    {
        if (image == NULL)
            return;
        set_hotspot(image->width * x, image->height * y);
    }

    void flip_horizontal()
    {
        horizontal_flip = !horizontal_flip;
    }

    void set_scale(double value)
    {
        collision->set_scale(value);
        scale_x = scale_y = value;
    }

    void set_zoom(double value)
    {
        set_scale(value / 100.0);
    }

    void set_angle(double value)
    {
        collision->set_angle(value);
        angle = value;
    }

    double get_zoom_x()
    {
        return scale_x * 100.0;
    }

    int get_width()
    {
        if (image == NULL)
            return 0;
        return image->width;
    }

    int get_height()
    {
        if (image == NULL)
            return 0;
        return image->height;
    }

    void draw()
    {
        if (image == NULL)
            return;
        blend_color.apply();
        draw_image(image, x, y, angle, scale_x, scale_y, horizontal_flip);
    }

    CollisionBase * get_collision()
    {
        return collision;
    }

    void paste(int dest_x, int dest_y, int src_x, int src_y, 
               int src_width, int src_height, int collision_type)
    {
        if (image == NULL) {
            std::cout << "Invalid image paste: " << filename << std::endl;
            return;
        }
        frame->layers[layer_index]->paste(image, dest_x, dest_y, 
            src_x, src_y, src_width, src_height, collision_type);
    }
};

ImageCache ActivePicture::image_cache;

#define CHOWDREN_PYTHON
#ifdef CHOWDREN_PYTHON
#include "python.h"
#endif

// event helpers

struct MathHelper
{
    double lhs;
};

MathHelper & operator*(double lhs, MathHelper& rhs)
{
    rhs.lhs = lhs;
    return rhs;
}

double operator*(const MathHelper& lhs, double rhs)
{
    return pow(lhs.lhs, rhs);
}

MathHelper & operator%(double lhs, MathHelper& rhs)
{
    rhs.lhs = lhs;
    return rhs;
}

double operator%(const MathHelper& lhs, double rhs)
{
    return fmod(lhs.lhs, rhs);
}

static MathHelper math_helper;

inline FrameObject * get_object_from_fixed(double fixed)
{
    return *((FrameObject**)&fixed);
}

inline ObjectList make_single_list(FrameObject * item)
{
    ObjectList new_list;
    new_list.push_back(item);
    return new_list;
}

bool check_overlap(ObjectList in_a, ObjectList in_b, 
                   ObjectList & out_a, ObjectList & out_b)
{
    out_a.clear();
    out_b.clear();
    ObjectList::const_iterator item1, item2;
    bool ret = false;
    for (item1 = in_a.begin(); item1 != in_a.end(); item1++) {
        bool added = false;
        for (item2 = in_b.begin(); item2 != in_b.end(); item2++) {
            FrameObject * f1 = (*item1);
            FrameObject * f2 = (*item2);
            if (!f1->overlaps(f2))
                continue;
            ret = true;
            if (!added) {
                added = true;
                out_a.push_back(f1);
            }
            out_b.push_back(f2);
        }
    }
    return ret;
}

bool check_not_overlap(ObjectList in_a, ObjectList in_b)
{
    ObjectList::const_iterator item1, item2;
    for (item1 = in_a.begin(); item1 != in_a.end(); item1++) {
        for (item2 = in_b.begin(); item2 != in_b.end(); item2++) {
            FrameObject * f1 = (*item1);
            FrameObject * f2 = (*item2);
            if (f1->overlaps(f2))
                return false;
        }
    }
    return true;
}

void pick_random(ObjectList & instances)
{
    FrameObject * instance = instances[randrange(instances.size())];
    instances = make_single_list(instance);
}

void open_process(std::string exe, std::string cmd, int pad)
{

};

void set_cursor_visible(bool value)
{
    return; // debug
    if (value)
        glfwEnable(GLFW_MOUSE_CURSOR);
    else
        glfwDisable(GLFW_MOUSE_CURSOR);
}

std::string get_platform()
{
#ifdef _WIN32
    return "Chowdren Windows";
#elif __APPLE__
    return "Chowdren OS X";
#elif __linux
    return "Chowdren Linux";
#endif
}

static ObjectList::iterator item;

#endif /* COMMON_H */