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

#define INIT_ST(){\
  /* Wont check for missing space in hashtable for the default tags */ \
  /* Because of course there is */ \
  STORE_SYMBOL ("R0", 0); \
  STORE_SYMBOL ("R1", 1); \
  STORE_SYMBOL ("R2", 2); \
  STORE_SYMBOL ("R3", 3); \
  STORE_SYMBOL ("R4", 4); \
  STORE_SYMBOL ("R5", 5); \
  STORE_SYMBOL ("R6", 6); \
  STORE_SYMBOL ("R7", 7); \
  STORE_SYMBOL ("R8", 8); \
  STORE_SYMBOL ("R9", 9); \
  STORE_SYMBOL ("R10", 10); \
  STORE_SYMBOL ("R11", 11); \
  STORE_SYMBOL ("R12", 12); \
  STORE_SYMBOL ("R13", 13); \
  STORE_SYMBOL ("R14", 14); \
  STORE_SYMBOL ("R15", 15); \
  STORE_SYMBOL ("SCREEN", 16384); \
  STORE_SYMBOL ("KBD", 24576); \
  STORE_SYMBOL ("SP", 0); \
  STORE_SYMBOL ("LCL", 1); \
  STORE_SYMBOL ("ARG", 2); \
  STORE_SYMBOL ("THIS", 3); \
  STORE_SYMBOL ("THAT", 4); \
}

#define INIT_COMP(){ \
STORE_COMP ("0", "0101010");    \
STORE_COMP ("1", "0111111");    \
STORE_COMP ("-1", "0111010");   \
STORE_COMP ("D", "0001100");    \
STORE_COMP ("A", "0110000");    \
STORE_COMP ("!D", "0001101");   \
STORE_COMP ("!A", "0110001");   \
STORE_COMP ("-D", "0001111");   \
STORE_COMP ("-A", "1100110");   \
STORE_COMP ("D+1", "0011111");  \
STORE_COMP ("A+1", "0110111");  \
STORE_COMP ("D-1", "0001110");  \
STORE_COMP ("A-1", "0110010");  \
STORE_COMP ("D+A", "0000010");  \
STORE_COMP ("D-A", "0010011");  \
STORE_COMP ("A-D", "0111000");  \
STORE_COMP ("D&A", "0000000");  \
STORE_COMP ("D|A", "0010101");  \
STORE_COMP ("M", "1110000");    \
STORE_COMP ("!M", "1110001");   \
STORE_COMP ("-M", "1110011");   \
STORE_COMP ("M+1", "1110111");  \
STORE_COMP ("M-1", "1110010");  \
STORE_COMP ("D+M", "1000010");  \
STORE_COMP ("D-M", "1010011");  \
STORE_COMP ("M-D", "1000111");  \
STORE_COMP ("D&M", "1000000");  \
STORE_COMP ("D|M", "1010101");  \
}

#endif
