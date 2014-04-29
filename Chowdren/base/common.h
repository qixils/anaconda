#ifndef CHOWDREN_COMMON_H
#define CHOWDREN_COMMON_H

#include "chowconfig.h"
#include "profiler.h"
#include "keys.h"
#include "manager.h"
#include "platform.h"
#include "include_gl.h"
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
#include <boost/unordered_map.hpp>
#include "coltree.h"
#include "input.h"
#include "movement.h"

extern std::string newline_character;

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
    const char * face;
    int size;
    bool bold;
    bool italic;
    bool underline;

    Font(const char * face, int size, bool bold, bool italic, bool underline);
};

#define BACK_WIDTH WINDOW_WIDTH
#define BACK_HEIGHT WINDOW_HEIGHT

typedef std::vector<BackgroundItem*> BackgroundItems;

class Background
{
public:
    BackgroundItems items;
    BackgroundItems col_items;
#ifdef CHOWDREN_USE_COLTREE
    StaticTree tree;
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
    FlatObjectList instances;
    FlatObjectList background_instances;
    bool visible;
    double scroll_x, scroll_y;
    Background * back;
    int index;
    bool scroll_active;
    int off_x, off_y;
    int x, y;
    int x1, y1, x2, y2;
    DynamicTree aabb_tree;
#ifdef CHOWDREN_USE_COLTREE
    StaticTree display_tree;
    StaticTree tree;
#endif

    Layer(double scroll_x, double scroll_y, bool visible, int index);
    ~Layer();
    void scroll(int off_x, int off_y, int dx, int dy);
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

#if defined(CHOWDREN_IS_WIIU) || defined(CHOWDREN_EMULATE_WIIU)
    int remote;
    void set_remote(int value);
#endif
};

typedef bool (*LoopCallback)(void*);

class DynamicLoop
{
public:
    LoopCallback callback;
    bool * running;
    int * index;

    DynamicLoop()
    {
    }

    void set(LoopCallback callback, bool * running, int * index)
    {
        this->callback = callback;
        this->running = running;
        this->index = index;
    }
};

typedef boost::unordered_map<std::string, DynamicLoop> DynamicLoops;

class Media;

class Frame
{
public:
    std::string name;
    int width, height;
    int index;
    GameManager * manager;
    FlatObjectList destroyed_instances;
    std::vector<Layer*> layers;
    DynamicLoops loops;
    Color background_color;
    GlobalValues * global_values;
    GlobalStrings * global_strings;
    Media * media;
    bool has_quit;
    int off_x, off_y, new_off_x, new_off_y;
    int last_key;
    int next_frame;
    unsigned int loop_count;
    double frame_time;
    unsigned int frame_iteration;
    int timer_base;
    float timer_mul;

    Frame(const std::string & name, int width, int height,
          Color background_color, int index, GameManager * manager);
    virtual void event_callback(int id);
    virtual void on_start();
    virtual void on_end();
    virtual void handle_events();
    bool update(float dt);
    void pause();
    void restart();
    void draw(int remote);
    void add_layer(double scroll_x, double scroll_y, bool visible);
    FrameObject * add_object(FrameObject * object, int layer_indcex);
    FrameObject * add_object(FrameObject * object, Layer * layer);
    void add_background_object(FrameObject * object, int layer_index);
    void destroy_object(FrameObject * object);
    void set_object_layer(FrameObject * object, int new_layer);
    int get_loop_index(const std::string & name);
    void set_timer(double value);
    void set_lives(int lives);
    void set_display_center(int x = -1, int y = -1);
    void update_display_center();
    int frame_left();
    int frame_right();
    int frame_top();
    int frame_bottom();
    void set_background_color(int color);
    void get_mouse_pos(int * x, int * y);
    int get_mouse_x();
    int get_mouse_y();
    bool test_background_collision(int x, int y);
    bool compare_joystick_direction(int n, int test_dir);
    bool is_joystick_direction_changed(int n);
    void clean_instances();
    void set_vsync(bool value);
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
    int forced_animation, forced_frame, forced_speed, forced_direction;
    unsigned int counter;
    double angle;
    SpriteCollision * collision;
    double x_scale, y_scale;
    int action_x, action_y;
    bool collision_box;
    bool stopped;
    float flash_time, flash_interval;
    unsigned int flags;
    int animation_finished;
    bool auto_rotate;
    bool transparent;
    int loop_count;

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
    void update_direction();
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
    int get_animation(int anim);
    CollisionBase * get_collision();
    void set_animation(int value);
    void set_direction(int value, bool set_movement = true);
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
    void toggle_flag(int index);
    bool is_flag_on(int index);
    bool is_flag_off(int index);
    int get_flag(int index);
    bool is_near_border(int border);
    bool is_animation_finished(int anim);
    void destroy();
    bool has_animation(int anim);
};

void set_font_path(const char * path);
void set_font_path(const std::string & path);
void init_font();

class FTSimpleLayout;

class Text : public FrameObject
{
public:
    std::vector<std::string> paragraphs;
    std::string text;
    unsigned int current_paragraph;
    bool initialized;
    int alignment;
    CollisionBase * collision;
    bool bold, italic;
    int size;
    std::string draw_text;
    bool draw_text_set;
    FTSimpleLayout * layout;

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
    void set_width(int w);
    int get_width();
    int get_height();
    void update_draw_text();
};

class FontInfo
{
public:
    static std::string vertical_tab;

    static int get_width(FrameObject * obj);
    static int get_height(FrameObject * obj);
    static void set_width(FrameObject * obj, int w);
};

class Backdrop : public FrameObject
{
public:
    Image * image;
    CollisionBase * collision;
#if defined(CHOWDREN_IS_WIIU) || defined(CHOWDREN_EMULATE_WIIU)
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
    Image * image;

    QuickBackdrop(int x, int y, int type_id);
    ~QuickBackdrop();
    CollisionBase * get_collision();
    void draw();
};

#define HIDDEN_COUNTER 0
#define IMAGE_COUNTER 1
#define VERTICAL_UP_COUNTER 2

class Counter : public FrameObject
{
public:
    Image * images[14];
    double value;
    double minimum, maximum;
    std::string cached_string;
    CollisionBase * collision;
    int type;
    float flash_time, flash_interval;
    Color color1;

    Counter(int x, int y, int type_id);
    ~Counter();
    CollisionBase * get_collision();
    Image * get_image(char c);
    void add(double value);
    void subtract(double value);
    void set_max(double value);
    void set_min(double value);
    void set(double value);
    void draw();
    void calculate_box();
    void update(float dt);
    void flash(float value);
};

class Lives : public FrameObject
{
public:
    Image * image;
    float flash_time;
    float flash_interval;

    Lives(int x, int y, int type_id);
    void update(float dt);
    void flash(float value);
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
    std::string get_string(const std::string & item);
    std::string get_string(const std::string & group, const std::string & item,
                           const std::string & def);
    std::string get_string(const std::string & item, const std::string & def);
    std::string get_string_index(const std::string & group, unsigned int index);
    std::string get_string_index(unsigned int index);
    std::string get_item_name(const std::string & group, unsigned int index);
    std::string get_item_name(unsigned int index);
    std::string get_group_name(unsigned int index);
    double get_value(const std::string & item);
    double get_value(const std::string & group, const std::string & item,
                     double def);
    double get_value(const std::string & item, double def);
    double get_value_index(const std::string & group, unsigned int index);
    double get_value_index(unsigned int index);
    void set_value(const std::string & group, const std::string & item,
                   int pad, double value);
    void set_value(const std::string & item, int pad, double value);
    void set_value(const std::string & item, double value);
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
    static void set_x(int x);
    static void set_y(int y);
    static bool has_focus();
    static bool is_maximized();
    static void set_focus(bool value);
    static void set_width(int w);
    static void set_height(int w);
    static void maximize();
    static void restore();
    static int get_width();
    static int get_height();
    static int get_screen_width();
    static int get_screen_height();
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
    void set_byte(unsigned char value, size_t addr);
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
    void set_x(int index, int x);
    void set_y(int index, int y);
    void set_position(int index, int x, int y);
    void set_alpha_coefficient(int index, int alpha);
    static double get_alterable(FrameObject * instance);
    static bool sort_func(FrameObject * a, FrameObject * b);
    void sort_alt_decreasing(int index, double def);
};

class Viewport : public FrameObject
{
public:
    int center_x, center_y;
    int src_width, src_height;
    GLuint texture;

    Viewport(int x, int y, int type_id);
    ~Viewport();
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
    void find_closest(ObjectList & instances, int x, int y);
    void find_closest(QualifierList & instances, int x, int y);
    FixedValue get_closest(int n);
    static float get_object_angle(FrameObject * a, FrameObject * b);
};

class TextBlitter : public FrameObject
{
public:
    std::string text;
    int char_width, char_height;
    Image * image;
    int * charmap;
    float flash_time, flash_interval;
    int alignment;

    TextBlitter(int x, int y, int type_id);
    void initialize(const std::string & charmap);
    void set_text(const std::string & text);
    void draw();
    void update(float dt);
    void flash(float value);
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

inline MathHelper & operator/(double lhs, MathHelper& rhs)
{
    rhs.lhs = lhs;
    return rhs;
}

inline int operator/(const MathHelper& lhs, double rhs)
{
    return int(lhs.lhs / rhs);
}

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
    if (rhs == 0.0)
        return 0.0;
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

inline void remove_list(FlatObjectList & a, FrameObject * obj)
{
    FlatObjectList::iterator it;
    for (it = a.begin(); it != a.end(); it++) {
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

inline FrameObject * get_single(const QualifierList & list)
{
    return list.back_selection();
}

inline FrameObject * get_single(ObjectList & list, int index)
{
    return list.get_wrapped_selection(index);
}

extern std::vector<int> int_temp;

template <bool save>
struct OverlapCallback
{
    FrameObject * instance;
    bool added;
    int * temp1;
    int * temp2;

    OverlapCallback(FrameObject * instance, int * temp1, int * temp2)
    : instance(instance), added(false), temp1(temp1), temp2(temp2)
    {
    }

    bool on_callback(void * data)
    {
        FrameObject * other = (FrameObject*)data;
        if (other == instance)
            return true;
        if (!temp1[other->index-1])
            return true;
        if (!instance->overlaps(other))
            return true;
        if (save) {
            if (instance->movement != NULL)
                instance->movement->add_collision(other);
            if (other->movement != NULL)
                other->movement->add_collision(instance);
        }
        temp2[other->index-1] = 1;
        added = true;
        return true;
    }
};

template <bool save>
inline bool check_overlap(ObjectList & list1, ObjectList & list2)
{
    int size = list2.size();
    if (size <= 0)
        return false;
    int_temp.resize(size*2);
    std::fill(int_temp.begin(), int_temp.end(), 0);

    // all currently selected objects
    int * temp1 = &int_temp[0];
    // new objects selected due to overlap
    int * temp2 = &int_temp[size];

    for (ObjectIterator it(list2); !it.end(); it++) {
        temp1[it.index-1] = 1;
    }

    bool ret = false;
    for (ObjectIterator it1(list1); !it1.end(); it1++) {
        FrameObject * instance = *it1;
        OverlapCallback<save> callback(instance, temp1, temp2);
        CollisionBase * col = instance->get_collision();
        if (col == NULL) {
            it1.deselect();
            continue;
        }
        int v[4];
        col->get_box(v);
        list2.tree.query(v, callback);
        if (callback.added)
            ret = true;
        else
            it1.deselect();
    }

    if (!ret)
        return false;

    for (ObjectIterator it(list2); !it.end(); it++) {
        if (!temp2[it.index-1])
            it.deselect();
    }

    return true;
}

template <bool save>
inline bool check_overlap(QualifierList & list1, ObjectList & list2)
{
    int size = list1.size();
    if (size <= 0)
        return false;
    int_temp.resize(size*2);
    std::fill(int_temp.begin(), int_temp.end(), 0);

    // all currently selected objects
    int * temp1 = &int_temp[0];
    // new objects selected due to overlap
    int * temp2 = &int_temp[size];

    int total_index = 0;
    for (int i = 0; i < list1.count; i++) {
        ObjectList & list = *list1.items[i];
        for (ObjectIterator it(list); !it.end(); it++) {
            temp1[total_index + it.index - 1] = 1;
        }
        total_index += list.size();
    }

    bool ret = false;
    for (ObjectIterator it2(list2); !it2.end(); it2++) {
        FrameObject * instance = *it2;
        CollisionBase * col = instance->get_collision();
        if (col == NULL) {
            it2.deselect();
            continue;
        }
        int v[4];
        col->get_box(v);
        bool added = false;
        int temp_offset = 0;
        for (int i = 0; i < list1.count; i++) {
            ObjectList & list = *list1.items[i];
            int * temp_off_1 = temp1 + temp_offset;
            int * temp_off_2 = temp2 + temp_offset;
            OverlapCallback<save> callback(instance, temp_off_1, temp_off_2);
            list.tree.query(v, callback);
            if (callback.added) {
                ret = added = true;
            }
            temp_offset += list.size();
        }
        if (!added)
            it2.deselect();
    }

    if (!ret)
        return false;

    total_index = 0;
    for (int i = 0; i < list1.count; i++) {
        ObjectList & list = *list1.items[i];
        for (ObjectIterator it(list); !it.end(); it++) {
            if (!temp2[total_index + it.index - 1])
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

template <bool save>
inline bool check_overlap(QualifierList & list1, QualifierList & list2)
{
    int size = list1.size();
    if (size <= 0)
        return false;
    int_temp.resize(size*2);
    std::fill(int_temp.begin(), int_temp.end(), 0);

    // all currently selected objects
    int * temp1 = &int_temp[0];
    // new objects selected due to overlap
    int * temp2 = &int_temp[size];

    int total_index = 0;
    for (int i = 0; i < list1.count; i++) {
        ObjectList & list = *list1.items[i];
        for (ObjectIterator it(list); !it.end(); it++) {
            temp1[it.index - 1 + total_index] = 1;
        }
        total_index += list.size();
    }

    bool ret = false;
    for (QualifierIterator it2(list2); !it2.end(); it2++) {
        FrameObject * instance = *it2;
        CollisionBase * col = instance->get_collision();
        if (col == NULL) {
            it2.deselect();
            continue;
        }
        int v[4];
        col->get_box(v);
        bool added = false;

        int temp_offset = 0;
        for (int i = 0; i < list1.count; i++) {
            ObjectList & list = *list1.items[i];
            int * temp_off_1 = temp1 + temp_offset;
            int * temp_off_2 = temp2 + temp_offset;
            OverlapCallback<save> callback(instance, temp_off_1, temp_off_2);
            list.tree.query(v, callback);
            if (callback.added) {
                ret = added = true;
            }
            temp_offset += list.size();
        }
        if (!added)
            it2.deselect();
    }

    if (!ret)
        return false;

    total_index = 0;
    for (int i = 0; i < list1.count; i++) {
        ObjectList & list = *list1.items[i];
        for (ObjectIterator it(list); !it.end(); it++) {
            if (!temp2[total_index + it.index - 1])
                it.deselect();
        }
        total_index += list.size();
    }

    return true;
}

struct NotOverlapCallback
{
    FrameObject * instance;

    NotOverlapCallback(FrameObject * instance)
    : instance(instance)
    {
    }

    bool on_callback(void * data)
    {
        FrameObject * other = (FrameObject*)data;
        if (other == instance)
            return true;
        if (!int_temp[other->index-1])
            return true;
        if (!instance->overlaps(other))
            return true;
        return false;
    }
};

inline bool check_not_overlap(ObjectList & list1, ObjectList & list2)
{
    int size = list2.size();
    if (size <= 0)
        return true;
    int_temp.resize(size);
    std::fill(int_temp.begin(), int_temp.end(), 0);

    for (ObjectIterator it(list2); !it.end(); it++) {
        int_temp[it.index-1] = 1;
    }

    for (ObjectIterator it1(list1); !it1.end(); it1++) {
        FrameObject * instance = *it1;
        CollisionBase * col = instance->get_collision();
        if (col == NULL) {
            it1.deselect();
            continue;
        }
        NotOverlapCallback callback(instance);
        int v[4];
        col->get_box(v);
        if (!list2.tree.query(v, callback))
            return false;
    }
    return true;
}

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
    return check_not_overlap(list2, list1);
}

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

inline void pick_random(ObjectList & instances)
{
    if (!instances.has_selection())
        return;
    int index = randrange(instances.get_selection_size());
    for (ObjectIterator it(instances); !it.end(); it++) {
        if (index == 0) {
            it.select_single();
            break;
        }
        index--;
    }
}

inline void spread_value(ObjectList & instances, int alt, int start)
{
    for (ObjectIterator it(instances); !it.end(); it++) {
        (*it)->values->set(alt, start);
        start++;
    }
}

inline void spread_value(QualifierList & instances, int alt, int start)
{
    for (QualifierIterator it(instances); !it.end(); it++) {
        (*it)->values->set(alt, start);
        start++;
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
