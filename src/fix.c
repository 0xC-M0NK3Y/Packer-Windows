#include <stdint.h>

#include <windows.h>

#include "struct.h"
#include "defines.h"

static inline PIMAGE_IMPORT_DESCRIPTOR get_current_import_descriptor(uint8_t *image_base) {
    PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)image_base;
	PIMAGE_NT_HEADERS nt_header = (PIMAGE_NT_HEADERS)((uintptr_t)image_base + dos_header->e_lfanew);
	IMAGE_DATA_DIRECTORY tmp = nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	return (PIMAGE_IMPORT_DESCRIPTOR)(tmp.VirtualAddress + (uintptr_t)image_base);
}

static int get_new_offset(const packed_exe_t *pckd_exe, uintptr_t func_call, uint32_t *offset) {

    uint8_t                     *image_base = NULL;
	PIMAGE_IMPORT_DESCRIPTOR    import_ptr = NULL;
    
    image_base = (uint8_t *)GetModuleHandle(NULL);
    if (image_base == NULL)
        return -1;
    import_ptr = get_current_import_descriptor(image_base);

    while (import_ptr->Name)
    {
        PIMAGE_THUNK_DATA orig = (PIMAGE_THUNK_DATA)(image_base + import_ptr->OriginalFirstThunk);
        PIMAGE_THUNK_DATA first = (PIMAGE_THUNK_DATA)(image_base + import_ptr->FirstThunk);

        while (orig->u1.AddressOfData)
        {
            PIMAGE_IMPORT_BY_NAME funcname = (PIMAGE_IMPORT_BY_NAME)(image_base + orig->u1.AddressOfData);

            if ((uintptr_t)&first->u1.AddressOfData == func_call) {
                int t = 0;
                for (int i = 0; pckd_exe->idata_section.packed_exe_import->symbols[i]; i++) {
                    for (int j = 1; pckd_exe->idata_section.packed_exe_import->symbols[i][j]; j++) {
                        if (strcmp(funcname->Name, pckd_exe->idata_section.packed_exe_import->symbols[i][j]) == 0) {
                            *offset = pckd_exe->idata_section.iat + t * sizeof(IMAGE_THUNK_DATA);
                            return 0;
                        }
                        t++;    
                    }
                }
            }
            first++;
            orig++;
        }
        import_ptr++;
    }
    return -1;
}

int fix_section_text(packed_exe_t *pckd_exe, int level) {
    int c = 0;

    for (size_t i = 0; i < pckd_exe->text_section.data_size; i++) {
        if (i < pckd_exe->text_section.bootloader_size) { // bootloader
            if (*(uintptr_t *)&pckd_exe->text_section.data[i] == DATATODECOMPRESS_PTR_IN_BOOTLOADER) {
                *(uintptr_t *)&pckd_exe->text_section.data[i] = pckd_exe->image_base + pckd_exe->rdata_section.header.VirtualAddress;
                c++;
            }
            if (*(uintptr_t *)&pckd_exe->text_section.data[i] == DATATODECOMPRESSSIZE_PTR_BOOTLOADER) {
                *(uintptr_t *)&pckd_exe->text_section.data[i] = pckd_exe->rdata_section.data_size;
                c++;
            }
            if (*(int *)&pckd_exe->text_section.data[i] == LEVEL_IN_BOOTLOADER) {
                *(int *)&pckd_exe->text_section.data[i] = level;
                c++;
            }
        }
#ifdef _WIN64
        if (*(uint16_t *)&pckd_exe->text_section.data[i] == 0x8b48 && *(uint16_t *)&pckd_exe->text_section.data[i+7] == 0xd0ff) {
            uintptr_t func = *(uint32_t *)&pckd_exe->text_section.data[i+3] + 7 + i;

            if (i < pckd_exe->text_section.bootloader_size) // bootloader
                func += (uintptr_t)pckd_exe->idata_section.bootloader_import->function;
            else if (i > pckd_exe->text_section.bootloader_size && i < pckd_exe->text_section.bootloader_size + pckd_exe->text_section.loader_size) // loader
                func += (uintptr_t)pckd_exe->idata_section.loader_import->function - pckd_exe->text_section.bootloader_size;
            else // compressor 
                func += (uintptr_t)pckd_exe->idata_section.decompressor_import->function - (pckd_exe->text_section.bootloader_size + pckd_exe->text_section.loader_size);
#else
        if (pckd_exe->text_section.data[i] == 0xa1 && *(uint16_t *)&pckd_exe->text_section.data[i+5] == 0xd0ff) {
            uintptr_t func = *(uint32_t *)&pckd_exe->text_section.data[i+1];
#endif
            uint32_t new_offset;
            if (get_new_offset(pckd_exe, func, &new_offset) < 0)
                return -1;
#ifdef _WIN64
            *(uint32_t *)&pckd_exe->text_section.data[i+3] = new_offset - pckd_exe->text_section.header.VirtualAddress - i - 7;
#else
            *(uint32_t *)&pckd_exe->text_section.data[i+1] = new_offset + pckd_exe->image_base;
#endif
        }
    }
    return (c == 3);
}

