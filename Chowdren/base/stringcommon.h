#ifndef STRING_H
#define STRING_H

#include <string>
#include <sstream>
#include <ctype.h>

inline double string_to_double(const std::string & in, double def = 0.0)
{
    std::istringstream input(in);
    double value;
    if (!(input >> value))
        return def;
    return value;
}

inline int string_to_int(const std::string & in, int def = 0.0)
{
    std::istringstream input(in);
    int value;
    if (!(input >> value))
        return def;
    return value;
}

inline const std::string & number_to_string(const std::string & value)
{
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

inline void replace_substring(std::string & str,
                              const std::string & from,
                              const std::string & to)
{
    if (from.empty())
        return;
    size_t start_pos = 0;
    while (true) {
        start_pos = str.find(from, start_pos);
        if (start_pos == std::string::npos)
            break;
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

#endif // STRING_H
