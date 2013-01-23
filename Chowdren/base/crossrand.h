#ifndef CROSSRAND_H
#define CROSSRAND_H

#define CROSS_RAND_MAX 0x7FFF

// portable rand functions

static unsigned int seed = 0;
inline void cross_srand(unsigned int value)
{
    seed = value;
}

inline unsigned int cross_rand()
{
    seed = seed * 214013 + 2531011;
    return (seed >> 16) & 0x7FFF;
}

#endif // CROSSRAND_H