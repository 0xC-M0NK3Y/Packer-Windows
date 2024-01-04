#include <stdint.h>

#include "struct.h"
#include "utils.h"

int create_section_rdata(packed_exe_t *pckd_exe, uint8_t *compressed_exe, size_t compressed_exe_size) {

    pckd_exe->rdata_section.data      = compressed_exe;
    pckd_exe->rdata_section.data_size = compressed_exe_size;

    memcpy(pckd_exe->rdata_section.header.Name, ".rdata", sizeof(".rdata"));
    pckd_exe->rdata_section.header.Misc.VirtualSize     = (uint32_t)ALIGN(pckd_exe->rdata_section.data_size, pckd_exe->file_align);;
    pckd_exe->rdata_section.header.VirtualAddress       = ALIGN((pckd_exe->text_section.header.VirtualAddress + pckd_exe->text_section.header.Misc.VirtualSize), pckd_exe->section_align);
    pckd_exe->rdata_section.header.SizeOfRawData        = (uint32_t)ALIGN(pckd_exe->rdata_section.data_size, pckd_exe->file_align);;
    pckd_exe->rdata_section.header.PointerToRawData     = ALIGN((pckd_exe->text_section.header.PointerToRawData + pckd_exe->text_section.header.SizeOfRawData), pckd_exe->file_align);
    pckd_exe->rdata_section.header.PointerToRelocations = 0;
    pckd_exe->rdata_section.header.NumberOfRelocations  = 0;
    pckd_exe->rdata_section.header.NumberOfLinenumbers  = 0;
    pckd_exe->rdata_section.header.Characteristics      = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ;

    return 0;
}