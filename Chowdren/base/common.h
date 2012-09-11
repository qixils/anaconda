#ifndef COMMON_H
#define COMMON_H

#include "SOIL.h"
#include <string>
#include <list>
#include <vector>
#include <map>
#include <GL/glfw.h>
#include <stdlib.h>

class Color
{
public:
    float r, g, b, a;

    Color(int r, int g, int b, int a = 255)
    {
        set(r, g, b, a);
    }

    Color(float r, float g, float b, float a = 1.0)
    {
        set(r, g, b, a);
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
};

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

void load_texture(const char *filename, int force_channels, 
                  unsigned int reuse_texture_ID, unsigned int flags,
                  GLuint * tex, int * width, int * height)
{
    unsigned char* img;
    int channels;
    img = SOIL_load_image(filename, width, height, &channels, force_channels);

    if( (force_channels >= 1) && (force_channels <= 4) )
    {
        channels = force_channels;
    }

    if(img == NULL)
    {
        *tex = 0;
        return;
    }

    *tex = SOIL_create_OGL_texture(img, *width, *height, channels,
        reuse_texture_ID, flags);

    SOIL_free_image_data(img);

    return;
}

class Image
{
public:
    std::string filename;
    int hotspot_x, hotspot_y, action_x, action_y;
    GLuint tex;
    int width, height;

    Image(std::string name, int hot_x, int hot_y, int act_x, int act_y) 
    : hotspot_x(hot_x), hotspot_y(hot_y), action_x(act_x), 
    action_y(act_y), tex(0)
    {
        filename = "./../../images/" + name + ".png";
    }

    void load()
    {
        if (tex != NULL)
            return;
        load_texture(filename.c_str(), 4, 0, SOIL_FLAG_POWER_OF_TWO,
            &tex, &width, &height);
        if (tex == 0) {
            printf("Could not load %s\n", filename.c_str());
        }
    }

    void draw(double x, double y)
    {
        load();

        x -= (double)hotspot_x;
        y -= (double)hotspot_y;
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex2d(x, y);
        glTexCoord2f(1.0, 0.0);
        glVertex2d(x + width, y);
        glTexCoord2f(1.0, 1.0);
        glVertex2d(x + width, y + height);
        glTexCoord2f(0.0, 1.0);
        glVertex2d(x, y + height);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }
};


// object types

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

class FrameObject
{
public:
    std::string name;
    double x, y;
    int id;
    AlterableValues * values;
    AlterableStrings * strings;

    FrameObject(std::string name, int x, int y, int type_id) 
    : name(name), x(x), y(y), id(type_id)
    {
    }

    void set_position(double x, double y)
    {
        this->x = x;
        this->y = y;
    }

    void set_x(double x)
    {
        this->x = x;
    }

    void set_y(double y)
    {
        this->y = y;
    }

    void create_alterables()
    {
        values = new AlterableValues;
        strings = new AlterableStrings;
    }

    virtual void draw() {}
    virtual void update(float dt) {}
};

enum AnimationIndex {
    STOPPED = 0,
    WALKING,
    RUNNING,
    APPEARING,
    DISAPPEARING,
    BOUNCING,
    SHOOTING,
    JUMPING,
    FALLING,
    CLIMBING,
    CROUCH,
    STAND,
    USER_DEFINED_1,
    USER_DEFINED_2,
    USER_DEFINED_3,
    USER_DEFINED_4
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
    std::map<AnimationIndex, std::map<int, Direction*>> animations;
    Image * image;

    AnimationIndex animation;
    int direction, frame;
    int counter;

    Active(std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id), animation(STOPPED), direction(0),
      frame(0), counter(0)
    {
        create_alterables();
    }

    void add_direction(AnimationIndex animation, int direction,
                       int min_speed, int max_speed, bool repeat,
                       int back_to)
    {
        animations[animation][direction] = new Direction(direction, min_speed,
            max_speed, repeat, back_to);
    }

    void add_image(AnimationIndex animation, int direction, Image * image)
    {
        animations[animation][direction]->frames.push_back(image);
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
    }

    void draw()
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        animations[animation][direction]->frames[frame]->draw(x, y);
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
    Image * image;

    Counter(Image * image, std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id), image(image)
    {
    }

    void draw()
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        image->draw(x, y);
    }
};

typedef std::vector<FrameObject*> ObjectList;

class Frame
{
public:
    std::string name;
    int width, height;
    int index;
    GameManager * manager;
    ObjectList instances;
    std::map<int, ObjectList> instance_classes;
    std::map<std::string, int> loop_indexes;
    Color background_color;

    Frame(std::string name, int width, int height, Color background_color,
          int index, GameManager * manager)
    : name(name), width(width), height(height), index(index), 
      background_color(background_color), manager(manager)
    {}

    virtual void on_start() {}
    virtual void on_end() {}
    virtual void handle_events() {}

    void update(float dt)
    {
        for (ObjectList::const_iterator iter = instances.begin(); 
             iter != instances.end(); iter++) {
            (*iter)->update(dt);
        }

        handle_events();
    }

    void draw() 
    {
        int window_width, window_height;
        glfwGetWindowSize(&window_width, &window_height);
        glViewport(0, 0, window_width, window_height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glClearColor(background_color.r, background_color.g, background_color.b,
                     background_color.a);
        glClear(GL_COLOR_BUFFER_BIT);

        for (ObjectList::const_iterator iter = instances.begin(); 
             iter != instances.end(); iter++) {
            (*iter)->draw();
        }

    }

    ObjectList & get_instances(int object_id)
    {
        return instance_classes[object_id];
    }

    void add_object(FrameObject * object)
    {
        instances.push_back(object);
        instance_classes[object->id].push_back(object);
    }

    ObjectList create_object(FrameObject * object)
    {
        add_object(object);
        ObjectList new_list;
        new_list.push_back(object);
        return new_list;
    }
};

static ObjectList::iterator item;

int randrange(int range)
{
    return (rand() * range) / RAND_MAX;
}

#endif /* COMMON_H */