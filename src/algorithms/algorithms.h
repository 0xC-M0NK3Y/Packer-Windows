#ifndef ALGORITHMS_H__
# define ALGORITHMS_H__

#include <stddef.h>

#include "../struct.h"

uint8_t *xor_compressor(const uint8_t *data, size_t data_size, size_t *out_size, int xor_key);
uint8_t *xor_decompressor(const uint8_t *data, size_t data_size, size_t *out_size, int xor_key);
void xor_decompressor_end();
extern const function_import_t xor_imports;

#endif /* algorithms.h */