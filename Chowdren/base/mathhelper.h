#ifndef CHOWDREN_MATHHELPER_H
#define CHOWDREN_MATHHELPER_H

#include <boost/type_traits/common_type.hpp>
#include "dynnum.h"

struct MathHelper
{
    double lhs;
    int lhs_int;
};

template <class T>
class DivideHelper
{
public:
    T lhs;

    DivideHelper(T lhs)
    : lhs(lhs)
    {
    }
};

// Divide Helper

template <class T, class U>
inline typename boost::common_type<T, U>::type
operator/(const DivideHelper<T> & lhs, U rhs)
{
    if (rhs == 0)
        return 0;
    return lhs.lhs / rhs;
}

#ifdef CHOWDREN_USE_DYNAMIC_NUMBER

template <class T>
inline DynamicNumber operator/(const DivideHelper<T> & lhs, DynamicNumber rhs)
{
    if (rhs == 0)
        return 0;
    return DynamicNumber(lhs.lhs) / rhs;
}

template <class T>
inline DynamicNumber operator/(const DivideHelper<DynamicNumber> & lhs, T rhs)
{
    if (rhs == 0)
        return 0;
    return lhs.lhs / DynamicNumber(rhs);
}

inline DynamicNumber operator/(const DivideHelper<DynamicNumber> & lhs,
                               DynamicNumber rhs)
{
    if (rhs == 0)
        return 0;
    return lhs.lhs / rhs;
}

#endif

// safe divide

template <class T>
inline DivideHelper<T> operator/(T lhs, MathHelper& rhs)
{
    return DivideHelper<T>(lhs);
}

#ifdef CHOWDREN_USE_DYNAMIC_NUMBER

inline DivideHelper<DynamicNumber> operator/(DynamicNumber lhs,
                                             MathHelper& rhs)
{
    return DivideHelper<DynamicNumber>(lhs);
}

#endif

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

#ifdef CHOWDREN_USE_DYNAMIC_NUMBER

inline MathHelper & operator*(DynamicNumber lhs, MathHelper& rhs)
{
    rhs.lhs = double(lhs);
    return rhs;
}

inline double operator*(const MathHelper& lhs, DynamicNumber rhs)
{
    return pow(lhs.lhs, double(rhs));
}

#endif

// float modulus

template <class T>
class ModulusHelper
{
public:
    T lhs;

    ModulusHelper(T lhs)
    : lhs(lhs)
    {
    }
};

// safe divide

inline double get_mod(double a, double b)
{
    return fmod(a, b);
}

inline float get_mod(float a, float b)
{
    return fmod(a, b);
}

inline int get_mod(int a, int b)
{
    return a % b;
}

inline unsigned int get_mod(unsigned int a, unsigned int b)
{
    return a % b;
}

template <class T, class U>
inline typename boost::common_type<T, U>::type
operator%(const ModulusHelper<T> & lhs, U rhs)
{
    if (rhs == 0)
        return 0;
    return get_mod((typename boost::common_type<T, U>::type)(lhs.lhs),
                   (typename boost::common_type<T, U>::type)(rhs));
}

template <class T>
inline ModulusHelper<T> operator%(T lhs, MathHelper& rhs)
{
    return ModulusHelper<T>(lhs);
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
