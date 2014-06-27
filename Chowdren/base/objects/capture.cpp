#include "objects/capture.h"

void CaptureObject::set_filename(const std::string & filename)
{
    this->filename = filename;
}

void CaptureObject::capture_frame()
{
    std::cout << "Capture frame not implemented" << std::endl;
}

void CaptureObject::capture_window()
{
    std::cout << "Capture window not implemented" << std::endl;
}

void CaptureObject::set_size(int w, int h)
{
    std::cout << "Capture set size not implemented: " << w << " " << h
        << std::endl;
}

void CaptureObject::set_origin(int x, int y)
{
    std::cout << "Capture set pos not implemented: " << x << " " << y
        << std::endl;
}
