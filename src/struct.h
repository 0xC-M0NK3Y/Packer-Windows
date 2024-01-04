#ifndef STRUCT_H__
# define STRUCT_H__

#include <stdint.h>

#include <windows.h>

typedef struct function_import {
    void    *function;
    char    ***symbols;
}   function_import_t;

typedef struct packed_exe {
    uintptr_t   image_base;
    size_t      headers_size;      
    uint16_t    section_align;
    uint16_t    file_align;

    IMAGE_DOS_HEADER    dos;
    IMAGE_NT_HEADERS    nt;

    struct {
        uint8_t                 *data;
        size_t                  data_size;
        void                    *decompressor;
        void                    *decompressor_end;
        size_t                  bootloader_size;
        size_t                  loader_size;
        size_t                  decompressor_size;
        IMAGE_SECTION_HEADER    header;
    }   text_section;

    struct {
        uint8_t                 *data;
        size_t                  data_size;
        IMAGE_SECTION_HEADER    header;
    }   rdata_section;

    struct {
        uint8_t                 *data;
        size_t                  data_size;
        const function_import_t *bootloader_import;
        const function_import_t *decompressor_import;
        const function_import_t *loader_import;
        function_import_t       *packed_exe_import;
        uint32_t                iat;
        uint32_t                iat_size;
        IMAGE_SECTION_HEADER    header;
    }   idata_section;
}   packed_exe_t;

#endif /* struct.h */