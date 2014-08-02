#ifndef GLOBALS_H
#define GLOBALS_H

#include "types.h"
#include "stringcommon.h"
#include <string>

class GlobalValues
{
public:
    vector<double> values;

    GlobalValues()
    {
    }

    double get(size_t index)
    {
        if (index >= values.size())
            return 0.0;
        return values[index];
    }

    int get_int(size_t index)
    {
        if (index >= values.size())
            return 0;
        return int(values[index]);
    }

    void set(size_t index, double value)
    {
        if (index >= values.size()) {
            values.resize(index + 1);
        }
        values[index] = value;
    }

    void add(size_t index, double value)
    {
        set(index, get(index) + value);
    }

    void sub(size_t index, double value)
    {
        set(index, get(index) - value);
    }
};

class GlobalStrings
{
public:
    vector<std::string> values;

    GlobalStrings()
    {
    }

    const std::string & get(size_t index)
    {
        if (index >= values.size()) {
            return empty_string;
        }
        return values[index];
    }

    void set(size_t index, const std::string & value)
    {
        if (index >= values.size())
            values.resize(index + 1);
        values[index] = value;
    }
};

#endif // GLOBALS_H
