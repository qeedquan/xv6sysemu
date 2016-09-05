BIN=xv6sysemu
CFLAGS=-Wall -Wextra -pedantic -ggdb -g3 -std=c11
LDFLAGS=-lelf

all:
	@echo use \"make amd64\" for x86_64 bit binary
	@echo use \"make i386\" for i386 bit binary

format:
	clang-format -i src/*.c src/*.h

amd64:
	nasm -f elf64 src/bits64.s
	cc -o $(BIN)64 src/*.c src/bits64.o $(CFLAGS) $(LDFLAGS) -m64

i386:
	nasm -f elf32 src/bits32.s
	cc -o $(BIN)32 src/*.c src/bits32.o $(CFLAGS) $(LDFLAGS) -m32

clean:
	rm -f src/*.o $(BIN)64 $(BIN)32
