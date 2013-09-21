#ifndef CHOWDREN_MATH_H
#define CHOWDREN_MATH_H

#define _USE_MATH_DEFINES
#include <math.h>

// math helpers

inline float mod(float a, float b)
{
    return a - b * floor(a / b);
}

template <class T>
inline T rad(T x)
{
    return x * (M_PI/180);
}

template <class T>
inline T deg(T x)
{
    return x * (180/M_PI);
}

inline double sin_deg(double x)
{
    return sin(rad(x));
}

inline double cos_deg(double x)
{
    return cos(rad(x));
}

inline double atan2d(double a, double b)
{
    return deg(atan2(a, b));
}

inline float get_length(float x, float y)
{
    return sqrt(x * x + y * y);
}

inline float get_distance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return get_length(dx, dy);
}

inline double get_direction(int x1, int y1, int x2, int y2)
{
    return atan2d(y2 - y1, x2 - x1) / -11.25;
}

inline int get_direction_int(int x1, int y1, int x2, int y2)
{
    return int(get_direction(x1, y1, x2, y2));
}

inline int int_round(double d)
{
    int v = (int)floor(d + 0.5);
    // MMF behaviour - don't ask me
    if (d - v > 0.5)
        v++;
    return v;
}

inline int int_min(int value1, int value2)
{
    return std::min<int>(value1, value2);
}

inline int int_max(int value1, int value2)
{
    return std::max<int>(value1, value2);
}

template <class T>
inline T clamp(T val)
{
    return std::max<T>(0, std::min<T>(val, 1));
}

inline int get_abs(int v)
{
    return abs(v);
}

inline float get_abs(float v)
{
    return fabs(v);
}

inline double get_abs(double v)
{
    return fabs(v);
}

#endif // CHOWDREN_MATH_H
