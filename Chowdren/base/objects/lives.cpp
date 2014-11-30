#include "objects/lives.h"

// Lives

Lives::Lives(int x, int y, int type_id)
: FrameObject(x, y, type_id), flash_interval(0.0f)
{
    collision = new InstanceBox(this);
}

Lives::~Lives()
{
    delete collision;
}

void Lives::flash(float value)
{
    flash_interval = value;
    flash_time = 0.0f;
}

void Lives::update()
{
    update_flash(flash_interval, flash_time);
}

void Lives::draw()
{
    blend_color.apply();

    int xx = x;
    int i = 0;
    while (i < manager.lives) {
        image->draw(xx, y);
        xx += image->width;
        i++;
    }
}
