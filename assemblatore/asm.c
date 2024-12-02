#include <stddef.h>
#include<stdio.h>
#include<malloc.h>
#define NIL -1
#define SYNTHAX_ERR -2
/* #define debug */

typedef struct {
    int* keys;
    size_t size;
}HashMap;
size_t search(char* key, HashMap* map);
size_t alloc_pos(char* key, HashMap* map);
int h1(char* key);
int h2(size_t key, size_t i);
void initHashMap(HashMap* map, size_t size);
int ascii2int(char* data, size_t start, size_t end);
int pow(size_t a, size_t b);
void int2bin16(int num, char* binary);
void loadTags(char* filepath , size_t* table_data, HashMap* table_keys);
void loadVariables(char* filepath , size_t* table_data, HashMap* table_keys);
int writeHack(char* hackpath, char* filepath, size_t* table_data, HashMap* table_keys);

int main(){
    // Usage: assembler file.hack -o a.out
    // Traduction table setup
    HashMap table_keys;
    initHashMap(&table_keys, 40);
    size_t  table_data[table_keys.size];
    for(int i = 0; i<table_keys.size; i++)
        table_data[i] = 0;

    table_data[alloc_pos("R0", &table_keys)] = 0;
    table_data[alloc_pos("R1", &table_keys)] = 1;
    table_data[alloc_pos("R2", &table_keys)] = 2;
    table_data[alloc_pos("R3", &table_keys)] = 3;
    table_data[alloc_pos("R4", &table_keys)] = 4;
    table_data[alloc_pos("R5", &table_keys)] = 5;
    table_data[alloc_pos("R6", &table_keys)] = 6;
    table_data[alloc_pos("R7", &table_keys)] = 7;
    table_data[alloc_pos("R8", &table_keys)] = 8;
    table_data[alloc_pos("R9", &table_keys)] = 9;
    table_data[alloc_pos("R10", &table_keys)] = 10;
    table_data[alloc_pos("R11", &table_keys)] = 11;
    table_data[alloc_pos("R12", &table_keys)] = 12;
    table_data[alloc_pos("R13", &table_keys)] = 13;
    table_data[alloc_pos("R14", &table_keys)] = 14;
    table_data[alloc_pos("R15", &table_keys)] = 15;
    table_data[alloc_pos("SCREEN", &table_keys)] = 16384;
    table_data[alloc_pos("KBD", &table_keys)] = 24576;

    //TODO: Flag get
    char* filepath = "Rect.asm";
    char* hackpath = "out.hack";

    loadTags(filepath, table_data, &table_keys);
    loadVariables(filepath, table_data, &table_keys);
    int err = writeHack(hackpath, filepath,table_data, &table_keys);
    if(err==SYNTHAX_ERR){
       return 1; 
    }

    

    #ifdef debug
    for(int i=0; i<table_keys.size; i++){
        printf("(%d:%lu)\t\t", i, table_data[i] );
        if(i%9==0&&i!=0)
            printf("\n");
    }
    printf("\n");
    #endif
   return 0;
}

int firstNonSpace(char* str){
    int startp=0;
    int run = 1;
    while(str[startp]!=' '){
        startp++;
    }
    return startp;
}


int writeHack(char* hackpath, char* filepath, size_t* table_data, HashMap* table_keys){
    HashMap allowedSynthax;
    initHashMap(&allowedSynthax, 12);
    alloc_pos("@", &allowedSynthax);
    alloc_pos("M", &allowedSynthax);
    alloc_pos("A", &allowedSynthax);
    alloc_pos("D", &allowedSynthax);
    alloc_pos("0", &allowedSynthax);
    alloc_pos("1", &allowedSynthax);
    alloc_pos("-", &allowedSynthax);

    FILE* read = fopen(filepath, "r");
    FILE* write = fopen(hackpath, "w");
    if(read==NULL || write==NULL)
        printf("Error reading file %s", filepath);

    char liner[80];
    size_t lenr;
    char linew[80];

    while(fgets(liner, lenr, read)!=NULL){
        int startp = firstNonSpace(liner);

        // not interesting
        if(liner[startp]=='('||liner[startp]=='/')
                continue;

        // is valid synthax
        char tmp [2];
        tmp[0] = liner[startp];
        tmp[1] = '\0';
        if(search(tmp, &allowedSynthax)==NIL)
            return SYNTHAX_ERR;

        // get end of line
        int end=startp+1;
        char value[80];
        for(;liner[end]!='\r' && liner[end]!='\n';end++)
            value[end-startp-1] = liner[end];
        value[end]='\0';

        char output[16];
        if(value[0]=='@'){ // a instr
            output[15] = '0';
            char* bin;
            int2bin16(ascii2int(liner, startp,end),  bin);
            for(int i = 1; i < 15; i++){
                output[i] = bin[i];
            }
        }else{ // c instr
            output[15] = '1';
            output[14] = '1';
            output[13] = '1';

            // possible format
            // 12345678; 8-4-1
            // ADM=X+Y;JMP
            // X+Y;JMP
            // 0;JMP
            int destp=0;
            int jmpp=-1;
            char comp = '\0';
            int compp = 0;
            for(int i = 0;value[i]!='\0'; i++){
                if(value[i]=='=')
                    destp = i;
                if(value[i]==';')
                    jmpp = i;
                /* if(value[i]=='+'||value[i]=='-'||value[i]=='&'){ */
                    /* compp = i; */
                /* } */
            }

            if(destp!=0){
                for(int i = 0; i<destp;i++){
                    switch(value[i]){
                        case 'A':
                            output[5] = '1'; // d1
                            break;
                        case 'D':
                            output[4] = '1'; // d2
                        case 'M':
                            output[3] = '1'; // d3
                    }
                }
            }else{
                output[5] = '0';
                output[4] = '0';
                output[3] = '0';
            }

            if(jmpp==-1){ // no semicolon
               return SYNTHAX_ERR;
            }else{
                // between = and ;
                for(int i=destp; i<jmpp; i++){ // compute
                    switch(jmpp-destp-1){
                       case 3: //arietà n=2 op
                           ...
                           break;
                        case 2:
                            break;

                    }
                }
            }
        }
    }

}

void loadVariables(char* filepath , size_t* table_data, HashMap* table_keys){
    FILE* fasm = fopen(filepath, "r");
    if(fasm==NULL)
        printf("Error reading file %s", filepath);
    char line[80];
    size_t len;
    size_t counter = 16;
    const size_t LEN = 80;

    while(fgets(line , LEN , fasm) != NULL){
        // search @
        size_t startp = 0;
        for(;; startp++)
            if(line[startp]=='@'||line[startp]=='\0')
                break;

        if(line[startp] == '@' && (line[startp+1]>=65)){ // @(etichetta | var | address) && letter (= !addr )
            char str[LEN];
            int i=startp+1;
            int c = 0;

            for(;line[i]!='\0'&&line[i]!='\r'&&line[i]!='\n'; i++) // copy line
                str[i-startp-1] = line[i];
            str[i-startp-1]='\0';

            if(search(str, table_keys)==NIL){ //not found -> it's a new var! Not a tag, or an existing var
                table_data[alloc_pos(str, table_keys)] = counter; // add to hash table
                counter++;
            }
        }else // not @
            continue;
    }
    fclose(fasm);
}


void loadTags(char* filepath , size_t* table_data, HashMap* table_keys){
    FILE* fasm = fopen(filepath, "r");

    if(fasm==(FILE* )-1)
        printf("Error reading file %s", filepath);

    char line[80];
    size_t len;

    size_t counter = 0;
    while(fgets(line , 80 , fasm) != NULL){
        if(line[0] == '('){ // etichetta
            char str[80];
            int i=1;
            for(;line[i]!=')'; i++)
                str[i-1] = line[i];
            str[i-1]='\0';
            table_data[alloc_pos(str, table_keys)] = counter;
        }else
            counter++;
    }
    fclose(fasm);
}

size_t search(char* key, HashMap* map){
    size_t hashedKey = h1(key);
    size_t pos = hashedKey % map->size;
    for (int i = 0; i<map->size; i++){
        if(map->keys[pos]==hashedKey){
            return pos;
        }else if (map->keys[pos] == NIL){
            return NIL;
        }else{ // occupied, rehash
            hashedKey=h2(hashedKey, i);
            pos = hashedKey%map->size;
        }
    }
    return NIL;
}

size_t alloc_pos(char* key, HashMap* map){
    size_t hashedKey = h1(key);
    size_t pos = hashedKey % map->size;
    for (int i = 0; i<map->size; i++){
       if (map->keys[pos] == NIL){ //avail
           #ifdef debug
           printf("Alloc data to %s in position %lu\n", key, (unsigned long) pos);
           #endif
           map->keys[pos] = hashedKey;
           return pos;
        }else{ // occupied, rehash
            hashedKey=h2(hashedKey, i);
            pos = hashedKey%map->size;
        }
    }
    return NIL; // should rebalance
}

void initHashMap(HashMap* map, size_t size){
    map->size = size;
    map->keys = (int*) malloc(sizeof(int)*(map->size));
    for (int i=0; i<map->size; i++){
        map->keys[i] = (int) NIL;
        printf("%d",map->keys[i]);
    }
}

int h1(char* key){
    size_t sum = 0;
    for(int i=0; key[i]!='\0'; i++)
        sum += key[i];
    return sum;
}

int h2(size_t key, size_t i){
    #define PRIME 43
    return (PRIME -i*(key%PRIME));
}

int ascii2int(char* data, size_t start, size_t end){
    size_t order = end-start;
    size_t sum = 0;
    for(int i=start; i<end; i++){
        sum += (data[i]-48)*pow(10, order);
        order--;
    }
    return sum;
}
void int2bin16(int num, char* binary){
    binary[15]=num>0?'0':'1';
    int complemento1 = num>0?0:1;

    for(int i=0; i<15; i++){
        if(num%2){ // disp
            binary[i] = !complemento1?'1':'0';
            num -=1;
        } else
            binary[i] = !complemento1?'0':'1';
        num /= 2;
    }
}

int pow(size_t a, size_t b){
    switch(b){
        case 0:
            return 1;
        case 1:
            return a;
        default:
            return pow(a*a, b-1); //there should be no tail rec
    }
}
