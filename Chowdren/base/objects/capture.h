#ifndef CHOWDREN_CAPTURE_H
#define CHOWDREN_CAPTURE_H

#include <string>

class CaptureObject
{
public:
    static std::string filename;

    static void set_filename(const std::string & filename);
    static void capture_frame();
    static void capture_window();
    static void set_origin(int x, int y);
    static void set_size(int w, int h);
};

#endif // CHOWDREN_CAPTURE_H
