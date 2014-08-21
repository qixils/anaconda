#include "objects/assarray.h"
#include "objects/blowfish.cpp"
#include "fileio.h"
#include <iostream>
#include "stringcommon.h"

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

    std::string src;

    if (!read_file(filename.c_str(), src))
        return;

    std::string dst;

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

inline void encode_method(std::string & str, int method)
{
    for (std::string::iterator i = str.begin(); i != str.end(); ++i)
        *i = char(*i + method);
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

void AssociateArray::add_value(const std::string & key, int value)
{
    (*map)[key].value += value;
}

void AssociateArray::sub_value(const std::string & key, int value)
{
    (*map)[key].value -= value;
}

void AssociateArray::set_string(const std::string & key,
                                const std::string & value)
{
    (*map)[key].string = value;
}

int AssociateArray::get_value(const std::string & key)
{
    ArrayMap::const_iterator it = map->find(key);
    if (it == map->end())
        return 0;
    return it->second.value;
}

const std::string & AssociateArray::get_string(const std::string & key)
{
    ArrayMap::const_iterator it = map->find(key);
    if (it == map->end())
        return empty_string;
    return it->second.string;
}

void AssociateArray::clear()
{
    map->clear();
}

bool AssociateArray::has_key(const std::string & key)
{
    return map->find(key) != map->end();
}

bool AssociateArray::count_prefix(const std::string & key, int count)
{
    return count_prefix(key) >= count;
}

int AssociateArray::count_prefix(const std::string & key)
{
    int n = 0;
    ArrayMap::const_iterator it;
    for (it = map->begin(); it != map->end(); it++) {
        const std::string & other = it->first;
        if (other.compare(0, key.size(), key) == 0)
            n++;
    }
    return n;
}

void AssociateArray::remove_key(const std::string & key)
{
    map->erase(key);
}

ArrayAddress AssociateArray::get_first()
{
    ArrayMap::const_iterator it;
    it = map->begin();
    if (it == map->end())
        return ArrayAddress();
    return ArrayAddress(it);
}

ArrayAddress AssociateArray::get_prefix(const std::string & prefix, int index,
                                        ArrayAddress start)
{
    ArrayMap::const_iterator it;
    if (start.null)
        it = map->begin();
    else
        it = start.it;

    for (; it != map->end(); it++) {
        const std::string & other = it->first;
        if (other.compare(0, prefix.size(), prefix) != 0)
            continue;
        if (index == 0)
            return ArrayAddress(it);
        index--;
    }
    return ArrayAddress();
}

const std::string & AssociateArray::get_key(ArrayAddress addr)
{
    if (addr.null)
        return empty_string;
    return addr.it->first;
}

void AssociateArray::save(BaseStream & stream, int method)
{
    stream.write(ARRAY_MAGIC, sizeof(ARRAY_MAGIC)-1);

    ArrayMap::iterator it;
    for (it = map->begin(); it != map->end(); it++) {
        AssociateArrayItem & item = it->second;
        std::string key = it->first;
        encode_method(key, method);
        std::string string = item.string;
        encode_method(string, method);
        std::string value = number_to_string(item.value);
        encode_method(value, method);

        int len = key.size() + string.size() + value.size();
        stream.write_string(number_to_string(len));
        stream << ' ';
        stream.write_string(key);
        stream << '\x00';
        stream.write_string(string);
        stream << '\x00';
        stream.write_string(value);
    }
}

void AssociateArray::save(const std::string & path, int method)
{
    FSFile fp(path.c_str(), "w");
    FileStream stream(fp);
    save(stream, method);
    fp.close();
}

void AssociateArray::save_encrypted(const std::string & path, int method)
{
    std::stringstream ss;
    DataStream stream(ss);
    save(stream, method);
    std::string src = ss.str();
    std::string dst;
    cipher.encrypt(&dst, src);

    FSFile fp(path.c_str(), "w");
    fp.write(&dst[0], dst.size());
    fp.close();
}

ArrayMap AssociateArray::global_map;
