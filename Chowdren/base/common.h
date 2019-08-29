#ifndef CHOWDREN_COMMON_H
#define CHOWDREN_COMMON_H

#include "chowconfig.h"
#include "include_gl.h"
#include "profiler.h"
#include "keydef.h"
#include "keyconv.h"
#include "manager.h"
#include "platform.h"
#include <string>
#include <list>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <algorithm>
#include <string.h>
#include "stringcommon.h"
#include "shader.h"
#include "datastream.h"
#include <ctype.h>
#include "globals.h"
#include "image.h"
#include "frameobject.h"
#include "collision.h"
#include "alterables.h"
#include "color.h"
#include "mathcommon.h"
#include "path.h"
#include "types.h"
#include "crossrand.h"
#include "utility.h"
#include "input.h"
#include "movement.h"
#include "intern.h"

extern std::string newline_character;
// string helpers

inline int string_find(const std::string & a, const std::string & b,
                       int pos)
{
    if (pos == -1)
        pos = 0;
    size_t ret = a.find(b, pos);
    if (ret == std::string::npos)
        return -1;
    return ret;
}

inline int string_rfind(const std::string & a, const std::string & b,
                        int pos)
{
    if (pos == -1)
        pos = 0;
    size_t ret = a.rfind(b, pos);
    if (ret == std::string::npos)
        return -1;
    return ret;
}

inline int string_size(const std::string & a)
{
    return a.size();
}

inline std::string lowercase_string(const std::string & str)
{
	std::string v(str);
    std::transform(v.begin(), v.end(), v.begin(),
                   static_cast<int (*)(int)>(tolower));
    return v;
}

inline std::string uppercase_string(const std::string & str)
{
	std::string v(str);
    std::transform(v.begin(), v.end(), v.begin(),
                   static_cast<int(*)(int)>(toupper));
    return v;
}

inline std::string right_string(const std::string & v, int count)
{
    count = clamp(count, 0, int(v.size()));
    int index = int(v.size()) - count;
    return v.substr(index, count);
}

inline std::string mid_string(const std::string & v, int index, int count)
{
    int size = int(v.size());
    index = clamp(index, 0, size);
    count = clamp(count, 0, size - index);
    return v.substr(index, count);
}

inline std::string left_string(const std::string & v, int count)
{
    count = clamp(count, 0, int(v.size()));
    return v.substr(0, count);
}

class Font
{
public:
    const char * face;
    int size;
    bool bold;
    bool italic;
    bool underline;

    Font(const char * face, int size, bool bold, bool italic, bool underline);
};

// static objects

class FTTextureFont;
FTTextureFont * get_font(int size);
void set_font_path(const char * path);
void set_font_path(const std::string & path);
bool init_font();

#define NONE_GRADIENT 0
#define VERTICAL_GRADIENT 1
#define HORIZONTAL_GRADIENT 2

void draw_gradient(int x1, int y1, int x2, int y2, int gradient_type,
                   Color & color, Color & color2, int alpha);

class File
{
public:
    static const std::string & get_appdata_directory();
    static void create_directory(const std::string & path);
    static bool file_exists(const std::string & path);
    static bool name_exists(const std::string & path);
    static bool directory_exists(const std::string & path);
    static void delete_file(const std::string & path);
    static bool file_readable(const std::string & path);
};

#include "extensions.h"

inline void reset_global_data()
{
#ifdef CHOWDREN_USE_INIPP
    INI::reset_global_data();
#endif
}

// event helpers

#include "mathhelper.h"

extern MathHelper math_helper;

inline bool double_equals_exact(double a, double b)
{
    return memcmp(&a, &b, sizeof(double)) == 0;
}

inline FrameObject * get_object_from_fixed(double fixed)
{
    // -1 as a double is
    // 00 00 00 00 00 00 F0 BF
    // which is quite unlikely to be a memory address
    if (double_equals_exact(fixed, 0.0) || double_equals_exact(fixed, -1.0))
        return NULL;
    FrameObject * p;
    memcpy(&p, &fixed, sizeof(FrameObject*));
    return p;
}

inline FrameObject * get_object_from_fixed(FixedValue fixed)
{
    return fixed.object;
}

inline void remove_list(FlatObjectList & a, FrameObject * obj)
{
    FlatObjectList::iterator it;
    for (it = a.begin(); it != a.end(); ++it) {
        if (*it != obj)
            continue;
        a.erase(it);
        break;
    }
}

inline FrameObject * get_single(const ObjectList & list)
{
    return list.back_selection();
}

inline FrameObject * get_single(ObjectList & list, int index)
{
    return list.get_wrapped_selection(index);
}

inline FrameObject * get_single(const ObjectList & list, FrameObject * def)
{
    FrameObject * ret = list.back_selection();
    if (ret == NULL)
        return def;
    return ret;
}

inline FrameObject * get_single(ObjectList & list, int index,
                                FrameObject * def)
{
    FrameObject * ret = list.get_wrapped_selection(index);
    if (ret == NULL)
        return def;
    return ret;
}

inline FrameObject * get_single(const QualifierList & list)
{
    return list.back_selection();
}

inline FrameObject * get_single(QualifierList & list, int index)
{
    return list.get_wrapped_selection(index);
}

inline FrameObject * get_single(const QualifierList & list, FrameObject * def)
{
    FrameObject * ret = list.back_selection();
    if (ret == NULL)
        return def;
    return ret;
}

inline FrameObject * get_single(QualifierList & list, int index,
                                FrameObject * def)
{
    FrameObject * ret = list.get_wrapped_selection(index);
    if (ret == NULL)
        return def;
    return ret;
}

extern vector<int> int_temp;

// FrameObject vs FrameObject

template <bool save>
inline bool check_overlap(FrameObject * obj1, FrameObject * obj2)
{
    if (!obj1->overlaps(obj2))
        return false;
    if (!save)
        return true;
    if (obj1->movement != NULL)
        obj1->movement->add_collision(obj2);
    if (obj2->movement != NULL)
        obj2->movement->add_collision(obj1);
    return true;
}

// ObjectList vs ObjectList

template <bool save>
inline bool check_overlap(ObjectList & list1, ObjectList & list2)
{
    int size = list2.size();
    if (size <= 0)
        return false;
    int_temp.resize(size);
    std::fill(int_temp.begin(), int_temp.end(), 0);

    bool ret = false;
    for (ObjectIterator it1(list1); !it1.end(); ++it1) {
        FrameObject * instance = *it1;
        InstanceCollision * col = instance->collision;
        if (col == NULL) {
            it1.deselect();
            continue;
        }
        bool added = false;
        for (ObjectIterator it2(list2); !it2.end(); ++it2) {
            FrameObject * other = *it2;
            if (other->collision == NULL) {
                it2.deselect();
                continue;
            }
            if (!check_overlap<save>(instance, other))
                continue;
            int_temp[it2.index-1] = 1;
            added = ret = true;
        }
        if (!added)
            it1.deselect();
    }

    if (!ret)
        return false;

    for (ObjectIterator it(list2); !it.end(); ++it) {
        if (!int_temp[it.index-1])
            it.deselect();
    }

    return true;
}

// FrameObject vs ObjectList

template <bool save>
inline bool check_overlap(FrameObject * obj, ObjectList & list)
{
    int size = list.size();
    if (size <= 0)
        return false;

    CollisionBase * col = obj->collision;
    if (col == NULL)
        return false;

    bool ret = false;
    for (ObjectIterator it(list); !it.end(); ++it) {
        FrameObject * other = *it;
        if (other->collision == NULL) {
            it.deselect();
            continue;
        }
        if (!check_overlap<save>(obj, other)) {
            it.deselect();
            continue;
        }
        ret = true;
    }

    return ret;
}

template <bool save>
inline bool check_overlap(ObjectList & list, FrameObject * obj)
{
    return check_overlap<save>(obj, list);
}

// QualifierList vs ObjectList

template <bool save>
inline bool check_overlap(QualifierList & list1, ObjectList & list2)
{
    int size = list1.size();
    if (size <= 0)
        return false;
    int_temp.resize(size);
    std::fill(int_temp.begin(), int_temp.end(), 0);

    bool ret = false;
    for (ObjectIterator it1(list2); !it1.end(); ++it1) {
        FrameObject * instance = *it1;
        if (instance->collision == NULL) {
            it1.deselect();
            continue;
        }
        bool added = false;
        int temp_offset = 0;
        for (int i = 0; i < list1.count; i++) {
            ObjectList & list = *list1.items[i];
            for (ObjectIterator it2(list); !it2.end(); ++it2) {
                FrameObject * other = *it2;
                if (other->collision == NULL) {
                    it2.deselect();
                    continue;
                }
                if (!check_overlap<save>(instance, other))
                    continue;
                added = ret = true;
                int_temp[temp_offset + it2.index - 1] = 1;
            }
            temp_offset += list.size();
        }
        if (!added)
            it1.deselect();
    }

    if (!ret)
        return false;

    int total_index = 0;
    for (int i = 0; i < list1.count; i++) {
        ObjectList & list = *list1.items[i];
        for (ObjectIterator it(list); !it.end(); ++it) {
            if (!int_temp[total_index + it.index - 1])
                it.deselect();
        }
        total_index += list.size();
    }

    return true;
}

template <bool save>
inline bool check_overlap(ObjectList & list1, QualifierList & list2)
{
    return check_overlap<save>(list2, list1);
}

// FrameObject vs QualifierList

template <bool save>
inline bool check_overlap(FrameObject * obj, QualifierList & list)
{
    int size = list.size();
    if (size <= 0)
        return false;

    if (obj->collision == NULL)
        return false;

    int_temp.resize(size);
    std::fill(int_temp.begin(), int_temp.end(), 0);

    bool ret = false;

    int temp_offset = 0;
    for (int i = 0; i < list.count; i++) {
        ObjectList & list2 = *list.items[i];
        for (ObjectIterator it(list2); !it.end(); ++it) {
            FrameObject * other = *it;
            if (other->collision == NULL) {
                it.deselect();
                continue;
            }
            if (!check_overlap<save>(obj, other)) {
                it.deselect();
                continue;
            }
            ret = true;
        }
    }

    return ret;
}

template <bool save>
inline bool check_overlap(QualifierList & list, FrameObject * instance)
{
    return check_overlap<save>(instance, list);
}

// QualifierList vs QualifierList

template <bool save>
inline bool check_overlap(QualifierList & list1, QualifierList & list2)
{
    int size = list1.size();
    if (size <= 0)
        return false;
    int_temp.resize(size);
    std::fill(int_temp.begin(), int_temp.end(), 0);

    bool ret = false;
    for (QualifierIterator it1(list2); !it1.end(); ++it1) {
        FrameObject * instance = *it1;
        if (instance->collision == NULL) {
            it1.deselect();
            continue;
        }
        bool added = false;

        int temp_offset = 0;
        for (int i = 0; i < list1.count; i++) {
            ObjectList & list = *list1.items[i];
            for (ObjectIterator it2(list); !it2.end(); ++it2) {
                FrameObject * other = *it2;
                if (other->collision == NULL) {
                    it2.deselect();
                    continue;
                }
                if (!check_overlap<save>(instance, other))
                    continue;
                added = ret = true;
                int_temp[temp_offset + it2.index - 1] = 1;
            }
            temp_offset += list.size();
        }
        if (!added)
            it1.deselect();
    }

    if (!ret)
        return false;

    int total_index = 0;
    for (int i = 0; i < list1.count; i++) {
        ObjectList & list = *list1.items[i];
        for (ObjectIterator it(list); !it.end(); ++it) {
            if (!int_temp[total_index + it.index - 1])
                it.deselect();
        }
        total_index += list.size();
    }

    return true;
}

// ObjectList vs ObjectList

inline bool check_not_overlap(ObjectList & list1, ObjectList & list2)
{
    for (ObjectIterator it1(list1); !it1.end(); ++it1) {
        FrameObject * instance = *it1;
        if (instance->collision == NULL)
            continue;
        ObjectList::iterator it2;
        for (it2 = list2.begin(); it2 != list2.end(); ++it2) {
            FrameObject * other = it2->obj;
            if (!instance->overlaps(other))
                continue;
            return false;
        }
    }
    return true;
}

// QualifierList vs ObjectList

inline bool check_not_overlap(QualifierList & list1, ObjectList & list2)
{
    for (int i = 0; i < list1.count; i++) {
        if (!check_not_overlap(*list1.items[i], list2))
            return false;
    }
    return true;
}

inline bool check_not_overlap(ObjectList & list1, QualifierList & list2)
{
    for (int i = 0; i < list2.count; i++) {
        if (!check_not_overlap(list1, *list2.items[i]))
            return false;
    }
    return true;
}

// QualifierList vs QualifierList

inline bool check_not_overlap(QualifierList & list1, QualifierList & list2)
{
    for (int i = 0; i < list1.count; i++) {
        for (int ii = 0; ii < list2.count; ii++) {
            if (!check_not_overlap(*list1.items[i], *list2.items[ii]))
                return false;
        }
    }
    return true;
}

// FrameObject vs ObjectList

inline bool check_not_overlap(FrameObject * obj, ObjectList & list)
{
    CollisionBase * col = obj->collision;
    if (col == NULL)
        return true;
    ObjectList::iterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        FrameObject * other = it->obj;
        if (!obj->overlaps(other))
            continue;
        return false;
    }
    return true;
}

// FrameObject vs QualifierList

inline bool check_not_overlap(FrameObject * obj, QualifierList & list)
{
    CollisionBase * col = obj->collision;
    if (col == NULL)
        return true;
    for (int i = 0; i < list.count; i++) {
        ObjectList & list2 = *list.items[i];

        ObjectList::iterator it;
        for (it = list2.begin(); it != list2.end(); ++it) {
            FrameObject * other = it->obj;
            if (!obj->overlaps(other))
                continue;
            return false;
        }
    }
    return true;
}

inline bool pick_random(ObjectList & instances)
{
    int size = 0;
    for (ObjectIterator it(instances); !it.end(); ++it) {
        if ((*it)->flags & (FADEOUT | DESTROYING)) {
            it.deselect();
            continue;
        }
        size++;
    }
    if (size == 0)
        return false;
    int index = randrange(size);
    for (ObjectIterator it(instances); !it.end(); ++it) {
        if (index == 0) {
            it.select_single();
            break;
        }
        index--;
    }
    return true;
}

inline bool pick_random(QualifierList & instances)
{
    int size = 0;
    for (QualifierIterator it(instances); !it.end(); ++it) {
        if ((*it)->flags & (FADEOUT | DESTROYING)) {
            it.deselect();
            continue;
        }
        size++;
    }
    if (size == 0)
        return false;
    int index = randrange(size);
    for (QualifierIterator it(instances); !it.end(); ++it) {
        if (index == 0) {
            it.select_single();
            break;
        }
        index--;
    }
    return true;
}

#ifdef CHOWDREN_USE_VALUEADD

inline void spread_value(QualifierList & instances, int key, int start,
                         int step)
{
    for (QualifierIterator it(instances); !it.end(); it++) {
        (*it)->get_extra_alterables().set_value(key, start);
        start += step;
    }
}

inline void spread_value(ObjectList & instances, int key, int start, int step)
{
    for (ObjectIterator it(instances); !it.end(); it++) {
        (*it)->get_extra_alterables().set_value(key, start);
        start += step;
    }
}

#endif

inline void spread_value(FrameObject * obj, int alt, int start)
{
    obj->alterables->values.set(alt, start);
}

inline void spread_value(ObjectList & instances, int alt, int start)
{
    for (ObjectIterator it(instances); !it.end(); ++it) {
        (*it)->alterables->values.set(alt, start);
        start++;
    }
}

inline void spread_value(QualifierList & instances, int alt, int start)
{
    for (QualifierIterator it(instances); !it.end(); ++it) {
        (*it)->alterables->values.set(alt, start);
        start++;
    }
}

inline int objects_in_zone(ObjectList & instances,
                           int x1, int y1, int x2, int y2)
{
    int count = 0;
    for (ObjectIterator it(instances); !it.end(); ++it) {
        FrameObject * obj = *it;
        if (obj->flags & (FADEOUT | DESTROYING))
            continue;
        int x = obj->get_x();
        int y = obj->get_y();
        if (x < x1 || x >= x2 || y < y1 || y >= y2)
            continue;
        count++;
    }
    return count;
}

inline int objects_in_zone(QualifierList & instances,
                           int x1, int y1, int x2, int y2)
{
    int count = 0;
    for (QualifierIterator it(instances); !it.end(); ++it) {
        FrameObject * obj = *it;
        if (obj->flags & (FADEOUT | DESTROYING))
            continue;
        int x = obj->get_x();
        int y = obj->get_y();
        if (x < x1 || x >= x2 || y < y1 || y >= y2)
            continue;
        count++;
    }
    return count;
}

inline void pick_objects_in_zone(ObjectList & instances,
                                 int x1, int y1, int x2, int y2)
{
    for (ObjectIterator it(instances); !it.end(); ++it) {
        FrameObject * obj = *it;
        if (obj->flags & (FADEOUT | DESTROYING)) {
            it.deselect();
            continue;
        }
        int x = obj->get_x();
        int y = obj->get_y();
        if (x < x1 || x >= x2 || y < y1 || y >= y2)
            it.deselect();
    }
}

inline void pick_objects_in_zone(QualifierList & instances,
                                 int x1, int y1, int x2, int y2)
{
    for (QualifierIterator it(instances); !it.end(); ++it) {
        FrameObject * obj = *it;
        if (obj->flags & (FADEOUT | DESTROYING)) {
            it.deselect();
            continue;
        }
        int x = obj->get_x();
        int y = obj->get_y();
        if (x < x1 || x >= x2 || y < y1 || y >= y2)
            it.deselect();
    }
}

inline void set_random_seed(int seed)
{
    cross_srand(seed);
}

inline void open_process(const std::string & exe, const std::string & cmd,
                         int pad)
{

}

inline void transform_pos(int & x, int & y, FrameObject * parent)
{
    double c, s;
    get_dir(parent->direction, c, s);
    int new_x = int(double(x) * c - double(y) * s);
    int new_y = int(double(x) * s + double(y) * c);
    x = new_x;
    y = new_y;
}

void swap_position(const FlatObjectList & value);

inline void set_cursor_visible(bool value)
{
    if (value)
        platform_show_mouse();
    else
        platform_hide_mouse();
}

inline std::string get_command_arg(const std::string & arg)
{
    // XXX implement, maybe
    return "";
}

std::string get_md5(const std::string & value);

inline float get_joystick_dummy(float value, int n)
{
    return value;
}

inline int get_ascii(const std::string & value)
{
    if (value.empty())
        return 0;
    return int((unsigned char)value[0]);
}

inline int reverse_color(int value)
{
    Color color(value);
    color.r = 255 - color.r;
    color.g = 255 - color.g;
    color.b = 255 - color.b;
    color.a = 0;
    return color.get_int();
}

inline std::string get_platform()
{
#ifdef _WIN32
    return "Chowdren Windows";
#elif __APPLE__
    return "Chowdren OS X";
#elif __linux
    return "Chowdren Linux";
#elif CHOWDREN_IS_WIIU
    return "Chowdren WiiU";
#else
    return "Chowdren ???";
#endif
}

#endif // CHOWDREN_COMMON_H
