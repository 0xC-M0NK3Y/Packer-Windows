#include <stdint.h>

#include <windows.h>

// TODO: find where I found this,
uint32_t pe_header_checksum(uint32_t *base, size_t size) {

    PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS nt;
    uint8_t *ptr;
    uint32_t sum = 0;
    size_t i;

    dos = (PIMAGE_DOS_HEADER)base;
    nt = (PIMAGE_NT_HEADERS)((uint8_t *)base + dos->e_lfanew);
    ptr = (uint8_t *)&nt->OptionalHeader.CheckSum;

    for (i = 0; i < (size/4); i++) {
        if (i == (((uintptr_t)ptr - (uintptr_t)base)/4)) // here before was hardcoded, fix with getting the pointeur to checksum in header to not compute it
            continue;
        sum += __builtin_uadd_overflow(base[i],sum,&sum);
    }
    if (size%4)
        sum += base[i];

    sum = (sum&0xffff) + (sum>>16);
    sum += (sum>>16);
    sum &= 0xffff;
    return (uint32_t)(sum+size);
}
