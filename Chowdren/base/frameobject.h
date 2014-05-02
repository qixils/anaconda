#ifndef CHOWDREN_FRAMEOBJECT_H
#define CHOWDREN_FRAMEOBJECT_H

#include "chowconfig.h"
#include "alterables.h"
#include "color.h"
#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include <algorithm>
#include <stdarg.h>
#undef max
#include "broadphase.h"

class InstanceCollision;
class Frame;
class Shader;
class Image;

typedef boost::unordered_map<std::string, double> ShaderParameters;

class FrameObject;
class Movement;
class Layer;

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
    int index;
    int depth; // only for background instances
    int x, y;
    int width, height;
    int direction;
    int id;
    AlterableValues * values;
    AlterableStrings * strings;
    Color blend_color;
    bool visible;
    Frame * frame;
    Layer * layer;
    Shader * shader;
    ShaderParameters * shader_parameters;
    bool destroying;
    bool scroll;
    int movement_count;
    Movement ** movements;
    Movement * movement;
    InstanceCollision * collision;
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

typedef std::vector<FrameObject*> FlatObjectList;

#define LAST_SELECTED 0

struct ObjectListItem
{
    FrameObject * obj;
    unsigned int next;
};

typedef std::vector<ObjectListItem> ObjectListItems;

/*
Layout of ObjectList:

ObjectListItems is a list of currently living instances.
This array also includes information about the currently selected instances.

Its layout is as such:

* First item, {NULL, start_of_list}
* First living instance
* Second living instance
* etc.

When the current selection is cleared, the first item points to the end of
the array, so the most recently added instance is always iterated first.
The next instance will be set to current_index-1, etc., until the first item
is met. The first item is then always the last item pointed to by another item.
*/

class ObjectList
{
public:
    ObjectListItems items;
    typedef ObjectListItems::iterator iterator;

    ObjectList()
    {
        items.resize(1);
        ObjectListItem & item = items[0];
        item.obj = NULL;
        item.next = LAST_SELECTED;
    }

    iterator begin()
    {
        iterator it = items.begin();
        it++;
        return it;
    }

    iterator end()
    {
        return items.end();
    }

    void add(FrameObject * obj)
    {
        int i = items.size();
        items.resize(i+1);
        ObjectListItem & item = items[i];
        item.obj = obj;
        item.next = items[0].next;
        items[0].next = i;
        obj->index = i;
    }

    ObjectList & clear_selection()
    {
        int size = items.size();
        items[0].next = size-1;
        for (int i = 1; i < size; i++)
            items[i].next = i-1;
        return *this;
    }

    int get_selection_size();

    void empty_selection()
    {
        items[0].next = LAST_SELECTED;
    }

    bool has_selection() const
    {
        return items[0].next != LAST_SELECTED;
    }

    FrameObject * get_wrapped_selection(int index);

    FrameObject * back_selection() const
    {
        if (!has_selection())
            return NULL;
        return items[items[0].next].obj;
    }

    FrameObject * back() const
    {
        return items.back().obj;
    }

    int size() const
    {
        return items.size()-1;
    }

    int total_size() const
    {
        return items.size();
    }

    bool empty() const
    {
        return items.size() == 1;
    }

    FrameObject* operator[](int index)
    {
        return items[index+1].obj;
    }

    void clear()
    {
        items.resize(1);
        items[0].next = LAST_SELECTED;
    }

    void copy(ObjectList & other)
    {
        items = other.items;
    }

    void remove(FrameObject * obj)
    {
        for (int i = obj->index+1; i < int(items.size()); i++) {
            items[i-1].obj = items[i].obj;
            items[i].obj->index = i-1;
        }

        items.resize(items.size()-1);
    }

    void select_single(FrameObject * obj)
    {
        items[0].next = obj->index;
        items[obj->index].next = LAST_SELECTED;
    }
};

class ObjectIterator
{
public:
    ObjectList & list;
    int index;
    int last;
    bool selected;

    ObjectIterator(ObjectList & list)
    : list(list), index(list.items[0].next), last(0), selected(true)
    {
    }

    FrameObject* operator*() const
    {
        return list.items[index].obj;
    }

    void operator++()
    {
        last = selected ? index : last;
        index = list.items[index].next;
        selected = true;
    }

    void operator++(int)
    {
        ++*this;
    }

    void deselect()
    {
        selected = false;
        list.items[last].next = list.items[index].next;
    }

    bool end() const
    {
        return index == LAST_SELECTED;
    }

    void select_single()
    {
        list.items[0].next = index;
        list.items[index].next = LAST_SELECTED;
    }
};

inline FrameObject * ObjectList::get_wrapped_selection(int index)
{
    if (!has_selection())
        return NULL;
    while (true) {
        for (ObjectIterator it(*this); !it.end(); it++) {
            if (index == 0)
                return *it;
            index--;
        }
    }
}

inline int ObjectList::get_selection_size()
{
    int size = 0;
    for (ObjectIterator it(*this); !it.end(); it++) {
        size++;
    }
    return size;
}

class QualifierList
{
public:
    int count;
    ObjectList ** items;
    ObjectList * last;

    QualifierList()
    : count(0), items(NULL), last(NULL)
    {
        // for copies
    }

    QualifierList(int n, ...)
    {
        count = n;
        items = new ObjectList*[count+1];
        va_list arguments;
        va_start(arguments, n);
        for (int i = 0; i < n; i++) {
            items[i] = va_arg(arguments, ObjectList*);
        }
        va_end(arguments);
        items[count] = NULL;
        last = items[count-1];
    }

    int size()
    {
        int size = 0;
        for (int i = 0; i < count; i++) {
            size += items[i]->size();
        }
        return size;
    }

    int get_selection_size()
    {
        int size = 0;
        for (int i = 0; i < count; i++) {
            size += items[i]->get_selection_size();
        }
        return size;
    }

    QualifierList & clear_selection()
    {
        for (int i = 0; i < count; i++) {
            items[i]->clear_selection();
        }
        return *this;
    }

    bool has_selection()
    {
        for (int i = 0; i < count; i++) {
            if (items[i]->has_selection())
                return true;
        }
        return false;
    }

    bool empty()
    {
        for (int i = 0; i < count; i++) {
            if (!items[i]->empty())
                return false;
        }
        return true;
    }

    FrameObject * back()
    {
        for (int i = 0; i < count; i++) {
            if (!items[i]->empty())
                return items[i]->back();
        }
        return NULL;
    }

    FrameObject * back_selection() const
    {
        for (int i = 0; i < count; i++) {
            if (items[i]->has_selection())
                return items[i]->back_selection();
        }
        return NULL;
    }

    FrameObject* operator[](int index)
    {
        for (int i = 0; i < count; i++) {
            ObjectList & list = *items[i];
            int size = list.size();
            if (index < size)
                return list[index];
            index -= size;
        }
        return NULL;
    }

    void select_single(FrameObject * obj)
    {
        int i;
        for (i = 0; i < count; i++) {
            ObjectList & list = *items[i];
            if (list.empty()) {
                list.empty_selection();
                continue;
            }
            if (list[0]->id != obj->id) {
                list.empty_selection();
                continue;
            }
            list.select_single(obj);
            break;
        }

        while (i < count) {
            items[i]->empty_selection();
            i++;
        }
    }

    void copy(QualifierList & list)
    {
        if (items == NULL) {
            // XXX this currently leaks, but we always declare copied lists
            // as static variables, so we should never have to free anything
            // anyway
            count = list.count;
            items = new ObjectList*[count+1];
            for (int i = 0; i < count; i++) {
                items[i] = new ObjectList;
            }
            items[count] = NULL;
            last = items[count-1];
        }

        for (int i = 0; i < count; i++) {
            items[i]->copy(*list.items[i]);
        }
    }
};

class QualifierIterator
{
public:
    ObjectList ** lists;
    ObjectList * list;
    int list_index;
    int index;
    int last;
    bool selected;

    QualifierIterator(QualifierList & in_list)
    : lists(in_list.items), last(0), selected(true), list_index(0)
    {
        next_list();
    }

    FrameObject* operator*() const
    {
        return list->items[index].obj;
    }

    void next_list()
    {
        while (true) {
            list = lists[list_index];
            if (list == NULL)
                break;
            index = list->items[0].next;
            if (index != LAST_SELECTED)
                break;
            list_index++;
        }
    }

    void operator++()
    {
        last = selected ? index : last;
        index = list->items[index].next;
        selected = true;
        if (index != LAST_SELECTED)
            return;
        list_index++;
        last = 0;
        next_list();
    }

    void operator++(int)
    {
        ++*this;
    }

    void deselect()
    {
        selected = false;
        list->items[last].next = list->items[index].next;
    }

    bool end()
    {
        return list == NULL;
    }

    void select_single()
    {
        int n = 0;
        while (true) {
            ObjectList * iter = lists[n];
            if (iter == NULL)
                break;
            if (iter != list)
                iter->empty_selection();
            n++;
        }
        list->items[0].next = index;
        list->items[index].next = LAST_SELECTED;
    }
};

class SavedSelection
{
public:
    unsigned int start;
    std::vector<int> items;

    SavedSelection()
    {
    }

    void clear()
    {
        items.clear();
    }

    void add(ObjectList & list)
    {
        if (items.size() == 0) {
            items.resize(list.size(), 0);
            start = list.items[0].next;
        } else
            start = std::max(list.items[0].next, start);
        for (ObjectIterator it(list); !it.end(); it++) {
            items[it.index-1] = 1;
        }
    }

    void restore(ObjectList & list)
    {
        list.items[0].next = start;
        int last = start;
        for (int i = start-1; i >= 1; i--) {
            if (!items[i-1])
                continue;
            list.items[last].next = i;
            last = i;
        }
    }
};

class SavedQualifierSelection
{
public:
    std::vector<SavedSelection> items;

    SavedQualifierSelection()
    {
    }

    void clear()
    {
        std::vector<SavedSelection>::iterator it;
        for (it = items.begin(); it != items.end(); it++) {
            it->clear();
        }
    }

    void add(QualifierList & list)
    {
        if (items.size() == 0)
            items.resize(list.count);

        for (int i = 0; i < list.count; i++) {
            items[i].add(*list.items[i]);
        }
    }

    void restore(QualifierList & list)
    {
        for (int i = 0; i < list.count; i++) {
            items[i].restore(*list.items[i]);
        }
    }
};

#endif // CHOWDREN_FRAMEOBJECT_H
