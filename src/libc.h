#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <err.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ucontext.h>
#include <libelf.h>

#define nelem(x) (sizeof(x) / sizeof(x[0]))

#define min(a, b) (((a) < (b)) ? (a) : (b))

#define USED(x) ((void)x)

#ifdef __x86_64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#define elf_getehdr elf64_getehdr
#define elf_getphdr elf64_getphdr
#define RIP REG_RIP
#define RSI REG_RSI
#define RSP REG_RSP
#define RAX REG_RAX
#define RDI REG_RDI
#define RDX REG_RDX
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#define elf_getehdr elf32_getehdr
#define elf_getphdr elf32_getphdr
#define RIP REG_EIP
#define RSI REG_ESI
#define RSP REG_ESP
#define RAX REG_EAX
#define RDI REG_EDI
#define RDX REG_EDX
#endif
