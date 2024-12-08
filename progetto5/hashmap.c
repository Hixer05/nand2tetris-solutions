#include "hashmap.h"
#include "intpow.h"
#include <malloc.h>
#include <string.h>

void
initHashTable (char **const hashTable, const size_t size, const size_t strLen)
{
  for (register int i = 0; i < size; i++)
    {
      hashTable[i] = malloc (sizeof (char) * size);
      hashTable[i][0] = '\0';
    }
}

static inline const size_t
hash1 (char const *const key)
{
  // polynomial rolling hash function.
  constexpr int p_factor = 35; // ~ input alphabet 25+10=35
  size_t sum = 0;
  for (register int i = 0; key[i] != '\0'; i++)
    sum += key[i] * ipow (p_factor, i);
  return sum;
}

static inline size_t
hash2 (const size_t hash, const int iter)
{
  // quadratic probing
  return hash + iter * iter;
}

[[nodiscard]] int
allocHashMap (char **const hashTable, char const *const key, const size_t size,
              size_t *const pos)
{
  size_t pos_l = hash1 (key) % size;
  for (register int i = 0; i < size; i++)
    {
      if (hashTable[pos_l][0] == '\0')
        { // store
          strcpy (hashTable[pos_l], key);
          *pos = pos_l;
          return 0;
        }
      else // occupied
        pos_l = hash2 (pos_l, i + 1)
                % size; // pos + 0^2 = pos; it'd be a useless cycle
    }
  return -1;
}

[[nodiscard]] int
searchHashMap (char **const hashTable, char const *const key,
               const size_t size, size_t *const pos)
{
  size_t pos_l = hash1 (key) % size;
  for (register int i = 0; i < size; i++)
    {
      if (hashTable[pos_l][0] == '\0') // not found
        return -1;
      else if (!strcmp (hashTable[pos_l], key))
        { // found
          *pos = pos_l;
          return 0;
        }
      else // occupied
        pos_l = hash2 (pos_l, i + 1) % size;
    }
  return -1;
}
