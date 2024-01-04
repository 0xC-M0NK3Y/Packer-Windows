#include <stdio.h>
#include <stdint.h>

#include <windows.h>

#include "struct.h"

uint8_t *open_file(const char *fname, size_t *fsize) {
    FILE    *fp;
    uint8_t *ret;
    size_t  size;

    fp = fopen(fname, "rb");
    if (fp == NULL)
        return NULL;
    fseek(fp, 0, SEEK_END);
    size = (size_t)ftell(fp);
    rewind(fp);
    ret = malloc(size);
    if (ret == NULL)
        return NULL;
    fread(ret, 1, size, fp);
    fclose(fp);
    *fsize = size;
    return ret;
}

int parse_pe(uint8_t *exe) {
    PIMAGE_DOS_HEADER dos;
    PIMAGE_NT_HEADERS nt;

    dos = (PIMAGE_DOS_HEADER)exe;
    nt =  (PIMAGE_NT_HEADERS)(exe + dos->e_lfanew);

    if (dos->e_magic != 0x5A4D)
        return -1;
    if (nt->Signature != 0x00004550)
        return -1;
#ifdef _WIN64
    if (nt->OptionalHeader.Magic != 0x20b)
#else
    if (nt->OptionalHeader.Magic != 0x10b)
#endif
        return -1;
    return 0;
}

void free_strtab(char **strs) {
    for (int i = 0; strs[i]; i++)
        free(strs[i]);
    free(strs);
}

char **copy_strtab(char **strs) {
    char    **ret;
    int     l = 0;

    if (strs == NULL)
        return NULL;

    for (l = 0; strs[l]; l++)
        ;
    ret = malloc((l + 1) * sizeof(char *));
    if (ret == NULL)
        return NULL;
    memset(ret, 0, (l + 1) * sizeof(char *));

    for (int i = 0; strs[i]; i++) {
        ret[i] = strdup(strs[i]);
        if (ret[i] == NULL)
            return free_strtab(ret), NULL;
    }
    return ret;
}

int add_to_strtab(char ***strs, char *str) {
    int     n = 0;
    char    **tmp;

    for (n = 0; (*strs)[n]; n++)
        ;
    tmp = malloc((n + 2)*sizeof(char *));
    if (tmp == NULL)
        return -1;
    for (n = 0; (*strs)[n]; n++)
        tmp[n] = (*strs)[n];
    tmp[n] = strdup(str);
    if (tmp[n] == NULL)
        return free(tmp), -1;
    tmp[n+1] = NULL;
    *strs = tmp;
    return 0;
}

// add with double \0 to set hint to 0
int add_str_to_buffer(char **buf, size_t *buflen, char *str) {
    void *tmp;
    size_t tmplen;

    tmplen = strlen(str);
    tmp = realloc(*buf, *buflen + tmplen + 2);
    if (tmp == NULL)
        return -1;
    *buf = tmp;
    memset(&(*buf)[*buflen], 0, tmplen + 2);
    memcpy(&(*buf)[*buflen], str, tmplen);
    *buflen += tmplen+2;
    return 0;
}