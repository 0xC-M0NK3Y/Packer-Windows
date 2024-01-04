#include <stddef.h>
#include <stdint.h>

#include <windows.h>

#include "../struct.h"

uint8_t *xor_decompressor(const uint8_t *data, size_t data_size, size_t *out_size, int xor_key);

const function_import_t xor_imports = {
    .function = xor_decompressor,
    .symbols  = (char**[]){
        (char *[]){"KERNEL32.dll", "VirtualAlloc", NULL},
        NULL
    }
};

// level is xorkey
uint8_t *xor_compressor(const uint8_t *data, size_t data_size, size_t *out_size, int xor_key) {
    uint8_t *ret;

    ret = VirtualAlloc(NULL, data_size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (ret == NULL)
        return NULL;
    for (size_t i = 0; i < data_size; i++)
        ret[i] = ((uint8_t *)data)[i] ^ ((uint8_t *)&xor_key)[i%sizeof(xor_key)];
    *out_size = data_size;

    return ret;
}

uint8_t *xor_decompressor(const uint8_t *data, size_t data_size, size_t *out_size, int xor_key) {
    uint8_t *ret;

    ret = VirtualAlloc(NULL, data_size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (ret == NULL)
        return NULL;
    for (size_t i = 0; i < data_size; i++)
        ret[i] = ((uint8_t *)data)[i] ^ ((uint8_t *)&xor_key)[i%sizeof(xor_key)];
    *out_size = data_size;

    return ret;
}
void xor_decompressor_end() {}
