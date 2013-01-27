#ifdef _WIN32

#include <windows.h>
#include <shellapi.h>

inline void open_url(const std::string & name)
{
    ShellExecute(NULL, "open", name.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

#elif __APPLE__

inline void open_url(const std::string & name)
{
    CFStringRef str = CFStringCreateWithCString(0, url.c_str(), 0);
    CFURLRef ref = CFURLCreateWithString(0, str, 0);
    LSOpenCFURLRef(ref, 0);
    CFRelease(ref);
    CFRelease(str);
}

#elif __linux

#include <sys/types.h>
#include <dirent.h>

inline void open_url(const std::string & name)
{
    std::string cmd("xdg-open '");
    cmd += url;
    cmd += "'";
    system(cmd.c_str());
}

#endif