#ifndef CHOWDREN_PATH_H
#define CHOWDREN_PATH_H

#include <string>
#include <algorithm>

inline std::string get_app_path()
{
    return "./";
}

inline std::string get_app_drive()
{
    return "";
}

inline std::string get_app_dir()
{
    return get_app_path();
}

#endif // CHOWDREN_PATH_H
