#ifndef _WIN32
#include <dirent.h>
#endif
#include "metavm.h"
#include "parse.h"
#include "vmt.h"
#include "wf.h"
#include <string.h>

int
main (int argc, char *argv[])
{
  /* const char *const vmPath = argc >= 2 ? argv[1] : NULL; */
  if (argc < 2)
    {
      printf ("VMtranslator file.vm\nOr...\nVmtranslator dir\n");
      return 1;
    }

  char vmPath[256];
  char asmPath[256 + 4];
  FILE *writef;
  int exit_code;

  strcpy (vmPath, argv[1]);
  strcpy (asmPath, vmPath);

  if (strstr (vmPath, ".vm"))
    { // file
      for (int i = strlen (asmPath) - 1; i > 0; i--)
        if (asmPath[i] == '.')
          strcpy (asmPath + i, ".asm");

      // init
      writef = fopen (asmPath, "w");
      if (!writef)
        {
          printf ("Error writing %s\n", asmPath);
          return 1;
        }
      fputs ("@256\nD=A\n@SP\nM=D\n\n", writef);
      fclose (writef);

      if (metaparse (vmPath, ".meta.vm") != 0)
        {
          printf ("Error in metaparse %s\n", vmPath);
          return -1;
        }

      for (int i = strlen (vmPath) - 1; i > 0; i--)
        if (vmPath[i] == '.')
          strcpy (vmPath + i, ".meta.vm");

      exit_code = parse (vmPath, asmPath);
      return exit_code;
    }
  else
    { // directory
      WIN_WARN ()

      DIR *d;
      struct dirent *dir;
      char file_list[256][256 * 2];
      int flsize = 0;

      strcat (asmPath, ".asm");
      // init
      writef = fopen (asmPath, "w");
      if (!writef)
        {
          printf ("Error writing %s\n", asmPath);
          return -1;
        }

      fputs ("@256\nD=A\n@SP\nM=D\n\n", writef);
      wfunctioncall ("call Sys.init 0\n", writef);

      d = opendir (vmPath);
      exit_code = 0;
      if (!d)
        {
          printf ("Failed to open dir %s\n", vmPath);
          return -1;
        }

      for (flsize = 0; (dir = readdir (d)) != NULL;)
        {
          if (dir->d_name[0] == '.')
            continue;
          if (strstr (dir->d_name, ".meta.vm"))
            continue;
          if (strstr (dir->d_name, ".vm"))
            {
              char relName[256 * 2 + 1];
              strcpy (relName, vmPath);
              strcat (relName, "/");
              strcat (relName, dir->d_name);
              strcpy (file_list[flsize], relName);
              flsize++;
              if (metaparse (relName, ".meta.vm"))
                {
                  printf ("Error in metaparse %s\n", relName);
                  closedir (d);
                  fclose (writef);
                  return -1;
                }
            }
        }

      closedir (d);

      d = opendir (vmPath);
      flsize = 0;
      if (!d)
        {
          printf ("Failed to open dir %s\n", vmPath);
          return -1;
        }

      for (flsize = 0; (dir = readdir (d)) != NULL;)
        {
          if (dir->d_name[0] == '.')
            continue;
          if (strstr (dir->d_name, ".meta.vm"))
            {
              char relName[256 * 2 + 1];
              strcpy (relName, vmPath);
              strcat (relName, "/");
              strcat (relName, dir->d_name);
              strcpy (file_list[flsize], relName);
              flsize++;
            }
        }

      closedir (d);
      for (int i = 0; i < flsize; i++)
        {
          char buff[256 * 3];
          printf ("Reading %s\n", file_list[i]);
          sprintf (buff, "\n\n//---%s---\n", file_list[i]);
          fputs (buff, writef);
          fclose (writef);

          exit_code = parse (file_list[i], asmPath);
          if (exit_code)
            {
              printf ("Error in file %s\n", dir->d_name);
              goto mainexit;
            }
        }
    }
mainexit:
  if (exit_code == 0)
    {
      printf ("Results written to %s.\n", asmPath);
    }
  return exit_code;
}
