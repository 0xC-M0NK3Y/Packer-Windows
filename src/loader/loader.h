#ifndef LOADER_H__
# define LOADER_H__

#include "../struct.h"

void loader(uint8_t *data);
void loader_end(void);

extern const function_import_t loader_imports;

#endif /* loader.h */