#include "binary.h"
#include "intpow.h"
#include <stdio.h>
#include <malloc.h>

int main ()
{
  char * cas = malloc(sizeof(char)*10);
  printf("%p", cas);
  free(cas);
  printf("%p", cas);
  cas = "asdf";
  printf("%p", cas);
  return 0;
}
