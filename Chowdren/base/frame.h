#ifndef CHOWDREN_FRAME_H
#define CHOWDREN_FRAME_H

#include <vector>
#include <list>
#include "broadphase.h"
#include "frameobject.h"

class BackgroundItem;
class CollisionBase;

typedef std::vector<BackgroundItem*> BackgroundItems;

class Background
{
public:
    BackgroundItems items;
    BackgroundItems col_items;

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

// #define LAYER_USE_STD_SORT
typedef std::list<FrameObject*> LayerInstances;

class Layer
{
public:
    LayerInstances instances;
    FlatObjectList background_instances;
    bool visible;
    double scroll_x, scroll_y;
    Background * back;
    int index;
    bool scroll_active;
    int off_x, off_y;
    int x, y;
    Broadphase broadphase;
    bool order_changed;
    bool wrap_x, wrap_y;

    Layer(int index, double scroll_x, double scroll_y, bool visible,
          bool wrap_x, bool wrap_y);
    ~Layer();
    void scroll(int off_x, int off_y, int dx, int dy);
    void set_position(int x, int y);
    void add_background_object(FrameObject * instance);
    void add_object(FrameObject * instance);
    void insert_object(FrameObject * instance, int index);
    void remove_object(FrameObject * instance);
    int get_level(FrameObject * instance);
    void set_level(FrameObject * instance, int index);
    void set_level(int old_index, int new_index);
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
    int virtual_width, virtual_height;
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
          int virtual_width, int virtual_height,
          Color background_color, int index, GameManager * manager);
    virtual void event_callback(int id);
    virtual void on_start();
    virtual void on_end();
    virtual void handle_events();
    virtual void update_objects(float dt) = 0;
    bool update(float dt);
    void pause();
    void restart();
    void draw(int remote);
    void add_layer(double scroll_x, double scroll_y, bool wrap_x,
                   bool wrap_y, bool visible);
    FrameObject * add_object(FrameObject * object, int layer_indcex);
    FrameObject * add_object(FrameObject * object, Layer * layer);
    void add_background_object(FrameObject * object, int layer_index);
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

#endif // CHOWDREN_FRAME_H
