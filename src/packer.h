#ifndef PACKER_H__
# define PACKER_H__

int     packer_pack_executable(char *executable, char *algorithm, char *out);
char    *packer_get_error(int error);

#endif /* packer.h */