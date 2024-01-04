#include <time.h>

#include <windows.h>

#include "struct.h"
#include "utils.h"

int create_headers(packed_exe_t *pckd_exe) {

    /* Dos header */
    pckd_exe->dos.e_magic  = 0x5A4D; // "MZ"
    pckd_exe->dos.e_lfanew = sizeof(pckd_exe->dos); // offset to nt header

    /* Signature */
    pckd_exe->nt.Signature = 0x00004550; // "PE\0\0"

    /* File header */
#ifdef _WIN64
    pckd_exe->nt.FileHeader.Machine = IMAGE_FILE_MACHINE_AMD64;
#else
    pckd_exe->nt.FileHeader.Machine = IMAGE_FILE_MACHINE_I386;
#endif
    pckd_exe->nt.FileHeader.NumberOfSections     = 3;
    pckd_exe->nt.FileHeader.TimeDateStamp        = (long unsigned int)time(NULL);
    pckd_exe->nt.FileHeader.PointerToSymbolTable = 0;
    pckd_exe->nt.FileHeader.NumberOfSymbols      = 0;
    pckd_exe->nt.FileHeader.SizeOfOptionalHeader = sizeof(pckd_exe->nt.OptionalHeader);
    pckd_exe->nt.FileHeader.Characteristics      = IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_LARGE_ADDRESS_AWARE | IMAGE_FILE_RELOCS_STRIPPED;

    /* Optionnal header */
#ifdef _WIN64
    pckd_exe->nt.OptionalHeader.Magic                       = 0x20B; // pe32+
#else
    pckd_exe->nt.OptionalHeader.Magic                       = 0x10B; // pe32
#endif
    pckd_exe->nt.OptionalHeader.MajorLinkerVersion          = 2;
    pckd_exe->nt.OptionalHeader.MinorLinkerVersion          = 35;
    pckd_exe->nt.OptionalHeader.SizeOfCode                  = (uint32_t)pckd_exe->text_section.header.SizeOfRawData;
    pckd_exe->nt.OptionalHeader.SizeOfInitializedData       = (uint32_t)(pckd_exe->text_section.header.SizeOfRawData + pckd_exe->rdata_section.header.SizeOfRawData + pckd_exe->idata_section.header.SizeOfRawData);
    pckd_exe->nt.OptionalHeader.SizeOfUninitializedData     = 0;
    pckd_exe->nt.OptionalHeader.AddressOfEntryPoint         = pckd_exe->text_section.header.VirtualAddress;
    pckd_exe->nt.OptionalHeader.BaseOfCode                  = pckd_exe->text_section.header.VirtualAddress;
#ifndef _WIN64
    pckd_exe->nt.OptionalHeader.BaseOfData                  = 0;
#endif
    pckd_exe->nt.OptionalHeader.ImageBase                   = pckd_exe->image_base;
    pckd_exe->nt.OptionalHeader.SectionAlignment            = pckd_exe->section_align;
    pckd_exe->nt.OptionalHeader.FileAlignment               = pckd_exe->file_align;
    pckd_exe->nt.OptionalHeader.MajorOperatingSystemVersion = 4;
    pckd_exe->nt.OptionalHeader.MinorOperatingSystemVersion = 0;
    pckd_exe->nt.OptionalHeader.MajorImageVersion           = 1;
    pckd_exe->nt.OptionalHeader.MinorImageVersion           = 0;
    pckd_exe->nt.OptionalHeader.MajorSubsystemVersion       = 4;
    pckd_exe->nt.OptionalHeader.MinorSubsystemVersion       = 0;
    pckd_exe->nt.OptionalHeader.Win32VersionValue           = 0;
    pckd_exe->nt.OptionalHeader.SizeOfImage                 = ALIGN((pckd_exe->idata_section.header.VirtualAddress + pckd_exe->idata_section.header.Misc.VirtualSize), pckd_exe->section_align);
    pckd_exe->nt.OptionalHeader.SizeOfHeaders               = (uint32_t)pckd_exe->headers_size;
    pckd_exe->nt.OptionalHeader.CheckSum                    = 0;
    pckd_exe->nt.OptionalHeader.Subsystem                   = IMAGE_SUBSYSTEM_WINDOWS_CUI;
    pckd_exe->nt.OptionalHeader.DllCharacteristics          = 0;
    pckd_exe->nt.OptionalHeader.SizeOfStackReserve          = 0x200000;
    pckd_exe->nt.OptionalHeader.SizeOfStackCommit           = 0x1000;
    pckd_exe->nt.OptionalHeader.SizeOfHeapReserve           = 0x100000;
    pckd_exe->nt.OptionalHeader.SizeOfHeapCommit            = 0x1000;
    pckd_exe->nt.OptionalHeader.LoaderFlags                 = 0;
    pckd_exe->nt.OptionalHeader.NumberOfRvaAndSizes         = 0x10;

    /* 0 export directory */
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress = 0;
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size           = 0;
    /* 1 import directory */
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = pckd_exe->idata_section.header.VirtualAddress;
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size           = pckd_exe->idata_section.header.SizeOfRawData;
    /* 2 ressource directory */
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress = 0;
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size           = 0;
    /* 3 exception directory */
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress = 0;
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size           = 0;
    /* 4 security directory */
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress = 0;
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].Size           = 0;
    /* 5 base relocation directory */
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = 0;
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size           = 0;
    /* 6 debug directory */
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress = 0;
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size           = 0;
    /* 7 description directory */
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_ARCHITECTURE].VirtualAddress = 0;
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_ARCHITECTURE].Size           = 0;
    /* 8 special direcotry */
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_GLOBALPTR].VirtualAddress = 0;
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_GLOBALPTR].Size           = 0;
    /* 9 thread storage directory */
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress = 0;
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size           = 0;
    /* 10 load configuration directory */
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress = 0;
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].Size           = 0;
    /* 11 bound import directory */
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress = 0;
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size           = 0;
    /* 12 import address table IAT directory */
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress = pckd_exe->idata_section.iat;
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size           = (uint32_t)pckd_exe->idata_section.iat_size;
    /* 13 delay import directory */
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress = 0;
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].Size           = 0;
    /* 14 CLR runtime header directory */
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress = 0;
    pckd_exe->nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].Size           = 0;
    /* 15 reserved */
    pckd_exe->nt.OptionalHeader.DataDirectory[15].VirtualAddress = 0;
    pckd_exe->nt.OptionalHeader.DataDirectory[15].Size           = 0;

    return 0;
}