#include "u.h"
#include "libc.h"
#include "dat.h"
#include "fns.h"

typedef struct Call Call;

struct Call {
	const char *name;
	void (*func)();
	intp (*stub)(mcontext_t *, void (*)());
};

static intp
fcall0(mcontext_t *m, void (*f)(void))
{
	USED(m);
	return ((intp(*)(void))f)();
}

static intp
fcall1(mcontext_t *m, void (*f)(void))
{
	return ((intp(*)(intp))f)(m->gregs[0]);
}

static intp
fcall2(mcontext_t *m, void (*f)(void))
{
	return ((intp(*)(intp, intp))f)(m->gregs[0], m->gregs[1]);
}

static intp
fcall3(mcontext_t *m, void (*f)(void))
{
	return ((intp(*)(intp, intp, intp))f)(m->gregs[0], m->gregs[1], m->gregs[2]);
}

static Call systab[] = {
    {NULL, NULL, NULL},
    {"fork", (void (*)(void))sysfork, fcall0},
    {"exit", (void (*)(void))sysexit, fcall0},
    {"wait", (void (*)(void))syswait, fcall0},
    {"pipe", (void (*)(void))syspipe, fcall1},
    {"read", (void (*)(void))sysread, fcall3},
    {"kill", (void (*)(void))syskill, fcall1},
    {"exec", (void (*)(void))sysexec, fcall2},
    {"fstat", (void (*)(void))sysfstat, fcall2},
    {"chdir", (void (*)(void))syschdir, fcall1},
    {"dup", (void (*)(void))sysdup, fcall1},
    {"getpid", (void (*)(void))sysgetpid, fcall0},
    {"sbrk", (void (*)(void))syssbrk, fcall1},
    {"sleep", (void (*)(void))syssleep, fcall1},
    {"uptime", (void (*)(void))sysuptime, fcall0},
    {"open", (void (*)(void))sysopen, fcall2},
    {"write", (void (*)(void))syswrite, fcall3},
    {"mknod", (void (*)(void))sysmknod, fcall3},
    {"unlink", (void (*)(void))sysunlink, fcall1},
    {"link", (void (*)(void))syslink, fcall2},
    {"mkdir", (void (*)(void))sysmkdir, fcall1},
    {"close", (void (*)(void))sysclose, fcall1},
};

static void
call(ucontext_t *u, mcontext_t *a)
{
	mcontext_t *m = &u->uc_mcontext;

	uint ax = m->gregs[RAX];
	if (ax >= nelem(systab) || !systab[ax].name)
		errx(1, "unknown system call %d", ax);

	m->gregs[RAX] = systab[ax].stub(a, systab[ax].func);
	m->gregs[RIP] += 2;
	*((uintptr *)SYSSTACK) = m->gregs[RIP];
	jumpureg(m);
}

void
trap(int sig, siginfo_t *info, void *ctx)
{
	ucontext_t *u = ctx;
	mcontext_t *m = &u->uc_mcontext;

	greg_t *pc = &m->gregs[RIP];
	greg_t *sp = &m->gregs[RSP];
	u32 *usp = (u32 *)m->gregs[RSP];

	u8 *cd = (u8 *)(*pc);
	if (cd[0] != 0xcd || cd[1] != 0x40) {
		char err[] = "segmentation fault occured within the program\n";
		write(2, err, sizeof(err) - 1);
		abort();
	}

	ucontext_t *uc = (ucontext_t *)((SYSSTACK + SYSSIZE - sizeof(ucontext_t)) & ~(sizeof(uintp) - 1));
	*uc = *u;

	mcontext_t *mc = (mcontext_t *)(((uintptr)uc - sizeof(mcontext_t)) & ~(sizeof(uintp) - 1));

	*sp = ((uintptr)mc) - (sizeof(uintp) * 8);
	*pc = (greg_t)call;

#ifdef __x86_64__
	mc->gregs[0] = m->gregs[RDI];
	mc->gregs[1] = m->gregs[RSI];
	mc->gregs[2] = m->gregs[RDX];
	m->gregs[RDI] = (greg_t)uc;
	m->gregs[RSI] = (greg_t)mc;
#else
	mc->gregs[0] = usp[1];
	mc->gregs[1] = usp[2];
	mc->gregs[2] = usp[3];
	((u32 *)*sp)[1] = (greg_t)uc;
	((u32 *)*sp)[2] = (greg_t)mc;
#endif

	USED(usp);
	USED(sig);
	USED(info);
}
