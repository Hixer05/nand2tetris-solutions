#ifndef VM_H
#define VM_H
#include <stddef.h>
#include <stdio.h>

inline int wfunctiondecl(char * const line, FILE*const writef);
inline void wfunctionreturn(FILE *const writef);
void wfunctioncall (char *const line, FILE *const writef);
void wifgoto(char*const line, FILE*const writef);
int wpush(char* const line, FILE* const writef);
int wpop(char* const line, FILE* const writef);
void wlabel(char*const line, FILE*const writef);
void wlt(FILE*const writef);
void wgoto(char*const line, FILE*const writef);
void wsub(FILE*const writef);
void wadd(FILE*const writef);
void wneg(FILE*const writef);
void weq(FILE*const writef);
void wgt(FILE*const writef);
void wnot(FILE*const writef);
void wand(FILE*const writef);
void wor(FILE*const writef);
#endif
