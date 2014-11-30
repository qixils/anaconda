#ifndef CHOWDREN_EDITEXT_H
#define CHOWDREN_EDITEXT_H

#include "frameobject.h"

class EditObject : public FrameObject
{
public:
    FRAMEOBJECT_HEAD(EditObject)

    EditObject(int x, int y, int type_id);
};

#endif // CHOWDREN_EDITEXT_H
