#ifndef CHOWDREN_PATH_H
#define CHOWDREN_PATH_H

#include <string>
#include <algorithm>
// for some reason, "unix" is not defined on OS X
#ifdef __APPLE__
#define unix
#endif
#include <platformstl/platformstl.hpp>
#include <platformstl/filesystem/path.hpp>
#include <platformstl/filesystem/directory_functions.hpp>

inline std::string get_app_path()
{
    return "./";
}

inline std::string convert_path(std::string value)
{
#ifndef _WIN32
    std::replace(value.begin(), value.end(), '\\', '/');
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