#ifndef CHOWDREN_MANAGER_H
#define CHOWDREN_MANAGER_H

#include "globals.h"
#include "color.h"
#include "fpslimit.h"
#include "include_gl.h"
#include "input.h"
#include "instancemap.h"

class Frame;
class Media;

class GameManager
{
public:
    Frame * frame;
    GlobalValues * values;
    GlobalStrings * strings;
    Media * media;
    FPSLimiter fps_limit;
    bool window_created;
    bool fullscreen;
    int off_x, off_y, x_size, y_size;
    int mouse_x, mouse_y;
    GLuint screen_texture;
    GLuint screen_fbo;
    Color fade_color;
    float fade_dir;
    float fade_value;
    int lives;
    float timer_mul;
    static InstanceMap instances;
#if CHOWDREN_IS_DEMO
    bool idle_timer_started;
    double idle_timer;
    double global_time;
    double show_build_timer;
    double reset_timer;
    double manual_reset_timer;
#endif
    InputList keyboard;
    InputList mouse;

    GameManager();
    void on_key(int key, bool state);
    void on_mouse(int key, bool state);
    bool update();
    int update_frame();
    void draw();
    void set_frame(int index);
    void set_framerate(int framerate);
    void set_window(bool fullscreen);
    void set_fullscreen_type(int type);
    bool is_fullscreen();
    void run();
    void reset_globals();
    void set_fade(const Color & color, float dir);
    void draw_fade();
};

extern FrameObject * default_active_instance;

inline ObjectList & get_instances(int object_id)
{
    ObjectList & list = GameManager::instances.items[object_id];
    list.clear_selection();
    return list;
}

inline ObjectList & get_all_instances(int object_id)
{
    return GameManager::instances.items[object_id];
}

inline QualifierList & get_qualifiers(QualifierList & list)
{
    list.clear_selection();
    return list;
}

inline FrameObject * get_instance(int object_id)
{
    ObjectList & list = GameManager::instances.items[object_id];
    if (list.empty())
        return NULL;
    return list.back();
}

inline FrameObject * get_instance(int object_id, int index)
{
    ObjectList & list = GameManager::instances.items[object_id];
    if (list.empty())
        return NULL;
    return list[index % list.size()];
}

inline FrameObject * get_qualifier(QualifierList & list)
{
    if (list.empty())
        return NULL;
    return list.back();
}

inline FrameObject * get_qualifier(QualifierList & list, int index)
{
    if (list.empty())
        return NULL;
    return list[index % list.size()];
}

inline FrameObject * get_active_instance(int object_id)
{
    ObjectList & list = GameManager::instances.items[object_id];
    if (list.empty())
        return default_active_instance;
    return list.back();
}

inline FrameObject * get_active_instance(int object_id, int index)
{
    ObjectList & list = GameManager::instances.items[object_id];
    if (list.empty())
        return default_active_instance;
    return list[index % list.size()];
}

extern GameManager * global_manager;

#endif // CHOWDREN_MANAGER_H
