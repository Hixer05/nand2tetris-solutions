#include "intpow.h"

const int
ipow (const int a, const int e)
{
  if(!e)
    return 1;
  int prod = a;
  for (register int i = 1; i < e; i++)
    prod *= a;
  return prod;
}
