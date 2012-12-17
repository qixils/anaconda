#include "include_gl.h"
#include <iostream>

#ifdef _WIN32
// apparently, QueryPerformanceCounter sucks on Windows. use timeGetTime!

#include "windows.h"

double chowdren_get_time()
{
    return timeGetTime() / 1000.0;
}

#else
#define chowdren_get_time glfwGetTime
#endif

class FPSLimiter
{
public:
    int framerate;
    double current_framerate;
    double old_time;
    double next_update;
    double dt;

    FPSLimiter() : framerate(-1)
    {
        old_time = chowdren_get_time();
#ifdef _WIN32
        timeBeginPeriod(1);
#endif
    }

    ~FPSLimiter()
    {
#ifdef _WIN32
        timeEndPeriod(1);
#endif
    }

    void set(int value)
    {
        framerate = value;
    }

    void start()
    {
        double current_time = chowdren_get_time();
        next_update = current_time + 1.0 / framerate;
        dt = current_time - old_time;
        old_time = current_time;
        if (dt < 0.0)
            dt = 0.001;
        current_framerate = 1.0 / dt;
    }

    void finish()
    {
        if (framerate >= 100)
            return;
        glfwSleep(next_update - chowdren_get_time());
    }

};