#ifndef CHOWDREN_COMMON_H
#define CHOWDREN_COMMON_H

#include "keys.h"
#include "config.h"
#include "manager.h"
#include "platform.h"
#include "include_gl.h"
#include "font.h"
#include <string>
#include <list>
#include <vector>
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
#include <cctype>
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
#include <stdarg.h>
#include <boost/unordered_map.hpp>
#include "coltree.h"

extern std::string newline_character;

template <typename A, typename B, typename C>
inline typename A::iterator set_map_value(A & map, const B & key, 
                                          const C & value)
{

    std::pair<typename A::iterator, bool> res = map.insert(
        std::make_pair(key, value));
    typename A::iterator it = res.first;
    if (!res.second)
        it->second = value;
    return it;
}

inline int randrange(int range)
{
    if (range == 0)
        return 0;
    return cross_rand() / (CROSS_RAND_MAX / range + 1);
}

inline bool random_chance(int a, int b)
{
    return randrange(b) < a;
}

inline int pick_random(int count, ...)
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

inline void split_string(const std::string &s, char delim, 
                  std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

inline void split_string(const std::string & str, const std::string & delims,
                  std::vector<std::string> &elems)
{
    std::string::size_type last_pos = str.find_first_not_of(delims, 0);
    std::string::size_type pos = str.find_first_of(delims, last_pos);

    while (std::string::npos != pos || std::string::npos != last_pos) {
        elems.push_back(str.substr(last_pos, pos - last_pos));
        last_pos = str.find_first_not_of(delims, pos);
        pos = str.find_first_of(delims, last_pos);
    }
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

inline std::string uppercase_string(std::string v)
{
    std::transform(v.begin(), v.end(), v.begin(), toupper);
    return v;
}

inline std::string right_string(const std::string & v, size_t count)
{
    size_t index = v.size() - count;
    return v.substr(index, count);
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
public:
    char * face;
    int size;
    bool bold;
    bool italic;
    bool underline;

    Font(char * face, int size, bool bold, bool italic, bool underline);
};

#define BACK_WIDTH WINDOW_WIDTH
#define BACK_HEIGHT WINDOW_HEIGHT

typedef std::vector<BackgroundItem*> BackgroundItems;

class Background
{
public:
    BackgroundItems items;
    BackgroundItems col_items;
#ifdef USE_COL_TREE
    CollisionTree tree;
#endif

    Background();
    ~Background();
    void reset(bool clear_items = true);
    void destroy_at(int x, int y);
    void paste(Image * img, int dest_x, int dest_y, 
               int src_x, int src_y, int src_width, int src_height, 
               int collision_type);
    void draw();
    bool collide(CollisionBase * a);
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
    bool scroll_active;
    int x, y;
    int x1, y1, x2, y2;

    Layer(double scroll_x, double scroll_y, bool visible, int index);
    ~Layer();
    void scroll(int dx, int dy);
    void set_position(int x, int y);
    void add_background_object(FrameObject * instance);
    void add_object(FrameObject * instance);
    void insert_object(FrameObject * instance, int index);
    void remove_object(FrameObject * instance);
    void set_level(FrameObject * instance, int index);
    int get_level(FrameObject * instance);
    void create_background();
    void destroy_backgrounds();
    void destroy_backgrounds(int x, int y, bool fine);
    bool test_background_collision(CollisionBase * a);
    bool test_background_collision(int x, int y);
    void paste(Image * img, int dest_x, int dest_y, 
               int src_x, int src_y, int src_width, int src_height, 
               int collision_type);
    void draw();

#ifdef CHOWDREN_IS_WIIU
    int remote;
    void set_remote(int value);
#endif
};

// typedef boost::unordered_map<std::string, bool> RunningLoops;
// typedef boost::unordered_map<std::string, int> LoopIndexes;

class InstanceMap
{
public:
    size_t num;
    ObjectList ** items;

    InstanceMap(size_t num);
    ObjectList & get(int id);
    void clear();
};

class Media;

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
    InstanceMap instance_classes;
    // LoopIndexes loop_indexes;
    // RunningLoops running_loops;
    Color background_color;
    GlobalValues * global_values;
    GlobalStrings * global_strings;
    Media * media;
    bool has_quit;
    int off_x, off_y;
    int last_key;
    bool key_presses[CHOWDREN_KEY_LAST + 1];
    bool key_releases[CHOWDREN_KEY_LAST + 1];
    bool mouse_presses[CHOWDREN_MOUSE_BUTTON_LAST + 1];
    int next_frame;
    unsigned int loop_count;
    double frame_time;
    unsigned int frame_iteration;
    int timer_base;

    Frame(const std::string & name, int width, int height, 
          Color background_color, int index, GameManager * manager);
    virtual void event_callback(int id);
    virtual void on_start();
    void on_end();
    virtual void handle_events();
    bool update(float dt);
    void pause();
    void restart();
    void on_key(int key, bool state);
    void on_mouse(int key, bool state);
    void draw(int remote);
    ObjectList & get_instances(int object_id);
    ObjectList get_instances(unsigned int qualifier[]);
    FrameObject * get_instance(int object_id);
    FrameObject * get_instance(unsigned int qualifier[]);
    void add_layer(double scroll_x, double scroll_y, bool visible);
    void add_object(FrameObject * object, int layer_index);
    void add_background_object(FrameObject * object, int layer_index);
    FrameObject * create_object(FrameObject * object, int layer_index);
    void destroy_object(FrameObject * object);
    void set_object_layer(FrameObject * object, int new_layer);
    int get_loop_index(const std::string & name);
    void set_timer(double value);
    void set_display_center(int x = -1, int y = -1);
    int frame_left();
    int frame_right();
    int frame_top();
    int frame_bottom();
    void set_background_color(int color);
    void get_mouse_pos(int * x, int * y);
    int get_mouse_x();
    int get_mouse_y();
    bool is_mouse_pressed_once(int key);
    bool is_key_released_once(int key);
    bool is_key_pressed_once(int key);
    bool test_background_collision(int x, int y);
    bool compare_joystick_direction(int n, int test_dir);
    bool is_joystick_direction_changed(int n);
};

// object types

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
    int loop_count;
    std::vector<Image*> frames;

    Direction(int index, int min_speed, int max_speed, int loop_count, 
              int back_to);
    Direction();
};

class Animation
{
public:
    Direction * dirs[32];

    Animation();
};

class Animations
{
public:
    int count;
    Animation ** items;

    Animations(int count);
};

class Active : public FrameObject
{
public:
    Animations * animations;

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
    float flash_time;
    float flash_interval;
    unsigned int flags;
    int animation_finished;

    Active(int x, int y, int type_id);
    void initialize_active();
    ~Active();
    void initialize_animations();
    void force_animation(int value);
    void force_frame(int value);
    void force_speed(int value);
    void force_direction(int value);
    void restore_animation();
    void restore_frame();
    void restore_speed();
    void add_direction(int animation, int direction,
                       int min_speed, int max_speed, int loop_count,
                       int back_to);
    void add_image(int animation, int direction, Image * image);
    void update_frame();
    void update_action_point();
    void update(float dt);
    void draw();
    Image * get_image();
    int get_action_x();
    int get_action_y();
    void set_angle(double angle, int quality = 0);
    double get_angle();
    int & get_frame();
    int get_speed();
    Direction * get_direction_data(int & dir);
    Direction * get_direction_data();
    int get_animation();
    CollisionBase * get_collision();
    void set_direction(int value);
    int & get_animation_direction();
    void set_scale(double scale);
    void set_x_scale(double value);
    void set_y_scale(double value);
    void paste(int collision_type);
    bool test_direction(int value);
    bool test_directions(int value);
    bool test_animation(int value);
    void stop_animation();
    void start_animation();
    void flash(float value);
    void enable_flag(int index);
    void disable_flag(int index);
    bool is_flag_on(int index);
    bool is_flag_off(int index);
    bool is_near_border(int border);
    bool is_animation_finished(int anim);
};

void set_font_path(const char * path);
void set_font_path(const std::string & path);
void init_font();

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
    bool bold, italic;
    std::string draw_text;
    bool draw_text_set;

    Text(int x, int y, int type_id);
    ~Text();
    void add_line(std::string text);
    void draw();
    void set_string(std::string value);
    void set_paragraph(unsigned int index);
    void next_paragraph();
    int get_index();
    int get_count();
    bool get_bold();
    bool get_italic();
    void set_bold(bool value);
    std::string get_paragraph(int index);
    CollisionBase * get_collision();
};

class Backdrop : public FrameObject
{
public:
    Image * image;
    CollisionBase * collision;
#ifdef CHOWDREN_IS_WIIU
    int remote;
#endif

    Backdrop(int x, int y, int type_id);
    ~Backdrop();
    CollisionBase * get_collision();
    void draw();
};

#define NONE_GRADIENT 0
#define VERTICAL_GRADIENT 1
#define HORIZONTAL_GRADIENT 2

class QuickBackdrop : public FrameObject
{
public:
    Color color;
    int gradient_type;
    Color color2;
    CollisionBase * collision;

    QuickBackdrop(int x, int y, int type_id);
    ~QuickBackdrop();
    CollisionBase * get_collision();
    void draw();
};

class Counter : public FrameObject
{
public:
    Image * images[14];
    double value;
    double minimum, maximum;
    std::string cached_string;

    Counter(int init, int min, int max, int x, int y, int type_id);
    Image * get_image(char c);
    void add(double value);
    void subtract(double value);
    void set_max(double value);
    void set_min(double value);
    void set(double value);
    void draw();
};

typedef boost::unordered_map<std::string, std::string> OptionMap;
typedef boost::unordered_map<std::string, OptionMap> SectionMap;

class INI : public FrameObject
{
public:
    static boost::unordered_map<std::string, SectionMap> global_data;
    std::string current_group;
    SectionMap data;
    std::vector<std::pair<std::string, std::string> > search_results;
    bool overwrite;
    bool read_only;
    bool auto_save;
    std::string filename;
    std::string global_key;

    INI(int x, int y, int type_id);
    static void reset_global_data();
    static int _parse_handler(void* user, const char* section, const char* name,
                             const char* value);
    void parse_handler(const std::string & section, const std::string & name,
                       const std::string & value);
    void set_group(const std::string & name, bool new_group);
    std::string get_string(const std::string & group, const std::string & item, 
                           const std::string & def);
    std::string get_string(const std::string & item, const std::string & def);
    std::string get_string_index(const std::string & group, unsigned int index);
    std::string get_string_index(unsigned int index);
    std::string get_item_name(const std::string & group, unsigned int index);
    std::string get_item_name(unsigned int index);
    std::string get_group_name(unsigned int index);
    double get_value(const std::string & group, const std::string & item, 
                     double def);
    double get_value(const std::string & item, double def);
    double get_value_index(const std::string & group, unsigned int index);
    double get_value_index(unsigned int index);
    void set_value(const std::string & group, const std::string & item, 
                   int pad, double value);
    void set_value(const std::string & item, int pad, double value);
    void set_string(const std::string & group, const std::string & item, 
                    const std::string & value);
    void set_string(const std::string & item, const std::string & value);
    void load_file(const std::string & fn, bool read_only = false, 
                   bool merge = false, bool overwrite = false);
    void load_string(const std::string & data, bool merge);
    void merge_file(const std::string & fn, bool overwrite);
    void get_data(std::stringstream & out);
    void save_file(const std::string & fn, bool force = true);
    std::string as_string();
    void save_file(bool force = true);
    void save_auto();
    int get_item_count(const std::string & section);
    int get_item_count();
    int get_group_count();
    bool has_group(const std::string & group);
    bool has_item(const std::string & group, const std::string & option);
    bool has_item(const std::string & option);
    void search(const std::string & group, const std::string & item, 
                const std::string & value);
    void delete_pattern(const std::string & group, const std::string & item, 
                        const std::string & value);
    void sort_group_by_name(const std::string & group);
    void reset(bool save = true);
    void delete_group(const std::string & group);
    void delete_group();
    void delete_item(const std::string & group, const std::string & item);
    void delete_item(const std::string & item);
    void set_global_data(const std::string & key);
    void merge_object(INI * other, bool overwrite);
    void merge_group(INI * other, const std::string & src_group, 
                     const std::string & dst_group, bool overwrite);
    void merge_map(const SectionMap & data2, bool overwrite);
    void merge_map(SectionMap & data2, const std::string & src_group, 
                   const std::string & dst_group, bool overwrite);
    size_t get_search_count();
    std::string get_search_result_group(int index);
    std::string get_item_part(const std::string & group, 
                              const std::string & item, int index,
                              const std::string & def);
    ~INI();
};

class StringTokenizer : public FrameObject
{
public:
    std::vector<std::string> elements;

    StringTokenizer(int x, int y, int type_id);
    void split(const std::string & text, const std::string & delims);
    const std::string & get(int index);
};

#ifdef _WIN32
#include "windows.h"
#include "shlobj.h"
#elif __APPLE__
#include <CoreServices/CoreServices.h>
#include <limits.h>
#elif __linux
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

class File
{
public:
    static const std::string & get_appdata_directory();
    static bool file_exists(const std::string & path);
    static bool name_exists(const std::string & path);
    static void delete_file(const std::string & path);
};

class WindowControl
{
public:
    static bool has_focus();
    static void set_focus(bool value);
};

class Workspace
{
public:
    std::string name;
    std::stringstream data;

    Workspace(BaseStream & stream);
    Workspace(const std::string & name);
};

typedef boost::unordered_map<std::string, Workspace*> WorkspaceMap;

class BinaryArray : public FrameObject
{
public:
    WorkspaceMap workspaces;
    Workspace * current;

    BinaryArray(int x, int y, int type_id);
    ~BinaryArray();
    void load_workspaces(const std::string & filename);
    void create_workspace(const std::string & name);
    void switch_workspace(const std::string & name);
    void switch_workspace(Workspace * workspace);
    bool has_workspace(const std::string & name);
    void load_file(const std::string & filename);
    std::string read_string(int pos, size_t size);
    size_t get_size();
};

class BinaryObject : public FrameObject
{
public:
    char * data;
    size_t size;

    BinaryObject(int x, int y, int type_id);
    ~BinaryObject();
    void load_file(const std::string & filename);
    void save_file(const std::string & filename);
    void set_byte(size_t addr, unsigned char value);
    void resize(size_t size);
    int get_byte(size_t addr);
    int get_short(size_t addr);
};

class ArrayObject : public FrameObject
{
public:
    bool is_numeric;
    int offset;
    double * array;
    std::string * strings;
    int x_size, y_size, z_size;

    ArrayObject(int x, int y, int type_id);
    ~ArrayObject();
    void initialize(bool is_numeric, int offset, int x, int y, int z);
    void clear();
    double & get_value(int x, int y);
    std::string & get_string(int x);
    void set_value(double value, int x, int y);
    void set_string(const std::string & value, int x);
};

class LayerObject : public FrameObject
{
public:
    int current_layer;
    static int sort_index;
    static bool sort_reverse;
    static double def;

    LayerObject(int x, int y, int type_id);
    void set_layer(int value);
    void hide_layer(int index);
    void show_layer(int index);
    void set_position(int index, int x, int y);
    static double get_alterable(FrameObject * instance);
    static bool sort_func(FrameObject * a, FrameObject * b);
    void sort_alt_decreasing(int index, double def);
};

class Viewport : public FrameObject
{
public:
    int center_x, center_y;
    int src_width, src_height;

    Viewport(int x, int y, int type_id);
    void set_source(int center_x, int center_y, int width, int height);
    void set_width(int w);
    void set_height(int h);
    void draw();
};

class AdvancedDirection : public FrameObject
{
public:
    FrameObject * closest;

    AdvancedDirection(int x, int y, int type_id);
    void find_closest(ObjectList instances, int x, int y);
    FixedValue get_closest(int n);
};

class TextBlitter : public FrameObject
{
public:
    std::string text;
    int char_width, char_height;
    Image * image;
    int * charmap;

    TextBlitter(int x, int y, int type_id);
    void initialize(const std::string & charmap);
    void set_text(const std::string & text);
    void draw();
};

typedef void (*ObstacleOverlapCallback)();
typedef void (*PlatformOverlapCallback)();

class PlatformObject : public FrameObject
{
public:
    FrameObject * instance;
    bool left, right;
    bool paused;

    int x_vel, y_vel;
    int max_x_vel, max_y_vel;
    int add_x_vel, add_y_vel;
    int x_move_count, y_move_count;
    int x_accel, x_decel;
    int gravity;
    int jump_strength;
    int jump_hold_height;
    int step_up;
    int slope_correction;
    bool on_ground;
    bool jump_through, through_collision_top;

    bool obstacle_collision;
    bool platform_collision;

    ObstacleOverlapCallback obstacle_callback;
    PlatformOverlapCallback platform_callback;

    PlatformObject(int x, int y, int type_id);
    void set_object(FrameObject * instance);
    virtual void call_overlaps_obstacle() = 0;
    virtual void call_overlaps_platform() = 0;
    bool overlaps_obstacle();
    bool overlaps_platform();
    bool is_falling();
    bool is_jumping();
    bool is_moving();
    void jump();
    void jump_in_air();
    void update(float dt);
};

typedef boost::unordered_map<std::string, Image*> ImageCache;

class ActivePicture : public FrameObject
{
public:
    Image * image;
    Image * cached_image;
    bool horizontal_flip;
    std::string filename;
    Color transparent_color;
    bool has_transparent_color;
    double scale_x, scale_y;
    double angle;
    SpriteCollision * collision;
    static ImageCache image_cache;

    ActivePicture(int x, int y, int type_id);
    ~ActivePicture();
    void remove_image();
    void load(const std::string & fn);
    void set_transparent_color(const Color & color);
    void set_hotspot(int x, int y);
    void set_hotspot_mul(float x, float y);
    void flip_horizontal();
    void set_scale(double value);
    void set_zoom(double value);
    void set_angle(double value, int quality = 0);
    double get_zoom_x();
    int get_width();
    int get_height();
    void draw();
    CollisionBase * get_collision();
    void paste(int dest_x, int dest_y, int src_x, int src_y, 
               int src_width, int src_height, int collision_type);
};

typedef std::vector<std::string> StringList;

class ListObject : public FrameObject
{
public:
    StringList lines;

    ListObject(int x, int y, int type_id);
    void load_file(const std::string & name);
    void add_line(const std::string & value);
    const std::string & get_line(int i);
    int get_count();
};

#include "extensions.h"

inline void reset_global_data()
{
    INI::reset_global_data();
}

// event helpers

struct MathHelper
{
    double lhs;
    int lhs_int;
};

inline MathHelper & operator*(double lhs, MathHelper& rhs)
{
    rhs.lhs = lhs;
    return rhs;
}

inline double operator*(const MathHelper& lhs, double rhs)
{
    return pow(lhs.lhs, rhs);
}

inline MathHelper & operator%(double lhs, MathHelper& rhs)
{
    rhs.lhs = lhs;
    return rhs;
}

inline double operator%(const MathHelper& lhs, double rhs)
{
    return fmod(lhs.lhs, rhs);
}

inline MathHelper & operator&(int lhs, MathHelper& rhs)
{
    rhs.lhs_int = lhs;
    return rhs;
}

inline int operator&(const MathHelper& lhs, int rhs)
{
    return lhs.lhs_int & rhs;
}

inline MathHelper & operator|(int lhs, MathHelper& rhs)
{
    rhs.lhs_int = lhs;
    return rhs;
}

inline int operator|(const MathHelper& lhs, int rhs)
{
    return lhs.lhs_int | rhs;
}

inline MathHelper & operator^(int lhs, MathHelper& rhs)
{
    rhs.lhs_int = lhs;
    return rhs;
}

inline int operator^(const MathHelper& lhs, int rhs)
{
    return lhs.lhs_int ^ rhs;
}

extern MathHelper math_helper;

inline FrameObject * get_object_from_fixed(double fixed)
{
    if (fixed == 0.0 || fixed == -1.0)
        return NULL;
    int64_t v;
    memcpy(&v, &fixed, sizeof(int64_t));
    intptr_t p = intptr_t(v);
    return (FrameObject*)p;
}

inline FrameObject * get_object_from_fixed(FixedValue fixed)
{
    return fixed.object;
}

inline ObjectList make_single_list(FrameObject * item)
{
    ObjectList new_list;
    new_list.push_back(item);
    return new_list;
}

inline void make_single_list(FrameObject * item, ObjectList & list)
{
    list.clear();
    list.push_back(item);
}

inline FrameObject * get_single(const ObjectList & list)
{
    if (list.empty())
        return NULL;
    return list[0];
}

inline bool check_overlap(ObjectList in_a, ObjectList in_b, 
                          ObjectList & out_a, ObjectList & out_b)
{
    out_a.clear();
    out_b.clear();
    ObjectList::const_iterator item1, item2;
    bool ret = false;
    for (item1 = in_a.begin(); item1 != in_a.end(); item1++) {
        FrameObject * f1 = (*item1);
        bool added = false;
        for (item2 = in_b.begin(); item2 != in_b.end(); item2++) {
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

inline bool check_not_overlap(ObjectList in_a, ObjectList in_b)
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

inline void pick_random(ObjectList & instances)
{
    FrameObject * instance = instances[randrange(instances.size())];
    instances = make_single_list(instance);
}

inline void spread_value(ObjectList & instances, int alt, int start)
{
    ObjectList::const_iterator item;
    for (item = instances.begin(); item != instances.end(); item++) {
        FrameObject * object = *item;
        object->values->set(alt, start);
        start++;
    }
}

inline void set_random_seed(int seed)
{
    cross_srand(seed);
}

inline void open_process(std::string exe, std::string cmd, int pad)
{

}

inline void set_cursor_visible(bool value)
{
    if (value)
        platform_show_mouse();
    else
        platform_hide_mouse();
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

static ObjectList::iterator item;

#endif // CHOWDREN_COMMON_H
