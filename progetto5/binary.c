#include "binary.h"
#include "intpow.h"
#include <stddef.h>

[[nodiscard]] const int inline int2bin16 (int addr, char *const binary)
{
  if (addr > ipow (2, 15) || addr < -ipow (2, 15))
    return -1;
  // most signf. bit to the left
  for (int i = 15; i >= 0; i--)
    {
      if ((addr >> i) & 1)
        binary[15 - i] = '1';
      else
        binary[15 - i] = '0';
    }
  return 0;
}

const int inline dec2int (char *const str, const size_t offset, int *const out)
{
  *out = 0;
  for (int i = 0; i <= offset; i++)
    {
      if (str[i] < '0' || str[i] > '9')
        return -1;
      *out += (str[i] - '0') * ipow (10, offset - i);
    }
  return 0;
}
