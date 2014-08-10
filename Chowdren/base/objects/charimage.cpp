#include "objects/charimage.h"
#include "include_gl.h"
#include "collision.h"

CharacterImageObject::CharacterImageObject(int x, int y, int type_id)
: FrameObject(x, y, type_id)
{
    collision = new InstanceBox(this);
}

CharacterImageObject::~CharacterImageObject()
{
    delete collision;
}

void CharacterImageObject::draw()
{
}

void CharacterImageObject::set_text(const std::string & text)
{
    this->text = text;
}

std::string CharacterImageObject::get_char(int index)
{
    return empty_string;
}

int CharacterImageObject::get_char_width(int alias, const std::string & c)
{
    return 0;
}
