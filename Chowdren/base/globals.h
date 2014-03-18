#ifndef GLOBALS_H
#define GLOBALS_H

#include <vector>
#include <string>

class GlobalValues
{
public:
    std::vector<double> values;

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
    std::vector<std::string> values;

    GlobalStrings()
    {
    }

    const std::string & get(size_t index)
    {
        if (index >= values.size()) {
            static std::string empty;
            return empty;
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
