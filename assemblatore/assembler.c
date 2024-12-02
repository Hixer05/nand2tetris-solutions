#include "assembler.h"
#include <stdio.h>
#include<malloc.h>
#include<stdio.h>

int main() {
    const size_t SYMBOL_TABLE_SIZE = 40; // symbols / 0.6
    HashMap symbol_keys;
    initHashMap(&symbol_keys, SYMBOL_TABLE_SIZE);
    size_t  symbol_data[SYMBOL_TABLE_SIZE];
    for(int i = 0; i<SYMBOL_TABLE_SIZE; i++)
        symbol_data[i] = 0;

    #define HASH(key, data, HDATA, HKEYS) (HDATA[alloc_pos(key,&HKEYS)]=data)
    #define SHASH(key, data, HDATA, HKEYS, MSG) { const int pos = alloc_pos(key, &HKEYS);\
    if(pos!=NA){HDATA[pos]=data;}else{printf(MSG);return 1;}}
    #define SYMBOL(key, data) (HASH(key,data, symbol_data, symbol_keys))
    #define SSYMBOL(key, data) (SHASH(key, data, symbol_data, symbol_keys, "Increase symbol table size\n"))

    // Wont check for missing space in hashtable for the default tags
    // Because of course there is
    SYMBOL("R0", 0);
    SYMBOL("R1", 1);
    SYMBOL("R2", 2);
    SYMBOL("R3", 3);
    SYMBOL("R4", 4);
    SYMBOL("R5", 5);
    SYMBOL("R6", 6);
    SYMBOL("R7", 7);
    SYMBOL("R8", 8);
    SYMBOL("R9", 9);
    SYMBOL("R10", 10);
    SYMBOL("R11", 11);
    SYMBOL("R12", 12);
    SYMBOL("R13", 13);
    SYMBOL("R14", 14);
    SYMBOL("R15", 15);
    SYMBOL("SCREEN", 16384);
    SYMBOL("KBD", 24576);

    //TODO: flag set
    const char* asmPath = "Rect.asm";
    const char* hackPath = "out.hack";
    const size_t MAX_READ = 32;

    // load tags
    {
        FILE* fasm = fopen(asmPath, "r");
        if(fasm==NULL)
            printf("Error reading file %s", asmPath);
        char rline[MAX_READ]; // longer lines are useless for assembly
        size_t counter = 0;
        while(fgets(rline, MAX_READ, fasm)){
            switch(rline[0]){
                case '(':{ // tags
                    char* str;
                    // copy to str
                    register int i = 1;
                    for(; rline[i]!=')'; i++)
                        str[i-1] = rline[i];
                    str[i-1]='\0';

                    SSYMBOL(str, counter);
                    break;
                }
                case '/': // comments
                    continue; // while loop
                default:
                    counter++;
                    break;
            }
        }
        fclose(fasm);
    }

    // write and load vars
    {

        FILE* fasm = fopen(asmPath, "r");
        if(fasm==NULL)
            printf("Error reading file %s", asmPath);
        char rline[MAX_READ]; // longer lines are useless for assembly
        size_t counter = 0;

    }

    return 0;
}

const int alloc_pos(const char* key, const HashMap* map){
    size_t hashedKey = hash1(key);
    const size_t size = map->size;
    size_t pos = hashedKey % size;

    for (register int i = 0; i<size; i++){
        if(map->keys[pos] == NA){
            map->keys[pos] = hashedKey;
            return pos;
        }else{
            hashedKey = hash2(hashedKey, i);
            pos = hashedKey % size;
        }
    }

    return NA;
}

const int search(const char* key, const HashMap* map){
    const size_t size = map->size;
    size_t hashedKey = hash1(key);
    size_t pos = hashedKey%size;

    for(register int i=0; i<size; i++){
        if(map->keys[pos]==hashedKey){
            return pos;
        }else if(map->keys[pos]==NA){
            return NA;
        }else{ // occupied
           hashedKey=hash2(hashedKey, i);
           pos = hashedKey%size;
        }
    }
    return NA;
}

void initHashMap(HashMap* map, const size_t size){
    map->size = size;
    map->keys = malloc(sizeof(int)*(size));
    for (register int i = 0; i<size; i++)
        map->keys[i] = NA;
}

const size_t hash1(const char* key){
    size_t sum = 0;
    for(register int i=0; key[i]!='\0'; i++)
        sum += key[i];
    return sum;
}
