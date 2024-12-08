#include "assembler.h"
#include "binary.h"
#include "hashmap.h"
#include "intpow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NA -1

int
main ()
{
  constexpr size_t MAX_LEN = 20;
  constexpr size_t SYMBOL_TABLE_SIZE = 4000; // 40; // symbols / 0.6
  char **const symbol_keys = malloc (SYMBOL_TABLE_SIZE * sizeof (char *));
  initHashTable (symbol_keys, SYMBOL_TABLE_SIZE, MAX_LEN);

  size_t symbol_data[SYMBOL_TABLE_SIZE];
  for (int i = 0; i < SYMBOL_TABLE_SIZE; i++)
    symbol_data[i] = 0;

#define HASH(key, data, HKEYS, HDATA, size, err_proc)                         \
  {                                                                           \
    size_t pos = 0;                                                           \
    if (allocHashMap (HKEYS, key, size, &pos) != 0)                           \
      {                                                                       \
        err_proc;                                                             \
      }                                                                       \
    HDATA[pos] = data;                                                        \
  }

#define SSYMBOL(key, data, err_proc)                                          \
  HASH (key, data, symbol_keys, symbol_data, SYMBOL_TABLE_SIZE, err_proc)
#define SYMBOL(key, data)                                                     \
  HASH (key, data, symbol_keys, symbol_data, SYMBOL_TABLE_SIZE,               \
        printf("ST missing space \n"); exit(1))

  // Wont check for missing space in hashtable for the default tags
  // Because of course there is
  SYMBOL ("R0", 0);
  SYMBOL ("R1", 1);
  SYMBOL ("R2", 2);
  SYMBOL ("R3", 3);
  SYMBOL ("R4", 4);
  SYMBOL ("R5", 5);
  SYMBOL ("R6", 6);
  SYMBOL ("R7", 7);
  SYMBOL ("R8", 8);
  SYMBOL ("R9", 9);
  SYMBOL ("R10", 10);
  SYMBOL ("R11", 11);
  SYMBOL ("R12", 12);
  SYMBOL ("R13", 13);
  SYMBOL ("R14", 14);
  SYMBOL ("R15", 15);
  SYMBOL ("SCREEN", 16384);
  SYMBOL ("KBD", 24576);
  SYMBOL ("SP", 0);
  SYMBOL ("LCL", 1);
  SYMBOL ("ARG", 2);
  SYMBOL ("THIS", 3);
  SYMBOL ("THAT", 4);

  // TODO: flag set
  char *const asmPath = "Rect.asm";
  char *const hackPath = "out.hack";
  const size_t MAX_READ = 80;

  // load tags
  {
    FILE *fasm = fopen (asmPath, "r");
    if (fasm == NULL)
      printf ("Error reading file %s", asmPath);
    char rline[MAX_READ]; // longer lines are useless for assembly
    size_t counter = 0;

    while (fgets (rline, MAX_READ, fasm))
      {
        switch (rline[0])
          {
          case '(':
            { // tag
              char key[MAX_READ];
              // copy to key
              register int i = 1;
              for (; rline[i] != ')'; i++)
                key[i - 1] = rline[i];
              key[i - 1] = '\0';

              SYMBOL (key, counter);
              break;
            }
          case '\r':
          case '\n':
          case '\0':
          case '/':   // comments
            continue; // while loop
          default:
            counter++;
            break;
          }
      }
    fclose (fasm);
  }

  // write and load vars
  {
    FILE *fasm = fopen (asmPath, "r");
    FILE *fhack = fopen (hackPath, "w");
    if (fasm == NULL || fhack == NULL)
      printf ("Error reading file %s", asmPath);
    char rline[MAX_READ]; // longer lines are useless for assembly
    char wline[17];       // 16bit + '\0'
    size_t counter = 16;

    // COMPARE HASH
    constexpr size_t COMP_SIZE = 47; // 47
    char **const comp_keys = malloc (sizeof (char *) * COMP_SIZE);
    initHashTable (comp_keys, COMP_SIZE, MAX_LEN);
    char *comp_data[COMP_SIZE];

#define COMP(key, val)                                                        \
  HASH (key, val, comp_keys, comp_data, COMP_SIZE, (*defhasherr) ())

    COMP ("0", "0101010");
    COMP ("1", "0111111");
    COMP ("-1", "0111010");
    COMP ("D", "0001100");
    COMP ("A", "0110000");
    COMP ("!D", "0001101");
    COMP ("!A", "0110001");
    COMP ("-D", "0001111");
    COMP ("-A", "1100110");
    COMP ("D+1", "0011111");
    COMP ("A+1", "0110111");
    COMP ("D-1", "0001110");
    COMP ("A-1", "0110010");
    COMP ("D+A", "0000010");
    COMP ("D-A", "0010011");
    COMP ("A-D", "0111000");
    COMP ("D&A", "0000000");
    COMP ("D|A", "0010101");
    COMP ("M", "1110000");
    COMP ("!M", "1110001");
    COMP ("-M", "1110011");
    COMP ("M+1", "1110111");
    COMP ("M-1", "1110010");
    COMP ("D+M", "1000010");
    COMP ("D-M", "1010011");
    COMP ("M-D", "1000111");
    COMP ("D&M", "1000000");
    COMP ("D|M", "1010101");

    // JMP HASH
    const size_t JMP_SIZE = 14;
    char **const jmp_keys = malloc (sizeof (char *) * JMP_SIZE);
    initHashTable (jmp_keys, JMP_SIZE, MAX_LEN);
    char *jmp_data[COMP_SIZE];
#define JMP(key, val)                                                         \
  HASH (key, val, jmp_keys, jmp_data, JMP_SIZE, (*defhasherr) ())

    JMP ("JGT", "001");
    JMP ("JEQ", "010");
    JMP ("JGE", "011");
    JMP ("JLT", "100");
    JMP ("JNE", "101");
    JMP ("JLE", "110");
    JMP ("JMP", "111");

    while (fgets (rline, MAX_READ, fasm))
      {
        switch (rline[0])
          {
          case '/':
          case '(':
          case '\r':
          case '\n':
            continue; // useless
          default: // not instr
              break;
          }

        // calc start point of instr (after tab)
        size_t startp = 0;
        while (rline[startp] == ' ' || rline[startp] == '\t')
          startp++;

        size_t end = startp + 1;
        while (
            !(rline[end] == '\r' || rline[end] == '\n' || rline[end] == ' '))
          end++;

        if (startp == end) // means a line like: `  \r\n` which is empty
          continue;        // skip

#define CPY(dest, str, start, offset)                                         \
  for (int i = 0; i < offset; i++)                                            \
    {                                                                         \
      dest[i] = str[start + i];                                               \
    }

        char bin[16] = {'0'};
        if (rline[startp] == '@') // a instr
          {
            int addr;
            if (rline[startp + 1] >= 'A')
              { // literal
                char key[MAX_LEN];
                CPY (key, rline, startp + 1, end - startp);
                key[end - startp - 1] = '\0';

                size_t pos = 0;
                if (!searchHashMap(symbol_keys, key, SYMBOL_TABLE_SIZE,
                                    &pos)) // trovato
                  addr = symbol_data[pos];
                else
                  { // var, return code of searchHashMap is non 0
                    SSYMBOL (key, counter, printf("failed %s w %lu\n", key, counter));
                    addr = counter;
                    counter++;
                  }
              }
            else
              { // numeral
                if (dec2int(rline + startp + 1, end - startp - 2, &addr))
                  printf ("ERRl\n");
                // TODO: err
              }
            if (int2bin16 (addr, bin))
              {
                  printf("Tried to convert %d", addr);
              }
            fputs (bin, fhack);
            fputc ('\n', fhack);
            continue; // while read
          }
        else
          {
            bin[0] = '1';
            bin[1] = '1';
            bin[2] = '1';
            int destp = 0; // position of '='; if = 0 means there's no =
                           // cuz = at the beg. it'd give invalid synthax err
            size_t compp = 0; // position of ';'; if = 0 means ...

            for (int i = startp; i < end; i++)
              {
                switch (rline[i])
                  {
                  case '=':
                    destp = i;
                    break;
                  case ';':
                  case ' ':
                    compp = i;
                    break;
                  }
                if (compp != 0)
                  break;
              }

            // COMPUTE COMP bin[3..9]
            if (!compp)
              compp = end;
            if (!destp)
              destp = startp - 1;
            char *comp = malloc (sizeof (char) * MAX_LEN);
            CPY (comp, rline, destp + 1, compp - destp - 1);
            comp[compp - destp - 1] = '\0';
            size_t pos = 0;
            if (searchHashMap (comp_keys, comp, COMP_SIZE, &pos))
              {
                printf ("Synthax error.\n");
                goto exit;
              }
            free (comp);
            comp = comp_data[pos];
            for (int i = 0; i < 7; i++)
              bin[i + 3] = comp[i];

            // COMPUTE DEST
            bin[10] = '0';
            bin[11] = '0';
            bin[12] = '0';
            if (destp != startp - 1)
              { // if there is destp  !!(destp)
                for (int i = startp; i < destp; i++)
                  {
                    switch (rline[i])
                      {
                      case 'A':
                        bin[10] = '1'; // d1
                        break;
                      case 'D':
                        bin[11] = '1'; // d2
                        break;
                      case 'M':
                        bin[12] = '1'; // d3
                        break;
                      }
                  }
              }
            // COMPUTE JMP
            char jmp[4] = "000";
            if (compp != end)
              { // there is jmp
                CPY (jmp, rline, compp + 1, 3);
                size_t pos = 0;
                if (searchHashMap (jmp_keys, jmp, JMP_SIZE, &pos))
                  {
                    printf ("Errore nel JMP\n");
                    goto exit;
                  }
                strcpy (jmp, jmp_data[pos]);
              }
            bin[13] = jmp[0];
            bin[14] = jmp[1];
            bin[15] = jmp[2];

            fputs (bin, fhack);
            fputc ('\n', fhack);
            continue;
          }
      } // while read
  exit:
    free (symbol_keys);
    free (comp_keys);
    free (jmp_keys);
    fclose (fhack);
    fclose (fasm);
  }

  return 0;
}

static inline void
defhasherr ()
{
  printf ("%s\n", "ERR");
  exit (1);
}
