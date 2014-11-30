#ifndef CHOWDREN_ARRAYEXT_H
#define CHOWDREN_ARRAYEXT_H

#include "frameobject.h"
#include <string>
#include "datastream.h"
#include "types.h"

class ArrayObject : public FrameObject
{
public:
    FRAMEOBJECT_HEAD(ArrayObject)

    bool is_numeric;
    int offset;
    double * array;
    std::string * strings;
    int x_size, y_size, z_size;
    int x_pos, y_pos, z_pos;

    ArrayObject(int x, int y, int type_id);
    ~ArrayObject();
    void initialize(bool is_numeric, int offset, int x, int y, int z);
    void clear();
    double get_value(int x=-1, int y=-1, int z=-1);
    const std::string & get_string(int x=-1, int y=-1, int z=-1);
    void set_value(double value, int x=-1, int y=-1, int z=-1);
    void set_string(const std::string & value, int x=-1, int y=-1, int z=-1);
    void load(const std::string & filename);
    void save(const std::string & filename);
    void expand(int x, int y, int z);

    inline void adjust_pos(int & x, int & y, int & z)
    {
        if (x == -1)
            x = x_pos;
        if (y == -1)
            y = y_pos;
        if (z == -1)
            z = z_pos;
        x -= offset;
        y -= offset;
        z -= offset;
    }

    inline int get_index(int x, int y, int z)
    {
        return x + y * x_size + z * x_size * y_size;
    }

    inline bool is_valid(int x, int y, int z)
    {
        return x >= 0 && y >= 0 && z >= 0 &&
               x < x_size && y < y_size && z < z_size;
    }
};

#endif // CHOWDREN_ARRAYEXT_H
