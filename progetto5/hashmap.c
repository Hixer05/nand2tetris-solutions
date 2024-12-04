#include "hashmap.h"

inline void initHashTable(char ** const hashTable, const size_t size)
{
    for(register int i = 0; i < size; i++)
        hashTable[i] = "\0";
}

static inline const int pow (const int a, const int e)
{
    int prod = a;
    for(register int i = 1; i<e; i++)
        prod *= a;
    return prod;
}

static inline const size_t hash1(char const * const key)
{
    // polynomial rolling hash function.
    constexpr int p_factor = 35; // ~ input alphabet 25+10=35
    size_t sum = 0;
    for(register int i = 0; key[i]!='\0'; i++)
        sum += key[i] * pow(p_factor, i);
    return sum;
}

static inline size_t hash2(const size_t hash, const int iter)
{
    // quadratic probing
    return hash + iter*iter;
}
