NAME64 = ./bin/x86_64/packer.exe
LIB64  = ./lib/x86_64/packer.a
NAME32 = ./bin/x86/packer.exe
LIB32  = ./lib/x86/packer.a
SRC    = $(wildcard src/*.c) $(wildcard src/*/*.c) $(wildcard src/*/*/*.c)
OBJ_64 = $(addprefix build/x86_64/, $(SRC:.c=.o))
OBJ_32 = $(addprefix build/x86/, $(SRC:.c=.o))
CFLAGS = -Wall -Wextra -Wno-format -masm=intel
CC64   = x86_64-w64-mingw32-gcc
CC32   = i686-w64-mingw32-gcc
AR64   = x86_64-w64-mingw32-ar rcs
AR32   = i686-w64-mingw32-ar rcs

all: $(NAME64) $(NAME32) $(LIB64) $(LIB32)
	mkdir -p ./include
	cp ./src/packer.h ./include

$(NAME64): $(OBJ_64)
	mkdir -p ./bin/x86_64
	$(CC64) $(CFLAGS) $(OBJ_64) -o $(NAME64)
$(NAME32): $(OBJ_32)
	mkdir -p ./bin/x86
	$(CC32) $(CFLAGS) $(OBJ_32) -o $(NAME32)
$(LIB64): $(OBJ_64)
	mkdir -p ./lib/x86_64
	$(AR64) $(LIB64) $(OBJ_64)
$(LIB32): $(OBJ_32)
	mkdir -p ./lib/x86
	$(AR32) $(LIB32) $(OBJ_32)

./build/x86_64/%.o: %.c
	mkdir -p $(shell dirname $@)
	$(CC64) $(CFLAGS) -c $< -o $@
./build/x86/%.o: %.c
	mkdir -p $(shell dirname $@)
	$(CC32) $(CFLAGS) -c $< -o $@

clean:
	rm -rf ./build/*

fclean: clean
	rm -rf ./bin/*
	rm -rf ./lib/*

re: fclean all

.phony : all clean fclean re
