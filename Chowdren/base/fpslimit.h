#ifdef _WIN32
// apparently, QueryPerformanceCounter sucks on Windows. use timeGetTime!

#include "windows.h"
#include "include_gl.h"

inline int get_period(int framerate)
{
    int period = 50;
    if (framerate > 10) {
        period = 1000 / (2 * framerate);
        if (period < 1)
            period = 1;
    }
    return period;
};

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
    }

    ~FPSLimiter()
    {
        set(-1);
    }

    void set(int value)
    {
#ifdef _WIN32
        if (framerate != -1)
            timeEndPeriod(get_period(framerate));
        if (value != -1)
            timeBeginPeriod(get_period(value));
#endif
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
        while (next_update - chowdren_get_time() > 0.0)
            glfwSleep(0.001f);
    }

};