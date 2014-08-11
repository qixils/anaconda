#ifndef CHOWDREN_FRAMEOBJECT_H
#define CHOWDREN_FRAMEOBJECT_H

#include "chowconfig.h"
#include "alterables.h"
#include "color.h"
#include <string>
#include "types.h"
#include <algorithm>
#include <stdarg.h>
#undef max
#include "broadphase.h"
#include <assert.h>

class InstanceCollision;
class Frame;
class Shader;
class Image;

typedef hash_map<std::string, double> ShaderParameters;

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

#define BACKGROUND_TYPE 1

enum ObjectFlags
{
    VISIBLE = (1 << 0),
    DESTROYING = (1 << 1),
    SCROLL = (1 << 2),
    FADEOUT = (1 << 3),
    BACKGROUND = (1 << 4)
};

#ifdef CHOWDREN_USE_VALUEADD

int hash_extra_key(const std::string & value);

class ExtraAlterables
{
public:
    hash_map<int, double> values;
    hash_map<int, std::string> strings;

    ExtraAlterables()
    {
    }

    double get_value(int key)
    {
        return values[key];
    }

    double get_value(const std::string & key)
    {
        return get_value(hash_extra_key(key));
    }

    const std::string & get_string(int key)
    {
        return strings[key];
    }

    const std::string & get_string(const std::string & key)
    {
        return get_string(hash_extra_key(key));
    }

    void set_value(int key, double value)
    {
        values[key] = value;
    }

    void set_string(int key, const std::string & value)
    {
        strings[key] = value;
    }
};

#endif

class FrameObject
{
public:
#ifndef NDEBUG
    std::string name;
#endif
    int index;
    int depth;
    int x, y;
    int width, height;
    int direction;
    int id;
    int flags;
    Alterables * alterables;
    Color blend_color;
    Frame * frame;
    Layer * layer;
    Shader * shader;
    ShaderParameters * shader_parameters;
    int movement_count;
    Movement ** movements;
    Movement * movement;
    InstanceCollision * collision;
#ifdef CHOWDREN_USE_BOX2D
    int body;
#endif

#ifdef CHOWDREN_USE_VALUEADD
    ExtraAlterables * extra_alterables;
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
    virtual float get_angle();
    virtual void set_angle(float angle, int quality = 0);
    void create_alterables();
    void set_visible(bool value);
    void set_blend_color(int color);
    virtual void draw();
    void draw_image(Image * img, int x, int y, double angle = 0.0,
                    double scale_x = 1.0, double scale_y = 1.0,
                    bool flip_x = false, bool flip_y = false);
    void begin_draw(int width, int height);
    void begin_draw();
    void end_draw();
    virtual void set_direction(int value, bool set_movement = true);
    virtual int get_direction();
    bool mouse_over();
    bool overlaps(FrameObject * other);
    void set_layer(int layer);
    void set_shader(Shader * shader);
    void set_shader_parameter(const std::string & name, double value);
    void set_shader_parameter(const std::string & name, Image & image);
    void set_shader_parameter(const std::string & name, const Color & color);
    double get_shader_parameter(const std::string & name);
    int get_level();
    void set_level(int index);
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
    void rotate_toward(int dir);
    void update_flash(float dt, float interval, float & time);
    bool test_direction(int value);
    bool test_directions(int value);
    virtual void flash(float value);
    virtual void set_animation(int value);
    virtual void set_backdrop_offset(int dx, int dy);
    void get_screen_aabb(int box[4]);

#ifdef CHOWDREN_USE_VALUEADD
    ExtraAlterables & get_extra_alterables()
    {
        if (extra_alterables == NULL)
            extra_alterables = new ExtraAlterables;
        return *extra_alterables;
    }
#endif
};

typedef vector<FrameObject*> FlatObjectList;

#define LAST_SELECTED 0

struct ObjectListItem
{
    FrameObject * obj;
    unsigned int next;

    ObjectListItem()
    {
    }
};

typedef vector<ObjectListItem> ObjectListItems;

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
    unsigned int saved_start;
    vector<int> saved_items;

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
        obj->index = i;
    }

    void add_back()
    {
        int i = items.size() - 1;
        ObjectListItem & item = items[i];
        item.next = items[0].next;
        items[0].next = i;
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
        assert(items.size() >= 1);
    }

    void remove(FrameObject * obj)
    {
        for (int i = obj->index+1; i < int(items.size()); i++) {
            items[i-1].obj = items[i].obj;
            items[i].obj->index = i-1;
        }

        items.resize(items.size()-1);
        assert(items.size() >= 1);
    }

    void select_single(FrameObject * obj)
    {
        items[0].next = obj->index;
        items[obj->index].next = LAST_SELECTED;
    }

    void save_selection();
    void restore_selection();
    void clear_saved_selection();
};

class ObjectIterator
{
public:
    ObjectList & list;
    int index;
    int last;
    bool selected;

#ifdef CHOWDREN_ITER_INDEX
    int current_index;
#endif

    ObjectIterator(ObjectList & list)
    : list(list), index(list.items[0].next), last(0), selected(true)
    {
#ifdef CHOWDREN_ITER_INDEX
        current_index = 0;
#endif
    }

    FrameObject* operator*() const
    {
        return list.items[index].obj;
    }

    void operator++()
    {
#ifdef CHOWDREN_ITER_INDEX
        current_index++;
#endif

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

    void empty_selection()
    {
        for (int i = 0; i < count; i++) {
            items[i]->empty_selection();
        }
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

    FrameObject * operator[](int index)
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

    void clear_saved_selection()
    {
        for (int i = 0; i < count; i++) {
            items[i]->clear_saved_selection();
        }
    }

    void save_selection()
    {
        for (int i = 0; i < count; i++) {
            items[i]->save_selection();
        }
    }

    void restore_selection()
    {
        for (int i = 0; i < count; i++) {
            items[i]->restore_selection();
        }
    }

    FrameObject * get_wrapped_selection(int index);
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

#ifdef CHOWDREN_ITER_INDEX
    int current_index;
#endif

    QualifierIterator(QualifierList & in_list)
    : lists(in_list.items), last(0), selected(true), list_index(0)
    {
#ifdef CHOWDREN_ITER_INDEX
        current_index = 0;
#endif
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
#ifdef CHOWDREN_ITER_INDEX
        current_index++;
#endif
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

inline FrameObject * QualifierList::get_wrapped_selection(int index)
{
    if (!has_selection())
        return NULL;
    while (true) {
        for (QualifierIterator it(*this); !it.end(); it++) {
            if (index == 0)
                return *it;
            index--;
        }
    }
}

// saving selection

inline void ObjectList::clear_saved_selection()
{
    saved_items.clear();
}

inline void ObjectList::save_selection()
{
    if (saved_items.size() == 0) {
        saved_items.resize(items.size(), 0);
        saved_start = items[0].next;
    } else
        saved_start = std::max(items[0].next, saved_start);
    for (ObjectIterator it(*this); !it.end(); it++) {
        saved_items[it.index-1] = 1;
    }
}

inline void ObjectList::restore_selection()
{
    items[0].next = saved_start;
    int last = saved_start;
    for (int i = saved_start-1; i >= 1; i--) {
        if (!saved_items[i-1])
            continue;
        items[last].next = i;
        last = i;
    }
    items[last].next = LAST_SELECTED;
}

void setup_default_instance(FrameObject * obj);

#endif // CHOWDREN_FRAMEOBJECT_H
