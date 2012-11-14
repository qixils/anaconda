#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

#include "globals.h"

class Frame;
class Media;

#include "fpslimit.h"

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
    GLuint screen_texture;
    GLuint screen_fbo;
    int off_x, off_y, x_size, y_size;
    int mouse_x, mouse_y;

    GameManager();
    void on_key(int key, int state);
    void on_mouse(int key, int state);
    int update();
    void draw();
    void set_frame(int index);
    void set_framerate(int framerate);
    void set_window(bool fullscreen);
    bool is_fullscreen();
    void run();
};

GameManager * global_manager;

#include "include_gl.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "config.h"
#include "common.h"
#include "images.h"
#include "fonts.h"

#ifndef NDEBUG
#define CHOWDREN_DEBUG
#endif

#define FRAMERATE 60

void _on_key(int key, int state);
void _on_mouse(int key, int state);

GameManager::GameManager() 
: frame(NULL), window_created(false), fullscreen(false), off_x(0), off_y(0),
  x_size(WINDOW_WIDTH), y_size(WINDOW_HEIGHT)
{
    glfwInit();

#ifdef CHOWDREN_STARTUP_WINDOW
    set_window(false);
#endif

    // application setup
    values = new GlobalValues;
    strings = new GlobalStrings;
    media = new Media;

    setup_globals(values, strings);

    // setup random generator from start
    srand((unsigned int)time(NULL));

    fps_limit.set(FRAMERATE);

    set_frame(0);
}

inline bool check_opengl_extension(const char * name)
{
    if (glewGetExtension(name) == GL_TRUE)
        return true;
    std::cout << "OpenGL extension '" << name << "' not supported." << std::endl;
    return false;
}

inline bool check_opengl_extensions()
{
    char * extensions[] = {
        "GL_EXT_framebuffer_object",
        "GL_ARB_vertex_shader",
        "GL_ARB_fragment_shader",
        "GL_ARB_texture_non_power_of_two",
        NULL
    };
    for (int i = 0; extensions[i] != NULL; i++)
        if (!check_opengl_extension(extensions[i]))
            return false;
    return true;
}

void GameManager::set_window(bool fullscreen)
{
    if (window_created)
        return;
    this->fullscreen = fullscreen;
    window_created = true;
/*#ifdef CHOWDREN_DEBUG
    glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif*/
    glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 0);
    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_FALSE);
    if (fullscreen) {
        GLFWvidmode desktop_mode;
        glfwGetDesktopMode(&desktop_mode);
        glfwOpenWindow(desktop_mode.Width, desktop_mode.Height, 
            desktop_mode.RedBits, desktop_mode.GreenBits, desktop_mode.BlueBits,
            0, 0, 0, GLFW_FULLSCREEN);
    } else
        glfwOpenWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, 0, 0, 0, 0, 
           GLFW_WINDOW);
    glfwEnable(GLFW_SYSTEM_KEYS);
    glfwSetWindowTitle(NAME);
    glfwSwapInterval(0);
    glfwDisable(GLFW_AUTO_POLL_EVENTS);
    glfwSetKeyCallback(_on_key);
    glfwSetMouseButtonCallback(_on_mouse);

    // OpenGL settings
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // initialize OpenGL extensions
    glewInit();

    // check extensions
    if (!check_opengl_extensions()) {
        std::cout << "Not all OpenGL extensions supported. Quitting..." 
            << std::endl;
        exit(EXIT_FAILURE);
        return;
    }

    // for fullscreen or window resize
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
}

bool GameManager::is_fullscreen()
{
    return fullscreen;
}

void GameManager::on_key(int key, int state)
{
    frame->on_key(key, state);
}

void GameManager::on_mouse(int key, int state)
{
    frame->on_mouse(key, state);
}

int GameManager::update()
{
    double dt = fps_limit.dt;
    bool ret = frame->update((float)dt);
    if (frame->next_frame != -1) {
        set_frame(frame->next_frame);
        return 2;
    }
    if (ret)
        return 1;
    else
        return 0;
}

void GameManager::set_framerate(int framerate)
{
    fps_limit.set(framerate);
}

void GameManager::draw()
{
    if (!window_created)
        return;

    int window_width, window_height;
    glfwGetWindowSize(&window_width, &window_height);
    if (window_width <= 0 || window_height <= 0)
        // for some reason, GLFW sets these properties to 0 when minimized.
        return;

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, screen_fbo);

    frame->draw();

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

    int x2 = off_x + x_size;
    int y2 = off_y + y_size;

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
}

void GameManager::set_frame(int index)
{
    if (frame != NULL) {
        frame->on_end();
    }
    frame = get_frames(this)[index];
    // set some necessary pointers
    frame->global_values = values;
    frame->global_strings = strings;
    frame->media = media;
    frame->on_start();
}

void GameManager::run()
{
    int measure_time = 0;

    while(true) {
        measure_time -= 1;
        bool measure_fps = false;
        if (measure_time <= 0) {
            measure_time = 200;
            measure_fps = true;
        }

        fps_limit.start();
        glfwPollEvents();

        // update mouse position
        glfwGetMousePos(&mouse_x, &mouse_y);
        mouse_x = (mouse_x - off_x) * (float(WINDOW_WIDTH) / x_size);
        mouse_y = (mouse_y - off_y) * (float(WINDOW_HEIGHT) / y_size);

        if (measure_fps)
            std::cout << "Framerate: " << fps_limit.current_framerate 
                << std::endl;

        double event_update_time = glfwGetTime();

        int ret = update();

        if (measure_fps)
            std::cout << "Event update took " << 
                glfwGetTime() - event_update_time << std::endl;

        if (ret == 0)
            break;
        else if (ret == 2)
            continue;

        if (!glfwGetWindowParam(GLFW_OPENED))
            break;

        double draw_time = glfwGetTime();

        draw();

        glfwSwapBuffers();

        if (measure_fps)
            std::cout << "Draw took " << glfwGetTime() - draw_time
                << std::endl;

        fps_limit.finish();
    }
}

void _on_key(int key, int state) 
{
    global_manager->on_key(key, state);
}

void _on_mouse(int key, int state) 
{
    global_manager->on_mouse(key, state);
}

#if 1 /* defined(CHOWDREN_DEBUG) || !defined(_WIN32) */
int main (int argc, char *argv[])
#else
int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
#endif
{
    GameManager manager = GameManager();
    global_manager = &manager;
    manager.run();
    return 0;
}