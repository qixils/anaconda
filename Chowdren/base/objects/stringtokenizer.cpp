// Copyright (c) Mathias Kaerlev 2012-2015.
//
// This file is part of Anaconda.
//
// Anaconda is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Anaconda is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Anaconda.  If not, see <http://www.gnu.org/licenses/>.

#include "objects/stringtokenizer.h"
#include "stringcommon.h"

// StringTokenizer

StringTokenizer::StringTokenizer(int x, int y, int type_id)
: FrameObject(x, y, type_id)
{
}

void StringTokenizer::split(const std::string & text,
                            const std::string & delims)
{
    elements.clear();
    split_string(text, delims, elements);
}

const std::string & StringTokenizer::get(int index)
{
    if (index < 0 || index >= int(elements.size()))
        return empty_string;
    return elements[index];
}
