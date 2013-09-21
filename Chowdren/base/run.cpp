#include <cstdlib>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include "manager.h"
#include "fpslimit.h"

GameManager * global_manager;

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "config.h"
#include "frames.h"
#include "common.h"
#include "fonts.h"
#include "crossrand.h"
#include "media.h"

#ifndef NDEBUG
#define CHOWDREN_DEBUG
#endif

#ifdef CHOWDREN_DEBUG
#define CHOWDREN_SHOW_DEBUGGER
#endif

GameManager::GameManager() 
: frame(NULL), window_created(false), fullscreen(false), off_x(0), off_y(0),
  x_size(WINDOW_WIDTH), y_size(WINDOW_HEIGHT), values(NULL), strings(NULL),
  fade_value(0.0f), fade_dir(0.0f)
{
#ifdef CHOWDREN_IS_DEMO
    idle_timer_started = false;
    global_time = show_build_timer = reset_timer = manual_reset_timer = 0.0;
#endif

    init_platform();

#ifdef CHOWDREN_STARTUP_WINDOW
    set_window(false);
#endif

    // application setup
    reset_globals();
    media = new Media;

    // setup random generator from start
    cross_srand((unsigned int)time(NULL));

    fps_limit.set(FRAMERATE);

    set_frame(0);
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenFramebuffersEXT(1, &screen_fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, screen_fbo);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
        GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, screen_texture, 0);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
#endif
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
    frame->on_key(key, state);
}

void GameManager::on_mouse(int key, bool state)
{
    frame->on_mouse(key, state);
}

int GameManager::update()
{
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
extern FTTextureFont * default_font;
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

    platform_begin_draw();

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, screen_fbo);

    frame->draw();

#ifdef CHOWDREN_IS_DEMO
    if (show_build_timer > 0.0) {
        std::string date(__DATE__);
        std::string tim(__TIME__);
        std::string val = date + " " + tim;
        glPushMatrix();
        glTranslatef(50, 50, 0);
        glScalef(5, -5, 5);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        default_font->Render(val.c_str(), val.size(), FTPoint(),
                             FTPoint(), RENDER_ALL);
        glPopMatrix();
    }
#endif

#ifdef CHOWDREN_IS_DESKTOP

    if (fade_dir != 0.0f) {
        glBegin(GL_QUADS);
        glColor4ub(fade_color.r, fade_color.g, fade_color.b,
                   int(fade_value * 255));
        glVertex2f(0.0f, 0.0f);
        glVertex2f(WINDOW_WIDTH, 0.0f);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        glVertex2f(0.0f, WINDOW_HEIGHT);
        glEnd();
    }

    bool resize = window_width != WINDOW_WIDTH || window_height != WINDOW_HEIGHT;

    if (resize) {
        // aspect-aware resize
        float aspect_width = window_width / float(WINDOW_WIDTH);
        float aspect_height = window_height / float(WINDOW_HEIGHT);

        if (aspect_width > aspect_height) {
            x_size = aspect_height * WINDOW_WIDTH;
            y_size = aspect_height * WINDOW_HEIGHT;
        } else {
            x_size = aspect_width * WINDOW_WIDTH;
            y_size = aspect_width * WINDOW_HEIGHT;
        }

        off_x = (window_width - x_size) / 2;
        off_y = (window_height - y_size) / 2;
    } else {
        off_x = off_y = 0;
        x_size = WINDOW_WIDTH;
        y_size = WINDOW_HEIGHT;
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

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

    platform_swap_buffers();
}

void GameManager::set_frame(int index)
{
#ifdef CHOWDREN_IS_DEMO
    idle_timer = 0.0;
    idle_timer_started = false;
#endif

    if (frame != NULL) {
        frame->on_end();
    }
    if (index == -2) {
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

void GameManager::run()
{
    int measure_time = 0;

    while(true) {
        measure_time -= 1;
        bool show_stats = false;
        if (measure_time <= 0) {
            measure_time = 200;
            show_stats = true;
        }

        fps_limit.start();
        platform_poll_events();

        // update mouse position
        platform_get_mouse_pos(&mouse_x, &mouse_y);
        mouse_x = (mouse_x - off_x) * (float(WINDOW_WIDTH) / x_size);
        mouse_y = (mouse_y - off_y) * (float(WINDOW_HEIGHT) / y_size);

        if (show_stats)
            std::cout << "Framerate: " << fps_limit.current_framerate 
                << std::endl;

        double event_update_time = platform_get_time();

        int ret = update();

        if (show_stats)
            std::cout << "Event update took " << 
                platform_get_time() - event_update_time << std::endl;

        if (ret == 0)
            break;
        else if (ret == 2)
            continue;

        if (platform_display_closed())
            break;

        double draw_time = platform_get_time();

        draw();

        if (show_stats) {
            std::cout << "Draw took " << platform_get_time() - draw_time
                << std::endl;
            platform_print_stats();
        }

        fps_limit.finish();
    }
    delete media;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
#else
int main (int argc, char *argv[])
#endif
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

    GameManager manager = GameManager();
    global_manager = &manager;
    manager.run();
    return 0;
}
