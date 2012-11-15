#ifndef CHOWDREN_PATH_H
#define CHOWDREN_PATH_H

#include <string>
#include <algorithm>
#include <platformstl/platformstl.hpp>
#include <platformstl/filesystem/directory_functions.hpp>
#include <platformstl/filesystem/path.hpp>

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

inline void create_directories(const std::string & value)
{
    platformstl::path path(value);
    path.pop();
    platformstl::create_directory_recurse(path);
}

#endif // CHOWDREN_PATH_H