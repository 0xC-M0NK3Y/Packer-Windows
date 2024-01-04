#include <stdio.h>
#include <stdint.h>

#include <assert.h>

#include "struct.h"
#include "algorithms/algorithms.h"
#include "utils.h"
#include "sections.h"
#include "loader/loader.h"
#include "bootloader/bootloader.h"
#include "header.h"
#include "checksum.h"
#include "fix.h"
#include "packer_erros.h"

static void free_packed_exe(packed_exe_t *pckd_exe) {
    free(pckd_exe->text_section.data);
    free(pckd_exe->rdata_section.data);
    free(pckd_exe->idata_section.data);
}

static int build_executable(packed_exe_t *pckd_exe, char *out) {

    uint32_t    checksum;
    uint8_t     *buffer;
    size_t      filesize;
    size_t      offset = 0;
    FILE        *fp;

    fp = fopen(out, "wb");
    if (fp == NULL)
        return -1;

    filesize = pckd_exe->idata_section.header.PointerToRawData + pckd_exe->idata_section.header.SizeOfRawData;
    buffer = malloc(filesize);
    if (buffer == NULL)
        return fclose(fp), -1;
    memset(buffer, 0, filesize);

    memcpy(&buffer[offset], (uint8_t *)&pckd_exe->dos, sizeof(IMAGE_DOS_HEADER));
    offset += sizeof(IMAGE_DOS_HEADER);
    memcpy(&buffer[offset], (uint8_t *)&pckd_exe->nt, sizeof(IMAGE_NT_HEADERS));
    offset += sizeof(IMAGE_NT_HEADERS);
    memcpy(&buffer[offset], (uint8_t *)&pckd_exe->text_section.header, sizeof(IMAGE_SECTION_HEADER));
    offset += sizeof(IMAGE_SECTION_HEADER);
    memcpy(&buffer[offset], (uint8_t *)&pckd_exe->rdata_section.header, sizeof(IMAGE_SECTION_HEADER));
    offset += sizeof(IMAGE_SECTION_HEADER);
    memcpy(&buffer[offset], (uint8_t *)&pckd_exe->idata_section.header, sizeof(IMAGE_SECTION_HEADER));
    offset += sizeof(IMAGE_SECTION_HEADER);

    memcpy(buffer + pckd_exe->text_section.header.PointerToRawData, pckd_exe->text_section.data, pckd_exe->text_section.data_size);
    memcpy(buffer + pckd_exe->rdata_section.header.PointerToRawData, pckd_exe->rdata_section.data, pckd_exe->rdata_section.data_size);
    memcpy(buffer + pckd_exe->idata_section.header.PointerToRawData, pckd_exe->idata_section.data, pckd_exe->idata_section.data_size);

    checksum = pe_header_checksum((uint32_t *)buffer, filesize);

    pckd_exe->nt.OptionalHeader.CheckSum = checksum;
    memcpy(buffer + sizeof(IMAGE_DOS_HEADER), (uint8_t *)&pckd_exe->nt, sizeof(IMAGE_NT_HEADERS));

    if (fwrite(buffer, 1, filesize, fp) != filesize)
        return fclose(fp), free(buffer), -1;
    fclose(fp);
    free(buffer);

    return 0;
}

int packer_pack_executable(char *executable, char *algorithm, char *out) {

    packed_exe_t    packed_exe;
    uint8_t         *compressed_exe;
    size_t          compressed_exe_size;
    uint8_t         *exe;
    size_t          exe_size;

    memset(&packed_exe, 0, sizeof(packed_exe));

#ifdef _WIN64
    assert(sizeof(uintptr_t) == sizeof(uint64_t));
#else
    assert(sizeof(uintptr_t) == sizeof(uint32_t));
#endif

    // open and parse exe
    exe = open_file(executable, &exe_size);
    if (exe == NULL)
        return PACKER_ERROR_OPEN_FILE;
    if (parse_pe(exe) < 0)
        return free(exe), PACKER_ERROR_CORRUPTED_EXECUTABLE;

    // TODO: look image base from original exe and try compute one better
    packed_exe.image_base    = 0x40000;
    packed_exe.section_align = 0x1000;
    packed_exe.file_align    = 0x200;
    packed_exe.headers_size  = ALIGN((sizeof(IMAGE_DOS_HEADER)
                                   + sizeof(IMAGE_NT_HEADERS)
                                   + 3 * sizeof(IMAGE_SECTION_HEADER)),
                             packed_exe.file_align);
    packed_exe.idata_section.loader_import     = &loader_imports;
    packed_exe.idata_section.bootloader_import = &bootloader_imports;

    // compress exe
    if (strcmp(algorithm, "xor") == 0) {
        uint8_t *tmp = NULL;

        tmp                                        = xor_compressor(exe, exe_size, &compressed_exe_size, 0x26262626);
        packed_exe.text_section.decompressor       = (void *)&xor_decompressor;
        packed_exe.text_section.decompressor_end   = (void *)&xor_decompressor_end;
        packed_exe.idata_section.decompressor_import = &xor_imports;

        if (tmp == NULL)
            return free(exe), PACKER_ERROR_COMRPESSING_EXECUTABLE;
        // do copy to permit free with free
        compressed_exe = malloc(compressed_exe_size);
        if (compressed_exe == NULL)
            return VirtualFree(tmp, compressed_exe_size, MEM_RELEASE), free(exe), PACKER_ERROR_COMRPESSING_EXECUTABLE;
        memcpy(compressed_exe, tmp, compressed_exe_size);
        VirtualFree(tmp, compressed_exe_size, MEM_RELEASE);
    } else {
        free(exe);
        return PACKER_ERROR_UNKNOWN_COMPRESSION_ALGORITHM;
    }
    free(exe);
    if (compressed_exe == NULL)
        return PACKER_ERROR_COMRPESSING_EXECUTABLE;

    // create sections
    if (create_section_text(&packed_exe) < 0)
        return free(compressed_exe), PACKER_ERROR_CREATING_TEXT_SECTION;
    create_section_rdata(&packed_exe, compressed_exe, compressed_exe_size);
    if (create_section_idata(&packed_exe) < 0)
        return free_packed_exe(&packed_exe), PACKER_ERROR_CREATING_IDATA_SECTION;
    // fix code
    if (fix_section_text(&packed_exe, 0x26262626) < 0)
        return free_packed_exe(&packed_exe), PACKER_ERROR_FIXING_TEXT_SECTION;
    // create headers
    create_headers(&packed_exe);
    // build final executable
    if (build_executable(&packed_exe, out) < 0)
        return free_packed_exe(&packed_exe), PACKER_ERROR_BUILDING_FINAL_EXECUTABLE;

    free_packed_exe(&packed_exe);

    return 0;
}

char *packer_get_error(int error) {
    switch (error) {
        case 0: return NULL;
        case PACKER_ERROR_OPEN_FILE:                     return "error opening file";
        case PACKER_ERROR_CORRUPTED_EXECUTABLE:          return "corrupted executable";
        case PACKER_ERROR_UNKNOWN_COMPRESSION_ALGORITHM: return "unknown compression algorithm";
        case PACKER_ERROR_COMRPESSING_EXECUTABLE:        return "error compressing executable";
        case PACKER_ERROR_CREATING_TEXT_SECTION:         return "error creating .text section";
        case PACKER_ERROR_CREATING_IDATA_SECTION:        return "error creating .idata section";
        case PACKER_ERROR_FIXING_TEXT_SECTION:           return "error fixing .text section";
        case PACKER_ERROR_BUILDING_FINAL_EXECUTABLE:     return "error building final executable";
        default: return "unknown error code";
    }
    return NULL;
}