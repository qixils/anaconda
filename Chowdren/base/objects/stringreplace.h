#ifndef CHOWDREN_STRINGREPLACE_H
#define CHOWDREN_STRINGREPLACE_H

#include <string>

class StringReplace
{
public:
    static std::string replace(std::string src,
                               const std::string & from,
                               const std::string & to);
};

#endif // CHOWDREN_STRINGREPLACE_H
