#ifndef CHOWDREN_ASSARRAY_H
#define CHOWDREN_ASSARRAY_H

#include "frameobject.h"
#include "objects/blowfish.h"
#include "types.h"

class AssociateArrayItem
{
public:
    int value;
    std::string string;

    AssociateArrayItem()
    : value(0)
    {
    }
};

typedef hash_map<std::string, AssociateArrayItem> ArrayMap;

class AssociateArray : public FrameObject
{
public:
    Blowfish cipher;

    ArrayMap * map;
    static ArrayMap global_map;

    AssociateArray(int x, int y, int type_id);
    ~AssociateArray();
    void load_encrypted(const std::string & filename, int method);
    void load_data(const std::string & data, int method);
    void set_value(const std::string & key, int value);
    void set_string(const std::string & key, const std::string & value);
    int get_value(const std::string & key);
    const std::string & get_string(const std::string & key);
    void set_key(const std::string & key);
    void clear();
};

#endif // CHOWDREN_ASSARRAY_H
