#ifndef CHOWDREN_FILECOMMON_H
#define CHOWDREN_FILECOMMON_H

#include <fstream>
#include <sys/stat.h>

inline void read_file(const char * filename, char ** data, size_t * ret_size,
                      bool binary = true)
{
    std::ifstream fp;
    fp.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (!fp) {
        std::cout << "Could not load file " << filename << std::endl;
        return;
    }

    size_t size = fp.tellg(); 
    fp.seekg(0, std::ios::beg);
    if (binary)
        *data = new char[size];
    else
        *data = new char[size + 1];
    fp.read(*data, size);
    fp.close();
    if (!binary)
        (*data)[size] = 0;
    *ret_size = size;
}

inline size_t get_file_size(const char * filename)
{
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

#endif // CHOWDREN_FILECOMMON_H