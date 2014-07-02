#include "objects/backimages.h"

BackImages::BackImages(int x, int y, int type_id)
: FrameObject(x, y, type_id), x_off(0), y_off(0)
{
}

class DefaultInstance : public BackImages
{
public:
    DefaultInstance()
    : BackImages(0, 0, 0)
    {
        setup_default_instance(this);
        collision = new InstanceBox(this);
        create_alterables();
    }
};

static DefaultInstance default_backimages;
FrameObject * default_backimages_instance = &default_backimages;
