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
