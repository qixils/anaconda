#include "objects/listext.h"
#include "chowconfig.h"

// ListObject

ListObject::ListObject(int x, int y, int type_id)
: FrameObject(x, y, type_id)
{

}

void ListObject::load_file(const std::string & name)
{
    std::string data;
    if (!read_file(name.c_str(), data))
        return;
    StringStream ss(data);
    std::string line;
    while (!ss.at_end()) {
        ss.read_line(line);
        add_line(line);
    }
}

void ListObject::add_line(const std::string & value)
{
    lines.push_back(value);
}

const std::string & ListObject::get_line(int i)
{
    return lines[i];
}

int ListObject::get_count()
{
    return lines.size();
}
