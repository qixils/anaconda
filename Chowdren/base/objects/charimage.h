#ifndef CHOWDREN_CHARIMAGE_H
#define CHOWDREN_CHARIMAGE_H

#include "frameobject.h"
#include <string>

class CharacterImageObject : public FrameObject
{
public:
    std::string text;

    CharacterImageObject(int x, int y, int type_id);
    ~CharacterImageObject();
    void set_text(const std::string & text);
    void draw();
};

#endif // CHOWDREN_CHARIMAGE_H
