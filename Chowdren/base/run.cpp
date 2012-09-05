#ifdef _WIN32
#include <windows.h>
#endif

class GameManager;

#include <GL/glfw.h>
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

#define FRAMERATE 85.0

class GameManager
{
public:
    Frame * frame;

    GameManager() : frame(NULL)
    {
    #ifdef DEBUG
        glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    #endif
        glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 0);
        glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
        glfwOpenWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, 0, 0, 0, 0, 
                       GLFW_WINDOW);
        glfwSetWindowTitle(NAME);
        glfwSwapInterval(0);

        // OpenGL settings
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        set_frame(0);
    }

    bool update(double dt)
    {
        frame->handle_events();
        return true;
    }

    void draw()
    {
        frame->draw();
        return;
    }

    void set_frame(int index)
    {
        if (frame != NULL)
            frame->on_end();
        frame = get_frames(this)[index];
        frame->on_start();
    }
};

#if 1 /* defined(DEBUG) || !defined(_WIN32) */
int main (int argc, char *argv[])
#else
int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
#endif
{
    // setup random generator from start
    srand((unsigned int)time(NULL));

    glfwInit();
    GameManager manager = GameManager();

    double current_time, old_time, dt, next_update;
    old_time = glfwGetTime();

    while(true) {
        current_time = glfwGetTime();
        dt = current_time - old_time;

        if (dt <= 0.0)
            continue;

        old_time = current_time;
        next_update = current_time + 1.0 / FRAMERATE;

        printf("%f\n", 1.0 / dt);
        if (!manager.update(dt))
            break;
        manager.draw();

        glfwSwapBuffers();
        if (!glfwGetWindowParam(GLFW_OPENED))
            break;

        dt = next_update - glfwGetTime();
        if (dt > 0.0) {
            glfwSleep(dt);
        }
    }
    return 0;
}