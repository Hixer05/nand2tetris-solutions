#include "vm.h"
#include "lib/binary.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
 * this prog makes strong use of TAGS
 */

#define MAX_RLINE_LEN 80

int
main (int argc, char *argv[])
{
    // __ DEF CONSTANT __
    constexpr size_t FUNCTION_HT_SIZE = 100;
    constexpr size_t KEY_MAX_LEN = 80;
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
                    strcpy (asmPath, ".hack");
                    asmPath[i + (sizeof (".hack") / sizeof (char))] = '\0';
                    break;
                }
        }

    FILE *const writef = fopen (asmPath, "w");

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
                                  // no break; let's see if @rline[startp] starts a smth else
                case '\0':
                case '\r':
                case '\n':
                case '/':
                    continue;
                case 'f': // function decl
                  if (wfunctiondecl (rline + startp, writef))
                    goto exit;
                  continue;
                case 'b':
                    wfunctionbreak (rline, writef);
                case 'c': // function call
                    wfunctioncall (rline, writef);
                case 'i': // if-goto

                default: // unrecog
                    break;
                }
        }

exit:
    fclose (readf);
    fclose (writef);
}

[[nodiscard]] char **const
initHashTable (const size_t size, const size_t strLen)
{
    char **const hashTable = malloc (sizeof (char *) * size);
    for (register int i = 0; i < size; i++)
        {
                    hashTable[i] = malloc (sizeof (char) * strLen);
                    if (!hashTable[i]) // if failed to malloc mem
                        return NULL;
                    else // no err
                        hashTable[i][0] = '\0';
        }
    return hashTable;
}

[[nodiscard]] int inline wfunctiondecl (char *const line, FILE *const writef)
{
    // NOTE line format: `function name locc`
    // We'll set a TAG to JMP to

    int locc;
    constexpr size_t fnamep = sizeof ("function ") / sizeof (char) - 1;
    char curfname[MAX_RLINE_LEN];
    strcpy (curfname, line + fnamep);
    // trim function's locc
    curfname[strlen(curfname)] = '\0';

    // save locc
    int offset = strlen(curfname)+fnamep+1;
    while (line[offset] >= '0' && line[offset] <= '9') // is num
      offset++;

    // FIXME: dec2int outputs int but can only parse uint,
    // though it's not a probl. for us now
    if (dec2int (line, offset, &locc))
    {
      printf ("Synthax error \n");
      return -1;
    }

    // (fname)
    // NOTE: could've used wlabel but it's just 4 lines
    {
        char wline[MAX_RLINE_LEN] = "(";
        strcat (wline, curfname);
        strcat (wline, ")\n");
        fputs (wline, writef);
    }

    // init local locc variables
    // @1
    // A = M // M[M[1]]
    fputs("@1\nA=M\n", writef);
    for(int i = 0; i<locc; i++){
        // M = 0 // .. = 0
        // A = A+1 // next
        fputs("M=0\nA=A+1\n",writef);
    }


    return 0;
}

// removes ' ' , '\r', '\n'
void trim(char* str){
    for(int i = 0; str[i]!='\0'; i++)
        switch(str[i]){
            case '\n':
            case '\r':
                str[i] = ' ';
            case ' ':
                strcpy(str+i, str+i+1);
                --i;
                break;
        }
}

void wlabel(char* const line, FILE * const writef){
    // NOTE: `label L`
    constexpr size_t labelp = sizeof("label ")/sizeof(char);
    char wline[MAX_RLINE_LEN] = "(";
    strcat(wline, line+labelp);
    trim(wline);
    strcat(wline, ")\n");
    fputs(wline, writef);
}

void
wfunctionbreak (char *const line, FILE *const writef)
{
    // NOTE line format: `break`
    // we will just jump back to caller
    fputs("@0\nD=M\nA=D\n0;JMP", writef);
}

void
wfunctioncall (char *const line, FILE *const writef)
{
    // NOTE line format: `call fname argc`
    // First first we must set M[0] to program line (after JMP)
    // To do this we'll set a TAG and @TAG, D=A, @0, M=D
    // This way we'll save the cur line in M[0]
    // We'll make this log_2(TAG) = 5, why? Cuz the ROM is ~32K word long
    // Then We will just JMP to FNAME TAG

    // get TAG
    static size_t timesCalled = 0; // src for TAG gen
    char TAG[17];
    (void) int2bin16(timesCalled, TAG); // errors can be ignored, see note
    // since this is 16 bit we'll just copy the bit 11..15
    for(int i = 0; i<6; i++)
      TAG[i]=TAG[i+10];
    TAG[6] = '\0';

    // get fname
    char fname[MAX_RLINE_LEN];
    int i = sizeof("call ")/sizeof(char);
    for (; line[i] != ' '; i++){
        fname[i] = line[i];
    }
    fname[i] = '\0';

    // set M[0] to TAG
    char wline[MAX_RLINE_LEN] = "@";
    strcat (wline, TAG);
    strcat (wline, "\nD=A\n@0\nM=D\n");
    fputs(wline, writef);

    // JMP to function
    wline[0] = '@';
    wline[1] = '\0'; // prev. used, need this for strcat
    strcat(wline, fname);
    strcat(wline, "\n0;JMP\n");
    fputs(wline, writef);

    // set return tag
    wline[0] = '(';
    wline[1] = '\0'; // prev. used, need this for strcat
    strcat(wline, TAG);
    strcat(wline, ")\n");
    fputs(wline, writef);

    timesCalled++;
}

void wpush(char *const line, FILE *const writef){
    //NOTE: `push segment address`
    // 1. Get M[0] (StackPointer)
    // 2. Get segment[address]
    // 3. M[M[0]] = segment[address]
    // 4. M[0] = M[0] + 1

}
