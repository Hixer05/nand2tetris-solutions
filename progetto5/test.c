#include "binary.h"
#include "intpow.h"
#include <stdio.h>

int
main ()
{
  int i = 1;
  char asdf[16];
  (void)int2bin16 (i, asdf);
  printf ("%d -> %s\n", i, asdf);
  return 0;
}
