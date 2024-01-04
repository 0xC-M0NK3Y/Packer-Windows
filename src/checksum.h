#ifndef CHECKSUM_H__
# define CHECKSUM_H__

#include <stdint.h>

uint32_t pe_header_checksum(uint32_t *base, size_t size);

#endif /* checksum.h */