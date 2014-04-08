#ifndef CHOWDREN_DATASTREAM_H
#define CHOWDREN_DATASTREAM_H

#include <sstream>
#include "platform.h"
#include <string.h>

class BaseStream
{
public:
    BaseStream()
    {

    }

    void read_string(std::string & str, size_t len)
    {
        str.resize(len, 0);
        read(&str[0], len);
    }

    void read(std::stringstream & out, size_t len)
    {
        std::string data;
        read_string(data, len);
        out << data;
    }

    BaseStream & operator>>(char &v)
    {
        if (!ensure_size(1))
            v = 0;
        else
            read(&v, 1);
        return *this;
    }

    BaseStream & operator>>(int &v)
    {
        if (!ensure_size(4)) {
            v = 0;
            return *this;
        }
        unsigned char data[4];
        read((char*)data, 4);
        v = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
        return *this;
    }

    BaseStream & operator>>(float &v)
    {
        int vv;
        *this >> vv;
        memcpy(&v, &vv, sizeof(float));
        return *this;
    }

    BaseStream & operator>>(short &v)
    {
        if (!ensure_size(2)) {
            v = 0;
            return *this;
        }
        unsigned char data[2];
        read((char*)data, 2);
        v = data[0] | (data[1] << 8);
        return *this;
    }

    BaseStream & operator>>(std::string & str)
    {
        read_delim(str, '\0');
        return *this;
    }

    inline bool ensure_size(size_t size)
    {
        return true;
    }

    inline BaseStream & operator>>(unsigned int &v)
    {
        return *this >> reinterpret_cast<int&>(v);
    }

    inline BaseStream & operator>>(unsigned char &v)
    {
        return *this >> reinterpret_cast<char&>(v);
    }

    inline BaseStream & operator>>(unsigned short &v)
    {
        return *this >> reinterpret_cast<short&>(v);
    }

    // subclasses implements this

    virtual void read(char * data, size_t len) = 0;
    virtual void read_delim(std::string & str, char delim) = 0;
    virtual void seek(size_t pos) = 0;
    virtual bool at_end() = 0;
};

class FileStream : public BaseStream
{
public:
    FSFile & fp;

    FileStream(FSFile & fp)
    : BaseStream(), fp(fp)
    {
    }

    void read(char * data, size_t len)
    {
        fp.read(data, len);
    }

    void read_delim(std::string & str, char delim)
    {
        fp.read_delim(str, delim);
    }

    void seek(size_t pos)
    {
        fp.seek(pos);
    }

    bool at_end()
    {
        return fp.at_end();
    }
};

class DataStream : public BaseStream
{
public:
    std::stringstream & stream;

    DataStream(std::stringstream & stream)
    : stream(stream)
    {
    }

    void read(char * data, size_t len)
    {
        stream.read(data, len);
    }

    void read_delim(std::string & str, char delim)
    {
        std::getline(stream, str, delim);
    }

    void seek(size_t pos)
    {
        stream.seekg(pos);
    }

    bool at_end()
    {
        return stream.peek() == EOF;
    }
};

#endif // CHOWDREN_DATASTREAM_H
