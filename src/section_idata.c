#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "struct.h"
#include "utils.h"

static void free_function_import(function_import_t *function_import) {
    if (function_import == NULL)
        return;
    for (int i = 0; function_import->symbols[i]; i++) {
        for (int j = 0; function_import->symbols[i][j]; j++)
            free(function_import->symbols[i][j]);
        free(function_import->symbols[i]);
    }
    free(function_import->symbols);
    free(function_import);
}

static function_import_t *merge_imports(const function_import_t *import1, const function_import_t *import2) {

    function_import_t   *ret;
    size_t              nb = 0;
    void                *tmp;

    if (import1 == NULL || import2 == NULL)
        return NULL;

    ret = malloc(sizeof(*ret));
    if (ret == NULL)
        return NULL;
    memset(ret, 0, sizeof(*ret));

    for (int i = 0; import1->symbols[i]; i++) {
        if (import1->symbols[i][0] == NULL) // malformed function_import_t, if the char ** is not NULL then it should have minimum the dll name as (char **)[0]
            return free_function_import(ret), NULL;
        tmp = realloc(ret->symbols, (nb + 2)*sizeof(char **));
        if (tmp == NULL)
            return free_function_import(ret), NULL;
        ret->symbols = tmp;
        ret->symbols[nb] = copy_strtab(import1->symbols[i]);
        if (ret->symbols[nb] == NULL)
            return free_function_import(ret), NULL;
        ret->symbols[nb+1] = NULL;
        nb++;
    }
    for (int i = 0; import2->symbols[i]; i++) {
        int f = -1;
        if (import2->symbols[i][0] == NULL)
            return free_function_import(ret), NULL;
        for (int j = 0; ret->symbols[j]; j++) {
            if (strcmp(import2->symbols[i][0], ret->symbols[j][0]) == 0) {
                f = j;
                break;
            }
        }
        if (f >= 0) {
            for (int j = 0; import2->symbols[i][j]; j++) {
                int p = 0;
                for (int k = 0; ret->symbols[f][k]; k++) {
                    if (strcmp(import2->symbols[i][j], ret->symbols[f][k]) == 0) {
                        p = 1;
                        break;
                    }
                }
                if (p == 0) {
                    if (add_to_strtab(&ret->symbols[f], import2->symbols[i][j]) < 0)
                        return free_function_import(ret), NULL;        
                }
            }
        } else {
            tmp = realloc(ret->symbols, (nb + 2)*sizeof(char **));
            if (tmp == NULL)
                return free_function_import(ret), NULL;
            ret->symbols = tmp;
            ret->symbols[nb] = copy_strtab(import2->symbols[i]);
            if (ret->symbols[nb] == NULL)
                return free_function_import(ret), NULL;
            ret->symbols[nb+1] = NULL;
            nb++;
        }
    }
    return ret;
}

static inline int get_dll_import_number(const function_import_t *imports) {
    int r = 0;
    for (r = 0; imports->symbols[r]; r++)
        ;
    return r;
}

static inline int get_function_import_number(const function_import_t *imports) {
    int r = 0;
    for (int i = 0; imports->symbols[i]; i++) {
        for (int j = 1; imports->symbols[i][j]; j++)
            r++;
    }
    return r;
}

static char *make_string_buffer_from_function_import(const function_import_t *imports, size_t *outlen) {
    char    *ret   = NULL;
    size_t  retlen = 0;

    for (int i = 0; imports->symbols[i]; i++) {
        for (int j = 0; imports->symbols[i][j]; j++) {
            if (add_str_to_buffer(&ret, &retlen, imports->symbols[i][j]) < 0)
                return free(ret), NULL;
        }
    }
    *outlen = retlen;
    return ret;
}

static uint8_t *create_import_table(packed_exe_t *pckd_exe, uint32_t virt_addr, size_t *outlen) {

    uint8_t                     *data;
    size_t                      data_size;
    function_import_t           *tmp_imports   = NULL;
    function_import_t           *imports       = NULL;
    PIMAGE_IMPORT_DESCRIPTOR    import_descr   = NULL;
    int                         dll_number;
    PIMAGE_THUNK_DATA           thunks         = NULL;
    int                         thunks_nb;
    int                         thunks_index   = 0;
    char                        *string_buffer = NULL;
    size_t                      string_buffer_size;
    size_t                      offset = 0;
    char                        *buf_ptr;

    tmp_imports = merge_imports(pckd_exe->idata_section.loader_import, pckd_exe->idata_section.decompressor_import);
    if (tmp_imports == NULL)
        goto error_free_and_exit;
    imports = merge_imports(tmp_imports, pckd_exe->idata_section.bootloader_import);
    if (imports == NULL)
        goto error_free_and_exit;
    free_function_import(tmp_imports);
    tmp_imports = NULL;

    string_buffer = make_string_buffer_from_function_import(imports, &string_buffer_size);
    if (string_buffer == NULL)
        goto error_free_and_exit;
    
    dll_number = get_dll_import_number(imports);
    import_descr = malloc(sizeof(IMAGE_IMPORT_DESCRIPTOR) * (dll_number + 1));
    if (import_descr == NULL)
        goto error_free_and_exit;
    memset(import_descr, 0, sizeof(IMAGE_IMPORT_DESCRIPTOR) * (dll_number + 1));

    thunks_nb = get_function_import_number(imports) + dll_number;

    thunks = malloc(thunks_nb * sizeof(IMAGE_THUNK_DATA));
    if (thunks == NULL)
        goto error_free_and_exit;
    memset(thunks, 0, thunks_nb * sizeof(IMAGE_THUNK_DATA));

    offset  = virt_addr + (dll_number+1) * sizeof(IMAGE_IMPORT_DESCRIPTOR) + (thunks_nb * 2) * sizeof(IMAGE_THUNK_DATA);
    buf_ptr = string_buffer;

    for (int i = 0; i < dll_number; i++) {
        import_descr[i].Characteristics    = 0;
        import_descr[i].FirstThunk         = virt_addr + sizeof(IMAGE_IMPORT_DESCRIPTOR) * (dll_number+1) + thunks_index * sizeof(IMAGE_THUNK_DATA);
        import_descr[i].TimeDateStamp      = 0;
        import_descr[i].ForwarderChain     = 0;
        import_descr[i].Name               = offset;
        import_descr[i].OriginalFirstThunk = virt_addr + sizeof(IMAGE_IMPORT_DESCRIPTOR) * (dll_number+1) + (thunks_index * 2) * sizeof(IMAGE_THUNK_DATA);

        offset  += strlen(buf_ptr);
        buf_ptr += strlen(buf_ptr) + 2;
        for (int j = 1; imports->symbols[i][j]; j++) {
            thunks[thunks_index].u1.ForwarderString = offset;
            offset  += strlen(buf_ptr) + 2;
            buf_ptr += strlen(buf_ptr) + 2;
            thunks_index++;
        }
        thunks_index++;
        offset += 2;
    }

    data_size = sizeof(IMAGE_IMPORT_DESCRIPTOR) * (dll_number+1)
                + sizeof(IMAGE_THUNK_DATA) * (thunks_nb * 2)
                + string_buffer_size;
    data = malloc(data_size);
    if (data == NULL)
        goto error_free_and_exit;
    memset(data, 0, data_size);

    offset = 0;

    memcpy(&data[offset], import_descr, sizeof(IMAGE_IMPORT_DESCRIPTOR) * (dll_number + 1));
    offset += sizeof(IMAGE_IMPORT_DESCRIPTOR) * (dll_number + 1);
    memcpy(&data[offset], thunks, thunks_nb * sizeof(IMAGE_THUNK_DATA));
    offset += thunks_nb * sizeof(IMAGE_THUNK_DATA);
    memcpy(&data[offset], thunks, thunks_nb * sizeof(IMAGE_THUNK_DATA));
    offset += thunks_nb * sizeof(IMAGE_THUNK_DATA);
    memcpy(&data[offset], string_buffer, string_buffer_size);

    pckd_exe->idata_section.iat               = import_descr[0].FirstThunk;
    pckd_exe->idata_section.iat_size          = thunks_nb * sizeof(IMAGE_THUNK_DATA);
    pckd_exe->idata_section.packed_exe_import = imports;

    free(string_buffer);
    free(import_descr);
    free(thunks);

    *outlen = data_size;
    
    return data;

error_free_and_exit:
    free_function_import(imports);
    free_function_import(tmp_imports);
    free(string_buffer);
    free(import_descr);
    free(thunks);
    return NULL;
}

int create_section_idata(packed_exe_t *pckd_exe) {

    memcpy(pckd_exe->idata_section.header.Name, ".idata", sizeof(".idata"));
    pckd_exe->idata_section.header.VirtualAddress       = ALIGN((pckd_exe->rdata_section.header.VirtualAddress + pckd_exe->rdata_section.header.Misc.VirtualSize), pckd_exe->section_align);
    pckd_exe->idata_section.header.PointerToRawData     = ALIGN((pckd_exe->rdata_section.header.PointerToRawData + pckd_exe->rdata_section.header.SizeOfRawData), pckd_exe->file_align);
    pckd_exe->idata_section.header.PointerToRelocations = 0;
    pckd_exe->idata_section.header.NumberOfRelocations  = 0;
    pckd_exe->idata_section.header.NumberOfLinenumbers  = 0;
    pckd_exe->idata_section.header.Characteristics      = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ;

    pckd_exe->idata_section.data = create_import_table(pckd_exe, pckd_exe->idata_section.header.VirtualAddress, &pckd_exe->idata_section.data_size);
    if (pckd_exe->idata_section.data == NULL)
        return -1;

    pckd_exe->idata_section.header.SizeOfRawData        = (uint32_t)ALIGN(pckd_exe->idata_section.data_size, pckd_exe->file_align);;
    pckd_exe->idata_section.header.Misc.VirtualSize     = (uint32_t)ALIGN(pckd_exe->idata_section.data_size, pckd_exe->file_align);;

    return 0;
}