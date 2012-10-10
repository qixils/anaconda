#ifdef _WIN32
#include <windows.h>
#endif

class GameManager;

#include "include_gl.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "config.h"
#include "common.h"
#include "images.h"
#include "fonts.h"

#ifndef NDEBUG
#define DEBUG
#endif

#define FRAMERATE 60.0

void _on_key(int key, int state);
void _on_mouse(int key, int state);

class GameManager
{
public:
    Frame * frame;
    GlobalValues values;
    GlobalStrings strings;
    Media media;

    GameManager() : frame(NULL)
    {
        glfwInit();
    #ifdef DEBUG
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
        setup_globals(&values, &strings);

        // setup random generator from start
        srand((unsigned int)time(NULL));

        set_frame(0);
    }

    void on_key(int key, int state)
    {
        frame->on_key(key, state);
    }

    void on_mouse(int key, int state)
    {
        frame->on_mouse(key, state);
    }

    bool update(double dt)
    {
        bool ret = frame->update((float)dt);
        if (frame->next_frame != -1) {
            set_frame(frame->next_frame);
            return true;
        }
        return ret;
    }

    void draw()
    {
        frame->draw();
        return;
    }

    void set_frame(int index)
    {
        if (frame != NULL) {
            frame->on_end();
            delete frame;
        }
        frame = get_frames(this)[index];
        // set some necessary pointers
        frame->global_values = &values;
        frame->global_strings = &strings;
        frame->media = &media;
        frame->on_start();
    }
};

static GameManager * global_manager;

void _on_key(int key, int state) 
{
    global_manager->on_key(key, state);
}

void _on_mouse(int key, int state) 
{
    global_manager->on_mouse(key, state);
}

#if 1 /* defined(DEBUG) || !defined(_WIN32) */
int main (int argc, char *argv[])
#else
int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
#endif
{
    GameManager manager = GameManager();
    global_manager = &manager;

    double current_time, old_time, dt, next_update;
    old_time = glfwGetTime();

    while(true) {
        current_time = glfwGetTime();
        dt = current_time - old_time;

        if (dt <= 0.0)
            continue;

        old_time = current_time;
        next_update = current_time + 1.0 / FRAMERATE;

        glfwPollEvents();

        if (!manager.update(dt))
            break;

        if (!glfwGetWindowParam(GLFW_OPENED))
            break;

        manager.draw();

        glfwSwapBuffers();

        dt = next_update - glfwGetTime();
        if (dt > 0.0) {
            glfwSleep(dt);
        }
    }
    return 0;
}