#include "objects/assarray.h"
#include "objects/blowfish.cpp"
#include "filecommon.h"
#include <iostream>

#define ARRAY_MAGIC "ASSBF1.0"

AssociateArray::AssociateArray(int x, int y, int type_id)
: FrameObject(x, y, type_id)
{
}

AssociateArray::~AssociateArray()
{
    if (map != &global_map)
        delete map;
}

void AssociateArray::set_key(const std::string & key)
{
    cipher.set_key(key);
}

void AssociateArray::load_encrypted(const std::string & filename,
                                    int method)
{
    clear();

    std::string dst;
    std::string src;

    read_file(filename.c_str(), src);
    cipher.decrypt(&dst, src);

    if (dst.compare(0, sizeof(ARRAY_MAGIC)-1, ARRAY_MAGIC) != 0) {
        std::cout << "Invalid magic for " << filename << std::endl;
        return;
    }

    load_data(dst.substr(sizeof(ARRAY_MAGIC)-1), method);
}

inline void decode_method(std::string & str, int method)
{
    for (std::string::iterator i = str.begin(); i != str.end(); ++i)
        *i = char(*i - method);
}

void AssociateArray::load_data(const std::string & data, int method)
{
    unsigned int pos = 0;

    while (pos < data.size()) {
        if (data[pos] == '\x00')
            // probably EOF, NULL-byte due to encryption padding
            break;
        int start = pos;

        while (data[pos] != ' ')
            pos++;
        int len = string_to_int(data.substr(start, pos-start));
        pos++;

        // read key
        start = pos;
        while (data[pos] != '\x00')
            pos++;
        std::string key = data.substr(start, pos-start);
        decode_method(key, method);
        pos++;
        len -= key.size();

        // read string
        start = pos;
        while (data[pos] != '\x00')
            pos++;
        std::string string = data.substr(start, pos-start);
        decode_method(string, method);
        pos++;
        len -= string.size();

        // read value
        std::string value = data.substr(pos, len);
        decode_method(value, method);
        pos += len;

        AssociateArrayItem & item = (*map)[key];
        item.value = string_to_int(value);
        item.string = string;
    }
}

void AssociateArray::set_value(const std::string & key, int value)
{
    (*map)[key].value = value;
}

void AssociateArray::set_string(const std::string & key,
                                const std::string & value)
{
    (*map)[key].string = value;
}

int AssociateArray::get_value(const std::string & key)
{
    return (*map)[key].value;
}

const std::string & AssociateArray::get_string(const std::string & key)
{
    return (*map)[key].string;
}

void AssociateArray::clear()
{
    map->clear();
}

ArrayMap AssociateArray::global_map;
