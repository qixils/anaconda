#ifndef CHOWDREN_BITARRAY_H
#define CHOWDREN_BITARRAY_H

#ifdef _WIN32
#define alloca _alloca
#else
#include <alloca.h>
#endif

#include <string.h>

class BaseBitArray
{
public:
    typedef unsigned long word_t;
    enum {
        WORD_SIZE = sizeof(word_t) * 8
    };

    word_t * data;

    BaseBitArray(void * data)
    : data((word_t*)data)
    {
    }

    unsigned long get(int index)
    {
        int i = index / WORD_SIZE;
        return data[i] & (1 << (index - i));
    }

    void set(int index)
    {
        int i = index / WORD_SIZE;
        data[i] |= 1 << (index - i);
    }

    void unset(int index)
    {
        int i = index / WORD_SIZE;
        data[i] &= ~(1 << (index - i));
    }
};

#define GET_BITARRAY_ITEMS(N) ((N) / BaseBitArray::WORD_SIZE + 1)
#define GET_BITARRAY_SIZE(N) (sizeof(BaseBitArray::word_t) *\
                              GET_BITARRAY_ITEMS(N))

class StackBitArray : public BaseBitArray
{
public:
    StackBitArray(void * data, int n)
    : BaseBitArray(data)
    {
        memset(data, 0, n);
    }
};

#define CREATE_BITARRAY_ZERO(N) StackBitArray(alloca(GET_BITARRAY_SIZE(N)),\
                                              GET_BITARRAY_SIZE(N))

#endif // CHOWDREN_BITARRAY_H
