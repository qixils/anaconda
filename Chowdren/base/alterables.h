#ifndef ALTERABLES_H
#define ALTERABLES_H

template <class T, size_t B>
class Alterables
{
public:
    T values[B];

    Alterables()
    {
        for (int i = 0; i < B; i++) {
            values[i] = T();
        }
    }

    T get(size_t index, T def = T())
    {
        if (index < 0 || index >= B)
            return def;
        return values[index];
    }

    void set(size_t index, T value)
    {
        if (index < 0 || index >= B)
            return;
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

typedef Alterables<double, 26> AlterableValues;
typedef Alterables<std::string, 10> AlterableStrings;

#endif // ALTERABLES_H