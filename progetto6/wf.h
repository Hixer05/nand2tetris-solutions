#ifndef WF_H_
#define WF_H_
#include <stddef.h>
#include <stdio.h>

int wfunctiondecl (const char *const line, FILE *const writef);
void wfunctionreturn (FILE *const writef);
void wfunctioncall (const char *const line, FILE *const writef);
void wifgoto (const char *const line, FILE *const writef);
int wpush (const char *const line, FILE *const writef);
int wpop (const char *const line, FILE *const writef);
void wlabel (const char *const line, FILE *const writef);
void wlt (FILE *const writef);
void wgoto (const char *const line, FILE *const writef);
void wsub (FILE *const writef);
void wadd (FILE *const writef);
void wneg (FILE *const writef);
void weq (FILE *const writef);
void wgt (FILE *const writef);
void wnot (FILE *const writef);
void wand (FILE *const writef);
void wor (FILE *const writef);
int wmove (const char *const line, FILE *const writef);
#endif // WF_H_
