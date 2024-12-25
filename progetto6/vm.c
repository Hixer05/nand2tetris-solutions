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
    curfname[strlen (curfname)] = '\0'; // FIXME: strlen(curfname) punta ben oltre locc

    // save locc
    int offset = strlen (curfname) + fnamep + 1;
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
    fputs ("@1\nA=M\n", writef);
    for (int i = 0; i < locc; i++)
        {
            // M = 0 // .. = 0
            // A = A+1 // next
            fputs ("M=0\nA=A+1\n", writef);
        }

    return 0;
}

// removes ' ' , '\r', '\n'
void
trim (char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
        switch (str[i])
            {
            case '\n':
            case '\r':
                str[i] = ' ';
            case ' ':
                strcpy (str + i, str + i + 1);
                --i;
                break;
            }
}

void
wlabel (char *const line, FILE *const writef)
{
    // NOTE: `label L`
    constexpr size_t labelp = sizeof ("label ") / sizeof (char);
    char wline[MAX_RLINE_LEN] = "(";
    strcat (wline, line + labelp);
    trim (wline);
    strcat (wline, ")\n");
    fputs (wline, writef);
}

void
wfunctionbreak (char *const line, FILE *const writef)
{
    // NOTE line format: `break`
    // we will just jump back to caller
    fputs ("@0\nD=M\nA=D\n0;JMP", writef);
}

void
wfunctioncall (char *const line, FILE *const writef)
{
    // parse line (need 4 later)
    // NOTE: `call F K`
    char fname[MAX_RLINE_LEN];
    char K[MAX_RLINE_LEN];
    sscanf (line, "call %s %s", fname, K);

    // set activation record
    // set return-address
    // @return-address // how do we gen this?? as we did
    // D=A
    // @0
    // @M // m[m[0]] == m[sp]; write to top stack
    // M=D  // (return address)
    // A=A+1

    // Generate return-address TAG
    // We'll make this log_2(TAG) = 16, why? Cuz the ROM is ~32K word long
    static size_t timesCalled = 0; // src for TAG gen
    char TAG[17];
    (void)int2bin16 (timesCalled, TAG); // errors can be ignored, see note
    TAG[16] = '\0';                     // REVIEW: necessary?

    // set return-tag
    {
        char wline[MAX_RLINE_LEN] = "@";
        strcat (wline, TAG);
        strcat (wline, "\nD=A\n@0\n@M\nM=D\nA=A+1\n");
        fputs (wline, writef);
    }

    // push LCL, ARG, THIS, THAT
    // @N
    // D=M // D = LCL/.../THAT (based on N)
    // @0
    // @M
    // M=D
    // A=A+1

    {
        char positions[] = { '1', '2', '3', '4' };
        for (int i = 0; i < 4; i++)
            {
                char wline[MAX_RLINE_LEN] = "@";
                wline[1] = positions[i];
                wline[2] = '\0';
                strcat (wline, "\nD=M\n@0\n@M\nM=D\nA=A+1\n");
                fputs (wline, writef);
            }
    }
    // ARG = SP-n-5
    // @0
    // D=M
    // @2
    // M=D
    // This twice:
    // @N // minus N = {5,N}
    // D=A
    // @2
    // M=M-D
    {
        char wline[MAX_RLINE_LEN] = "@0\nD=M\n@2\nM=D\n@";
        char *N[] = { "5", K };
        // REVIEW: does K have \n?
        for (int i = 0; i < 2; i++)
            {
                strcat (wline, N[i]);
                strcat (wline, "\nD=A\n@2\nM=M-D\n");
            }
        fputs (wline, writef);
    }
    // LCL = SP
    fputs ("@0\nD=M\n@1\nM=D\n", writef);

    // goto FNAME
    {
        char wline[MAX_RLINE_LEN] = "@";
        strcat (wline, fname);
        strcat (wline, "\n0;JMP\n");
        fputs (wline, writef);
    }

    // finally set (return-tag)
    {
        char wline[MAX_RLINE_LEN] = "(";
        strcat (wline, TAG);
        strcat (wline, ")\n");
    }
}

void
wpush (char *const line, FILE *const writef)
{
    // NOTE: `push segment address`
    char segment[20]; // ball-park idc
    char address[5];  // up to 99'999
    sscanf (line, "push %s %s", segment, address);
    const char *poss_seg[] = { "argument", "local", "constant", "static" }; // sorted by prob of appear.
    const char *assoc_pos[] = { "2", "1", "", "" };
    for (int i = 0; i < 4; i++)
        {
            if (!strcmp (address, poss_seg[i]))
                continue;
            switch (i)
                {
                case 0:
                case 1:
                { // get pos to read, read in D, get sp, write to sp
                        char wline[MAX_RLINE_LEN] = "@"; //"N\nD=M\n@addr\nA=A+D\nD=M\n@0\n@M\nM=D";
                        strcat(wline, assoc_pos[i]);
                        strcat(wline, "\nD=M\n@addr\nA=A+D\nD=M\n@0\n@M\nM=D\nA=A+1\n");
                        fputs(wline, writef);
                        break;
                    }
                case 2:
                {// get address, M[SP] = adrress;; this is the way constant works
                    char wline[MAX_RLINE_LEN]="@";
                    strcat(wline, address);
                    strcat(wline, "\nD=A\n@0\n@M\nM=D\nA=A+1\n");
                }
                    break;
                case 3:
                {
                    char wline[MAX_RLINE_LEN]="@16\nD=A\n@";
                    strcat(wline, address);
                    strcat(wline, "\nA=A+D\nD=M\n@0\n@M\nM=D\nA=A+1\n");
                    fputs(wline, writef);
                }
                }
            break;
        }
}

void wpop(char*const line, FILE*const writef){
    char segment[10], address[5];
    sscanf(line, "pop %s %s", segment, address);
    const char *poss_seg[] = { "argument", "local", "constant", "static" }; // sorted by prob of appear.
    const char *assoc_pos[] = { "2", "1", "", "" };
    for (int i = 0; i < 4; i++)
        {
            if (!strcmp (address, poss_seg[i]))
                continue;
            switch (i)
                {
                case 0:
                case 1:
                { // edit ARG/LCL--
                        char wline[MAX_RLINE_LEN] = "@";
                        strcat(wline, assoc_pos[i]);
                        strcat(wline, "\nM=M-1\n");
                        fputs(wline, writef);
                        break;
                    }
                case 2: // pop const, lol
                    break;
                case 3:
                    fputs("@16\nM=M-1\n", writef);
                }
            break;
        }
}
