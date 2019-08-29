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
