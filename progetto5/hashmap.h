#ifndef HASHMAP_H_
#define HASHMAP_H_

#include<stddef.h>

inline void initHashTable(char ** const hashTable, const size_t size);
static inline const size_t hash1(char const * const key);
static inline size_t hash2(const size_t hash, const int iter);
static inline const int pow (const int a, const int e);

// ___ Functions which might return errors ___
[[nodiscard]] int insertHashMap(char * const hashMap, void * const hashData, char const * const key, void * const data);
[[nodiscard]] int searchHashMap(char * const hashMap, char const * const key, size_t* const pos);

#endif // HASHMAP_H_
