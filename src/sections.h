#ifndef SECTIONS_H__
# define SECTIONS_H__

#include <stdint.h>

#include "struct.h"

int create_section_text(packed_exe_t *pckd_exe);
int create_section_rdata(packed_exe_t *pckd_exe, uint8_t *compressed_exe, size_t compressed_exe_size);
int create_section_idata(packed_exe_t *pckd_exe);

#endif /* sections.h */