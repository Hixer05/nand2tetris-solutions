#include "wf.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"

void
getsegment (char *const segment)
{
  switch (segment[0])
    {
    case 'l':
      strcpy (segment, "LCL");
      break;
    case 'a':
      strcpy (segment, "ARG");
      break;
    case 'c':
      strcpy (segment, "C");
      break;
    case 's':
      strcpy (segment, "16");
    case 't':
      if (segment[2] == 'i')
        strcpy (segment, "THIS");
      else if (segment[2] == 'a')
        strcpy (segment, "THAT");
      else if (segment[2] == 'm') // temp
        strcpy (segment, "5");
      break;
    case 'p':
      strcpy (segment, "3");
      break;
    default:
      strcpy (segment, "E");
      break;
    }
}

int
wmove (const char *const line, FILE *const writef)
{
  char seg1[20], loc1[10], seg2[20], loc2[10];
  char wline[MAX_RLINE_LEN * 2];
  sscanf (line, " move %s %s %s %s ", seg1, loc1, seg2, loc2);

#ifdef DEBUG
  sprintf (wline, "\n//move %s %s %s %s\n", seg1, loc1, seg2, loc2);
  fputs (wline, writef);
#endif

  getsegment (seg1);
  getsegment (seg2);
  bool isTAL = false; // is T/A/L
  bool isStatic = false;
  switch (seg2[0])
    {
    case '5': // TEMP
    case '3':
      isTAL = false;
      break;
    case 'T':
    case 'A':
    case 'L':
      isTAL = true;
      break;
    case '1':
      isStatic = true;
      break;
    case 'C':
      break;
    default:
      printf ("Error move seg2 %s \n%s\n", seg2, line);
      return -1;
    }
  switch (seg1[0])
    {
    case '1':
      if (isTAL)
        {
          sprintf (wline,
                   "@%s\nD=M\n@%s\nD=D+A\n@R15\nM=D\n" // R15=SEG2+LOC2
                   "@Xxx.%s\nD=M\n@R15\nA=M\nM=D\n" // store SEG1+LOC1 to R15
                   ,
                   seg2, loc2, loc1);
          fputs (wline, writef);
          return 0;
        }
      else if (isStatic)
        {
          sprintf (wline, "@Xxx.%s\nD=M\n@Xxx.%s\nM=D\n", loc1, loc2);
          fputs (wline, writef);
          return 0;
        }
      else
        {
          /* sprintf (wline, "@Xxx.%s\nD=M\n@%d\nM=D\n", loc1, */
          /*          atoi (loc1) + atoi (seg1)); */
          /* fputs (wline, writef); */
          printf ("Tried to move to const\n");
          return -1;
        }
    case 'C':
      if (isTAL)
        {
          if (atoi (loc1) == 0 || atoi (loc1) == 1)
            {
              sprintf (wline,
                       "@%s\nD=M\n@%s\nA=D+A\nM=%d\n" // R15=SEG2+LOC2
                       ,
                       seg2, loc2, atoi (loc1));
              fputs (wline, writef);
              return 0;
            }
          sprintf (wline,
                   "@%s\nD=M\n@%s\nD=D+A\n@R15\nM=D\n" // R15=SEG2+LOC2
                   "@%d\nD=A\n@R15\nA=M\nM=D\n" // store SEG1+LOC1 to R15
                   ,
                   seg2, loc2, atoi (loc1));
          fputs (wline, writef);
          return 0;
        }
      else if (isStatic)
        {
          if (atoi (loc1) == 0 || atoi (loc1) == 1)
            {
              sprintf (wline, "@Xxx.%s\nM=%s\n", loc2, loc1);
              fputs (wline, writef);
              return 0;
            }
          sprintf (wline, "@%s\nD=A\n@Xxx.%s\nM=D\n", loc1, loc2);
          fputs (wline, writef);
          return 0;
        }
      else
        {
          if (atoi (loc1) == 0 || atoi (loc1) == 1)
            {
              sprintf (wline, "@%d\nM=%d\n", atoi (loc2) + atoi (seg2),
                       atoi (loc1));
              fputs (wline, writef);
              return 0;
            }
          sprintf (wline, "@%d\nD=A\n@%d\nM=D\n", atoi (loc1),
                   atoi (loc2) + atoi (seg2));
          fputs (wline, writef);
          return 0;
        }
    case '5': // TEMP
    case '3':
      if (isTAL)
        {
          sprintf (wline,
                   "@%s\nD=M\n@%s\nD=D+A\n@R15\nM=D\n" // R15=SEG2+LOC2
                   "@%d\nD=M\n@R15\nA=M\nM=D\n" // store SEG1+LOC1 to R15
                   ,
                   seg2, loc2, atoi (seg1) + atoi (loc1));
          fputs (wline, writef);
          return 0;
        }
      else if (isStatic)
        {
          sprintf (wline, "@%d\nD=M\n@Xxx.%s\nM=D\n",
                   atoi (loc1) + atoi (seg1), loc2);
          fputs (wline, writef);
          return 0;
        }
      else
        {
          sprintf (wline, "@%d\nD=M\n@%d\nM=D\n", atoi (loc1) + atoi (seg1),
                   atoi (loc2) + atoi (seg2));
          fputs (wline, writef);
          return 0;
        }
      break;

    case 'T':
    case 'A':
    case 'L':
      if (isTAL)
        {
          sprintf (wline,
                   "@%s\nD=M\n@%s\nD=D+A\n@R15\nM=D\n" // R15=SEG2+LOC2
                   "@%s\nD=M\n@%s\nA=D+A\nD=M\n@R15\nA=M\nM=D\n",
                   seg2, loc2, seg1, loc1);
          fputs (wline, writef);
          return 0;
        }
      else if (isStatic)
        {
          sprintf (wline, "@%s\nD=M\n@%s\nA=D+A\nD=M\n@Xxx.%s\nM=D\n", seg1,
                   loc1, loc2);
          fputs (wline, writef);
          return 0;
        }
      else
        {
          sprintf (wline, "@%s\nD=M\n@%s\nA=D+A\nD=M\n@%d\nM=D\n", seg1, loc1,
                   atoi (loc2) + atoi (seg2));
          fputs (wline, writef);
          return 0;
        }
      //
      break;
    default:
      printf ("ERROR move seg1 %s \n%s\n", seg1, line);
      return -1;
    }
}

int
wsum (const char *const line, FILE *const writef)
{
#ifdef DEBUG
  fprintf (writef, "//%s", line);
#endif
  char seg[2][20], loc[2][20];
  sscanf (line, " sum %s %s %s %s \n", seg[0], loc[0], seg[1], loc[1]);
  getsegment (seg[0]);
  getsegment (seg[1]);
  char wline[100];
  switch (seg[0][0])
    {
    case '1':
      sprintf (wline, "@Xxx.%s\nD=M\n", loc[0]);
      break;
    case 'C':
      sprintf (wline, "@%s\nD=A\n", loc[0]);
      break;
    case '5':
    case '3':
      sprintf (wline, "@%d\nA=D+A\nD=M\n", atoi (loc[0]) + atoi (seg[0]));
      break;
    case 'T':
    case 'A':
    case 'L':
      sprintf (wline, "@%s\nD=A\n@%s\nA=D+M\nD=M\n", loc[0], seg[0]);
      break;
    case 'E':
    default:
      return -1;
    }
  fputs (wline, writef);
  switch (seg[1][0])
    {
    case 'C':
      sprintf (wline, "@%s\nD=D+A\n@SP\nM=M+1\nA=M-1\nM=D\n", loc[1]);
      break;
    case '5':
    case '3':
      sprintf (wline, "@%d\nD=D+M\n@SP\nM=M+1\nA=M-1\nM=D\n",
               atoi (seg[1]) + atoi (loc[1]));
      break;
    case '1':
      sprintf (wline, "@Xxx.%s\nD=D+M\n@SP\nM=M+1\nA=M-1\nM=D\n", loc[1]);
      break;
    case 'T':
    case 'A':
    case 'L':
      sprintf (wline,
               "@R15\nM=D\n@%s\nD=A\n@%s\nA=D+M\nD=M\n@R15\nD=D+M\n@SP\nM=M+"
               "1\nA=M-1\nM=D\n",
               loc[1], seg[1]);
      break;
    case 'E':
    default:
      return -1;
    }

  fputs (wline, writef);

  return 0;
}

void
wneg (FILE *const writef)
{
#ifdef DEBUG
  fputs ("\n//neg\n", writef);
#endif
  fputs ("@SP\nA=M-1\nM=-M\n", writef);
}

void
wadd (FILE *const writef)
{
#ifdef DEBUG
  fputs ("\n//add\n", writef);
#endif
  static bool iswritten = false;
  static int timesCalled = 0;
  if (!iswritten)
    {
      fputs ("@$ADD0\nD=A\n", writef);
      fputs ("($ADD$)\n@R15\nM=D\n@SP\nM=M-1\nA=M\nD=M\nA=A-1\nM=D+M\n@R15\nA="
             "M\n0;JMP\n" // a+b
             "($ADD0)\n",
             writef);
      timesCalled++; // IMPORTANT!!!
      iswritten = true;
    }
  else
    {
      char wline[100];
      sprintf (wline, "@$ADD%d\nD=A\n@$ADD$\n0;JMP\n($ADD%d)\n", timesCalled,
               timesCalled);
      fputs (wline, writef);
      timesCalled++;
    }
}

void
wsub (FILE *const writef)
{
  static bool written = false;
  static int timesCalled = 0;
#ifdef DEBUG
  fputs ("\n//sub\n", writef);
#endif

  if (!written)
    {
      fprintf (writef,
               "@$SUB%d\nD=A\n($SUB$)\n@R15\nM=D\n"
               "@SP\nM=M-1\nA=M\nD=-M\nA=A-1\nM=D+M\n"
               "@R15\nA=M\n0;JMP\n($SUB%d)\n",
               timesCalled, timesCalled);
      written = true;
    }
  else
    {
      fprintf (writef, "@$SUB%d\nD=A\n@$SUB$\n0;JMP\n($SUB%d)\n", timesCalled,
               timesCalled);
    }
  timesCalled++;
}

int
wpop (const char *const line, FILE *const writef)
{
  // NOTE line format: `pop segment x`
  char seg[20], x[10];
  sscanf (line, " pop %s %s", seg, x);

  // comment
  char wline[MAX_RLINE_LEN * 5] = "";
  sprintf (wline, "\n//pop %s %s\n", seg, x);
  fputs (wline, writef);
  strcpy (wline, "");

  static bool poptemp0 = false;
  static int timesCalled = 0;

  switch (seg[0])
    {
    case 'a':
      strcpy (seg, "@ARG");
      break;
    case 'l':
      strcpy (seg, "@LCL");
      break;
    case 't':            // pointer +0/+1
      if (seg[1] == 'e') // temp
        {
          if (atoi (x) == 0)
            {
              if (!poptemp0)
                {
                  fprintf (
                      writef,
                      "@POPS0%d\nD=A\n($POPSUBR)\n@R15\nD=M\n@SP\nM=M-1\n"
                      "A=M\nD=M\n@5\nM=D\n@R15\nA=M\n0;JMP\n(POPS0%d)\n",
                      timesCalled, timesCalled);
                  timesCalled++;
                  poptemp0 = true;
                  return 0;
                }
              else
                {
                  sprintf (wline,
                           "@POPS0%d\nD=A\n@$POPSUBR\n0;JMP\n(POPS0%d)\n",
                           timesCalled, timesCalled);
                  fputs (wline, writef);
                  timesCalled++;
                  return 0;
                }
            }
          sprintf (wline, "@SP\nM=M-1\nA=M\nD=M\n@%d\nM=D\n", atoi (x) + 5);
          fputs (wline, writef);
          return 0;
        }
      else if (seg[2] == 'i')
        { // this
          strcpy (seg, "@THIS");
          break;
        }
      else
        { // that
          strcpy (seg, "@THAT");
          break;
        }
      printf ("Error: %s\n", line);
      return -1;
    case 'p':
      if (strstr (x, "1"))
        {
          fputs ("@SP\nM=M-1\nA=M\nD=M\n@THAT\nM=D\n", writef);
          return 0;
        }
      else if (strstr (x, "0"))
        {
          fputs ("@SP\nM=M-1\nA=M\nD=M\n@THIS\nM=D\n", writef);
          return 0;
        }
      printf ("Error: %s\n", line);
      return -1;
    case 's':
      sprintf (wline, "@SP\nM=M-1\nA=M\nD=M\n@Xxx.%s\nM=D\n", x);
      fputs (wline, writef);
      return 0;
    case 'c': // impossibile; we discard the value
      fputs ("@SP\nM=M-1\n", writef);
      return 0;
    default:
      printf ("error: default exit\n");
      return -1;
    }
  sprintf (
      wline,
      "@%s\nD=A\n%s\nM=D+M\n@SP\nM=M-1\nA=M\nD=M\n%s\nA=M\nM=D\n@%s\nD=A\n%"
      "s\nM=M-D\n",
      x, seg, seg, x, seg);
  fputs (wline, writef);
  return 0;
}

int
wpush (const char *const line, FILE *const writef)
{
  // NOTE line format: `push segment k`
  char seg[20], loc[10];
  char wline[MAX_RLINE_LEN * 5] = "";
  sscanf (line, " push %s %s", seg, loc);

  // comment
#ifdef DEBUG
  sprintf (wline, "\n//push %s %s\n", seg, loc);
  fputs (wline, writef);
  strcpy (wline, "");
#endif

  getsegment (seg);

  switch (seg[0])
    {
    case '5':
    case '3':
      sprintf (wline, "@%d\nD=M\n@SP\nM=M+1\nA=M-1\nM=D\n",
               atoi (seg) + atoi (loc));
      fputs (wline, writef);
      return 0;

    case '1':
      sprintf (wline, "@Xxx.%s\nD=M\n@SP\nM=M+1\nA=M-1\nM=D\n", loc);
      fputs (wline, writef);
      return 0;

    case 'C':
      if (atoi (loc) == 0 || atoi (loc) == 1)
        {
          sprintf (wline, "@SP\nM=M+1\nA=M-1\nM=%d\n", atoi (loc));
          fputs (wline, writef);
        }
      else
        {
          sprintf (wline, "@%d\nD=A\n@SP\nM=M+1\nA=M-1\nM=D\n", atoi (loc));
          fputs (wline, writef);
        }
      return 0;

    case 'L':
      if (atoi (loc) == 0)
        {
          fprintf (writef, "@%s\nA=M\nD=M\n@SP\nM=M+1\nA=M-1\nM=D\n", seg);
          return 0;
        }
    case 'T':
    case 'A':
      sprintf (wline,
               "@%s\nD=A\n@%s\nA=M\nA=D+A\nD=M\n@SP\nM=M+1\nA=M-1\nM=D\n", loc,
               seg);
      fputs (wline, writef);
      return 0;
    default:
      printf ("Error push \n%s\n", line);
      return -1;
    }
  return -1;
}

void
wgoto (const char *const line, FILE *const writef)
{
  // NOTE `goto L`
  char label[60];
  char buffer[MAX_RLINE_LEN * 3];
  size_t j = 0;
  sscanf (line, " goto %s ", label);
  j += sprintf (buffer + j, "\n//goto\n@%s\n0;JMP\n", label);
  fputs (buffer, writef);
}

void
wlt (FILE *const writef)
{
  // NOTE: `lt`
  //  *(SP-2) < *(SP-1) as a<b
  //  a-b<0?T:F
  //  then we pop b and overwrite on a
  static int timesCalled = 0;
  char wline[MAX_RLINE_LEN * 5];
#ifdef DEBUG
  fputs ("\n//lt\n", writef);
#endif

  if (timesCalled == 0)
    {
      sprintf (wline, "@$LT%d\nD=A\n", timesCalled);
      fputs (wline, writef);
      fputs ("($LT$)\n@R15\nM=D\n"
             "@SP\nM=M-1\nA=M-1\nD=M\nA=A+1\nD=D-M\n" // a-b
             "@$LTT\nD;JLT\n"                         // if a<b goto LTTrue
             "@SP\nA=M-1\nM=0\n"                      // else a=0
             "@$LTE\n0;JMP\n"                         // and exit
             "($LTT)\n@SP\nA=M-1\nM=-1\n"             // LTT
             "($LTE)\n"                               // exit
             "@R15\nA=M\n0;JMP\n"                     // exit subr
             ,
             writef);
      sprintf (wline, "($LT%d)\n", timesCalled);
      fputs (wline, writef);
    }
  else
    {
      sprintf (wline,
               "@$LT%d\nD=A\n"
               "@$LT$\n0;JMP\n($LT%d)\n",
               timesCalled, timesCalled);
      fputs (wline, writef);
    }
  timesCalled++;
}

void
weq (FILE *const writef)
{
  // NOTE literally the same as wlt
  // a-b=0?T:F
  static int timesCalled = 0;
  char wline[MAX_RLINE_LEN * 5];
#ifdef DEBUG
  fputs ("\n//eq\n", writef);
#endif

  if (timesCalled == 0)
    {
      sprintf (wline, "@$EQ%d\nD=A\n", timesCalled);
      fputs (wline, writef);
      fputs ("($EQ$)\n@R15\nM=D\n"
             "@SP\nM=M-1\nA=M-1\nD=M\nA=A+1\nD=D-M\n" // a-b
             "@$EQT\nD;JEQ\n"                         // if eq goto EQTrue
             "@SP\nA=M-1\nM=0\n"                      // else a=0
             "@$EQE\n0;JMP\n"                         // and exit
             "($EQT)\n@SP\nA=M-1\nM=-1\n"             // EQT
             "($EQE)\n"                               // exit
             "@R15\nA=M\n0;JMP\n"                     // exit subr
             ,
             writef);
      sprintf (wline, "($EQ%d)\n", timesCalled);
      fputs (wline, writef);
    }
  else
    {
      sprintf (wline,
               "@$EQ%d\nD=A\n"
               "@$EQ$\n0;JMP\n($EQ%d)\n",
               timesCalled, timesCalled);
      fputs (wline, writef);
    }
  timesCalled++;
}

void
wgt (FILE *const writef)
{
  // NOTE literally the same as wlt
  // a-b>0?T:F
  static int timesCalled = 0;
  char wline[MAX_RLINE_LEN * 5];
#ifdef DEBUG
  fputs ("\n//gt\n", writef);
#endif

  if (timesCalled == 0)
    {
      sprintf (wline, "@$GT%d\nD=A\n", timesCalled);
      fputs (wline, writef);
      fputs ("($GT$)\n@R15\nM=D\n"
             "@SP\nM=M-1\nA=M-1\nD=M\nA=A+1\nD=D-M\n" // a-b
             "@$GTT\nD;JGT\n"                         // if eq goto GTTrue
             "@SP\nA=M-1\nM=0\n"                      // else a=0
             "@$GTE\n0;JMP\n"                         // and exit
             "($GTT)\n@SP\nA=M-1\nM=-1\n"             // GTT
             "($GTE)\n"                               // exit
             "@R15\nA=M\n0;JMP\n"                     // exit subr
             ,
             writef);
      sprintf (wline, "($GT%d)\n", timesCalled);
      fputs (wline, writef);
    }
  else
    {
      sprintf (wline,
               "@$GT%d\nD=A\n"
               "@$GT$\n0;JMP\n($GT%d)\n",
               timesCalled, timesCalled);
      fputs (wline, writef);
    }
  timesCalled++;
}

void
wnot (FILE *const writef)
{
#ifdef DEBUG
  fputs ("\n//not\n", writef);
#endif
  fputs ("@SP\nA=M-1\nM=!M\n", writef);
}

void
wand (FILE *const writef)
{
#ifdef DEBUG
  fputs ("\n//and\n", writef);
#endif
  fputs ("@SP\nM=M-1\nA=M\nD=M\nA=A-1\nM=D&M\n", writef);
}

void
wor (FILE *const writef)
{
#ifdef DEBUG
  fputs ("\n//or\n", writef);
#endif
  fputs ("@SP\nM=M-1\nA=M\nD=M\nA=A-1\nM=D|M\n", writef);
}

void
wlabel (const char *const line, FILE *const writef)
{
  // NOTE `label L`
  char label[60];
  sscanf (line, " label %s ", label);
#ifdef DEBUG
  fputs ("\n//label\n", writef);
#endif
  fputs ("(", writef);
  fputs (label, writef);
  fputs (")\n", writef);
}
void
wifgoto (const char *const line, FILE *const writef)
{
  // NOTE line format: `if-goto L`
  // check stack if bool true (-1)
  // or false(0)
  char label[60];
  sscanf (line, " if-goto %s ", label);
#ifdef DEBUG
  fputs ("\n//ifgoto\n", writef);
#endif
  fputs ("@SP\nM=M-1\nA=M\nD=M\n@", writef); // if-goto also pops
  fputs (label, writef);
  fputs ("\nD;JNE // J if true\n", writef);
}

int
wfunctiondecl (const char *const line, FILE *const writef)
{
  // NOTE line format: `function name locc`
  // We'll set a TAG to JMP to

  char fname[MAX_RLINE_LEN - 20];
  int locc;
  sscanf (line, " function %s %d", fname, &locc);

  char wline[MAX_RLINE_LEN * 5];
  sprintf (wline, "\n//fun %s %d\n", fname, locc);
  fputs (wline, writef);

  // (fname)
  strcpy (wline, "(");
  strcat (wline, fname);
  strcat (wline, ")\n");
  fputs (wline, writef);

  // NOTE: correct LCL already set by call
  // We save M=M+1;..;@SP (2) for every local var at the cost of 5
  // Since many functions don't have many args we make a special case
  if (locc < 2)
    {
      for (int i = 0; i < locc; i++) // f: n*4; (1:4, 2:8, 3:12, 4:16)
        fputs ("@SP\nM=M+1\nA=M\nM=0\n", writef);
    }
  else
    {
      sprintf (wline, "@%d\nD=A\n@SP\nM=D+M\nA=M-D\n", locc);
      fputs (wline, writef);
      for (int i = 0; i < locc; i++) // f: n*2 + 5; (1:7, 2:9, 3:11, 4:13)
        fputs ("M=0\nA=A+1\n", writef);
    }

  return 0;
}

void
wfunctionreturn (FILE *const writef)
{
  // NOTE line format: `return`
  static bool written = false;
  if (!written)
    {
      fputs (
          "\n//return subroutine \n"
          "($return$)\n"                        // tag to jmp to
          "//FRAME=LCL\n@LCL\nD=M\n@R13\nM=D\n" /* FRAME=LCL */
          "//RET=*(FRAME-5)\n@R13\nD=M\n@5\nD=D-A\nA=D\nD=M\n@R14\nM=D\n" /* RET=*(FRAME-5)
                                                                           */
          "//Arg[0]=reval\n@SP\nM=M-1\nA=M\nD=M\n@ARG\nA=M\nM=D\n" /* Copy
                                                                      return
                                                                      val to
                                                                      arg[0] ;
                                                                    */
          "//SP=ARG+1\n@ARG\nD=M+1\n@SP\nM=D\n"    // SP=ARG+1 // restore sp
          "//that\n@R13\nA=M-1\nD=M\n@THAT\nM=D\n" // restore that
          "//this\n@R13\nD=M\n@2\nA=D-A\nD=M\n@THIS\nM=D\n" // restore this
          "//arg\n@R13\nD=M\n@3\nA=D-A\nD=M\n@ARG\nM=D\n"   // restore arg
          "//lcl\n@R13\nD=M\n@4\nA=D-A\nD=M\n@LCL\nM=D\n"   // restore lcl
          "//return\n@R14\nA=M\n0;JMP\n",
          writef);
      written = true;
    }
  else
    {
      fputs ("@$return$\n0;JMP\n", writef);
    }
}

void
wfunctioncall (const char *const line, FILE *const writef)
{
  // NOTE line format: `call fname argc`
  // To do this we'll push a TAG:
  // @TAG, D=A, @0, A=M, M=D, @0, M=M+1

  static bool written1 = 0;
  static size_t timesCalled = 0; // src for TAG gen
  char TAG[10] = "$C";           // log(32k) ~ 5
  char fname[MAX_RLINE_LEN - 20], argc[10];
  char wline[MAX_RLINE_LEN * 5];
  size_t j = 0;

  // get TAG
  sprintf (TAG + 2, "%lu", timesCalled);
  // get fname
  sscanf (line, " call %s %s", fname, argc);
  // comment
#ifdef DEBUG
  j += sprintf (wline + j, "\n//call %s %s\n", fname, argc);
#endif

#define PUSH_D "@SP\nM=M+1\nA=M-1\nM=D\n"

  // push TAG
  j += sprintf (wline + j, "@%s\nD=A\n" PUSH_D, TAG);

  // constexpr from here
  if (!written1)
    {
      // to exit
      j += sprintf (wline + j, "@$%s\nD=A\n", TAG);
      // Subroutine entrypoint
      j += sprintf (wline + j, "($CALL$)\n@R15\nM=D\n");
      // push LCL
      j += sprintf (wline + j, "@LCL\nD=M\n" PUSH_D);
      // push ARG
      j += sprintf (wline + j, "@ARG\nD=M\n" PUSH_D);
      // THIS
      j += sprintf (wline + j, "@THIS\nD=M\n" PUSH_D);
      // push THAT
      j += sprintf (wline + j, "@THAT\nD=M\n" PUSH_D);
      // Subroutine exit
      j += sprintf (wline + j, "@R15\nA=M\n0;JMP\n");
      j += sprintf (wline + j, "($%s)\n", TAG); // subroutine return point
      written1 = true;
    }
  else
    {
      // Mem return and jump to subroutine
      j += sprintf (wline + j, "@$%s\nD=A\n@$CALL$\n0;JMP\n", TAG);
      j += sprintf (wline + j, "($%s)\n", TAG); // subroutine return point
    }

  // reposition ARG
  // ARG = sp - argc - 5;
  j += sprintf (wline + j, "@SP\nD=M\n@%s\nD=D-A\n@5\nD=D-A\n@ARG\nM=D\n",
                argc);
  // LCL = SP
  j += sprintf (wline + j, "@SP\nD=M\n@LCL\nM=D\n");
  // JMP to function
  j += sprintf (wline + j, "@%s\n0;JMP\n", fname);

  // set return tag
  j += sprintf (wline + j, "(%s)\n", TAG);
  fputs (wline, writef);
  timesCalled++;
}
