#include "objects/arrayext.h"
#include "chowconfig.h"
#include "fileio.h"
#include "datastream.h"
#include <algorithm>
#include "stringcommon.h"

// ArrayObject

ArrayObject::ArrayObject(int x, int y, int type_id)
: FrameObject(x, y, type_id), array(NULL), strings(NULL)
{

}

void ArrayObject::initialize(bool numeric, int offset, int x, int y, int z)
{
    this->offset = offset;
    is_numeric = numeric;
    x_size = x;
    y_size = y;
    z_size = z;
    x_pos = y_pos = z_pos = offset;
    clear();
}

#define CT_ARRAY_MAGIC "CNC ARRAY"
#define ARRAY_MAJOR_VERSION 2
#define ARRAY_MINOR_VERSION 0

#define NUMERIC_FLAG 1
#define TEXT_FLAG 2
#define BASE1_FLAG 4

void ArrayObject::load(const std::string & filename)
{
    FSFile fp(convert_path(filename).c_str(), "r");
    if (!fp.is_open()) {
        std::cout << "Could not load array " << filename << std::endl;
        return;
    }

    FileStream stream(fp);

    std::string magic;
    stream.read_string(magic, sizeof(CT_ARRAY_MAGIC));

    if (magic != CT_ARRAY_MAGIC) {
        std::cout << "Invalid CT_ARRAY_MAGIC" << std::endl;
        return;
    }

    if (stream.read_int16() != ARRAY_MAJOR_VERSION) {
        std::cout << "Invalid ARRAY_MAJOR_VERSION" << std::endl;
        return;
    }

    if (stream.read_int16() != ARRAY_MINOR_VERSION) {
        std::cout << "Invalid ARRAY_MINOR_VERSION" << std::endl;
        return;
    }

    x_size = stream.read_int32();
    y_size = stream.read_int32();
    z_size = stream.read_int32();

    int flags = stream.read_int32();
    is_numeric = (flags & NUMERIC_FLAG) != 0;
    offset = int((flags & BASE1_FLAG) != 0);

    delete[] array;
    delete[] strings;
    array = NULL;
    strings = NULL;
    clear();

    for (int i = 0; i < x_size * y_size * z_size; i++) {
        if (is_numeric) {
            array[i] = double(stream.read_int32());
        } else {
            stream.read_string(strings[i], stream.read_int32());
        }
    }

    fp.close();
}

void ArrayObject::save(const std::string & filename)
{
    std::cout << "Array object save not implemented: " << filename
        << std::endl;
}

double ArrayObject::get_value(int x, int y, int z)
{
    adjust_pos(x, y, z);
    if (!is_valid(x, y, z))
        return 0;
    return array[get_index(x, y, z)];
}

const std::string & ArrayObject::get_string(int x, int y, int z)
{
    adjust_pos(x, y, z);
    if (!is_valid(x, y, z))
        return empty_string;
    return strings[get_index(x, y, z)];
}

void ArrayObject::set_value(double value, int x, int y, int z)
{
    adjust_pos(x, y, z);
    expand(x, y, z);
    array[get_index(x, y, z)] = value;
}

void ArrayObject::set_string(const std::string & value, int x, int y, int z)
{
    adjust_pos(x, y, z);
    expand(x, y, z);
    strings[get_index(x, y, z)] = value;
}

void ArrayObject::expand(int x, int y, int z)
{
    x = std::max(x+1, x_size);
    y = std::max(y+1, y_size);
    z = std::max(z+1, z_size);
    if (x == x_size && y == y_size && z == z_size)
        return;

#ifndef NDEBUG
    // probably good to signify that something should be preallocated
    std::cout << "Expanding " << get_name() << "from " <<
        "(" << x_size << ", " << y_size << ", " << z_size << ")" <<
        " to" <<
        "(" << x << ", " << y << ", " << z << ")" << std::endl;
#endif

    int old_x = x_size;
    int old_y = y_size;
    int old_z = z_size;

    x_size = x;
    y_size = y;
    z_size = z;

    if (is_numeric) {
        double * old_array = array;
        array = NULL;
        clear();
        for (int x = 0; x < old_x; x++)
        for (int y = 0; y < old_y; y++)
        for (int z = 0; z < old_z; z++) {
            double value = old_array[x + y * old_x + z * old_x * old_y];
            array[get_index(x, y, z)] = value;
        }
        delete[] old_array;
    } else {
        std::string * old_array = strings;
        strings = NULL;
        clear();
        for (int x = 0; x < old_x; x++)
        for (int y = 0; y < old_y; y++)
        for (int z = 0; z < old_z; z++) {
            const std::string & value = old_array[x + y * old_x +
                                                  z * old_x * old_y];
            strings[get_index(x, y, z)] = value;
        }
        delete[] old_array;
    }
}

ArrayObject::~ArrayObject()
{
    if (is_numeric)
        delete[] array;
    else
        delete[] strings;
}

void ArrayObject::clear()
{
    if (is_numeric) {
        delete[] array;
        array = new double[x_size * y_size * z_size]();
    } else {
        delete[] strings;
        strings = new std::string[x_size * y_size * z_size];
    }
}
