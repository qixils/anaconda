#include "objects/stringparser.h"
#include "stringcommon.h"
#include "filecommon.h"

StringParser::StringParser(int x, int y, int id)
: FrameObject(x, y, id), has_split(false)
{

}

void StringParser::add_delimiter(const std::string & v)
{
    if (v.size() != 1) {
        std::cout << "Delimiter size " << v.size() << " not supported"
            << std::endl;
        return;
    }
    delimiters += v;
}

void StringParser::load(const std::string & filename)
{
    read_file(filename.c_str(), value);
}

void StringParser::split()
{
    if (has_split)
        return;
    split_string(value, delimiters, elements);
    has_split = true;
}

void StringParser::set(const std::string & v)
{
    value = v;
    has_split = false;
}

const std::string & StringParser::get_element(int i)
{
    split();
    static std::string empty_string("");
    if (i < 0 || i >= int(elements.size()))
        return empty_string;
    return elements[i];
}
