#ifndef CHOWDREN_STRINGPARSER_H
#define CHOWDREN_STRINGPARSER_H

#include <string>
#include <vector>
#include "frameobject.h"

class StringParser : public FrameObject
{
public:
    std::vector<std::string> elements;
    std::string delimiters;
    std::string value;
    bool has_split;

    StringParser(int x, int y, int id);
    void split();
    void load(const std::string & filename);
    void set(const std::string & value);
    void add_delimiter(const std::string & delim);
    const std::string & get_element(int index);
    std::string replace(const std::string & from, const std::string & to);
};

#endif // CHOWDREN_STRINGPARSER_H
