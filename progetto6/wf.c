#include "wf.h"
#include "shared.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
            // NOTE: !!if the vm ever uses an ASM VARIABLE THIS WILL BREAK!!
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
wmove (char *const line, FILE *const writef)
{
    char seg1[20], loc1[10], seg2[20], loc2[10];
    char wline[MAX_RLINE_LEN * 2];
    sscanf (line, " move %s %s %s %s ", seg1, loc1, seg2, loc2);
    getsegment (seg1);
    getsegment (seg2);
    bool isTAL = false; // is T/A/L
    switch (seg2[0])
        {
        case '5': // TEMP
        case '3':
        case '1':
            isTAL = false;
            break;
        case 'T':
        case 'A':
        case 'L':
            isTAL = true;
            break;
        default:
            printf ("Error move seg2 %s \n%s\n", seg2, line);
            return -1;
        }
    switch (seg1[0])
        {
        case 'C':
            if (isTAL)
                {
                    sprintf (wline,
                             "@%s\nD=M\n@%s\nD=D+A\n@R15\nM=D\n" // R15=SEG2+LOC2
                             "@%d\nD=A\n@R15\nA=M\nM=D\n"        // store SEG1+LOC1 to R15
                             ,
                             seg2, loc2, atoi (loc1));
                    fputs (wline, writef);
                    return 0;
                }
            else
                {
                    sprintf (wline, "@%d\nD=A\n@%d\nM=D", atoi (seg1), atoi (loc2) + atoi (seg2));
                    fputs (wline, writef);
                    return 0;
                }
        case '5': // TEMP
        case '3':
        case '1':
            if (isTAL)
                {
                    sprintf (wline,
                             "@%s\nD=M\n@%s\nD=D+A\n@R15\nM=D\n" // R15=SEG2+LOC2
                             "@%d\nD=M\n@R15\nA=M\nM=D\n"        // store SEG1+LOC1 to R15
                             ,
                             seg2, loc2, atoi (seg1) + atoi (loc1));
                    fputs (wline, writef);
                    return 0;
                }
            else
                {
                    sprintf (wline, "@%d\nD=M\n@%d\nM=D", atoi (loc1) + atoi (seg1),
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
            else
                {
                    sprintf (wline, "@%s\nD=M\n@%s\nA=D+A\nD=M\n@%d\nM=D", seg1, loc1,
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
    fputs ("@SP\nM=M-1\nA=M\nD=M\nA=A-1\nM=D+M\n" // a+b
           ,
           writef);
}

void
wsub (FILE *const writef)
{
#ifdef DEBUG
    fputs ("\n//sub\n", writef);
#endif
    fputs ("@SP\nM=M-1\nA=M\nD=-M\nA=A-1\nM=M+D\n" // -b; a+(-b) (in loc)
           ,
           writef);
}

int
wpop (char *const line, FILE *const writef)
{
    // NOTE line format: `pop segment x`
    char seg[20], x[10];
    sscanf (line, " pop %s %s", seg, x);

    // comment
    char wline[MAX_RLINE_LEN * 5] = "";
    sprintf (wline, "\n//pop %s %s\n", seg, x);
    fputs (wline, writef);
    strcpy (wline, "");

    switch (seg[0])
        {
        case 'a':
            strcpy (seg, "@ARG");
            break;
        case 'l':
            strcpy (seg, "@LCL");
            break;
        case 't':              // pointer +0/+1
            if (seg[1] == 'e') // temp
                {
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
                    sprintf (wline, "@SP\nM=M-1\nA=M\nD=M\n@THIS\nA=A+%s\nM=D", x);
                    fputs (wline, writef);
                    return 0;
                }
            else if (strstr (x, "0"))
                {
                    sprintf (wline, "@SP\nM=M-1\nA=M\nD=M\n@THIS\nM=D");
                    fputs (wline, writef);
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
    sprintf (wline,
             "@%s\nD=A\n%s\nM=D+M\n@SP\nM=M-1\nA=M\nD=M\n%s\nA=M\nM=D\n@%s\nD=A\n%s\nM=M-D\n", x,
             seg, seg, x, seg);
    fputs (wline, writef);
    return 0;
}

int
wpush (char *const line, FILE *const writef)
{
    // NOTE line format: `push segment k`
    char seg[20], k[10];
    sscanf (line, " push %s %s", seg, k);

    // comment
    char wline[MAX_RLINE_LEN * 5] = "";
    sprintf (wline, "\n//push %s %s\n", seg, k);
    fputs (wline, writef);
    strcpy (wline, "");
    size_t j = 0;

    switch (seg[0])
        {
        case 'a': // argument -> @ARG
            strcpy (seg, "@ARG");
            break;
        case 'l': // local -> @LCL
            strcpy (seg, "@LCL");
            break;
        case 's': // static -> 16
            sprintf (wline, "@Xxx.%s\nD=M\n@SP\nM=M+1\nA=M-1\nM=D\n", k);
            fputs (wline, writef);
            return 0;
        case 't':
            if (seg[1] == 'e') // temp
                {
                    sprintf (wline,
                             "@%s\nD=A\n@5\nA=D+A\n"
                             "D=M\n@SP\nM=M+1\nA=M-1\nM=D\n",
                             k);
                    fputs (wline, writef);
                    return 0;
                }
            if (seg[2] == 'i')
                { // this
                    strcpy (seg, "@THIS");
                    break;
                }
            else
                { // that
                    strcpy (seg, "@THAT");
                    break;
                }
            printf ("Error: No temp, that, or this.\n%s\n", line);
            return -1;
        case 'p':
            if (strstr (k, "0"))
                {
                    sprintf (wline, "@THIS\nD=M\n@SP\nM=M+1\nA=M\nM=D\n");
                    fputs (wline, writef);
                    return 0;
                }
            else if (strstr (k, "1"))
                {
                    sprintf (wline, "@THIS\nA=A+%s\nD=M\n@SP\nM=M+1\nA=M\nM=D\n", k);
                    fputs (wline, writef);
                }
            else
                {
                    printf ("Error: Pointer access eithe 0 or 1\n%s\n", line);
                    return -1;
                }
        case 'c': // constant -> special case
            if (atoi (k) == 0)
                {
                    fputs ("@SP\nM=M+1\nA=M-1\nM=0\n", writef);
                    return 0;
                }
            sprintf (wline, "@%s\nD=A\n@SP\nM=M+1\nA=M-1\nM=D\n", k);
            fputs (wline, writef);
            return 0;
        default:
            printf ("Error: default exit.\n");
            return -1;
        }
    j += sprintf (wline + j, "%s\nD=M\n@%s\nA=D+A\nD=M\n@SP\nM=M+1\nA=M-1\nM=D\n", seg, k);
    fputs (wline, writef);
    return 0;
}

void
wgoto (char *const line, FILE *const writef)
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
            sprintf (wline, "@$LT%d\nD=A\n@R15\nM=D\n", timesCalled);
            fputs (wline, writef);
            fputs ("($LT$)\n@SP\nM=M-1\nA=M-1\nD=M\nA=A+1\nD=D-M\n" // a-b
                   "@$LTT\nD;JLT\n"                                 // if a<b goto LTTrue
                   "@SP\nA=M-1\nM=0\n"                              // else a=0
                   "@$LTE\n0;JMP\n"                                 // and exit
                   "($LTT)\n@SP\nA=M-1\nM=-1\n"                     // LTT
                   "($LTE)\n"                                       // exit
                   "@R15\nA=M\n0;JMP\n"                             // exit subr
                   ,
                   writef);
            sprintf (wline, "($LT%d)\n", timesCalled);
            fputs (wline, writef);
        }
    else
        {
            sprintf (wline,
                     "@$LT%d\nD=A\n@R15\nM=D\n"
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
            sprintf (wline, "@$EQ%d\nD=A\n@R15\nM=D\n", timesCalled);
            fputs (wline, writef);
            fputs ("($EQ$)\n@SP\nM=M-1\nA=M-1\nD=M\nA=A+1\nD=D-M\n" // a-b
                   "@$EQT\nD;JEQ\n"                                 // if eq goto EQTrue
                   "@SP\nA=M-1\nM=0\n"                              // else a=0
                   "@$EQE\n0;JMP\n"                                 // and exit
                   "($EQT)\n@SP\nA=M-1\nM=-1\n"                     // EQT
                   "($EQE)\n"                                       // exit
                   "@R15\nA=M\n0;JMP\n"                             // exit subr
                   ,
                   writef);
            sprintf (wline, "($EQ%d)\n", timesCalled);
            fputs (wline, writef);
        }
    else
        {
            sprintf (wline,
                     "@$EQ%d\nD=A\n@R15\nM=D\n"
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
            sprintf (wline, "@$GT%d\nD=A\n@R15\nM=D\n", timesCalled);
            fputs (wline, writef);
            fputs ("($GT$)\n@SP\nM=M-1\nA=M-1\nD=M\nA=A+1\nD=D-M\n" // a-b
                   "@$GTT\nD;JGT\n"                                 // if eq goto GTTrue
                   "@SP\nA=M-1\nM=0\n"                              // else a=0
                   "@$GTE\n0;JMP\n"                                 // and exit
                   "($GTT)\n@SP\nA=M-1\nM=-1\n"                     // GTT
                   "($GTE)\n"                                       // exit
                   "@R15\nA=M\n0;JMP\n"                             // exit subr
                   ,
                   writef);
            sprintf (wline, "($GT%d)\n", timesCalled);
            fputs (wline, writef);
        }
    else
        {
            sprintf (wline,
                     "@$GT%d\nD=A\n@R15\nM=D\n"
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
wlabel (char *const line, FILE *const writef)
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
wifgoto (char *const line, FILE *const writef)
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
wfunctiondecl (char *const line, FILE *const writef)
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
    for (int i = 0; i < locc; i++)
        wpush ("push constant 0\n", writef);

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
                "($return$)\n"                                                  // tag to jmp to
                "//FRAME=LCL\n@LCL\nD=M\n@R13\nM=D\n"                           /* FRAME=LCL */
                "//RET=*(FRAME-5)\n@R13\nD=M\n@5\nD=D-A\nA=D\nD=M\n@R14\nM=D\n" /* RET=*(FRAME-5) */
                "//Arg[0]=reval\n@SP\nM=M-1\nA=M\nD=M\n@ARG\nA=M\nM=D\n" /* Copy return val to
                                                                            arg[0] ; */
                "//SP=ARG+1\n@ARG\nD=M+1\n@SP\nM=D\n"                    // SP=ARG+1 // restore sp
                "//that\n@R13\nA=M-1\nD=M\n@THAT\nM=D\n"                 // restore that
                "//this\n@R13\nD=M\n@2\nA=D-A\nD=M\n@THIS\nM=D\n"        // restore this
                "//arg\n@R13\nD=M\n@3\nA=D-A\nD=M\n@ARG\nM=D\n"          // restore arg
                "//lcl\n@R13\nD=M\n@4\nA=D-A\nD=M\n@LCL\nM=D\n"          // restore lcl
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
wfunctioncall (char *const line, FILE *const writef)
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
            j += sprintf (wline + j, "@$%s\nD=A\n@R15\nM=D\n", TAG);
            // Subroutine entrypoint
            j += sprintf (wline + j, "($CALL$)\n");
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
            j += sprintf (wline + j, "@$%s\nD=A\n@R15\nM=D\n($CALL$)\n0;JMP\n", TAG);
            j += sprintf (wline + j, "($%s)\n", TAG); // subroutine return point
        }

    // reposition ARG
    // ARG = sp - argc - 5;
    j += sprintf (wline + j, "@SP\nD=M\n@%s\nD=D-A\n@5\nD=D-A\n@ARG\nM=D\n", argc);
    // LCL = SP
    j += sprintf (wline + j, "@SP\nD=M\n@LCL\nM=D\n");
    // JMP to function
    j += sprintf (wline + j, "@%s\n0;JMP\n", fname);

    // set return tag
    j += sprintf (wline + j, "(%s)\n", TAG);
    fputs (wline, writef);
    timesCalled++;
}
