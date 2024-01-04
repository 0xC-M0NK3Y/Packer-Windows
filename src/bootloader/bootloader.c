#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "../defines.h"
#include "../struct.h"

void bootloader(void);

const function_import_t bootloader_imports = {
    .function = bootloader,
    .symbols  = (char **[]) {
        NULL
    }
};

void bootloader(void) {
    uint8_t    *(*decompressor)(const uint8_t *, size_t, size_t *, int) = (void *)DECOMPRESSOR_PTR_IN_BOOTLOADER;
    void    (*loader)(void *)       = (void *)LOADER_PTR_IN_BOOTLOADER;
    void    *data_to_decompress     = (void *)DATATODECOMPRESS_PTR_IN_BOOTLOADER;
    size_t  data_to_decompress_size = (size_t)DATATODECOMPRESSSIZE_PTR_BOOTLOADER;
    int     level                   = (int)LEVEL_IN_BOOTLOADER;
    void    *decompressed;
    size_t  decompressed_size;

    decompressed = decompressor(data_to_decompress, data_to_decompress_size, &decompressed_size, level);
    if (decompressed != NULL) 
        loader(decompressed);
}
void bootloader_end(void) {}