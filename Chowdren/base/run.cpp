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

    GameManager();
    void on_key(int key, int state);
    void on_mouse(int key, int state);
    int update();
    void draw();
    void set_frame(int index);
    void set_framerate(int framerate);
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

GameManager::GameManager() : frame(NULL)
{
    glfwInit();
#ifdef CHOWDREN_DEBUG
    glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
    glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 0);
    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
    glfwOpenWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, 0, 0, 0, 0, 
                   GLFW_WINDOW);
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
    frame->draw();
    return;
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

    int measure_time = 0;

    while(true) {
        measure_time -= 1;
        bool measure_fps = false;
        if (measure_time <= 0) {
            measure_time = 200;
            measure_fps = true;
        }

        manager.fps_limit.start();

        glfwPollEvents();

        if (measure_fps)
            std::cout << "Framerate: " << manager.fps_limit.current_framerate 
                << std::endl;

        double event_update_time = glfwGetTime();

        int ret = manager.update();
        if (ret == 0)
            break;
        else if (ret == 2)
            continue;

        if (!glfwGetWindowParam(GLFW_OPENED))
            break;

        if (measure_fps)
            std::cout << "Event update took " << 
                glfwGetTime() - event_update_time << std::endl;

        double draw_time = glfwGetTime();

        manager.draw();

        glfwSwapBuffers();

        if (measure_fps)
            std::cout << "Draw took " << glfwGetTime() - draw_time
                << std::endl;

        manager.fps_limit.finish();
    }
    return 0;
}