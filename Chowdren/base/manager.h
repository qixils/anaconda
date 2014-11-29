#ifndef CHOWDREN_MANAGER_H
#define CHOWDREN_MANAGER_H

#include "events.h"
#include "globals.h"
#include "color.h"
#include "fpslimit.h"
#include "include_gl.h"
#include "input.h"
#include "pool.h"
#include "chowconfig.h"

class Frame;

class GameManager
{
public:
    Frame * frame;
    GlobalValues * values;
    GlobalStrings * strings;
    FPSLimiter fps_limit;
    bool window_created;
    bool fullscreen;
    int off_x, off_y, x_size, y_size;
    int mouse_x, mouse_y;
    Color fade_color;
    float fade_dir;
    float fade_value;
    int score;
    int lives;
    bool player_died;
    float dt;
    float timer_mul;
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

    // player controls
    int up, down, left, right, button1, button2, button3, button4;
    int last_control_flags, control_flags;
    int control_type;
    bool ignore_controls;

    GameManager();
    void init();
    void on_key(int key, bool state);
    void on_mouse(int key, bool state);
    bool update();
    int update_frame();
    void draw();
    void set_frame(int index);
    void set_framerate(int framerate);
    void set_window(bool fullscreen);
    void set_window_scale(int scale);
    void set_fullscreen_type(int type);
    bool is_fullscreen();
    void run();
    void reset_globals();
    void set_fade(const Color & color, float dir);
    void draw_fade();
};

inline FrameObject * get_instance(ObjectList & list)
{
    if (list.empty())
        return NULL;
    return list.back();
}

inline FrameObject * get_instance(ObjectList & list, int index)
{
    if (list.empty())
        return NULL;
    int size = list.size();
    index = (size - 1) - (index % size);
    return list[index];
}

inline FrameObject * get_instance(ObjectList & list, FrameObject * def)
{
    if (list.empty())
        return def;
    return list.back();
}

inline FrameObject * get_instance(ObjectList & list, int index,
                                  FrameObject * def)
{
    if (list.empty())
        return def;
    index = (list.size() - 1) - (index % list.size());
    return list[index];
}

inline FrameObject * get_qualifier(QualifierList & list)
{
    return list.back();
}

inline FrameObject * get_qualifier(QualifierList & list, int index)
{
    if (list.empty())
        return NULL;
    int size = list.size();
    index = (size - 1) - (index % size);
    return list[index];
}

inline FrameObject * get_qualifier(QualifierList & list, FrameObject * def)
{
    FrameObject * ret = list.back();
    if (ret == NULL)
        return def;
    return ret;
}

inline FrameObject * get_qualifier(QualifierList & list, int index,
                                   FrameObject * def)
{
    if (list.empty())
        return def;
    int size = list.size();
    index = (size - 1) - (index % size);
    return list[index];
}

extern FrameObject * default_active_instance;
extern FrameObject * default_picture_instance;
#ifdef CHOWDREN_USE_BLITTER
extern FrameObject * default_blitter_instance;
#endif

extern GameManager manager;

#endif // CHOWDREN_MANAGER_H
