#ifndef CHOWDREN_PATH_H
#define CHOWDREN_PATH_H

#include <string>

inline std::string get_app_path()
{
    return "./";
}

inline std::string convert_path(std::string value)
{
#ifdef _WIN32
    // don't do anything on Windows yet
#else
    std::replace(value.begin(), value.end(), '\\', '/');
#endif
    return value;
}

#endif // CHOWDREN_PATH_H