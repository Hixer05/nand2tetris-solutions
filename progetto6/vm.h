#ifndef VM_H
#define VM_H
#include <stddef.h>
#include <stdio.h>

inline int wfunctiondecl(char * const line, FILE*const writef);
inline void wfunctionreturn(char*const line, FILE *const writef);
void wfunctioncall (char *const line, FILE *const writef);
void wifgoto(char*const line, FILE*const writef);
int wpush(char* const line, FILE* const writef);
int wpop(char* const line, FILE* const writef);

#endif
