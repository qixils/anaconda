#ifndef CHOWDREN_PATH_H
#define CHOWDREN_PATH_H

#include <string>
#include <algorithm>

inline std::string get_app_path()
{
    return "./";
}

inline std::string convert_path(std::string value)
{
#if defined(CHOWDREN_DEBUG) && defined(_WIN32)
    if (value.find('\\') != std::string::npos)
        __debugbreak();
#endif
    return value;
}

#endif // CHOWDREN_PATH_H