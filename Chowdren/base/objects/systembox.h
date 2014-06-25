#ifndef CHOWDREN_SYSTEMBOX_H
#define CHOWDREN_SYSTEMBOX_H

#include "frameobject.h"
#include <string>

#define PATTERN_IMAGE 0
#define CENTER_IMAGE 1
#define TOPLEFT_IMAGE 2

class SystemBox : public FrameObject
{
public:
    Image * image;
    int type;
    std::string text;

    SystemBox(int x, int y, int type_id);
    ~SystemBox();
    void set_text(const std::string & text);
    void set_size(int w, int h);
    void draw();
};

#endif // CHOWDREN_SYSTEMBOX_H
