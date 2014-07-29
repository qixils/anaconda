#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#endif

#include "manager.h"
#include "fpslimit.h"

GameManager * global_manager;

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "chowconfig.h"
#include "frames.h"
#include "common.h"
#include "fonts.h"
#include "crossrand.h"
#include "media.h"

#if defined(CHOWDREN_IS_DESKTOP)
#include "SDL.h"
#endif

#ifdef CHOWDREN_IS_EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#ifndef NDEBUG
#define CHOWDREN_DEBUG
#endif

// #ifdef CHOWDREN_DEBUG
#define CHOWDREN_SHOW_DEBUGGER
// #endif

GameManager::GameManager()
: frame(NULL), window_created(false), fullscreen(false), off_x(0), off_y(0),
  x_size(WINDOW_WIDTH), y_size(WINDOW_HEIGHT), values(NULL), strings(NULL),
  fade_value(0.0f), fade_dir(0.0f), lives(0), ignore_controls(false),
  last_control_flags(0), control_flags(0)
{
#ifdef CHOWDREN_USE_PROFILER
    PROFILE_SET_DAMPING(0.0);
#endif

    global_manager = this;

#ifdef CHOWDREN_IS_DEMO
    idle_timer_started = false;
    global_time = show_build_timer = reset_timer = manual_reset_timer = 0.0;
#endif

    platform_init();

#ifdef CHOWDREN_STARTUP_WINDOW
    set_window(false);
#endif

    // application setup
    reset_globals();
    setup_keys(this);
    media = new Media;

    // setup random generator from start
    cross_srand((unsigned int)time(NULL));

    fps_limit.set(FRAMERATE);

#ifdef CHOWDREN_IS_HFA
    set_frame(27);
#else
    set_frame(0);
#endif
}

void GameManager::reset_globals()
{
    if (values != NULL)
        delete values;
    if (strings != NULL)
        delete strings;
    values = new GlobalValues;
    strings = new GlobalStrings;
    setup_globals(values, strings);
}

void GameManager::set_window(bool fullscreen)
{
    if (window_created)
        return;
    this->fullscreen = fullscreen;
    window_created = true;

    platform_create_display(fullscreen);

    // OpenGL settings
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    init_shaders();

    // for fullscreen or window resize
#ifdef CHOWDREN_IS_DESKTOP
    glGenTextures(1, &screen_texture);
    glBindTexture(GL_TEXTURE_2D, screen_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH, WINDOW_HEIGHT, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#ifdef CHOWDREN_QUICK_SCALE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenFramebuffers(1, &screen_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, screen_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen_texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

#ifdef CHOWDREN_VSYNC
    platform_set_vsync(true);
#else
    platform_set_vsync(false);
#endif
}

void GameManager::set_window_scale(int scale)
{
    std::cout << "Set window scale: " << scale << std::endl;
}

bool GameManager::is_fullscreen()
{
    return fullscreen;
}

void GameManager::on_key(int key, bool state)
{
#ifdef CHOWDREN_IS_DEMO
    idle_timer_started = true;
    idle_timer = 0.0;
#endif

#ifdef CHOWDREN_IS_DESKTOP
    if (state && key == SDLK_RETURN) {
        bool alt = keyboard.is_pressed(SDLK_LALT) ||
                   keyboard.is_pressed(SDLK_RALT);
        if (alt) {
            fullscreen = !fullscreen;
            platform_set_fullscreen(fullscreen);
        }
        return;
    }
#endif

    if (state)
        keyboard.add(key);
    else
        keyboard.remove(key);
    if (state)
        frame->last_key = key;
}

void GameManager::on_mouse(int key, bool state)
{
    if (state)
        mouse.add(key);
    else
        mouse.remove(key);
}

int GameManager::update_frame()
{
#ifdef CHOWDREN_USE_PROFILER
    PROFILE_FUNC();
#endif

    double dt = fps_limit.dt;
    if (fade_dir != 0.0f) {
        fade_value += fade_dir * (float)dt;
        if (fade_value <= 0.0f || fade_value >= 1.0f) {
            fade_dir = 0.0f;
            fade_value = std::min(1.0f, std::max(0.0f, fade_value));
            return 2;
        }
        return 1;
    }

    if (frame->next_frame != -1 && fade_dir == 0.0f) {
        set_frame(frame->next_frame);
    }

#ifdef CHOWDREN_IS_DEMO
    global_time += dt;
    if (idle_timer_started) {
        idle_timer += dt;
        reset_timer += dt;
    }
    if (platform_should_reset())
        manual_reset_timer += dt;
    else
        manual_reset_timer = 0.0;
    if (idle_timer >= 60 || manual_reset_timer >= 3.0) {
        set_frame(-2);
    }
    if (reset_timer >= 60*10) {
        reset_timer = -10.0;
        media->stop_samples();
        set_frame(5);
    }
    if (global_time <= 60) {
        show_build_timer -= dt;
        if (platform_show_build_info())
            show_build_timer = 3.0;
    }
#endif

    bool ret = frame->update((float)dt);
    if (ret)
        return 1;
    else
        return 0;
}

void GameManager::set_framerate(int framerate)
{
    fps_limit.set(framerate);
}

#ifdef CHOWDREN_IS_DEMO
#include "font.h"
FTTextureFont * get_font(int size);
#endif

void GameManager::draw()
{
    if (!window_created)
        return;

    int window_width, window_height;
    platform_get_size(&window_width, &window_height);
    if (window_width <= 0 || window_height <= 0)
        // for some reason, GLFW sets these properties to 0 when minimized.
        return;

#ifdef CHOWDREN_FORCE_REMOTE
    platform_set_remote_value(CHOWDREN_REMOTE_TARGET);
#endif

    PROFILE_FUNC();

    PROFILE_BEGIN(platform_begin_draw);
    platform_begin_draw();
    PROFILE_END();

    glBindFramebuffer(GL_FRAMEBUFFER, screen_fbo);
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);


    PROFILE_BEGIN(frame_draw);
#ifdef CHOWDREN_IS_WIIU
    int remote_setting = platform_get_remote_value();
    if (remote_setting == CHOWDREN_HYBRID_TARGET) {
        platform_set_display_target(CHOWDREN_REMOTE_TARGET);
        frame->draw(CHOWDREN_REMOTE_TARGET);
        draw_fade();
        platform_set_display_target(CHOWDREN_TV_TARGET);
        frame->draw(CHOWDREN_TV_TARGET);
        draw_fade();
    } else {
        platform_set_display_target(CHOWDREN_TV_TARGET);
        frame->draw(CHOWDREN_HYBRID_TARGET);
        draw_fade();
        if (remote_setting == CHOWDREN_REMOTE_TARGET) {
            platform_clone_buffers();
            platform_set_display_target(CHOWDREN_REMOTE_TARGET);
            frame->draw(CHOWDREN_REMOTE_ONLY);
        }
    }
#else
    frame->draw(CHOWDREN_HYBRID_TARGET);
    draw_fade();
#endif
    PROFILE_END();

    glLoadIdentity();

#ifdef CHOWDREN_IS_DEMO
    if (show_build_timer > 0.0) {
        std::string date(__DATE__);
        std::string tim(__TIME__);
        std::string val = date + " " + tim;
        glPushMatrix();
        glTranslatef(50, 50, 0);
        glScalef(5, -5, 5);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        get_font(24)->Render(val.c_str(), val.size(), FTPoint(),
                             FTPoint());
        glPopMatrix();
    }
#endif

#ifdef CHOWDREN_IS_DESKTOP
    bool resize = window_width != WINDOW_WIDTH || window_height != WINDOW_HEIGHT;

    if (resize) {
        // aspect-aware resize
        float aspect_width = window_width / float(WINDOW_WIDTH);
        float aspect_height = window_height / float(WINDOW_HEIGHT);

        float aspect = std::min(aspect_width, aspect_height);

#ifdef CHOWDREN_QUICK_SCALE
        aspect = std::max(std::min(1.0f, aspect), float(floor(aspect)));
#endif
        x_size = aspect * WINDOW_WIDTH;
        y_size = aspect * WINDOW_HEIGHT;

        off_x = (window_width - x_size) / 2;
        off_y = (window_height - y_size) / 2;
    } else {
        off_x = off_y = 0;
        x_size = WINDOW_WIDTH;
        y_size = WINDOW_HEIGHT;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // resize the window contents if necessary (fullscreen mode)

    glViewport(0, 0, window_width, window_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, window_width, window_height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    int x2 = off_x + x_size;
    int y2 = off_y + y_size;

    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, screen_texture);
    glDisable(GL_BLEND);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0);
    glVertex2i(off_x, off_y);
    glTexCoord2f(1.0, 1.0);
    glVertex2i(x2, off_y);
    glTexCoord2f(1.0, 0.0);
    glVertex2i(x2, y2);
    glTexCoord2f(0.0, 0.0);
    glVertex2i(off_x, y2);
    glEnd();
    glEnable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

#endif // CHOWDREN_IS_DESKTOP

    PROFILE_BEGIN(platform_swap_buffers);
    platform_swap_buffers();
    PROFILE_END();
}

void GameManager::draw_fade()
{
    if (fade_dir == 0.0f)
        return;
    glLoadIdentity();
    glBegin(GL_QUADS);
    glColor4ub(fade_color.r, fade_color.g, fade_color.b,
               int(fade_value * 255));
    glVertex2f(0.0f, 0.0f);
    glVertex2f(WINDOW_WIDTH, 0.0f);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glVertex2f(0.0f, WINDOW_HEIGHT);
    glEnd();
}

void GameManager::set_frame(int index)
{
    ignore_controls = false;

#ifdef CHOWDREN_IS_DEMO
    idle_timer = 0.0;
    idle_timer_started = false;
#endif

#ifndef CHOWDREN_SAMPLES_OVER_FRAMES
    media->stop_samples();
#endif
    if (frame != NULL) {
        frame->on_end();
    }
    if (index == -2) {
        platform_begin_draw();
        media->stop_samples();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        platform_swap_buffers();
#ifdef CHOWDREN_IS_DEMO
        reset_timer = 0.0;
#endif
        index = 0;
        // reset_global_data();
        reset_globals();
    }

    std::cout << "Setting frame: " << index << std::endl;
    frame = get_frames(this)[index];

    // set some necessary pointers
    frame->global_values = values;
    frame->global_strings = strings;
    frame->media = media;
    frame->on_start();
    std::cout << "Frame set" << std::endl;
}

void GameManager::set_fade(const Color & color, float fade_dir)
{
    fade_color = color;
    this->fade_dir = fade_dir;
    if (fade_dir < 0.0f)
        fade_value = 1.0f;
    else
        fade_value = 0.0f;
}

static int get_player_control_flags(int player);

bool GameManager::update()
{
    static int measure_time = 0;
    measure_time -= 1;
    bool show_stats = false;
    if (measure_time <= 0) {
        measure_time = 200;
        show_stats = true;
    }

    // update input
    keyboard.update();
    mouse.update();

    int new_control = get_player_control_flags(1);
    control_flags = new_control & ~(last_control_flags);
    last_control_flags = new_control;

    fps_limit.start();
    platform_poll_events();

    // update mouse position
    platform_get_mouse_pos(&mouse_x, &mouse_y);
    mouse_x = (mouse_x - off_x) * (float(WINDOW_WIDTH) / x_size);
    mouse_y = (mouse_y - off_y) * (float(WINDOW_HEIGHT) / y_size);

    if (show_stats)
        std::cout << "Framerate: " << fps_limit.current_framerate
            << std::endl;

    if (platform_has_error()) {
        if (platform_display_closed())
            return false;
    } else {
        double event_update_time = platform_get_time();

        int ret = update_frame();

        if (show_stats)
            std::cout << "Event update took " <<
                platform_get_time() - event_update_time << std::endl;

        if (ret == 0)
            return false;
        else if (ret == 2)
            return true;

        if (window_created && platform_display_closed())
            return false;
    }

    double draw_time = platform_get_time();

    draw();

    if (show_stats) {
        std::cout << "Draw took " << platform_get_time() - draw_time
            << std::endl;
        int count = 0;
        for (int i = 0; i < MAX_OBJECT_ID; i++) {
            count += GameManager::instances.items[i].size();
        }
        std::cout << "Instance count: " << count << std::endl;
        platform_print_stats();
    }

    fps_limit.finish();

#ifdef CHOWDREN_USE_PROFILER
    static int profile_time = 0;
    profile_time -= 1;
    if (profile_time <= 0) {
        profile_time += 500;
        PROFILE_UPDATE();
        PROFILE_OUTPUT("./profile.txt");
    }
#endif

    return true;
}

#ifdef CHOWDREN_IS_EMSCRIPTEN
static void _emscripten_run()
{
    global_manager->update();
}
#endif

void GameManager::run()
{
#ifdef CHOWDREN_IS_EMSCRIPTEN
    emscripten_set_main_loop(_emscripten_run, 0, 1);
#else
    while (true) {
        if (!update())
            break;
    }
    delete media;
    platform_exit();
#endif
}

InstanceMap GameManager::instances;

// InputList

#define INPUT_STATE_PRESSED 0
#define INPUT_STATE_HOLD 1
#define INPUT_STATE_RELEASED 2

void InputList::add(int v)
{
    std::vector<InputState>::iterator it;
    for (it = items.begin(); it != items.end(); it++) {
        InputState & s = *it;
        if (s.key != v)
            continue;
        s.state = INPUT_STATE_PRESSED;
        return;
    }
    InputState s;
    s.key = v;
    s.state = INPUT_STATE_PRESSED;
    items.push_back(s);
}

void InputList::remove(int v)
{
    std::vector<InputState>::iterator it;
    for (it = items.begin(); it != items.end(); it++) {
        InputState & s = *it;
        if (s.key != v)
            continue;
        s.state = INPUT_STATE_RELEASED;
        return;
    }
}

bool InputList::is_pressed(int v)
{
    std::vector<InputState>::const_iterator it;
    for (it = items.begin(); it != items.end(); it++) {
        const InputState & s = *it;
        if (s.key != v)
            continue;
        return s.state != INPUT_STATE_RELEASED;
    }
    return false;
}

bool InputList::is_pressed_once(int v)
{
    std::vector<InputState>::const_iterator it;
    for (it = items.begin(); it != items.end(); it++) {
        const InputState & s = *it;
        if (s.key != v)
            continue;
        return s.state == INPUT_STATE_PRESSED;
    }
    return false;
}

bool InputList::is_any_pressed()
{
    std::vector<InputState>::const_iterator it;
    for (it = items.begin(); it != items.end(); it++) {
        const InputState & s = *it;
        if (s.state == INPUT_STATE_RELEASED)
            continue;
        return true;
    }
    return false;
}

bool InputList::is_any_pressed_once()
{
    std::vector<InputState>::const_iterator it;
    for (it = items.begin(); it != items.end(); it++) {
        const InputState & s = *it;
        if (s.state != INPUT_STATE_PRESSED)
            continue;
        return true;
    }
    return false;
}

bool InputList::is_released_once(int v)
{
    std::vector<InputState>::const_iterator it;
    for (it = items.begin(); it != items.end(); it++) {
        const InputState & s = *it;
        if (s.key != v)
            continue;
        return s.state == INPUT_STATE_RELEASED;
    }
    return false;
}

void InputList::clear()
{
    items.clear();
}

void InputList::update()
{
    std::vector<InputState>::iterator it = items.begin();
    while (it != items.end()) {
        InputState & s = *it;
        if (s.state != INPUT_STATE_RELEASED) {
            s.state = INPUT_STATE_HOLD;
            it++;
            continue;
        }
        it = items.erase(it);
    }
}

// input helpers

bool is_mouse_pressed(int button)
{
    if (button < 0)
        return false;
    return global_manager->mouse.is_pressed(button);
}

bool is_key_pressed(int button)
{
    if (button < 0)
        return false;
    return global_manager->keyboard.is_pressed(button);
}

bool is_any_key_pressed()
{
    return global_manager->keyboard.is_any_pressed();
}

bool is_any_key_pressed_once()
{
    return global_manager->keyboard.is_any_pressed_once();
}

bool is_mouse_pressed_once(int key)
{
    if (key < 0)
        return false;
    return global_manager->mouse.is_pressed_once(key);
}

bool is_key_released_once(int key)
{
    if (key < 0)
        return false;
    return global_manager->keyboard.is_released_once(key);
}

bool is_key_pressed_once(int key)
{
    if (key < 0)
        return false;
    return global_manager->keyboard.is_pressed_once(key);
}

enum {
    JOYSTICK_BUTTON1 = CHOWDREN_BUTTON_A,
    JOYSTICK_BUTTON2 = CHOWDREN_BUTTON_B,
    JOYSTICK_BUTTON3 = CHOWDREN_BUTTON_X,
    JOYSTICK_BUTTON4 = CHOWDREN_BUTTON_Y,
};

static int get_player_control_flags(int player)
{
    GameManager * m = global_manager;
    if (m->ignore_controls)
        return 0;

    int flags = 0;

    if (m->control_type == CONTROL_KEYBOARD) {
        if (is_key_pressed(m->up))
            flags |= CONTROL_UP;
        if (is_key_pressed(m->down))
            flags |= CONTROL_DOWN;
        if (is_key_pressed(m->left))
            flags |= CONTROL_LEFT;
        if (is_key_pressed(m->right))
            flags |= CONTROL_RIGHT;
        if (is_key_pressed(m->button1))
            flags |= CONTROL_BUTTON1;
        if (is_key_pressed(m->button2))
            flags |= CONTROL_BUTTON2;
        if (is_key_pressed(m->button3))
            flags |= CONTROL_BUTTON3;
        if (is_key_pressed(m->button4))
            flags |= CONTROL_BUTTON4;
    } else {
        flags |= get_joystick_direction_flags(player);
        if (is_joystick_pressed(player, JOYSTICK_BUTTON1))
            flags |= CONTROL_BUTTON1;
        if (is_joystick_pressed(player, JOYSTICK_BUTTON2))
            flags |= CONTROL_BUTTON2;
        if (is_joystick_pressed(player, JOYSTICK_BUTTON3))
            flags |= CONTROL_BUTTON3;
        if (is_joystick_pressed(player, JOYSTICK_BUTTON4))
            flags |= CONTROL_BUTTON4;
    }
    return flags;
}

bool is_player_pressed(int player, int flags)
{
    GameManager * m = global_manager;
    if (m->ignore_controls)
        return false;
    if (flags == 0)
        return m->last_control_flags == 0;
    return (m->last_control_flags & flags) == flags;
}

bool is_player_pressed_once(int player, int flags)
{
    GameManager * m = global_manager;
    if (m->ignore_controls)
        return false;
    if (flags == 0)
        return m->control_flags == 0;
    return (m->control_flags & flags) == flags;
}

// main function

int main(int argc, char *argv[])
{
#if defined(_WIN32) && defined(CHOWDREN_SHOW_DEBUGGER)
    int outHandle, errHandle, inHandle;
    FILE *outFile, *errFile, *inFile;
    AllocConsole();
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);

    outHandle = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
    errHandle = _open_osfhandle((long)GetStdHandle(STD_ERROR_HANDLE), _O_TEXT);
    inHandle = _open_osfhandle((long)GetStdHandle(STD_INPUT_HANDLE), _O_TEXT);

    outFile = _fdopen(outHandle, "w" );
    errFile = _fdopen(errHandle, "w");
    inFile =  _fdopen(inHandle, "r");

    *stdout = *outFile;
    *stderr = *errFile;
    *stdin = *inFile;

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    std::ios::sync_with_stdio();
#endif

    GameManager manager;

    manager.run();
    return 0;
}
