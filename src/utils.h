#ifndef UTILS_H__
# define UTILS_H__

#include <stdint.h>

#include "struct.h"

uint8_t *open_file(const char *fname, size_t *fsize);
int     parse_pe(uint8_t *exe);
int     is_pe_64(uint8_t *exe);
char    **copy_strtab(char **strs);
int     add_to_strtab(char ***strs, char *str);
int     add_str_to_buffer(char **buf, size_t *buflen, char *str);

#define ALIGN(x, alignement) ((x | (alignement-1)) + 1)

#endif /* utils.h */