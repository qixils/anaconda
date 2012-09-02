#ifndef COMMON_H
#define COMMON_H

#include "SOIL.h"
#include <string>
#include <list>
#include <vector>
#include <map>

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

class Image
{
public:
    char * name;
    int hotspot_x, hotspot_y, action_x, action_y;

    Image(char * name, int hot_x, int hot_y, int act_x, int act_y) 
    : name(name), hotspot_x(hot_x), hotspot_y(hot_y), action_x(act_x), 
    action_y(act_y) 
    {

    }
    
/*    def load_icons(self, size):
        self.load_pixmap()
        icons = []
        for i in xrange(self.pixmap.width() / size):
            pixmap = self.pixmap.copy(i * size, 0, size, size)
            icon = QIcon(pixmap)
            icons.append(icon)
        return icons
    
    def load_pixmap(self):
        if self.pixmap is None:
            self.pixmap = QPixmap(os.path.join('images', self.name + '.png'))
        return self.pixmap
    
    def load(self, hotspot_x = None, hotspot_y = None):
        self.load_pixmap()
        if hotspot_x is None:
            hotspot_x = self.hotspot_x
        if hotspot_y is None:
            hotspot_y = self.hotspot_y
        return Sprite(self, self.pixmap, hotspot_x, hotspot_y)*/

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
};

class Active : public FrameObject
{
public:
    Active(std::string name, int x, int y, int type_id) 
    : FrameObject(name, x, y, type_id) 
    {
        create_alterables();
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
    std::vector<FrameObject*> instances;
    std::map<int, ObjectList> instance_classes;
    std::map<std::string, int> loop_indexes;

    Frame(std::string name, int width, int height, int index, 
          GameManager * manager)
    : name(name), width(width), height(height), index(index), manager(manager)
    {}

    virtual void on_start() {}
    virtual void on_end() {}
    virtual void handle_events() {}
    virtual void draw() {}

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
    return (int)(range * 0.5);
}

#endif /* COMMON_H */