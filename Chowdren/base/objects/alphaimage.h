#ifndef CHOWDREN_ALPHAIMAGE_H
#define CHOWDREN_ALPHAIMAGE_H

#include "frameobject.h"

/*
This object is actually never rendered, but Heart Forth, Alicia depends on
the storage of some values.
*/

class AlphaImageObject : public FrameObject
{
public:
    AlphaImageObject(int x, int y, int type_id);
};

#endif // CHOWDREN_ALPHAIMAGE_H
