#include <stdint.h>

#include <windows.h>

#include "../struct.h"

void loader(uint8_t *data);

const function_import_t loader_imports = {
    .function = loader,
    .symbols  = (char **[]) {
        (char *[]){"KERNEL32.dll", "VirtualAlloc", "LoadLibraryA", "GetProcAddress", "ExitProcess", NULL},
        NULL
    }
};

static int parse_exe(uint8_t *ptr);
static void *map_exe(uint8_t *ptr);
static int loadimport_exe(uint8_t *ptr);
__attribute__((always_inline))
static inline void	*inline_memset(void *s, int c, size_t n) {
	for (size_t i = 0; i < n; i++)
		((uint8_t *)s)[i] = (uint8_t)c;
	return s;
}
__attribute__((always_inline))
static inline void	*inline_memcpy(void *dest, const void *src, size_t n) {
	for (size_t i = 0; i < n; i++)
		((uint8_t *)dest)[i] = ((uint8_t *)src)[i];
	return dest;
}
__attribute__((always_inline))
static inline uint32_t get_entrypoint_exe(uint8_t *ptr) {
    return ((PIMAGE_NT_HEADERS)(ptr + ((PIMAGE_DOS_HEADER)ptr)->e_lfanew))->OptionalHeader.AddressOfEntryPoint;
}

// minimalistic windows loader
void loader(uint8_t *data) {

    uint8_t *mapped_exe;
    uint32_t entry_point;

    if (parse_exe(data) < 0)
        ExitProcess(1);
    mapped_exe = map_exe(data);
    if (mapped_exe == NULL) 
        ExitProcess(1);
    if (loadimport_exe(mapped_exe) < 0)
        ExitProcess(1);
    // TODO: memory protection
    entry_point = get_entrypoint_exe(data);
    // just for fun
#ifdef _WIN64
    asm volatile ("mov %%rax, %0\n\t"
                  "call rax"
                  :
                  :"r"((uintptr_t)mapped_exe+(uintptr_t)entry_point)
                  :"rax");
#else
    asm volatile ("mov %%eax, %0\n\t"
                  "call eax"
                  :
                  :"r"((uintptr_t)mapped_exe+(uintptr_t)entry_point)
                  :"eax");
#endif
    ExitProcess(0);
}

static int parse_exe(uint8_t *ptr) {

    PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS nt;

    dos = (PIMAGE_DOS_HEADER)ptr;
    if (dos->e_magic != 0x5A4D)
        return -1;
    nt = (PIMAGE_NT_HEADERS)(ptr + dos->e_lfanew);
    if (nt->Signature != 0x00004550)
        return -1;
#ifdef _WIN64
    if (nt->OptionalHeader.Magic != 0x20B)
        return -1;
#else
    if (nt->OptionalHeader.Magic != 0x10B)
        return -1;
#endif
    return 0;
}


static int reloc_exe(uint8_t *ptr, uintptr_t exe_ptr, uintptr_t image_base) {

    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(ptr + ((PIMAGE_DOS_HEADER)ptr)->e_lfanew);
    PIMAGE_BASE_RELOCATION reloc = (PIMAGE_BASE_RELOCATION)(ptr + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);

    while (reloc->VirtualAddress != 0) {
        uint32_t entry = (uint32_t)((reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD));
        uint16_t *tmp = (uint16_t *)((PIMAGE_BASE_RELOCATION)reloc + 1);
        for (uint32_t i = 0; i < entry; i++) {
            uint8_t type = (uint8_t)(tmp[i] >> 12);
            uint16_t offset = tmp[i] & 0xFFF;
            uintptr_t *address = (uintptr_t *)(ptr + reloc->VirtualAddress + offset);
            if (type == IMAGE_REL_BASED_HIGHLOW || type == IMAGE_REL_BASED_DIR64) {
                *address -= image_base;
            	*address += exe_ptr;
            }
        }
        reloc = (PIMAGE_BASE_RELOCATION)(((uint8_t *)reloc) + reloc->SizeOfBlock); 
    }
	return 0;
}

static void *map_exe(uint8_t *ptr) {

    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(ptr + ((PIMAGE_DOS_HEADER)ptr)->e_lfanew);
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt);

    uint8_t *ret = VirtualAlloc((void *)nt->OptionalHeader.ImageBase, nt->OptionalHeader.SizeOfImage, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (ret == NULL) {
        ret = VirtualAlloc(NULL, nt->OptionalHeader.SizeOfImage, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (ret == NULL)
            return NULL;
    }
    
    inline_memset(ret, 0, nt->OptionalHeader.SizeOfImage);
    inline_memcpy(ret, ptr, nt->OptionalHeader.SizeOfHeaders);
    for (int i = 0; i < nt->FileHeader.NumberOfSections; i++) {
        if (section[i].PointerToRawData)
            inline_memcpy(ret + section[i].VirtualAddress, ptr + section[i].PointerToRawData, section[i].SizeOfRawData);
    }
    if ((uintptr_t)nt->OptionalHeader.ImageBase != (uintptr_t)ret)
        reloc_exe(ret, (uintptr_t)ret, (uintptr_t)nt->OptionalHeader.ImageBase);
    return ret;
}

static int loadimport_exe(uint8_t *ptr) {

    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)(ptr + ((PIMAGE_DOS_HEADER)ptr)->e_lfanew);
    PIMAGE_IMPORT_DESCRIPTOR imports = (PIMAGE_IMPORT_DESCRIPTOR)(ptr + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    while (imports->Name)
    {
        HANDLE module = LoadLibraryA((char *)(ptr + imports->Name));
        if (module == NULL)
            return -__LINE__;
        PIMAGE_THUNK_DATA view_thunk = (PIMAGE_THUNK_DATA)(ptr + imports->OriginalFirstThunk);
        PIMAGE_THUNK_DATA set_thunk = (PIMAGE_THUNK_DATA)(ptr + imports->FirstThunk);
        if (imports->OriginalFirstThunk == 0)
            view_thunk = (PIMAGE_THUNK_DATA) set_thunk;
        while (view_thunk->u1.AddressOfData)
        {
            if (IMAGE_SNAP_BY_ORDINAL(view_thunk->u1.Ordinal)) {
				uintptr_t adress = (uintptr_t) GetProcAddress(module, ((LPCSTR)IMAGE_ORDINAL(view_thunk->u1.Ordinal)));
                if (adress == 0)
                    return -1;
				*(uintptr_t *)set_thunk = adress;
            } else {
                uintptr_t adress = (uintptr_t) GetProcAddress(module, ((PIMAGE_IMPORT_BY_NAME)(ptr + view_thunk->u1.AddressOfData))->Name);
                if (adress == 0)
                    return -1;
				*(uintptr_t *)set_thunk = adress;
            }
            view_thunk++;
            set_thunk++;
        }
        imports++;
    }
    imports->TimeDateStamp = 1;
    return 0;
}

void loader_end(void) {}