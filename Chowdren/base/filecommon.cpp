#include <vector>

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
