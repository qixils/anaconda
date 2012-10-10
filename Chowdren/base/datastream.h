#include <iostream>
#include <istream>

class DataStream
{
public:
    std::iostream & stream;
    bool big_endian;

    DataStream(std::iostream & stream, bool big_endian = false)
    : stream(stream), big_endian(big_endian)
    {}

    DataStream(std::ifstream & stream, bool big_endian = false)
    : stream((std::iostream&)stream), big_endian(big_endian)
    {}

    void read(std::string & str, size_t len)
    {
        str.resize(len, 0);
        stream.read(&str[0], len);
    }

    DataStream & operator>>(char &v)
    {
        if (!ensure_size(1))
            v = 0;
        else
            stream.read(&v, 1);
        return *this;
    }

    DataStream & operator>>(int &v)
    {
        if (!ensure_size(4)) {
            v = 0;
            return *this;
        }
        unsigned char data[4];
        stream.read((char*)data, 4);
        if (big_endian)
            v = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
        else
            v = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
        return *this;
    }

    DataStream & operator>>(std::string & str)
    {
        std::getline(stream, str, '\0');
        return *this;
    }

    inline bool ensure_size(size_t size)
    {
        return true;
    }

    inline bool eof()
    {
        return stream.eof();
    }

    inline DataStream & operator>>(unsigned int &v)
    {
        return *this >> reinterpret_cast<int&>(v);
    }

    inline DataStream & operator>>(unsigned char &v)
    {
        return *this >> reinterpret_cast<char&>(v);
    }
};