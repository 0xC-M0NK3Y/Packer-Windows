#ifndef BOOTLOADER_H__
# define BOOTLOADER_H__

#include "../struct.h"

void bootloader(void);
void bootloader_end(void);

extern const function_import_t bootloader_imports;

#endif /* bootloader.h */