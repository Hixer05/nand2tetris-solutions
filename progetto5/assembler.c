#include "assembler.h"
#include "binary.h"
#include "hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: design free and exit system for errs
// TODO: 14 bytes at use in exit, check allo malloc

int
main (int argc, char *argv[])
{
  // __ MAIN CONST DEF __
  constexpr size_t KEY_MAX_LEN = 20;
  constexpr size_t SYMBOL_TABLE_SIZE = 4000; // 40; // symbols / 0.6
  constexpr size_t MAX_RLINE_LEN = 80;

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

  INIT_ST();

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

  // compute hackPath
  {
    register int i = 0;
    for (; asmPath[i] != '.' && asmPath[i] != '\0'; i++)
      {
        hackPath[i] = asmPath[i];
      }
    strcpy (hackPath + i, ".hack");
  }


  // __ LOAD TAGS __
  {
    // ++ LOCAL DEF ++
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
            { // == TAG FOUND ==
              char key[KEY_MAX_LEN];
              // Copy to key
              register int i = 1;
              for (; rline[i] != ')'; i++) // whilst it isn't tag end
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
          default: // lines which must be counted
            counter++;
            break;
          }
      }

    // ++ LOCAL EXIT OP ++
    fclose (fasm);
  }

  // __ WRITE & LOAD __
  {
    // ++ LOCAL DEF ++
    FILE *fasm = fopen (asmPath, "r");
    FILE *fhack = fopen (hackPath, "w");
    if (fasm == NULL || fhack == NULL)
      {
        printf ("Error reading file %s, and writing %s\n", asmPath, hackPath);
        // TODO: free mem handle
        exit (1);
      }

    char rline[MAX_RLINE_LEN]; // longer lines are useless for assembly
    size_t variableCounter = 16;

    // COMP HASH
    constexpr size_t COMP_SIZE = 47; // 47
    char **const comp_keys = initHashTable (COMP_SIZE, KEY_MAX_LEN);
    if (!comp_keys)
      goto exit;
    char *comp_data[COMP_SIZE];
    INIT_COMP();

    // JMP HASH
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

    // ++ COMPUTE ++
    while (fgets (rline, MAX_RLINE_LEN, fasm))
      {
        switch (rline[0]) // if is_instruction else next line
          {
          case '/':
          case '(':
          case '\r':
          case '\n':
            continue; // useless
          default:    // An instruction
            break;
          }

        // compute start point of instr)
        size_t startp = 0;
        while (rline[startp] == ' ' || rline[startp] == '\t')
          startp++;

        // compute end point
        size_t end = startp + 1;
        while (!(rline[end] == '\r' ||
                 rline[end] == '\n' ||
                 rline[end] == ' '))
          end++;

        if (startp == end) // means a line like: `  \r\n` which is empty
          continue;

        char bin[16] = { '0' }; // init binary output

        // NOTE: WRITE BEGINS
        if (rline[startp] == '@')
          {
            // === NOTE: A INSTRUCTION ===
            int addr = 0; // A address
            // NOTE: == GET ADDRESS AS INT ==
            if (rline[startp + 1] >= 'A') // LITERAL AKA TAGS/VARS
              {
                char key[KEY_MAX_LEN];
                CPY (key, rline, startp + 1, end - startp);
                key[end - startp - 1] = '\0';

                size_t pos = 0;
                // ALREADY DEFINED?
                if (!searchHashMap (symbol_keys, key, SYMBOL_TABLE_SIZE,
                                    &pos))
                  addr = symbol_data[pos];
                else
                  { // NOT DEFINED, return code of searchHashMap is non 0
                    STORE_SYMBOL_WERR (
                        key, variableCounter,
                        printf ("failed %s w %lu\n", key, variableCounter));
                    addr = variableCounter;
                    variableCounter++;
                  }
              }
            else // NUMERAL (FIXED ADDR)
              {
                // Parse decimal number
                if (dec2int (rline + startp + 1, end - startp - 2, &addr))
                {
                  // TODO: error handle
                  printf ("ERRl\n");
                }
              }
            // === NOTE: WRITE ADDRESS ==
            if (int2bin16 (addr, bin))
              {
                printf ("Tried to convert %d", addr);
              }
            // GOTO WRITE OP (after else)
          }
        else
          {
            // == NOTE: C INSTRUCTION ==
            bin[0] = '1';
            bin[1] = '1';
            bin[2] = '1';

            int destp = 0; // position of '='; if = 0 means there's no =
                           // cuz = at the beg. it'd give invalid synthax err
            size_t compp = 0; // position of ';'; if = 0 means ...
            // get destp and compp
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
            // copy resulting comp to bin
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
            // GOTO WRITE OP
         }
        // WRITE OP
        fputs (bin, fhack);
        fputc ('\n', fhack);
      } // while read

    // ++ LOCAL EXIT ++
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
