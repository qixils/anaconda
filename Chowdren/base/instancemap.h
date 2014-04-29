#ifndef CHOWDREN_INSTANCEMAP_H
#define CHOWDREN_INSTANCEMAP_H

#include "chowconfig.h"
#include "frameobject.h"

template <int size>
class BaseMap
{
public:
    ObjectList items[size];

    BaseMap()
    {
    }

    void clear()
    {
        for (unsigned int i = 0; i < size; i++) {
            items[i].clear();
        }
    }
};

typedef BaseMap<MAX_OBJECT_ID> InstanceMap;

#endif // CHOWDREN_INSTANCEMAP_H
