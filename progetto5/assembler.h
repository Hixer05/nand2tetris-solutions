#ifndef ASSEMBLER_H
#define ASSEMBLER_H

const int firstNonSpace(char * const str);
const int ascii_dec2int(char * const str, char * const end);
void  int2ascii_bin16(int addr, char * const binary);
static inline void defhasherr();

#endif
