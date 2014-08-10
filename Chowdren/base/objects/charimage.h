#ifndef CHOWDREN_CHARIMAGE_H
#define CHOWDREN_CHARIMAGE_H

#include "frameobject.h"
#include <string>

class CharacterImageObject : public FrameObject
{
public:
    std::string text;
    int x_off;

    CharacterImageObject(int x, int y, int type_id);
    ~CharacterImageObject();
    void set_text(const std::string & text);
    void draw();
    std::string get_char(int index);
    int get_char_width(int alias, const std::string & c);
};

#endif // CHOWDREN_CHARIMAGE_H
