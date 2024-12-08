#ifndef BINARY_H_
#define BINARY_H_
#include <stddef.h>

const int dec2int (char *const str, const size_t offset, int *const out);
[[nodiscard]] const int int2bin16 (int addr, char *const binary);

#endif // BINARY_H_
