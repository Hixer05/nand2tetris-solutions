#include "assembler.h"
#include "hashmap.h"
#include "intpow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NA -1



int main() {
    constexpr size_t MAX_LEN = 20;
    constexpr size_t SYMBOL_TABLE_SIZE = 40; // symbols / 0.6
    char ** const symbol_keys = malloc(SYMBOL_TABLE_SIZE*sizeof(char*));
    initHashTable(symbol_keys, SYMBOL_TABLE_SIZE, MAX_LEN);

    size_t  symbol_data[SYMBOL_TABLE_SIZE];
    for(int i = 0; i<SYMBOL_TABLE_SIZE; i++)
        symbol_data[i] = 0;

#define HASH(key, data, HKEYS, HDATA, size, err_proc) {\
        size_t pos = 0; \
        if(allocHashMap( HKEYS, key, size, &pos)!=0){err_proc;} \
        HDATA[pos]=data;\
}\

#define SSYMBOL(key, data, err_proc) HASH(key, data, symbol_keys, symbol_data, SYMBOL_TABLE_SIZE, err_proc)
#define SYMBOL(key, data) HASH(key, data, symbol_keys, symbol_data, SYMBOL_TABLE_SIZE, (*defhasherr)())

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
    char * const asmPath = "Rect.asm";
    char * const hackPath = "out.hack";
    const size_t MAX_READ = 80;

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
                    char key[MAX_READ];
                    // copy to key
                    register int i = 1;
                    for(; rline[i]!=')'; i++)
                        key[i-1] = rline[i];
                    key[i-1]='\0';

                    SYMBOL(key, counter);
                    break;
                }
                case '/': // comments
                    continue; // while loop
                    break; //useless?
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
        FILE* fhack = fopen(hackPath, "w");
        if(fasm==NULL || fhack==NULL)
            printf("Error reading file %s", asmPath);
        char rline[MAX_READ]; // longer lines are useless for assembly
        char wline[17]; // 16bit + '\0'
        size_t counter = 16;

        // COMPARE HASH
        constexpr size_t COMP_SIZE = 47; // 47
        char ** const comp_keys = malloc(sizeof(char*)*COMP_SIZE);
        initHashTable(comp_keys, COMP_SIZE, MAX_LEN);
        char * comp_data[COMP_SIZE];

#define COMP(key, val) HASH(key, val, comp_keys, comp_data, COMP_SIZE, (*defhasherr)())

        COMP("0",  "0101010");
        COMP("1", "1111110");
        COMP("-1", "0101110");
        COMP("D", "0011000");
        COMP("A", "0000110");
        COMP("!D", "1011000");
        COMP("!A", "1000110");
        COMP("-D", "1111000");
        COMP("-A", "1100110");
        COMP("D+1", "1111100");
        COMP("A+1", "1110110");
        COMP("D-1", "0111000");
        COMP("A-1", "0100110");
        COMP("D+A", "0100000");
        COMP("D-A", "1100100");
        COMP("A-D", "0001110");
        COMP("D&A", "0000000");
        COMP("D|A", "1010100");
        COMP("M", "0000111");
        COMP("!M", "100011");
        COMP("-M", "1100111");
        COMP("M+1", "1110111");
        COMP("M-1", "0100111");
        COMP("D+M", "0100001");
        COMP("D-M", "1100101");
        COMP("D&M", "0000001");
        COMP("D|M", "1010101");

        // JMP HASH
        const size_t JMP_SIZE = 14;
        char** const jmp_keys = malloc(sizeof(char*)*JMP_SIZE);
        initHashTable(jmp_keys, JMP_SIZE, MAX_LEN);
        char * jmp_data[COMP_SIZE];
#define JMP(key, val) HASH(key, val, jmp_keys, jmp_data, JMP_SIZE, (*defhasherr)())

        JMP("JGT", "100");
        JMP("JEQ", "010");
        JMP("JGE", "110");
        JMP("JLT", "001");
        JMP("JNE", "101");
        JMP("JLE", "011");
        JMP("JMP", "111");

      // first non space
        while(fgets(rline, MAX_READ, fasm)){
            const int startp = firstNonSpace(rline);
            size_t end = startp+1;
            while(!(rline[end]=='\r'||rline[end]=='\n'||rline[end]==' ')&&end<MAX_READ){end++;}

            if(startp==NA) // empty line
                continue;
            printf("%s", rline);
            switch(rline[startp]){
                case '(':
                case '/':
                    continue; // next line
                    break; // useless?
                case 'A':
                case 'D':
                case 'M':
                case '-':
                case '1':
                case '0':
                    goto CINSTR;
                    break;
                case '@':
                    goto AINSTR;
                    break;
                default:
                    printf("Invalid synthax\n");
                    goto exit;
             }
#define CPY(dest, str, start, offset) for(int i = 0;i<offset; i++){dest[i]=str[start+i];}dest[offset]='\0';

             char bin[16];
        AINSTR:
             size_t addr;
             if(rline[startp+1]>='A'){ //literal
                 char * const key;
                 CPY(key, rline, startp, end-startp);
                 size_t pos = 0;
                 if(!searchHashMap(symbol_keys, key, SYMBOL_TABLE_SIZE, &pos)) // trovato
                     addr = symbol_data[pos];
                 else{ // var, return code of searchHashMap is 0
                     SYMBOL(key, counter);
                     addr = counter;
                     counter++;
                 }
             }else{ //numeral
                 addr = ascii_dec2int(rline, rline+end);
             }
             int2ascii_bin16(addr, bin);
             fputs(bin, fhack);
             continue; // while read
        CINSTR:
             bin[13] = '1';
             bin[14] = '1';
             bin[15] = '1';
             int destp = -1; // position of '='; if = -1 means there's no =
                               // cuz = at the beg. it'd give invalid synthax err
             size_t compp = 0; // position of ';'; if = 0 means ...

             for(int i = startp; i<end; i++){
                 switch(rline[i]){
                     case '=':
                         destp = i;
                         break;
                     case ';':
                         compp=i;
                         break;
                 }
                 if(compp!=0)
                     break;
             }

             // COMPUTE COMP bin[6..12]
             if(!compp){
                 printf("Missing ;\n");
                  goto exit;
             }

             char * comp;
             // destp+1 = 0; if missing
             CPY(comp, rline, destp+1, compp-destp-1);
             size_t pos = 0;
             if(searchHashMap(comp_keys, comp, COMP_SIZE, &pos)){
                  printf("Synthax error.\n");
                  goto exit;
             }
             comp = comp_data[pos];
             for(int i = 0; i<7; i++)
                 bin[i+6] = comp[i];

             // COMPUTE DEST
             if(!(destp+1)){ // if there is destp  !!(destp)
                 for(int i = startp; i<destp;i++){
                     switch(rline[i]){
                        case 'A':
                            bin[5] = '1'; // d1
                            break;
                        case 'D':
                            bin[4] = '1'; // d2
                        case 'M':
                            bin[3] = '1'; // d3
                    }
                }
             }else{
                 bin[3]=0;
                 bin[4]=0;
                 bin[5]=0;
             }

             // COMPUTE JMP
             char * jmp="000";
             if(compp!=end-1) {// there is jmp
                 CPY(jmp, rline, compp+1, end-compp-1);
                 size_t pos = 0;
                 if(searchHashMap(jmp_keys, jmp, JMP_SIZE, &pos)){
                     printf("Errore nel JMP\n");
                      goto exit;
                 }
                 jmp = jmp_data[pos];
             }
             CPY(bin, jmp, 0, 3); // we can do this

             // TODO: FIX SHIT
             char laden[16];
             for(int i = 0; i < 16; i++){
                 laden[16-i] = bin [i];
             }
             for(int i = 0; i < 16; i++){
                 bin [i] = laden [i];
             }
             fputs(bin, fhack);
             continue; // while read
        }
        exit:
        free(symbol_keys);
        free(comp_keys);
        free(jmp_keys);
        fclose(fhack);
        fclose(fasm);
    }

    return 0;
}

static inline void defhasherr()
{
    printf("%s\n","ERR");
    exit(1);
}

void  int2ascii_bin16( int addr, char * const binary){
    binary[15]=addr>0?'0':'1';
    _Bool complemento1 = addr>0?0:1;
    addr = addr>0?addr:-addr;
    // most signf. bit to the left
    for(int i=0; i<16; i++){
        if(addr%2){ // disp
            binary[i] = !complemento1?'1':'0';
            addr -=1;
        } else // pari
            binary[i] = !complemento1?'0':'1';
        addr /= 2;
    }
}

const int ascii_dec2int(char * const str, char * const end){
    size_t order = str-end;
    size_t sum=0;
    for(int i=0; i<order; i++){
        sum += (str[i]-48)*ipow(10, order);
        order--;
    }
    return sum;
}

const int firstNonSpace(char * const str){
    size_t startp = 0;
    for(register int i = 0; str[i])
}
