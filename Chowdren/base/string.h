#ifndef STRING_H
#define STRING_H

#include <string>
#include <sstream>

inline double string_to_double(std::string in, double def = 0.0)
{
    std::istringstream input(in);
    double value;
    if (!(input >> value))
        return def;
    return value;
}

template <class T>
inline std::string number_to_string(T value)
{
    std::ostringstream input;
    input << value;
    return input.str();
}

#endif // STRING_H