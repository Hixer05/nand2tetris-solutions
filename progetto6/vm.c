#ifndef _WIN32
#include <dirent.h>
#endif
#include "parse.h"
#include "wf.h"
#include <string.h>

#ifdef _WIN32
#define WIN_WARN()                                                                                 \
    {                                                                                              \
        printf ("VMtranslator file.vm\n Can translate only files on Windows.\n");                  \
        return 1;                                                                                  \
    }
#endif
#ifndef _WIN32
#define WIN_WARN()                                                                                 \
    {                                                                                              \
    }
#endif

int
main (int argc, char *argv[])
{
    /* const char *const vmPath = argc >= 2 ? argv[1] : NULL; */
    char vmPath[256];
    if(argc<2)
    {
        printf ("VMtranslator file.vm\nOr...\nVmtranslator dir\n");
        return 1;
    }

    char asmPath[256];
    FILE *writef;
    int exit_code;

    strcpy(vmPath, argv[1]);
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

            exit_code = parse (vmPath, asmPath);
            return exit_code;
        }
    else
        { // directory
            WIN_WARN ()

            DIR *d;
            struct dirent *dir;

            strcat (asmPath, ".vm");
            // init
            writef = fopen (asmPath, "w");
            if (!writef)
                {
                    printf ("Error opening %s\n", asmPath);
                    return -1;
                }

            wfunctioncall ("call Sys.init 0\n", writef);
            fputs ("@256\nD=A\n@SP\nM=D\n\n", writef);

            d = opendir (vmPath);
            exit_code = 0;
            if (!d)
                {
                    printf ("Failed to open dir %s\n", vmPath);
                    return -1;
                }

            while ((dir = readdir (d)) != NULL)
                {
                    if (dir->d_name[0] == '.')
                        continue;
                    if (strstr (dir->d_name, ".vm"))
                        {
                            char buff[256 * 2], absVmPath[256 * 2];

                            strcpy (absVmPath, vmPath);
                            strcat (absVmPath, "/");
                            strcat (absVmPath, dir->d_name);

                            printf ("Reading %s\n", absVmPath);
                            sprintf (buff, "\n\n//---%s---\n", dir->d_name);
                            fputs (buff, writef);
                            fclose (writef);

                            exit_code = parse (absVmPath, asmPath);
                            if (exit_code)
                                {
                                    printf ("Error in file %s\n", dir->d_name);
                                    goto mainexit;
                                }
                        }
                }
        mainexit:
            closedir (d);
            return exit_code;
        }
}
