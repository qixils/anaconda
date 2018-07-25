#ifndef CHOWDREN_LISTEXT_H
#define CHOWDREN_LISTEXT_H

#include "frameobject.h"
#include <string>
#include "datastream.h"
#include "types.h"

typedef vector<std::string> StringList;

class ListObject : public FrameObject
{
public:
    FRAMEOBJECT_HEAD(ListObject)

    StringList lines;

    ListObject(int x, int y, int type_id);
    void load_file(const std::string & name);
    void add_line(const std::string & value);
    const std::string & get_line(int i);
    int get_count();
};

#endif // CHOWDREN_LISTEXT_H
