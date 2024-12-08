#include "assembler.h"
#include "binary.h"
#include "hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main (int argc, char *argv[])
{
  constexpr size_t KEY_MAX_LEN = 20;
  constexpr size_t SYMBOL_TABLE_SIZE = 4000; // 40; // symbols / 0.6
  char **const symbol_keys = initHashTable (SYMBOL_TABLE_SIZE, KEY_MAX_LEN);
  if (!symbol_keys)
    {
      // TODO: free mem handle
      printf ("Failed to allocate memory, do you have enough?\n");
      exit (1);
    }

  size_t symbol_data[SYMBOL_TABLE_SIZE];
  for (int i = 0; i < SYMBOL_TABLE_SIZE; i++)
    symbol_data[i] = 0;

  // Wont check for missing space in hashtable for the default tags
  // Because of course there is
  STORE_SYMBOL ("R0", 0);
  STORE_SYMBOL ("R1", 1);
  STORE_SYMBOL ("R2", 2);
  STORE_SYMBOL ("R3", 3);
  STORE_SYMBOL ("R4", 4);
  STORE_SYMBOL ("R5", 5);
  STORE_SYMBOL ("R6", 6);
  STORE_SYMBOL ("R7", 7);
  STORE_SYMBOL ("R8", 8);
  STORE_SYMBOL ("R9", 9);
  STORE_SYMBOL ("R10", 10);
  STORE_SYMBOL ("R11", 11);
  STORE_SYMBOL ("R12", 12);
  STORE_SYMBOL ("R13", 13);
  STORE_SYMBOL ("R14", 14);
  STORE_SYMBOL ("R15", 15);
  STORE_SYMBOL ("SCREEN", 16384);
  STORE_SYMBOL ("KBD", 24576);
  STORE_SYMBOL ("SP", 0);
  STORE_SYMBOL ("LCL", 1);
  STORE_SYMBOL ("ARG", 2);
  STORE_SYMBOL ("THIS", 3);
  STORE_SYMBOL ("THAT", 4);

  char *const asmPath = argc > 1 ? argv[1] : NULL;
  if (!asmPath)
    {
      printf ("Missing file\n");
      FREE_MAP (symbol_keys, SYMBOL_TABLE_SIZE);
      free (symbol_keys);
      // TODO: free mem handle
      exit (1);
    }

  char *const hackPath = malloc (sizeof (asmPath) + sizeof (".hack"));
  if (!hackPath)
    {
      FREE_MAP (symbol_keys, SYMBOL_TABLE_SIZE);
      free (symbol_keys);
      // TODO: free mem handle
      exit (1);
    }
  {
    register int i = 0;
    for (; asmPath[i] != '.' && asmPath[i] != '\0'; i++)
      {
        hackPath[i] = asmPath[i];
      }
    strcpy (hackPath + i, ".hack");
  }

  const size_t MAX_RLINE_LEN = 80;

  // load tags
  {
    FILE *fasm = fopen (asmPath, "r");
    if (!fasm)
      {
        printf ("Error reading file %s\n", asmPath);
        FREE_MAP (symbol_keys, SYMBOL_TABLE_SIZE);
        free (symbol_keys);
        // TODO: free mem handle
        exit (1);
      }

    char rline[MAX_RLINE_LEN]; // longer lines are useless for assembly
    size_t counter = 0;

    while (fgets (rline, MAX_RLINE_LEN, fasm))
      {
        switch (rline[0])
          {
          case '(':
            { // tag
              char key[KEY_MAX_LEN];
              // copy to key
              register int i = 1;
              for (; rline[i] != ')'; i++)
                key[i - 1] = rline[i];
              key[i - 1] = '\0';

              STORE_SYMBOL (key, counter);
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
      {
        printf ("Error reading file %s, and writing %s\n", asmPath, hackPath);
        // TODO: free mem handle
        exit (1);
      }
    char rline[MAX_RLINE_LEN]; // longer lines are useless for assembly
    size_t counter = 16;

    // COMPARE STORE_IN_HASH
    constexpr size_t COMP_SIZE = 47; // 47
    char **const comp_keys = initHashTable (COMP_SIZE, KEY_MAX_LEN);
    if (!comp_keys)
      goto exit;
    char *comp_data[COMP_SIZE];

    STORE_COMP ("0", "0101010");
    STORE_COMP ("1", "0111111");
    STORE_COMP ("-1", "0111010");
    STORE_COMP ("D", "0001100");
    STORE_COMP ("A", "0110000");
    STORE_COMP ("!D", "0001101");
    STORE_COMP ("!A", "0110001");
    STORE_COMP ("-D", "0001111");
    STORE_COMP ("-A", "1100110");
    STORE_COMP ("D+1", "0011111");
    STORE_COMP ("A+1", "0110111");
    STORE_COMP ("D-1", "0001110");
    STORE_COMP ("A-1", "0110010");
    STORE_COMP ("D+A", "0000010");
    STORE_COMP ("D-A", "0010011");
    STORE_COMP ("A-D", "0111000");
    STORE_COMP ("D&A", "0000000");
    STORE_COMP ("D|A", "0010101");
    STORE_COMP ("M", "1110000");
    STORE_COMP ("!M", "1110001");
    STORE_COMP ("-M", "1110011");
    STORE_COMP ("M+1", "1110111");
    STORE_COMP ("M-1", "1110010");
    STORE_COMP ("D+M", "1000010");
    STORE_COMP ("D-M", "1010011");
    STORE_COMP ("M-D", "1000111");
    STORE_COMP ("D&M", "1000000");
    STORE_COMP ("D|M", "1010101");

    // JMP STORE_IN_HASH
    const size_t JMP_SIZE = 14;
    char **const jmp_keys = initHashTable (JMP_SIZE, KEY_MAX_LEN);
    if (!jmp_keys)
      goto exit;
    char *jmp_data[COMP_SIZE];

    STORE_JMP ("JGT", "001");
    STORE_JMP ("JEQ", "010");
    STORE_JMP ("JGE", "011");
    STORE_JMP ("JLT", "100");
    STORE_JMP ("JNE", "101");
    STORE_JMP ("JLE", "110");
    STORE_JMP ("JMP", "111");

    while (fgets (rline, MAX_RLINE_LEN, fasm))
      {
        switch (rline[0])
          {
          case '/':
          case '(':
          case '\r':
          case '\n':
            continue; // useless
          default:    // not instr
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

        char bin[16] = { '0' };
        if (rline[startp] == '@') // a instr
          {
            int addr;
            if (rline[startp + 1] >= 'A')
              { // literal
                char key[KEY_MAX_LEN];
                CPY (key, rline, startp + 1, end - startp);
                key[end - startp - 1] = '\0';

                size_t pos = 0;
                if (!searchHashMap (symbol_keys, key, SYMBOL_TABLE_SIZE,
                                    &pos)) // trovato
                  addr = symbol_data[pos];
                else
                  { // var, return code of searchHashMap is non 0
                    STORE_SYMBOL_WERR (
                        key, counter,
                        printf ("failed %s w %lu\n", key, counter));
                    addr = counter;
                    counter++;
                  }
              }
            else
              { // numeral
                if (dec2int (rline + startp + 1, end - startp - 2, &addr))
                  printf ("ERRl\n");
                // TODO: err
              }
            if (int2bin16 (addr, bin))
              {
                printf ("Tried to convert %d", addr);
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
            char *comp = malloc (sizeof (char) * KEY_MAX_LEN);
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
    FREE_MAP (symbol_keys, SYMBOL_TABLE_SIZE);
    FREE_MAP (comp_keys, COMP_SIZE);
    FREE_MAP (jmp_keys, JMP_SIZE);

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

[[nodiscard]] char **const
initHashTable (const size_t size, const size_t strLen)
{
  _Bool _free = 0;
  char **const hashTable = malloc (sizeof (char *) * size);
  for (register int i = 0; i < size; i++)
    {
      if (!_free) // normal behav
        {
          hashTable[i] = malloc (sizeof (char) * strLen);
          if (!hashTable[i]) // if failed to malloc mem
            {
              i = 0; // error start to free ALL MEM from i=0
              _free = 1;
              continue;
            }
          else // no err
            hashTable[i][0] = '\0';
        }
      else if (_free && hashTable[i]) // if free and hashTable is non NULL; if
                                      // NULL no need to free
        free (hashTable[i]);
      else if (_free && !hashTable[i]) // reached unalloc area
        break;
    }

  if (_free)
    {
      free (hashTable);
      return NULL;
    }
  return hashTable;
}
