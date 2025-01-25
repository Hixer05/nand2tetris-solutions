#include "pretranslate.h"
#include <string.h>
#include <stdio.h>

// For every file:
// if this sequence
// push seg1 k
// pop seg2 x
// translate to `move seg1 k seg2 x`
// output to file.pp.vm
// NOTE: list is valid
#define MAX_RLINE 80
int pushpop(char list[256][256*2], int size){
    for(int j = 0; j<size; j++){
        // output name
        char outputName[256*2+3];
        strcpy(outputName, list[j]);
        for(int i=strlen(outputName); i>0; i--){
            if(outputName[i]=='.'){
                strcpy(outputName+i, ".pp.vm");
            }
        }

        // open files
        FILE* readf = fopen(list[j], "r");
        FILE* writef = fopen(outputName, "w");
        if(readf==NULL||writef==NULL){
            printf("Pretranslate error: %s %s\n",list[j], outputName);
            return -1;
        }
        char prev[MAX_RLINE]="", line[MAX_RLINE]="";
        while(fgets(line, MAX_RLINE, readf)){
            if (strstr(line, "push")) {
                char next[MAX_RLINE]="";
                if (fgets(next, MAX_RLINE, readf)) {
                    if (strstr(next, "pop")) {
                        char pseg[20], ploc[10], cseg[20], cloc[10], wline[MAX_RLINE]="";
                        sscanf(line, " push %s %s ", pseg, ploc);
                        sscanf(next, " pop %s %s ", cseg, cloc);
                        sprintf(wline, "move %s %s %s %s\n", pseg, ploc, cseg, cloc);
                        fputs(wline, writef);
                    } else {
                        fputs(line, writef);
                        fputs(next, writef);
                    }
                    strcpy(prev, next);
                } else {
                    fputs(line, writef);
                    break;
                }
            } else {
                fputs(line, writef);
                strcpy(prev, line);
            }
        }
        fclose(readf);
        fclose(writef);
    }
    return 0;
}
