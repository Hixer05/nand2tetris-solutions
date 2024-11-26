#include <stddef.h>
#include<stdio.h>
#include<malloc.h>
#define NIL -1

#define debug

typedef struct {
    size_t* keys;
    size_t size;
}HashMap;
size_t search(char* key, HashMap map);
size_t alloc_pos(char* key, HashMap* map);
size_t h1(char* key);
size_t h2(size_t key, size_t i);
void initHashMap(HashMap* map, size_t size);
size_t ascii2int(char* data, size_t start, size_t end);
size_t pow(size_t a, size_t b);
void int2bin16(int num, char* binary);

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
    char* filepath = "test.asm";
    char* output_path = "a.out";
#ifdef debug
    for(int i = 0; i < 40; i++ ){
        printf("%u\n", (unsigned int) table_data[i]);
    }
#endif

    // file open and parse
    FILE* fasm = fopen(filepath, "r");
    char line[80];
    size_t len;

    FILE* fhack = fopen(output_path, "w");
    char* linew = NULL;
    size_t lenw;

    // prima passata
    size_t counter = 0;
    while(fgets(line , 80 , fasm) != NULL){
        if(line[0] == '('){ // etichetta
            char* str;
            int i=1;
            for(;; i++){
                str[i] = line[i];
            }
            str[i]='\0';
        }
    }

    fclose(fasm);
    free(line);
    fasm = fopen(filepath, "r");


    // traduzione
    while((getline(&line, &len, fasm))!= -1){
        if(line[0]=='@'){
            // TODO: non ho implementato le etichette! Manca la prima passata!!!:w
            //
            char buf[16];
            /* buf[0] = 0; non-necessary by A-instr, int2bin16*/
            int dec = ascii2int(line, 1, 15);
            int2bin16(dec, buf);
            fputs(buf, fhack);
        }else if(line[0]==';')
    }

    // Close
    fclose(fasm);
    free(table_keys.keys);
    free(line);
    return 0;
}

size_t search(char* key, HashMap map){
    size_t hashedKey = h1(key);
    size_t pos = hashedKey % map.size;
    for (int i = 0; i<map.size; i++){
        if(map.keys[pos]==hashedKey){
            return pos;
        }else if (map.keys[pos] == NIL){
            return NIL;
        }else{ // occupied, rehash
            hashedKey=h2(hashedKey, i);
            pos = hashedKey%map.size;
        }
    }
    return NIL;
}

size_t alloc_pos(char* key, HashMap* map){
    size_t hashedKey = h1(key);
    size_t pos = hashedKey % map->size;
    for (int i = 0; i<map->size; i++){
       if (map->keys[pos] == NIL){ //avail
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
    map->keys = (size_t*) malloc(sizeof(size_t)*(map->size));
    for (int i=0; i<map->size; i++){
        map->keys[i] = NIL;
    }
}

size_t h1(char* key){
    size_t sum = 0;
    for(int i=0; key[i]!='\0'; i++)
        sum += key[i];
    return sum;
}

size_t h2(size_t key, size_t i){
    #define PRIME 43
    return (PRIME -i*(key%PRIME));
}

size_t ascii2int(char* data, size_t start, size_t end){
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

size_t pow(size_t a, size_t b){
    switch(b){
        case 0:
            return 1;
        case 1:
            return a;
        default:
            return pow(a*a, b-1); //there should be no tail rec
    }
}
