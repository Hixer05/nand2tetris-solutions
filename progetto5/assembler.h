#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#include<stddef.h>

static inline void defhasherr ();
[[nodiscard]] static inline char** const initHashTable (const size_t size, const size_t strLen);

#define STORE_IN_HASH(key, data, HKEYS, HDATA, size, err_proc)                \
  {                                                                           \
    size_t pos = 0;                                                           \
    if (allocHashMap (HKEYS, key, size, &pos) != 0)                           \
      {                                                                       \
        err_proc;                                                             \
      }                                                                       \
    HDATA[pos] = data;                                                        \
  }

#define FREE_MAP(map, size)                                                   \
  {                                                                           \
    for (int i = 0; i < size; i++)                                            \
      {                                                                       \
        free (map[i]);                                                        \
      }                                                                       \
  }

#define STORE_SYMBOL_WERR(key, data, err_proc)                                \
  STORE_IN_HASH (key, data, symbol_keys, symbol_data, SYMBOL_TABLE_SIZE,      \
                 err_proc)
#define STORE_SYMBOL(key, data)                                               \
  STORE_IN_HASH (key, data, symbol_keys, symbol_data, SYMBOL_TABLE_SIZE,      \
                 printf ("ST missing space \n");                              \
                 exit (1))

#define STORE_COMP(key, val)                                                        \
  STORE_IN_HASH (key, val, comp_keys, comp_data, COMP_SIZE, (*defhasherr) ())

#define STORE_JMP(key, val)                                                         \
  STORE_IN_HASH (key, val, jmp_keys, jmp_data, JMP_SIZE, (*defhasherr) ())

#define CPY(dest, str, start, offset)                                         \
  for (int i = 0; i < offset; i++)                                            \
    {                                                                         \
      dest[i] = str[start + i];                                               \
    }



#endif
