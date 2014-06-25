#ifndef CHOWDREN_MATHHELPER_H
#define CHOWDREN_MATHHELPER_H

struct MathHelper
{
    double lhs;
    int lhs_int;
};

template <class T>
class DivideHelper
{
public:
    const T & lhs;

    DivideHelper(const T & lhs)
    : lhs(lhs)
    {
    }
};

// Divide Helper

// double arg

template <class T>
inline double operator/(const DivideHelper<T> & lhs, double rhs)
{
    if (rhs == 0)
        return 0;
    return lhs.lhs / rhs;
}

// int arg

inline int operator/(const DivideHelper<int> & lhs, int rhs)
{
    if (rhs == 0)
        return 0;
    return lhs.lhs / rhs;
}

inline double operator/(const DivideHelper<double> & lhs, int rhs)
{
    if (rhs == 0)
        return 0;
    return lhs.lhs / rhs;
}

inline float operator/(const DivideHelper<float> & lhs, int rhs)
{
    if (rhs == 0)
        return 0;
    return lhs.lhs / rhs;
}

// float arg

inline float operator/(const DivideHelper<int> & lhs, float rhs)
{
    if (rhs == 0)
        return 0;
    return lhs.lhs / rhs;
}

inline double operator/(const DivideHelper<double> & lhs, float rhs)
{
    if (rhs == 0)
        return 0;
    return lhs.lhs / rhs;
}

inline float operator/(const DivideHelper<float> & lhs, float rhs)
{
    if (rhs == 0)
        return 0;
    return lhs.lhs / rhs;
}

// safe divide

template <class T>
inline DivideHelper<T> operator/(const T & lhs, MathHelper& rhs)
{
    return DivideHelper<T>(lhs);
}

// power

inline MathHelper & operator*(double lhs, MathHelper& rhs)
{
    rhs.lhs = lhs;
    return rhs;
}

inline double operator*(const MathHelper& lhs, double rhs)
{
    return pow(lhs.lhs, rhs);
}

// float modulus

inline MathHelper & operator%(double lhs, MathHelper& rhs)
{
    rhs.lhs = lhs;
    return rhs;
}

inline double operator%(const MathHelper& lhs, double rhs)
{
    if (rhs == 0.0)
        return 0.0;
    return fmod(lhs.lhs, rhs);
}

// bitwise AND

inline MathHelper & operator&(int lhs, MathHelper& rhs)
{
    rhs.lhs_int = lhs;
    return rhs;
}

inline int operator&(const MathHelper& lhs, int rhs)
{
    return lhs.lhs_int & rhs;
}

// bitwise OR

inline MathHelper & operator|(int lhs, MathHelper& rhs)
{
    rhs.lhs_int = lhs;
    return rhs;
}

inline int operator|(const MathHelper& lhs, int rhs)
{
    return lhs.lhs_int | rhs;
}

// bitwise XOR

inline MathHelper & operator^(int lhs, MathHelper& rhs)
{
    rhs.lhs_int = lhs;
    return rhs;
}

inline int operator^(const MathHelper& lhs, int rhs)
{
    return lhs.lhs_int ^ rhs;
}

#endif // CHOWDREN_MATHHELPER_H
