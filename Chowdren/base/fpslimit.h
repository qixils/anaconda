#ifndef CHOWDREN_FPSLIMIT_H
#define CHOWDREN_FPSLIMIT_H

#include "platform.h"
#include <iostream>
#include <algorithm>

#ifdef _WIN32
#include "windows.h"
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
        old_time = platform_get_time();
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
        double current_time = platform_get_time();
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
        platform_sleep(next_update - platform_get_time());
    }

};

#endif // CHOWDREN_FPSLIMIT_H
