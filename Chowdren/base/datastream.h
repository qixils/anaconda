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

    // read

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

    // write

    BaseStream & operator<<(char v)
    {
        write(&v, 1);
        return *this;
    }

    BaseStream & operator<<(int v)
    {
        unsigned char data[4];
        data[0] = v & 0xFF;
        data[1] = (v >> 8) & 0xFF;
        data[2] = (v >> 16) & 0xFF;
        data[3] = (v >> 24) & 0xFF;
        write((char*)data, 4);
        return *this;
    }

    inline BaseStream & operator<<(unsigned int v)
    {
        return *this << (int)(v);
    }

    inline BaseStream & operator<<(unsigned char v)
    {
        return *this << (char)(v);
    }

    // subclasses implements this

    virtual void write(char * data, size_t len) = 0;
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

    void write(char * data, size_t len)
    {
        fp.write(data, len);
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

    void write(char * data, size_t len)
    {
        stream.write(data, len);
    }
};

#endif // CHOWDREN_DATASTREAM_H
