#ifndef WF_H_
#define WF_H_
#include<stdio.h>
#include<stddef.h>

int wfunctiondecl(char * const line, FILE*const writef);
void wfunctionreturn(FILE *const writef);
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

#endif // WF_H_
