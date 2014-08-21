#include "fileio.h"

FSFile::FSFile()
: handle(NULL), closed(true)
{
}

FSFile::FSFile(const char * filename, const char * mode)
{
    open(filename, mode);
}

FSFile::~FSFile()
{
    close();
}

bool FSFile::is_open()
{
    return !closed;
}

void FSFile::read_delim(std::string & line, char delim)
{
    while (true) {
        int c = getc();
        if (c == EOF)
            break;
        if (c == delim)
            break;
        line += c;
    }
}

void FSFile::read_line(std::string & line)
{
    return read_delim(line, '\n');
}

// stdio I/O wrappers

extern "C" {

void * fsfile_fopen(const char * filename, const char * mode)
{
    return (void*)(new FSFile(filename, mode));
}

int fsfile_fclose(void * fp)
{
    ((FSFile*)fp)->close();
    return 0;
}

int fsfile_fseek(void * fp, long int offset, int origin)
{
    return int(((FSFile*)fp)->seek(offset, origin));
}

size_t fsfile_fread(void * ptr, size_t size, size_t count, void * fp)
{
    return ((FSFile*)fp)->read(ptr, size*count);
}

long int fsfile_ftell(void * fp)
{
    return ((FSFile*)fp)->tell();
}

}

// convenience functions

bool read_file(const char * filename, char ** data, size_t * ret_size,
               bool binary)
{
    FSFile fp(filename, "r");
    if (!fp.is_open()) {
        std::cout << "Could not load file " << filename << std::endl;
        return false;
    }
    fp.seek(0, SEEK_END);
    size_t size = fp.tell();
    fp.seek(0, SEEK_SET);
    if (binary)
        *data = new char[size];
    else
        *data = new char[size + 1];
    fp.read(*data, size);
    fp.close();
    if (!binary)
        (*data)[size] = 0;
    *ret_size = size;
    return true;
}

bool read_file(const char * filename, std::string & dst, bool binary)
{
    FSFile fp(filename, "r");
    if (!fp.is_open()) {
        std::cout << "Could not load file " << filename << std::endl;
        return false;
    }
    fp.seek(0, SEEK_END);
    size_t size = fp.tell();
    fp.seek(0, SEEK_SET);
    if (binary)
        dst.resize(size);
    else
        dst.resize(size + 1);
    fp.read(&dst[0], size);
    fp.close();
    if (!binary)
        dst[size] = 0;
    return true;
}

bool read_file_c(const char * filename, char ** data, size_t * ret_size,
                 bool binary)
{
    FSFile fp(filename, "r");
    if (!fp.is_open()) {
        std::cout << "Could not load file " << filename << std::endl;
        return false;
    }
    fp.seek(0, SEEK_END);
    size_t size = fp.tell();
    fp.seek(0, SEEK_SET);
    if (binary)
        *data = (char*)malloc(size);
    else
        *data = (char*)malloc(size+1);
    fp.read(*data, size);
    fp.close();
    if (!binary)
        (*data)[size] = 0;
    *ret_size = size;
    return true;
}
