#ifndef STRING_H
#define STRING_H

#include <string>
#include <sstream>
#include <ctype.h>

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

inline std::string to_lower(std::string str)
{
    for (std::string::iterator i = str.begin(); i != str.end(); ++i)
        *i = static_cast<char>(tolower(*i));
    return str;
}

#endif // STRING_H