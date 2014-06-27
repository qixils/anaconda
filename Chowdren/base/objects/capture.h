#ifndef CHOWDREN_CAPTURE_H
#define CHOWDREN_CAPTURE_H

#include <string>

class CaptureObject
{
public:
    std::string filename;

    void set_filename(const std::string & filename);
    void capture_frame();
    void capture_window();
    void set_origin(int x, int y);
    void set_size(int w, int h);
};

#endif // CHOWDREN_CAPTURE_H
