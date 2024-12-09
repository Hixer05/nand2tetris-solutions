#ifndef VM_H
#define VM_H
#include <stddef.h>
#include <stdio.h>

[[nodiscard]] inline char** const initHashTable (const size_t size, const size_t strLen);
[[nodiscard]] inline int wfunctiondecl(char * const line, FILE*const writef);
inline void wfunctionbreak(char*const line, FILE *const writef);
void wfunctioncall (char *const line, FILE *const writef, char * const fnameset);

#endif
