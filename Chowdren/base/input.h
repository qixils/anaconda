#ifndef CHOWDREN_INPUT_H
#define CHOWDREN_INPUT_H

#include <vector>

class InputState
{
public:
    int key;
    int state;
};

enum {
    CHOWDREN_BUTTON_INVALID = 0,
    CHOWDREN_BUTTON_A,
    CHOWDREN_BUTTON_B,
    CHOWDREN_BUTTON_X,
    CHOWDREN_BUTTON_Y,
    CHOWDREN_BUTTON_BACK,
    CHOWDREN_BUTTON_GUIDE,
    CHOWDREN_BUTTON_START,
    CHOWDREN_BUTTON_LEFTSTICK,
    CHOWDREN_BUTTON_RIGHTSTICK,
    CHOWDREN_BUTTON_LEFTSHOULDER,
    CHOWDREN_BUTTON_RIGHTSHOULDER,
    CHOWDREN_BUTTON_DPAD_UP,
    CHOWDREN_BUTTON_DPAD_DOWN,
    CHOWDREN_BUTTON_DPAD_LEFT,
    CHOWDREN_BUTTON_DPAD_RIGHT,
    CHOWDREN_BUTTON_MAX
};

enum {
    CHOWDREN_AXIS_INVALID = 0,
    CHOWDREN_AXIS_LEFTX,
    CHOWDREN_AXIS_LEFTY,
    CHOWDREN_AXIS_RIGHTX,
    CHOWDREN_AXIS_RIGHTY,
    CHOWDREN_AXIS_TRIGGERLEFT,
    CHOWDREN_AXIS_TRIGGERRIGHT,
    CHOWDREN_AXIS_MAX
};

class InputList
{
public:
    std::vector<InputState> items;

    void add(int v);
    void remove(int v);
    bool is_pressed_once(int v);
    bool is_pressed(int v);
    bool is_released_once(int v);
    bool is_any_pressed();
    bool is_any_pressed_once();
    void clear();
    void update();
};

bool is_mouse_pressed(int button);
bool is_key_pressed(int button);
bool is_key_pressed(int key);
bool is_any_key_pressed();
bool is_any_key_pressed_once();
bool is_mouse_pressed_once(int key);
bool is_key_released_once(int key);
bool is_key_pressed_once(int key);

int get_joystick_direction(int n);
int get_joystick_direction_flags(int n);
bool test_joystick_direction_flags(int n, int flags);
int get_joystick_dpad(int n);
float get_joystick_dpad_degrees(int n);
float get_joystick_lt(int n);
float get_joystick_rt(int n);
float get_joystick_x(int n);
float get_joystick_y(int n);

int remap_button(int n);

#endif // CHOWDREN_INPUT_H
