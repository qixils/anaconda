#ifndef CHOWDREN_STRINGCOMMON_H
#define CHOWDREN_STRINGCOMMON_H

#include <string>
#include <sstream>
#include <ctype.h>
#include <vector>
#include <algorithm>

extern std::string empty_string;

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

inline void to_lower(std::string & str)
{
    std::transform(str.begin(), str.end(), str.begin(), tolower);
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

inline void split_string(const std::string & s, char delim,
                         std::vector<std::string> & elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

inline void split_string(const std::string & str, const std::string & delims,
                         std::vector<std::string> & elems)
{
    std::string::size_type last_pos = str.find_first_not_of(delims, 0);
    std::string::size_type pos = str.find_first_of(delims, last_pos);

    while (std::string::npos != pos || std::string::npos != last_pos) {
        elems.push_back(str.substr(last_pos, pos - last_pos));
        last_pos = str.find_first_not_of(delims, pos);
        pos = str.find_first_of(delims, last_pos);
    }
}

#endif // CHOWDREN_STRINGCOMMON_H
