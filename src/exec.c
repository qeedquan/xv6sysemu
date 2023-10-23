#include "u.h"
#include "libc.h"
#include "dat.h"
#include "fns.h"

static bool
inrange(uintptr s, uintptr e, uintptr p)
{
	return s <= p && p <= e;
}

static bool
ismapped(uintptr s, uintptr e)
{
	char name[256];
	snprintf(name, sizeof(name), "/proc/%d/maps", getpid());

	FILE *fp = fopen(name, "rt");
	if (!fp)
		errx(1, "failed to get memory mappings for process");

	char line[1024];
	while (fgets(line, sizeof(line), fp)) {
		uintptr start, end;
		if (sscanf(line, "%" SCNxPTR "-%" SCNxPTR, &start, &end) == 2) {
			end--;
			if (inrange(start, end, s) || inrange(start, end, e) ||
			    inrange(s, e, start) || inrange(s, e, end)) {
				fclose(fp);
				return true;
			}
		}
	}

	fclose(fp);
	return false;
}

void
exec(int argc, char *argv[])
{
	int fd = open(argv[0], O_RDONLY);
	if (fd < 0)
		errx(1, "open: %s", strerror(errno));

	struct stat st;
	if (fstat(fd, &st) < 0)
		errx(1, "fstat: %s", strerror(errno));

	u8 *mem = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (mem == MAP_FAILED)
		errx(1, "mmap: %s", strerror(errno));

	Elf *elf = elf_begin(fd, ELF_C_READ, NULL);
	if (!elf)
		errx(1, "elf_begin: %s", elf_errmsg(-1));

	Elf_Ehdr *ehdr = elf_getehdr(elf);
	if (!ehdr)
		errx(1, "elf_getehdr: %s", elf_errmsg(-1));

	Elf_Phdr *phdr = elf_getphdr(elf);
	if (!phdr)
		errx(1, "elf_getphdr: %s", elf_errmsg(-1));

	if (elf_kind(elf) != ELF_K_ELF)
		errx(1, "elf_kind: unsupported executable format");

	uintptr sp;
	uintp *stack = NULL;
	uintp ustack[3 + MAXARG + 1];

	int flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED;
	int prot = PROT_READ | PROT_WRITE | PROT_EXEC;
	for (size_t i = 0; i < ehdr->e_phnum; i++) {
		Elf_Phdr *p = phdr + i;
		if (p->p_type != ELF_PROG_LOAD)
			continue;

		if (p->p_memsz < p->p_filesz)
			errx(1, "elf: invalid executable size");

		if (p->p_vaddr >= 0xffffffffull || p->p_memsz >= 0xffffffffull ||
		    (u64)(p->p_vaddr + PGROUNDUP(p->p_memsz)) >= 0xffffffffull)
			errx(1, "elf: virtual address too large %llx", (uvlong)p->p_vaddr);

		if ((p->p_vaddr % PGSIZE) != 0)
			errx(1, "elf: invalid virtual address %llx", (uvlong)p->p_vaddr);

		if (ismapped(p->p_vaddr, p->p_vaddr + PGROUNDUP(p->p_memsz)))
			errx(1, "elf: virtual address %llx has an overlapping mapping", (uvlong)p->p_vaddr);

		if ((off_t)p->p_offset >= st.st_size || (off_t)p->p_filesz >= st.st_size || st.st_size - p->p_filesz < p->p_offset)
			errx(1, "elf: invalid header offsets");

		void *addr = mmap((void *)p->p_vaddr, PGROUNDUP(p->p_memsz), prot, flags, -1, 0);
		if (addr == MAP_FAILED)
			errx(1, "mmap: failed to map virtual address %llx: %s", (uvlong)p->p_vaddr, strerror(errno));

		memmove(addr, mem + p->p_offset, p->p_filesz);
		stack = (uintp *)PGROUNDUP(p->p_vaddr + p->p_memsz);
	}

	if (ismapped((uintptr)stack, (uintptr)stack + PGSIZE * 2))
		errx(1, "stack address %p has overlapping an mapping", (void *)stack);
	stack = mmap(stack, PGSIZE * 2, prot, flags, -1, 0);
	if (stack == MAP_FAILED)
		errx(1, "mmap: %s", strerror(errno));
	sp = ((uintptr)stack) + PGSIZE * 2;

	if (ismapped(SYSSTACK, SYSSTACK + SYSSIZE))
		errx(1, "sysstack address %llx has an overlapping mapping", SYSSTACK);
	if (mmap((void *)SYSSTACK, SYSSIZE, prot, flags, -1, 0) == MAP_FAILED)
		errx(1, "mmap: %s", strerror(errno));

	for (int i = 0; i < argc; i++) {
		if (i >= MAXARG)
			errx(1, "exec: too many arguments");

		size_t len = strlen(argv[i]) + 1;
		if (len > 32)
			errx(1, "argument too long");

		sp = (sp - len) & ~(sizeof(uintp) - 1);
		ustack[3 + i] = sp;
		memmove((void *)sp, argv[i], len);
	}
	ustack[3 + argc] = 0;

	ustack[0] = ehdr->e_entry;
	ustack[1] = argc;
	ustack[2] = sp - (argc + 1) * sizeof(uintp);

	size_t len = (3 + argc + 1) * sizeof(uintp);
	sp -= len;
	memmove((void *)sp, ustack, len);

	munmap(mem, st.st_size);
	elf_end(elf);
	close(fd);

	retuser((uintp)sp, (uintp)argc, (uintp)argv);
}
