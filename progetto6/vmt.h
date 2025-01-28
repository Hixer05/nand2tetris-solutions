#ifndef VMT_H_
#define VMT_H_

#ifdef _WIN32
#define WIN_WARN()                                                            \
  {                                                                           \
    printf ("VMtranslator file.vm\n Can translate single files only on "      \
            "Windows.\n");                                                    \
    return 1;                                                                 \
  }
#endif
#ifndef _WIN32
#define WIN_WARN()                                                            \
  {                                                                           \
  }
#endif

#endif // VMT_H_
