#include "objects/stringreplace.h"
#include "stringcommon.h"

std::string StringReplace::replace(std::string src,
                                   const std::string & from,
                                   const std::string & to)
{
    replace_substring(src, from, to);
    return src;
}
