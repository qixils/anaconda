#include "objects/systembox.h"
#include "include_gl.h"
#include "collision.h"
#include <iostream>

SystemBox::SystemBox(int x, int y, int type_id)
: FrameObject(x, y, type_id)
{
    collision = new InstanceBox(this);
}

SystemBox::~SystemBox()
{
    delete collision;
}

void SystemBox::draw()
{
    if (type == PATTERN_IMAGE) {
        glEnable(GL_SCISSOR_TEST);
        glc_scissor_world(x, y, width, height);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        for (int xx = x; xx < x + width; xx += image->width)
        for (int yy = y; yy < y + height; yy += image->height) {
            draw_image(image, xx + image->hotspot_x, yy + image->hotspot_y);
        }
        glDisable(GL_SCISSOR_TEST);
    } else if (type == CENTER_IMAGE) {
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        int xx = x + width / 2 - image->width / 2;
        int yy = y + height / 2 - image->height / 2;
        draw_image(image, xx + image->hotspot_x, yy + image->hotspot_y);
    } else if (type == TOPLEFT_IMAGE) {
        draw_image(image, x + image->hotspot_x, y + image->hotspot_y);
    }
}

void SystemBox::hide_border_1()
{
    // std::cout << "Hide System Box border 1 not implemented" << std::endl;
}

void SystemBox::hide_border_2()
{
    // std::cout << "Hide System Box border 1 not implemented" << std::endl;
}

void SystemBox::hide_fill()
{
    // std::cout << "Hide System Box fill not implemented" << std::endl;
}

void SystemBox::set_size(int w, int h)
{
    width = w;
    height = h;
    collision->update_aabb();
}

void SystemBox::set_text(const std::string & text)
{
    this->text = text;
}

void SystemBox::set_border_1(Color color)
{

}

void SystemBox::set_border_2(Color color)
{

}

class DefaultInstance : public SystemBox
{
public:
    DefaultInstance()
    : SystemBox(0, 0, 0)
    {
        setup_default_instance(this);
        collision = new InstanceBox(this);
        create_alterables();
    }
};

static DefaultInstance default_systembox;
FrameObject * default_systembox_instance = &default_systembox;