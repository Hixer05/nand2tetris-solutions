#include "metavm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// open `file.vm` and applies metavm optimizations to `file.meta.vm`
// file.vm must be valid
int
metaparse (const char *const vmPath, const char *const extension)
{
    char metaPath[256 * 2];
    strcpy (metaPath, vmPath);
    for (int i = strlen (metaPath); i > 0; i--)
        {
            if (metaPath[i] == '.')
                {
                    strcpy (metaPath + i, extension);
                    break;
                }
        }

    // copy
    FILE *const vmf = fopen (vmPath, "r");
    FILE *const mef = fopen (metaPath, "w");
    if (!vmf)
        {
            printf ("Error reading %s\n", vmPath);
            return -1;
        }
    if (!mef)
        {
            printf ("Error reading %s\n", metaPath);
            fclose (vmf);
            return -1;
        }
    /* printf ("Copying %s to %s\n", vmPath, metaPath); */
    /* char line[80]; */
    /* while (fgets (line, 80, vmf)) */
    /*     { */
    /*         fputs (line, mef); */
    /*     } */

    /* printf ("Pushpop on %s -> %s\n",vmPath, metaPath); */
    int exit_code = 0;
    exit_code += pushpop (vmPath, metaPath);

    return exit_code;
}

// if this sequence
// push seg1 k
// pop seg2 x
// translate to `move seg1 k seg2 x`
#define MAX_RLINE 80
int
pushpop (const char *const vmPath, const char *const metaPath)
{
    // open files
    FILE *readf = fopen (vmPath, "r");
    if (readf == NULL)
        {
            printf ("Metavm error can't open: %s\n", vmPath);
            return -1;
        }
    FILE *writef = fopen (metaPath, "w");
    if (writef == NULL)
        {
            printf ("Metavm error can't open: %s\n", metaPath);
            fclose (readf);
            return -1;
        }

    // READ FILE INTO MEMORY
    fseek (readf, 0, SEEK_END);
    long fsize = ftell (readf);
    fseek (readf, 0, SEEK_SET); /* same as rewind(readf); */

    char *string = malloc (fsize + 1);
    fread (string, fsize, 1, readf);
    fclose (readf);

    long lines = 0;
    for (int i = 0; i < fsize; i++){
       if(string[i]=='\n')
           lines++;
    }

    char output[lines][80];
    int line = 0;
    int check_line = 0;
    char* cur = strtok(string, "\n");
    char prev[80] = "";
    bool flag = false;
    while(cur!=NULL){
        if(check_line>=lines)
            break;
        char tmp[100];
        if(sscanf(cur, " //%s ", tmp)==1)
        {
            cur = strtok(NULL, "\n");
            continue;
        }

        if(strstr(cur, "push")){
            flag = true;
            strcpy(prev, cur);
        }
        if(strstr(cur, "pop")&&flag){
            // write to output-1
            char wline[80], s1[20], s2[20], l1[10], l2[10];
            sscanf(prev, " push %s %s ", s1, l1);
            sscanf(cur, " pop %s %s ", s2, l2);
            sprintf(wline, "move %s %s %s %s \n",s1,l1,s2,l2);
            strcpy(output[line-1], wline);
            /* printf("Wrote: %s", wline); */
            flag = false;
        }else{
            strcpy(output[line], cur);
            strcat(output[line], "\n");
            line++;
            if(!strstr(cur, "push")){ // should be push
                flag = false;
            }
        }
        check_line++;
        cur = strtok(NULL, "\n");
    }

    for(int i = 0; i<line; i++){
        fputs(output[i], writef);
    }

    fclose(writef);
    free (string);
    return 0;
}
