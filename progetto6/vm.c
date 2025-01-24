#include "vm.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <dirent.h>
#endif

#define MAX_RLINE_LEN 80

int file(const char*const vmPath);

int
main (int argc, char *argv[])
{
    // __ DEF CONSTANT __
    const char *const vmPath = argc >= 2 ? argv[1] : NULL; // err check further down
    /* char filePath[256+3] = "./"; */
    if(strstr(vmPath, ".vm")){
        /* strcat(filePath, vmPath); */
        /* printf("%s\n", filePath); */
        return file(vmPath);
    }else{// directory
#ifdef _WIN32
        printf("VMtranslator file.vm\n Can translate only files on Windows.\n");
        return 1;
#endif
#ifndef _WIN32
        DIR *d;
        struct dirent *dir;
        d = opendir(vmPath);
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                if(dir->d_name[0]=='.')
                    continue;
                else if(strstr(dir->d_name, ".vm")){
                    char absPath[256*2]="";
                    strcat(absPath, vmPath);
                    strcat(absPath, "/");
                    strcat(absPath, dir->d_name);
                    printf("Reading %s\n",  dir->d_name);
                    file(absPath);
                }
            }
            closedir(d);
        }
#endif
  }
}

int file(const char*const vmPath){
    char rline[MAX_RLINE_LEN];
    FILE *const readf = fopen (vmPath, "r");
    if (!readf)
        {
            printf ("Error reading file %s\n", vmPath);
            return 1;
        }

    char asmPath[MAX_RLINE_LEN];
    strcpy (asmPath, vmPath);
    for (int i = strlen(vmPath)-1; i > 0; i--)
        {
            if (asmPath[i] == '.')
                {
                    strcpy (asmPath+i, ".asm");
                    break;
                }
        }

    printf("Writing to %s\n", asmPath);

    FILE *const writef = fopen (asmPath, "w");
    if(!writef){
        printf("Error writing file %s",asmPath);
        return 1;
    }

    fputs("@256\nD=A\n@SP\nM=D\n", writef);


    while (fgets(rline, MAX_RLINE_LEN, readf)!=NULL)
        {
            size_t startp = 0;
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
                case 'a':
                    if(rline[startp+1]=='n')
                        wand(writef);
                    else
                        wadd(writef);
                    continue;
                case 'f': // function decl
                    if (wfunctiondecl (rline + startp, writef))
                    {
                        printf("Function declaration error:%s\n", rline);
                        goto exit;
                    }
                  continue;
                case 'e':
                    weq(writef);
                    continue;
                case 'o':
                    wor(writef);
                    continue;
                case 'r':
                    wfunctionreturn (writef);
                    continue;
                case 's':
                    wsub(writef);
                    continue;
                case 'n':
                    if(rline[startp+1]=='e')
                        wneg(writef);
                    else
                        wnot(writef);
                    continue;
                case 'c': // function call
                    wfunctioncall (rline, writef);
                    continue;
                case 'i': // if-goto
                    wifgoto(rline, writef);
                    continue;
                case 'p':
                    if(rline[startp+1]=='o'){
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
                    continue;
                case 'l':
                    if(rline[startp+1]=='a'){//lable
                        wlabel(rline, writef);
                    }else{
                        wlt(writef);
                    }
                    continue;
                case 'g':
                    if(rline[startp+1]=='t')
                        wgt(writef);
                    else
                        wgoto(rline, writef);
                    continue;
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
    return 0;
}

void wneg(FILE*const writef){
    fputs("\n//neg\n@SP\nA=M-1\nM=-M\n",writef);
}


void wadd(FILE*const writef){
    //literally the same as wsub
     fputs("\n//add\n@SP\nA=M-1\nD=M\nA=A-1\nD=D+M\n" // a+b
          "M=D\n@SP\nM=M-1\n" // pop to a
          , writef);
}

void wsub(FILE*const writef){

    fputs("\n//sub\n@SP\nA=M-1\nA=A-1\nD=M\nA=A+1\nD=D-M\n" // a-b
          "A=A-1\nM=D\n@SP\nM=M-1\n" // pop to a
          , writef);

}

int wpop(char* const line, FILE* const writef){
    // NOTE line format: `pop segment x`
    char seg[10], x[10];
    sscanf(line, "pop %s %s", seg, x);

    //comment
    char wline[MAX_RLINE_LEN*2]="";
    sprintf(wline, "\n//pop %s %s\n", seg, x);
    fputs(wline, writef);
    strcpy(wline, "");

    switch(seg[0]){
        case 'a':
            sprintf(wline, "@%s\nD=A\n@ARG\nM=M+D\n@SP\nM=M-1\nA=M\nD=M\n@ARG\nA=M\nM=D\n@%s\nD=A\n@ARG\nM=M-D\n", x,x);
            fputs(wline, writef);
            return 0;
        case 'l':
            sprintf(wline, "@%s\nD=A\n@LCL\nM=M+D\n@SP\nM=M-1\nA=M\nD=M\n@LCL\nA=M\nM=D\n@%s\nD=A\n@LCL\nM=M-D\n", x,x);
            fputs(wline,writef);
            return 0;
        case 's':
            sprintf(wline, "@SP\nM=M-1\nA=M\nD=M\n@Xxx.%s\nM=D\n", x);
            fputs(wline, writef);
            return 0;
        case 'c': // impossibile; we discard the value
            fputs("@SP\nM=M-1\n", writef);
            return 0;
        default:
            return -1;
    }
    return -1;
}

int wpush(char* const line, FILE* const writef){
    //NOTE line format: `push segment k`
    char seg[10];
    char k[10];
    sscanf(line, "push %s %s", seg, k);

    //comment
    char wline[MAX_RLINE_LEN] = "";
    sprintf(wline, "\n//push %s %s\n", seg, k);
    fputs(wline, writef);
    strcpy(wline, "");
    size_t j = 0;

    switch(seg[0]){
        case 'a': // argument -> @ARG
            strcpy(seg, "@ARG");
            break;
        case 'l': // local -> @LCL
            strcpy(seg, "@LCL");
            break;
        case 's': // static -> 16
            sprintf(wline, "@Xxx.%s\nD=M\n@SP\nM=M+1\nA=M-1\nM=D\n", k);
            fputs(wline, writef);
            return 0;
        case 'c': // constant -> special case
            sprintf(wline, "@%s\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n", k);
            fputs(wline, writef);
            return 0;
        default:
            return -1;
    }
    j+=sprintf(wline+j, "%s\nD=M\n@%s\nA=D+A\nD=M\n@SP\nM=M+1\nA=M-1\nM=D\n", seg, k);
    fputs(wline, writef);
    return 0;
}

void wgoto(char*const line, FILE*const writef){
    //NOTE `goto L`
    char label[60];
    char buffer[MAX_RLINE_LEN];
    size_t j = 0;
    sscanf(line, "goto %s ", label);
    j+=sprintf(buffer+j, "\n//goto\n@%s\n0;JMP\n", label);
    fputs(buffer, writef);
}

void wlt(FILE*const writef){
    //NOTE: `lt`
    // *(SP-2) < *(SP-1) as a<b
    // a-b<0?T:F
    // then we pop b and overwrite on a
    static size_t timesCalled = 0;
    char Ttag[7] = "L", Etag[7]="l"; // 2^15 = 32k ; log(32k) ~ 5
    sprintf(Ttag+1, "%lu", timesCalled++); // if Less then jump to TAG
    strcpy(Etag+1, Ttag+1);
    char buffer[MAX_RLINE_LEN*5];
    size_t j = 0;
    j+=sprintf(buffer+j, "\n//compute lt\n@SP\nA=M-1\nA=A-1\nD=M\nA=A+1\nD=D-M\n"); //compute a-b
    j+=sprintf(buffer+j, "//ifgoto gate\n@%s\nD;JLT\n",Ttag); //if a-b<0 goto TTAG
    j+=sprintf(buffer+j, "//else\n@SP\nA=M-1\nA=A-1\nM=0\n@%s\n0;JMP\n", Etag); // else a=0; goto ETAG
    j+=sprintf(buffer+j, "//then\n(%s)\n@SP\nA=M-1\nA=A-1\nM=-1\n(%s)\n", Ttag, Etag);
    j+=sprintf(buffer+j, "@SP\nM=M-1\n"); // pop b
    fputs(buffer, writef);
}

void weq(FILE*const writef){
    // NOTE literally the same as wlt
    // a-b=0?T:F
    static size_t timesCalled = 0;
    char Ttag[7] = "E", Etag[7]="e"; // 2^15 = 32k ; log(32k) ~ 5
    sprintf(Ttag+1, "%lu", timesCalled++); // if Less then jump to TAG
    strcpy(Etag+1, Ttag+1);
    char buffer[MAX_RLINE_LEN*5];
    size_t j = 0;
    j+=sprintf(buffer+j, "\n//compute eq\n@SP\nA=M-1\nA=A-1\nD=M\nA=A+1\nD=D-M\n"); //compute a-b
    j+=sprintf(buffer+j, "@%s\nD;JEQ\n",Ttag); //if a-b=0 goto TTAG
    j+=sprintf(buffer+j, "@SP\nA=M-1\nA=A-1\nM=0\n@%s\n0;JMP\n", Etag); // else a=0; goto ETAG
    j+=sprintf(buffer+j, "(%s)\n@SP\nA=M-1\nA=A-1\nM=-1\n(%s)\n", Ttag, Etag); //then
    j+=sprintf(buffer+j, "@SP\nM=M-1\n"); // pop b
    fputs(buffer, writef);
}

void wgt(FILE*const writef){
     // NOTE literally the same as wlt
    // a-b>0?T:F
    static size_t timesCalled = 0;
    char Ttag[7] = "G", Etag[7]="g"; // 2^15 = 32k ; log(32k) ~ 5
    sprintf(Ttag+1, "%lu", timesCalled++); // if Less then jump to TAG
    strcpy(Etag+1, Ttag+1);
    char buffer[MAX_RLINE_LEN*5];
    size_t j = 0;
    j+=sprintf(buffer+j, "\n//compute gt\n@SP\nA=M-1\nA=A-1\nD=M\nA=A+1\nD=D-M\n"); //compute a-b
    j+=sprintf(buffer+j, "@%s\nD;JGT\n",Ttag); //if a-b>0 goto TTAG
    j+=sprintf(buffer+j, "@SP\nA=M-1\nA=A-1\nM=0\n@%s\n0;JMP\n", Etag); // else a=0; goto ETAG
    j+=sprintf(buffer+j, "(%s)\n@SP\nA=M-1\nA=A-1\nM=-1\n(%s)\n", Ttag, Etag); //then
    j+=sprintf(buffer+j, "@SP\nM=M-1\n"); // pop b
    fputs(buffer, writef);
}

void wnot(FILE*const writef){
    // a==0?-1:0
    /* static size_t timesCalled = 0; */
    /* char Ttag[7] = "N", Etag[7]="n"; // 2^15 = 32k ; log(32k) ~ 5 */
    /* sprintf(Ttag+1, "%lu", timesCalled++); // if Less then jump to TAG */
    /* strcpy(Etag+1, Ttag+1); */
    /* char buffer[MAX_RLINE_LEN*5]; */
    /* size_t j = 0; */
    /* j+=sprintf(buffer+j, "\n//not gate\n@SP\nA=M-1\n@%s\nM;JEQ\n",Ttag); //if a=0 goto TTAG */
    /* j+=sprintf(buffer+j, "@SP\nA=M-1\nM=0\n@%s\n0;JMP\n", Etag); // else a=0; goto ETAG */
    /* j+=sprintf(buffer+j, "(%s)\n@SP\nA=M-1\nM=-1\n(%s)\n", Ttag, Etag); //then */
    fputs("\n//not\n@SP\nA=M-1\nM=!M\n", writef);

}

void wand(FILE*const writef){
    // -1 = T | 0 = F
    // ---------------
    // -1, -1 -> -1x
    //  0,  0 -> 0 x
    // -1,  0 -> 0 x
    //  0, -1 -> 0 x
    /* static size_t timesCalled = 0; */
    /* char Etag[7] = "A", Ftag[7]="a"; // 2^15 = 32k ; log(32k) ~ 5 */
    /* sprintf(Etag+1, "%lu", timesCalled++); */
    /* strcpy(Ftag+1, Etag+1); */
    /* char buffer[MAX_RLINE_LEN*5]; */
    /* size_t j = 0; */
    /* j+=sprintf(buffer+j, "\n//and\n@SP\nA=M-1\n@%s\nM;JEQ\n",Ftag); // b=0? goto FALSE */
    /* j+=sprintf(buffer+j, "@SP\nA=M-1\n@%s\nM;JEQ\n",Ftag); // else check a */
    /* j+=sprintf(buffer+j, "@SP\nA=M-1\nA=A-1\n@%s\nM;JEQ\n", Ftag); // a=0? goto FALSE */
    /* j+=sprintf(buffer+j, "@SP\nA=M-1\nA=A-1\nM=-1\n@%s\nM;JEQ\n", Etag); // set true and exit */
    /* j+=sprintf(buffer+j, "(%s)\n@SP\nA=M-1\nA=A-1\nM=0\n@%s\n0;JMP\n",Ftag, Etag); //set FALSE and exit */
    /* j+=sprintf(buffer+j, "(%s)\n@SP\nM=M-1\n", Etag); // EXIT: POP B */

    fputs("\n//and\n@SP\nM=M-1\nA=M-1\nD=M\nA=A+1\nD=D&M\nA=A-1\nM=D\n", writef);

}

void wor(FILE*const writef){
    /* static size_t timesCalled = 0; */
    /* char Etag[7] = "A", Ttag[7]="a"; // 2^15 = 32k ; log(32k) ~ 5 */
    /* sprintf(Etag+1, "%lu", timesCalled++); */
    /* strcpy(Ttag+1, Etag+1); */
    char buffer[MAX_RLINE_LEN*5];
    size_t j = 0;

    j+=sprintf(buffer+j, "\n//or\n@SP\nM=M-1\nA=M-1\nD=M\nA=A+1\nD=D|M\nA=A-1\nM=D\n");

    /* j+=sprintf(buffer+j, "\n//or\n@SP\nA=M-1\n@%s\nM;JLT\n",Ttag); // b<0? goto TRUE */
    /* j+=sprintf(buffer+j, "@SP\nA=M-1\n@%s\nM;JLT\n",Ttag); // else check a */
    /* j+=sprintf(buffer+j, "@SP\nA=M-1\nA=A-1\n@%s\nM;JEQ\n", Ttag); // a=0? goto TRUE */
    /* j+=sprintf(buffer+j, "@SP\nA=M-1\nA=A-1\nM=-1\n@%s\nM;JEQ\n", Etag); // set true and exit */
    /* j+=sprintf(buffer+j, "(%s)\n@SP\nA=M-1\nA=A-1\nM=0\n@%s\n0;JMP\n",Ttag, Etag); //set FALSE and exit */
    /* j+=sprintf(buffer+j, "(%s)\n@SP\nM=M-1\n", Etag); // EXIT: POP B */
    fputs(buffer, writef);


}

void wlabel(char*const line, FILE*const writef){
    // NOTE `label L`
    char label[60];
    sscanf(line, "label %s ", label);
    fputs("\n//label\n(", writef);
    fputs(label, writef);
    fputs(")\n",writef);
}
void wifgoto(char*const line, FILE*const writef){
    // NOTE line format: `if-goto L`
    // check stack if bool true (-1)
    // or false(0)
    char label[10];
    sscanf(line, "if-goto %s ", label);
    fputs("\n//ifgoto\n@SP\nM=M-1\nA=M\nD=M\n@", writef); // if-goto also pops
    fputs(label, writef);
    fputs("\nD;JLT // J if true\n", writef);
}

int wfunctiondecl (char *const line, FILE *const writef)
{
    // NOTE line format: `function name locc`
    // We'll set a TAG to JMP to

    char fname[MAX_RLINE_LEN-20];
    int locc;
    sscanf(line, "function %s %d", fname, &locc);

    char wline[MAX_RLINE_LEN];
    sprintf(wline, "\n//fun %s %d\n", fname, locc);
    fputs(wline,writef);

    // (fname)
    strcpy(wline, "(");
    strcat (wline, fname);
    strcat (wline, ")\n");
    fputs (wline, writef);

    // NOTE: correct LCL already set by call
    for(int i = 0; i<locc;i++)
        wpush("push constant 0\n", writef);

    return 0;
}

void
wfunctionreturn ( FILE *const writef)
{
    // NOTE line format: `return`
    fputs("\n//return \n"
          "//FRAME=LCL\n@LCL\nD=M\n@R13\nM=D\n" /* FRAME=LCL */
          "//RET=*(FRAME-5)\n@R13\nD=M\n@5\nD=D-A\nA=D\nD=M\n@R14\nM=D\n" /* RET=*(FRAME-5) */
          "//Arg[0]=reval\n@SP\nM=M-1\nA=M\nD=M\n@ARG\nA=M\nM=D\n" /* Copy return val to arg[0] ; */
          "//SP=ARG+1\n@ARG\nD=M+1\n@SP\nM=D\n" // SP=ARG+1 // restore sp
          "//that\n@R13\nA=M-1\nD=M\n@THAT\nM=D\n" // restore that
          "//this\n@R13\nD=M\n@2\nA=D-A\nD=M\n@THIS\nM=D\n" // restore this
          "//arg\n@R13\nD=M\n@3\nA=D-A\nD=M\n@ARG\nM=D\n" // restore arg
          "//lcl\n@R13\nD=M\n@4\nA=D-A\nD=M\n@LCL\nM=D\n" // restore lcl
          "//return\n@R14\nA=M\n0;JMP\n"
          ,writef);

}

void
wfunctioncall (char *const line, FILE *const writef)
{
    // NOTE line format: `call fname argc`
    // To do this we'll push a TAG:
    // @TAG, D=A, @0, A=M, M=D, @0, M=M+1

    // get TAG
    static size_t timesCalled = 0; // src for TAG gen
    char TAG[7] = "C"; // log(32k) ~ 5
    sprintf(TAG+1, "%lu", timesCalled);

    // get fname
    char fname[MAX_RLINE_LEN-20], argc[10];
    sscanf(line, "call %s %s", fname, argc);
    // comment
    char wline[MAX_RLINE_LEN*6] = "";
    size_t j = 0;
    j+=sprintf(wline+j, "\n//call %s %s\n", fname, argc);

#define PUSH_A "D=A\n@SP\nM=M+1\nA=M-1\nM=D\n"

    // push TAG
    j+=sprintf(wline+j, "@%s\n" PUSH_A, TAG);
    // push LCL
    j+=sprintf(wline+j, "@LCL\nA=M\n" PUSH_A);
    // push ARG
    j+=sprintf(wline+j, "@ARG\nA=M\n" PUSH_A);
    //THIS
    j+=sprintf(wline+j, "@THIS\nA=M\n" PUSH_A);
    //push THAT
    j+=sprintf(wline+j, "@THAT\nA=M\n" PUSH_A);
    fputs(wline, writef);
    strcpy(wline, "");
    j = 0;

    // reposition ARG
    // ARG = sp - argc - 5;
    j+=sprintf(wline+j, "@SP\nD=M\n@%s\nD=D-A\n@5\nD=D-A\n@ARG\nM=D\n", argc);
    // LCL = SP
    j+=sprintf(wline+j, "@SP\nD=M\n@LCL\nM=D\n");

    // JMP to function
    j+=sprintf(wline+j, "@%s\n0;JMP\n", fname);

    // set return tag
    j+=sprintf(wline+j, "(%s)\n", TAG);
    fputs(wline, writef);

    timesCalled++;
}
