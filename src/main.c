#include "u.h"
#include "libc.h"
#include "dat.h"
#include "fns.h"

static noreturn void
usage(void)
{
	errx(2, "usage: cmd [args]");
}

int
main(int argc, char *argv[])
{
	int c;
	while ((c = getopt(argc, argv, "")) != -1) {
		switch (c) {
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (argc < 1)
		usage();

	if (elf_version(EV_CURRENT) == EV_NONE)
		errx(1, "elf_version: %s", elf_errmsg(-1));

	char *altstack = calloc(SIGSTKSZ, 1);
	if (!altstack)
		errx(1, "calloc: %s", strerror(errno));

	stack_t ss = {
	    .ss_sp = altstack,
	    .ss_size = SIGSTKSZ,
	};
	if (sigaltstack(&ss, NULL) < 0)
		errx(1, "sigaltstack: %s", strerror(errno));

	sigset_t set;
	if (sigfillset(&set) < 0)
		errx(1, "sigfillset: %s", strerror(errno));

	struct sigaction act = {
	    .sa_sigaction = trap,
	    .sa_flags = SA_SIGINFO | SA_ONSTACK,
	    .sa_mask = set,
	};
	if (sigaction(SIGSEGV, &act, NULL) < 0)
		errx(1, "sigaction: %s", strerror(errno));

	exec(argc, argv);
	return 0;
}
