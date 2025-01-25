#include "wf.h"
#include "shared.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

void
wneg (FILE *const writef)
{
    fputs ("\n//neg\n@SP\nA=M-1\nM=-M\n", writef);
}

void
wadd (FILE *const writef)
{
    // literally the same as wsub
    fputs ("\n//add\n@SP\nA=M-1\nD=M\nA=A-1\nM=D+M\n" // a+b
           "@SP\nM=M-1\n"                             // pop to a
           ,
           writef);
}

void
wsub (FILE *const writef)
{
    fputs ("\n//sub\n@SP\nA=M-1\nA=A-1\nD=M\nA=A+1\nD=D-M\n" // a-b
           "A=A-1\nM=D\n@SP\nM=M-1\n"                        // pop to a
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
                    sprintf (wline,
                             "@%s\nD=A\n@5\nD=D+A\n@15\nM=D\n" // store (5 + x) in tmp var
                             "@SP\nM=M-1\nA=M\nD=M\n@15\nA=M\nM=D\n",
                             x);
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
             "@%s\nD=A\n%s\nM=M+D\n@SP\nM=M-1\nA=M\nD=M\n%s\nA=M\nM=D\n@%s\nD=A\n%s\nM=M-D\n", x,
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
            sprintf (wline, "@%s\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", k);
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
    static size_t timesCalled = 0;
    char Ttag[10] = "L", Etag[10] = "l";      // 2^15 = 32k ; log(32k) ~ 5
    sprintf (Ttag + 1, "%lu", timesCalled++); // if Less then jump to TAG
    strcpy (Etag + 1, Ttag + 1);
    char buffer[MAX_RLINE_LEN * 5];
    size_t j = 0;
    j += sprintf (buffer + j,
                  "\n//compute lt\n@SP\nA=M-1\nA=A-1\nD=M\nA=A+1\nD=D-M\n"); // compute a-b
    j += sprintf (buffer + j, "//ifgoto gate\n@%s\nD;JLT\n", Ttag);          // if a-b<0 goto TTAG
    j += sprintf (buffer + j, "//else\n@SP\nA=M-1\nA=A-1\nM=0\n@%s\n0;JMP\n",
                  Etag); // else a=0; goto ETAG
    j += sprintf (buffer + j, "//then\n(%s)\n@SP\nA=M-1\nA=A-1\nM=-1\n(%s)\n", Ttag, Etag);
    j += sprintf (buffer + j, "@SP\nM=M-1\n"); // pop b
    fputs (buffer, writef);
#ifdef DEBUG
    printf ("Info(wlt) j:%lu\t/%lu\n", j, sizeof (buffer));
#endif
}

void
weq (FILE *const writef)
{
    // NOTE literally the same as wlt
    // a-b=0?T:F
    static size_t timesCalled = 0;
    char Ttag[10] = "E", Etag[10] = "e";      // 2^15 = 32k ; log(32k) ~ 5
    sprintf (Ttag + 1, "%lu", timesCalled++); // if Less then jump to TAG
    strcpy (Etag + 1, Ttag + 1);
    char buffer[MAX_RLINE_LEN * 5];
    size_t j = 0;
    j += sprintf (buffer + j,
                  "\n//compute eq\n@SP\nA=M-1\nA=A-1\nD=M\nA=A+1\nD=D-M\n"); // compute a-b
    j += sprintf (buffer + j, "@%s\nD;JEQ\n", Ttag);                         // if a-b=0 goto TTAG
    j += sprintf (buffer + j, "@SP\nA=M-1\nA=A-1\nM=0\n@%s\n0;JMP\n", Etag); // else a=0; goto ETAG
    j += sprintf (buffer + j, "(%s)\n@SP\nA=M-1\nA=A-1\nM=-1\n(%s)\n", Ttag, Etag); // then
    j += sprintf (buffer + j, "@SP\nM=M-1\n");                                      // pop b
    fputs (buffer, writef);
#ifdef DEBUG
    printf ("Info(weq) j:%lu\t/%lu\n", j, sizeof (buffer));
#endif
}

void
wgt (FILE *const writef)
{
    // NOTE literally the same as wlt
    // a-b>0?T:F
    static size_t timesCalled = 0;
    char Ttag[10] = "G", Etag[10] = "g";      // 2^15 = 32k ; log(32k) ~ 5
    sprintf (Ttag + 1, "%lu", timesCalled++); // if Less then jump to TAG
    strcpy (Etag + 1, Ttag + 1);
    char buffer[MAX_RLINE_LEN * 5];
    size_t j = 0;
    j += sprintf (buffer + j,
                  "\n//compute gt\n@SP\nA=M-1\nA=A-1\nD=M\nA=A+1\nD=D-M\n"); // compute a-b
    j += sprintf (buffer + j, "@%s\nD;JGT\n", Ttag);                         // if a-b>0 goto TTAG
    j += sprintf (buffer + j, "@SP\nA=M-1\nA=A-1\nM=0\n@%s\n0;JMP\n", Etag); // else a=0; goto ETAG
    j += sprintf (buffer + j, "(%s)\n@SP\nA=M-1\nA=A-1\nM=-1\n(%s)\n", Ttag, Etag); // then
    j += sprintf (buffer + j, "@SP\nM=M-1\n");                                      // pop b
    fputs (buffer, writef);
#ifdef DEBUG
    printf ("Info(wgt) j:%lu\t/%lu\n", j, sizeof (buffer));
#endif
}

void
wnot (FILE *const writef)
{
    fputs ("\n//not\n@SP\nA=M-1\nM=!M\n", writef);
}

void
wand (FILE *const writef)
{
    fputs ("\n//and\n@SP\nM=M-1\nA=M-1\nD=M\nA=A+1\nD=D&M\nA=A-1\nM=D\n", writef);
}

void
wor (FILE *const writef)
{
    fputs ("\n//or\n@SP\nM=M-1\nA=M-1\nD=M\nA=A+1\nD=D|M\nA=A-1\nM=D\n", writef);
}

void
wlabel (char *const line, FILE *const writef)
{
    // NOTE `label L`
    char label[60];
    sscanf (line, " label %s ", label);
    fputs ("\n//label\n(", writef);
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
    fputs ("\n//ifgoto\n@SP\nM=M-1\nA=M\nD=M\n@", writef); // if-goto also pops
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
                "($return$)"                                                    // tag to jmp to
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
    j += sprintf (wline + j, "\n//call %s %s\n", fname, argc);

#define PUSH_A "D=A\n@SP\nM=M+1\nA=M-1\nM=D\n"

    // push TAG
    j += sprintf (wline + j, "@%s\n" PUSH_A, TAG);

    // constexpr from here
    if(!written1){
        // to exit
        j+=sprintf(wline+j, "@$%s\nD=A\n@R15\nM=D\n", TAG);
        // Subroutine entrypoint
        j+= sprintf (wline + j, "($CALL$)\n");
        // push LCL
        j += sprintf (wline + j, "@LCL\nA=M\n" PUSH_A);
        // push ARG
        j += sprintf (wline + j, "@ARG\nA=M\n" PUSH_A);
        // THIS
        j += sprintf (wline + j, "@THIS\nA=M\n" PUSH_A);
        // push THAT
        j += sprintf (wline + j, "@THAT\nA=M\n" PUSH_A);
        // Subroutine exit
        j+=sprintf(wline+j, "@R15\nA=M\n0;JMP\n");
        j+=sprintf(wline+j, "($%s)\n", TAG); //subroutine return point
        written1 = true;
    }else{
        // Mem return and jump to subroutine
        j+=sprintf(wline+j, "@$%s\nD=A\n@R15\nM=D\n($CALL$)\n0;JMP\n", TAG);
        j+=sprintf(wline+j, "($%s)\n", TAG); //subroutine return point
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

#ifdef DEBUG
    printf ("Info(return) j:%lu\t/%lu\n", j, sizeof (wline));
#endif
}
