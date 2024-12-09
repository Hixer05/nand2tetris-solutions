#ifndef HASHMAP_H_
#define HASHMAP_H_
#include <stddef.h>

static inline const size_t hash1 (char const *const key);
static inline size_t hash2 (const size_t hash, const int iter);

// ___ Functions which might return errors ___
[[nodiscard]] int allocHashMap (char **const hashTable, char const *const key,
                                const size_t size, size_t *const pos);

[[nodiscard]] int searchHashMap (char **const hashTable, char const *const key,
                                 const size_t size, size_t *const pos);

#endif // HASHMAP_H_
