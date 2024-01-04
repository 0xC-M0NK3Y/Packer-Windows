#include <stdlib.h>
#include <string.h>

#include <stdint.h>

#include <assert.h>

#include <windows.h> 

#include "struct.h"
#include "bootloader/bootloader.h"
#include "loader/loader.h"
#include "utils.h"
#include "defines.h"

static inline size_t compute_text_section_data_size(const packed_exe_t *pckd_exe) {
    return ((uintptr_t)pckd_exe->text_section.decompressor_end - (uintptr_t)pckd_exe->text_section.decompressor)
        + ((uintptr_t)bootloader_end - (uintptr_t)bootloader) + ((uintptr_t)loader_end - (uintptr_t)loader);
}

int create_section_text(packed_exe_t *pckd_exe) {

    uint8_t     *data     = NULL;
    size_t      data_size = 0;
    size_t      offset    = 0;
    uint32_t    decompressor_addr;
    uint32_t    loader_addr;

    data_size = compute_text_section_data_size(pckd_exe);
    data = malloc(data_size);
    if (data == NULL)
        return -1;
    memset(data, 0x90, data_size);

    pckd_exe->text_section.bootloader_size = ((uintptr_t)bootloader_end - (uintptr_t)bootloader);
    memcpy(&data[offset], (void *)bootloader, pckd_exe->text_section.bootloader_size);
    offset += pckd_exe->text_section.bootloader_size;

    pckd_exe->text_section.loader_size = ((uintptr_t)loader_end - (uintptr_t)loader);
    loader_addr = offset;
    memcpy(&data[offset], (void *)loader, pckd_exe->text_section.loader_size);
    offset += pckd_exe->text_section.loader_size;

    pckd_exe->text_section.decompressor_size = ((uintptr_t)pckd_exe->text_section.decompressor_end - (uintptr_t)pckd_exe->text_section.decompressor);
    decompressor_addr = offset;
    memcpy(&data[offset], (void *)pckd_exe->text_section.decompressor, pckd_exe->text_section.decompressor_size);
    offset += pckd_exe->text_section.decompressor_size;

    memcpy(pckd_exe->text_section.header.Name, ".text", sizeof(".text"));

    pckd_exe->text_section.header.Misc.VirtualSize     = (uint32_t)ALIGN(data_size, pckd_exe->file_align);;
    pckd_exe->text_section.header.VirtualAddress       = ALIGN((uint32_t)pckd_exe->headers_size, pckd_exe->section_align);
    pckd_exe->text_section.header.SizeOfRawData        = (uint32_t)ALIGN(data_size, pckd_exe->file_align);;
    pckd_exe->text_section.header.PointerToRawData     = ALIGN((uint32_t)pckd_exe->headers_size, pckd_exe->file_align);
    pckd_exe->text_section.header.PointerToRelocations = 0;
    pckd_exe->text_section.header.NumberOfRelocations  = 0;
    pckd_exe->text_section.header.NumberOfLinenumbers  = 0;
    pckd_exe->text_section.header.Characteristics      = IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA;

    int r = 0;
    for (size_t i = 0; i < pckd_exe->text_section.bootloader_size; i++) {
        if (*(uintptr_t *)&data[i] == DECOMPRESSOR_PTR_IN_BOOTLOADER) {
            *(uintptr_t *)&data[i] = pckd_exe->image_base + pckd_exe->text_section.header.VirtualAddress + decompressor_addr;
            r++;
        } else if (*(uintptr_t *)&data[i] == LOADER_PTR_IN_BOOTLOADER) {
            *(uintptr_t *)&data[i] = pckd_exe->image_base + pckd_exe->text_section.header.VirtualAddress + loader_addr;
            r++;
        }
    }

    if (r != 2)
        return free(data), -1;

    pckd_exe->text_section.data      = data;
    pckd_exe->text_section.data_size = data_size;

    return 0;
}