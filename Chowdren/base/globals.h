#ifndef GLOBALS_H
#define GLOBALS_H

#include <vector>
#include <string>

template <class T>
class Globals
{
public:
    std::vector<T> values;

    Globals()
    {

    }

    T get(size_t index)
    {
        if (index < 0 || index >= values.size())
            return T();
        return values[index];
    }

    void set(size_t index, T value)
    {
        if (index < 0)
            return;
        if (index >= values.size())
            values.resize(index + 1);
        values[index] = value;
    }

    void add(size_t index, T value)
    {
        set(index, get(index) + value);
    }

    void sub(size_t index, T value)
    {
        set(index, get(index) - value);
    }
};

typedef Globals<double> GlobalValues;
typedef Globals<std::string> GlobalStrings;

#endif // GLOBALS_H