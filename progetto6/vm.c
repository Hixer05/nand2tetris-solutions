#include "vm.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
 * this prog makes strong use of TAGS
 */

#define MAX_RLINE_LEN 80
#define FUNCTION_HT_SIZE 100
#define KEY_MAX_LEN 80

int
main (int argc, char *argv[])
{
    // __ DEF CONSTANT __
    const char *const vmPath = argc >= 2 ? argv[1] : NULL; // err check further down

    // __ COMPUTE __
    char rline[MAX_RLINE_LEN];
    FILE *const readf = fopen (vmPath, "r");
    if (!readf)
        {
            printf ("Error reading file %s\n", vmPath);
            return 1;
        }

    char asmPath[MAX_RLINE_LEN];
    strcpy (asmPath, vmPath);
    for (int i = 0; i < MAX_RLINE_LEN; i++)
        {
            if (asmPath[i] == '.')
                {
                    strcpy (asmPath+i, ".hack");
                    asmPath[i + (sizeof (".hack") / sizeof (char))] = '\0';
                    break;
                }
        }

    printf("Writing to %s\n", asmPath);

    FILE *const writef = fopen (asmPath, "w");
    if(!writef){
        printf("Error writing file %s",asmPath);
        return 1;
    }

    while (fgets (rline, MAX_RLINE_LEN, readf))
        {
            register size_t startp = 0;
            switch (rline[startp])
                {
                case ' ': // commands or other stuff might be indented
                    while (startp < MAX_RLINE_LEN && rline[startp] == ' ')
                        startp++;
                    if (startp + 1 == MAX_RLINE_LEN)
                        continue; // empty line!
                                  // no break; let's see if @rline[startp] starts smth else
                case '\0':
                case '\r':
                case '\n':
                case '/':
                    continue;
                case 'f': // function decl
                  if (wfunctiondecl (rline + startp, writef))
                  {
                    printf("Function declaration error:%s\n", rline);
                    goto exit;
                  }
                  continue;
                case 'b':
                    wfunctionreturn (rline, writef);
                    continue;
                case 'c': // function call
                    wfunctioncall (rline, writef);
                    continue;
                case 'i': // if-goto
                    wifgoto(rline, writef);
                    continue;
                case 'p':
                    if(rline[1]=='o'){
                        if(wpop(rline, writef)){
                            printf("Error pop:%s", rline);
                            goto exit;
                        }
                    }else{
                        if(wpush(rline, writef)){
                            printf("Error push:%s", rline);
                            goto exit;
                        }
                    }
                default: // unrecog
                    #ifndef DEBUG
                    printf("Unrecognized synthax:\n%s", rline);
                    goto exit;
                    #endif
                    break;
                }
        }

exit:
    fclose (readf);
    fclose (writef);
}

int wpop(char* const line, FILE* const writef){
    // NOTE line format: `pop segment x`
    // comput *K = seg+x
    // @$seg ; D=A ; @$x ; D=D+A; @K; M=D ;
    // get stack[sp-1]
    // @SP ; A=A-1; A=M ; D=M;
    // copy to segment[x]
    // @K; A=M; A=M ; M=D;
    // stack--

    char seg[10], x[10];
    sscanf(line, "push %s %s", seg, x);

    //comment
    char wline[MAX_RLINE_LEN]="";
    sprintf(wline, "//pop %s %s\n", seg, x);
    fputs(wline, writef);
    strcpy(wline, "");

    switch(seg[0]){
        case 'a':
            strcat(wline, "@ARG");
            break;
        case 'l':
            strcat(wline, "@LCL");
            break;
        case 's':
            strcat(wline, "@16");
        case 'c': // impossibile; we discard the value
            fputs("@SP\nM=M-1\n", writef);
            return 0;
    }
    strcat(wline, "\nD=A\n@");
    strcat(wline, x);
    strcat(wline, "\bD=D+A\n@K\nM=D\n@SP\nA=A-1\nA=M\n");
    fputs(wline, writef); // too long
    strcpy(wline, "D=M\n@K\nA=M\nA=M\nM=D\n@0\nM=M-1\n");
    fputs(wline, writef);
    return 0;
}

int wpush(char* const line, FILE* const writef){
    //NOTE line format: `push segment k`
    // switch segment
    // get K = segment[k]
    // @$SEGMENT ; D=A; @$K ; A=A+D; D=M;
    // load segment[k] to stack:
    // @SP; A=M ; M = D;
    // SP++

    char seg[10];
    char k[10];
    sscanf(line, "push %s %s", seg, k);

    //comment
    char wline[MAX_RLINE_LEN] = "";
    sprintf(wline, "//push %s %s\n", seg, k);
    fputs(wline, writef);
    strcpy(wline, "");

    switch(seg[0]){
        case 'a': // argument -> @ARG
            strcat(wline, "@ARG\n");
            break;
        case 'l': // local -> @LCL
            strcat(wline, "@LCL\n");
            break;
        case 's': // static -> 16
            strcat(wline, "@16\n");
            break;
        case 'c': // constant -> special case
            strcat(wline, "@");
            strcat(wline, k);
            strcat(wline, "\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
            fputs(wline, writef);
            return 0;
        default:
            return -1;
    }
    strcat(wline, "D=A\n@");
    strcat(wline, k);
    strcat(wline, "\nA=A+D\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n");
    fputs(wline, writef);
    return 0;
}


// TODO
void wifgoto(char*const line, FILE*const writef){
   // NOTE line format:

}

// FIXME
int wfunctiondecl (char *const line, FILE *const writef)
{
    // NOTE line format: `function name locc`
    // We'll set a TAG to JMP to

    char fname[MAX_RLINE_LEN], locc[10];
    sscanf(line, "function %s %s", fname, locc);

    // (fname)
    char wline[MAX_RLINE_LEN] = "(";
    strcat (wline, fname);
    strcat (wline, ")\n");
    fputs (wline, writef);

    return 0;
}

// FIXME
void
wfunctionreturn (char *const line, FILE *const writef)
{
    // NOTE line format: `return`
    // we will just jump back to caller

}

// FIXME
void
wfunctioncall (char *const line, FILE *const writef)
{
    // NOTE line format: `call fname argc`
    // To do this we'll push a TAG:
    // @TAG, D=A, @0, A=M, M=D, @0, M=M+1


    // get TAG
    static size_t timesCalled = 0; // src for TAG gen
    char TAG[10] = "V";
    sprintf(TAG+1, "%lu", timesCalled);

    // get fname
    char fname[MAX_RLINE_LEN], argc[10];
    sscanf(line, "call %s %s", fname, argc);
    // comment
    char wline[MAX_RLINE_LEN] = "";
    sprintf(wline, "//call %s %s\n", fname, argc);
    fputs(wline, writef);

#define PUSH_A "D=A\n@0\nA=M\nM=D\n@0\nM=M+1\n"

    // push TAG
    strcpy(wline, "@");
    strcat (wline, TAG);
    strcat (wline, "\n" PUSH_A);
    fputs(wline, writef);

    // push LCL
    strcpy(wline, "");
    strcat(wline, "@LCL\n" PUSH_A);
    fputs(wline, writef);

    // push ARG
    strcpy(wline, "");
    strcat(wline, "@ARG\n" PUSH_A);
    fputs(wline, writef);

    // push THIS
    strcpy(wline, "");
    strcat(wline, "@THIS\n" PUSH_A);
    fputs(wline, writef);

    //push THAT
    strcpy(wline, "");
    strcat(wline, "@THAT\n" PUSH_A);
    fputs(wline, writef);

    // reposition ARG
    strcpy(wline, "@");
    strcat(wline, argc);

    // ARG (D) = sp - argc - 5; A=D;
    strcat(wline, "@SP\nA=M\nD=M\n@5\nD=D-A\n@ARG\nA=M\nM=D\n");
    fputs(wline, writef);
    fputs(PUSH_A, writef);

    // LCL = SP
    fputs("@SP\nD=M\n@LCL\nM=D\n", writef);

    // JMP to function
    strcpy(wline, "@");
    strcat(wline, fname);
    strcat(wline, "\n0;JMP\n");
    fputs(wline, writef);

    // set return tag
    strcpy(wline, "(");
    strcat(wline, TAG);
    strcat(wline, ")\n");
    fputs(wline, writef);

    timesCalled++;
}
