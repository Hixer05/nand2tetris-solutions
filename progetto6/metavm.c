#include "metavm.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// open `file.vm` and applies metavm optimizations to `file.meta.vm`
// file.vm must be valid
int
metaparse (const char *const vmPath, const char *const extension)
{
  char metaPath[256 * 2];
  char tmpPath[256 * 2];
  strcpy (metaPath, vmPath);
  strcpy (tmpPath, vmPath);
  for (int i = strlen (metaPath); i > 0; i--)
    {
      if (metaPath[i] == '.')
        {
          strcpy (metaPath + i, extension);
          strcpy (tmpPath + i, ".tmp");
          break;
        }
    }

  if (pushpop (vmPath, tmpPath))
    return -1;
  // copy
  /* FILE *const metaf = fopen (metaPath, "r"); */
  /* FILE *const tempf = fopen(tmpPath, "w"); */
  /* if (!tempf) */
  /*   { */
  /*     printf ("Error writing %s\n", tmpPath); */
  /*     return -1; */
  /*   } */
  /* if (!metaf) */
  /*   { */
  /*     printf ("Error reading %s\n", metaPath); */
  /*     fclose (tempf); */
  /*     return -1; */
  /*   } */

  /* char line[100]; */
  /* while(fgets(line, 100, metaf)) */
  /*   fputs(line, tempf); */
  /* fclose(tempf); */
  /* fclose(metaf); */

  if (add (tmpPath, metaPath))
    return -1;

  return 0;
}

// if this sequence
// push seg1 k
// pop seg2 x
// translate to `move seg1 k seg2 x`
#define MAX_RLINE 80
int
pushpop (const char *const vmPath, const char *const metaPath)
{
  // open files
  FILE *readf = fopen (vmPath, "r");
  if (readf == NULL)
    {
      printf ("Metavm error can't open: %s\n", vmPath);
      return -1;
    }
  FILE *writef = fopen (metaPath, "w");
  if (writef == NULL)
    {
      printf ("Metavm error can't open: %s\n", metaPath);
      fclose (readf);
      return -1;
    }

  // READ FILE INTO MEMORY
  fseek (readf, 0, SEEK_END);
  long fsize = ftell (readf);
  fseek (readf, 0, SEEK_SET); /* same as rewind(readf); */

  char *string = malloc (fsize + 1);
  fread (string, fsize, 1, readf);
  fclose (readf);

  long lines = 0;
  for (int i = 0; i < fsize; i++)
    {
      if (string[i] == '\n')
        lines++;
    }

  char output[lines][80];
  int line = 0;
  int check_line = 0;
  char *cur = strtok (string, "\n");
  char prev[80] = "";
  bool flag = false;
  while (cur != NULL)
    {
      if (check_line >= lines)
        break;
      char tmp[100];
      if (sscanf (cur, " //%s ", tmp) == 1)
        {
          cur = strtok (NULL, "\n");
          continue;
        }

      if (strstr (cur, "push"))
        {
          flag = true;
          strcpy (prev, cur);
        }
      if (strstr (cur, "pop") && flag)
        {
          // write to output-1
          char wline[80], s1[20], s2[20], l1[10], l2[10];
          sscanf (prev, " push %s %s ", s1, l1);
          sscanf (cur, " pop %s %s ", s2, l2);
          sprintf (wline, "move %s %s %s %s \n", s1, l1, s2, l2);
          strcpy (output[line - 1], wline);
          /* printf("Wrote: %s", wline); */
          flag = false;
        }
      else
        {
          strcpy (output[line], cur);
          strcat (output[line], "\n");
          line++;
          if (!strstr (cur, "push"))
            { // should be push
              flag = false;
            }
        }
      check_line++;
      cur = strtok (NULL, "\n");
    }

  for (int i = 0; i < line; i++)
    {
      fputs (output[i], writef);
    }

  fclose (writef);
  free (string);
  return 0;
}

int
add (const char *const vmPath, const char *const metaPath)
{

  FILE *readf = fopen (vmPath, "r");
  if (readf == NULL)
    {
      printf ("Metavm error can't open: %s\n", vmPath);
      return -1;
    }
  FILE *writef = fopen (metaPath, "w");
  if (writef == NULL)
    {
      printf ("Metavm error can't open: %s\n", metaPath);
      fclose (readf);
      return -1;
    }

  // READ FILE INTO MEMORY
  fseek (readf, 0, SEEK_END);
  long fsize = ftell (readf);
  fseek (readf, 0, SEEK_SET); /* same as rewind(readf); */

  char *string = malloc (fsize + 1);
  fread (string, fsize, 1, readf);
  fclose (readf);

  long lines = 0;
  for (int i = 0; i < fsize; i++)
    {
      if (string[i] == '\n')
        lines++;
    }

  /* char output[lines][80]; */
  /* int line = 0; */
  int check_line = 0;
  char *cur = strtok (string, "\n");
  char prev[2][80] = { "" };
  size_t pushcount = 0;
  while (cur != NULL)
    {
      if (check_line >= lines)
        break;
      char tmp[100];
      if (sscanf (cur, " //%s ", tmp) == 1)
        {
          cur = strtok (NULL, "\n");
          continue;
        }

      if (strstr (cur, "add") && pushcount == 2)
        {
          char wline[100];
          char data[4][20] = { 0 };
          sscanf (prev[0], "push %s %s ", data[0], data[1]);
          sscanf (prev[1], "push %s %s ", data[2], data[3]);
          /* printf (">%s\t\t\t>%s\n", prev[0], prev[1]); */
          /* printf (">>%s\t\t\t>%s\n", data[0], data[1]); */
          /* printf (">>%s\t\t\t>%s\n", data[2], data[3]); */
          /* printf ("---\n"); */
          sprintf (wline, "sum %s %s %s %s \n", data[0], data[1], data[2],
                   data[3]);
          fputs (wline, writef);
          pushcount = 0;
        }
      else if (strstr (cur, "push"))
        {
          if (pushcount < 2)
            {
              strcpy (prev[pushcount], cur);
              pushcount++;
            }
          else
            {
              fprintf(writef, "%s\n", prev[0]);
              strcpy (prev[0], prev[1]);
              strcpy (prev[1], cur);
            }
        }
      else
        {
          for (int i = 0; i < pushcount; i++)
            fprintf (writef, "%s\n", prev[i]);
          pushcount = 0;
          fputs (cur, writef);
          fputc ('\n', writef);
        }

      check_line++;
      cur = strtok (NULL, "\n");
    }
  for (int i = 0; i < pushcount; i++)
    {
      fprintf (writef, "%s\n", prev[i]);
    }
  pushcount = 0;
  /* for (int i = 0; i < line; i++) */
  /*   { */
  /*     fputs (output[i], writef); */
  /*   } */

  fclose (writef);
  free (string);
  return 0;
}
