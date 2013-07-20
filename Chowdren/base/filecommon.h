#ifndef CHOWDREN_FILECOMMON_H
#define CHOWDREN_FILECOMMON_H

#include <iostream>
#include "platform.h"

inline void read_file(const char * filename, char ** data, size_t * ret_size,
                      bool binary = true)
{
    FSFile fp(filename, "r");
    if (!fp.is_open()) {
        std::cout << "Could not load file " << filename << std::endl;
        return;
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
}

#endif // CHOWDREN_FILECOMMON_H
