#ifndef CHOWDREN_MATH_H
#define CHOWDREN_MATH_H

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

#endif // CHOWDREN_MATH_H