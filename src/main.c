#include <stdio.h>

#include "packer.h"

int main(int argc, char **argv)
{
    int r;

    if (argc != 3) {
        printf("Usage %s <exe> <out>\n", argv[0]);
        return 1;
    }
    r = packer_pack_executable(argv[1], "xor", argv[2]);
    if (r < 0) {
        puts(packer_get_error(r));
        return 1;
    }
    return 0;
}