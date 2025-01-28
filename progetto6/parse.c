#include "shared.h"
#include "wf.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

int
parse (const char *const vmPath, const char *const asmPath)
{
  char rline[MAX_RLINE_LEN];
  FILE *const readf = fopen (vmPath, "r");
  int exit_code = 0;
  if (!readf)
    {
      printf ("Error reading file %s\n", vmPath);
      return 1;
    }

  FILE *const writef = fopen (asmPath, "a");
  if (!writef)
    {
      printf ("Error writing file %s", asmPath);
      return 1;
    }

  while (fgets (rline, MAX_RLINE_LEN, readf) != NULL)
    {
      char instr[20];
      if (sscanf (rline, " %s ", instr) != 1)
        {
          // emptyline
          continue;
        }

      switch (instr[0])
        {
        case '\0':
        case '\r':
        case '\n':
        case '/':
          break;
        case 'a':
          if (instr[1] == 'n')
            wand (writef);
          else
            wadd (writef);
          break;
        case 'f': // function decl
          if (wfunctiondecl (rline, writef))
            {
              printf ("Function declaration error:%s\n", rline);
              exit_code += 1;
              goto exit;
            }
          break;
        case 'e':
          weq (writef);
          break;
        case 'o':
          wor (writef);
          break;
        case 'r':
          wfunctionreturn (writef);
          break;
        case 's':
          wsub (writef);
          break;
        case 'n':
          if (instr[1] == 'e')
            wneg (writef);
          else
            wnot (writef);
          break;
        case 'c': // function call
          wfunctioncall (rline, writef);
          break;
        case 'i': // if-goto
          wifgoto (rline, writef);
          break;
        case 'p':
          if (instr[1] == 'o')
            {
              if (wpop (rline, writef))
                {
                  printf ("Error pop:%s\n", rline);
                  exit_code += 1;
                  goto exit;
                }
            }
          else
            {
              if (wpush (rline, writef))
                {
                  printf ("Error push:%s\n", rline);
                  exit_code += 1;
                  goto exit;
                }
            }
          break;
        case 'l':
          if (instr[1] == 'a')
            { // lable
              wlabel (rline, writef);
            }
          else
            {
              wlt (writef);
            }
          break;
        case 'g':
          if (instr[1] == 't')
            wgt (writef);
          else
            wgoto (rline, writef);
          break;
        case 'm':
          if (wmove (rline, writef))
            {
              exit_code += 1;
              goto exit;
            }
          break;
        default: // unrecog
#ifndef DEBUG
          printf ("Unrecognized synthax:\n%s", rline);
          exit_code += 1;
          goto exit;
#endif
          break;
        }
      strcpy (instr, "");
    }
exit:
  if (readf != NULL)
    {
      fflush (readf);
      /* fclose (readf); */
    }
  else
    {
      printf ("%s failure closing\n", vmPath);
      exit_code += 1;
    }
  if (writef != NULL)
    {
      fflush (writef);
      fclose (writef);
    }
  else
    {
      exit_code += 1;
      printf ("%s failure closing\n", asmPath);
    }
  return exit_code;
}
