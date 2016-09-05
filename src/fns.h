void exec(int, char *[]);

void trap(int, siginfo_t *, void *);

intp sysfork(void);
intp sysexit(void);
intp syswait(void);
intp syspipe(intp[2]);
intp sysread(intp, void *, intp);
intp syskill(intp);
intp sysexec(char *, char *[]);
intp sysfstat(intp, Stat *);
intp syschdir(char *);
intp sysdup(intp);
intp sysgetpid(void);
intp syssbrk(intp);
intp syssleep(intp);
intp sysuptime(void);
intp sysopen(char *, intp);
intp syswrite(intp, void *, intp);
intp sysmknod(char *, intp, intp);
intp sysunlink(char *);
intp syslink(char *, char *);
intp sysmkdir(char *);
intp sysclose(intp);

void retuser(uintp, uintp, uintp);
void jumpureg(mcontext_t *);
