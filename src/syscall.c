#include "u.h"
#include "libc.h"
#include "dat.h"
#include "fns.h"

typedef struct Fd Fd;

struct Fd {
	int fd;

	struct dirent **namelist;
	int npos, nlist;

	u8 buf[sizeof(Dirent)];
	int pos;
};

static Fd fds[NOFILE];
static int nfd;

static Fd *
addfd(int fd)
{
	if (nfd >= NOFILE)
		return NULL;
	Fd *f = &fds[nfd++];
	memset(f, 0, sizeof(*f));
	f->fd = fd;
	return f;
}

static Fd *
getfd(int fd)
{
	for (int i = 0; i < nfd; i++) {
		if (fds[i].fd == fd)
			return &fds[i];
	}
	return NULL;
}

static void
freefd(int fd)
{
	Fd *f = getfd(fd);
	if (!f)
		return;

	for (int i = 0; i < f->nlist; i++)
		free(f->namelist[i]);
	free(f->namelist);

	*f = fds[--nfd];
}

intp
sysfork(void)
{
	return fork();
}

intp
sysexit(void)
{
	exit(0);
}

intp
syswait(void)
{
	int wstatus;
	int pid = wait(&wstatus);
	return pid;
}

intp
syspipe(intp fd[2])
{
	int pipefd[2];
	int ret = pipe(pipefd);
	fd[0] = pipefd[0];
	fd[1] = pipefd[1];
	return ret;
}

intp
sysread(intp fd, void *buf, intp len)
{
	Fd *f = getfd(fd);
	if (!f)
		return read(fd, buf, len);

	if (f->namelist)
		goto readdir;

	errno = 0;
	intp n = read(fd, buf, len);
	if (n < 0 && errno == EISDIR)
		goto readdir;

	return n;

readdir:
	if (!f->namelist) {
		f->nlist = scandirat(fd, ".", &f->namelist, NULL, alphasort);
		if (f->nlist < 0)
			return -1;
	}

	for (n = 0; len > 0;) {
		struct dirent *di;

		if (f->pos == sizeof(f->buf) || n == 0) {
		retry:
			if (f->npos >= f->nlist)
				break;
			else
				di = f->namelist[f->npos++];

			if (strlen(di->d_name) >= DIRSIZ)
				goto retry;

			f->buf[0] = di->d_ino & 0xff;
			f->buf[1] = di->d_ino >> 8;
			memmove(f->buf + 2, di->d_name, DIRSIZ - 1);
			f->buf[DIRSIZ + 1] = '\0';
			f->pos = 0;
		}

		intp m = min(len, (intp)(sizeof(f->buf) - f->pos));
		memmove(buf, f->buf + f->pos, m);
		f->pos += m;
		n += m;
		len -= m;
	}
	return n;
}

intp
syskill(intp pid)
{
	return kill(pid, SIGKILL);
}

intp
sysexec(char *name, char *argv[])
{
	ulong sz = 0;
	for (int i = 0; argv[i]; i++) {
		if (i >= MAXARG)
			return -1;
		sz += strlen(argv[i]);
	}
	if (sz >= PGSIZE)
		return -1;

	return execvp(name, argv);
}

intp
sysfstat(intp fd, Stat *buf)
{
	struct stat st;

	int ret = fstat(fd, &st);
	if (S_ISREG(st.st_mode))
		buf->type = T_FILE;
	else if (S_ISDIR(st.st_mode))
		buf->type = T_DIR;
	else
		buf->type = T_DEV;

	buf->dev = st.st_dev;
	buf->ino = st.st_ino;
	buf->nlink = st.st_nlink;
	buf->size = st.st_size;
	return ret;
}

intp
syschdir(char *name)
{
	return chdir(name);
}

intp
sysdup(intp fd)
{
	int dfd = dup(fd);
	if (dfd < 0 || !addfd(fd))
		close(fd);
	return dfd;
}

intp
sysgetpid(void)
{
	return getpid();
}

intp
syssbrk(intp increment)
{
	return (intp)sbrk(increment);
}

intp
syssleep(intp sec)
{
	return sleep(sec);
}

intp
sysuptime(void)
{
	return clock();
}

intp
sysopen(char *name, intp flags)
{
	if (flags & O_CREATE)
		flags = (flags | O_CREAT) ^ O_CREATE;

	int fd;
	if (flags & O_CREAT)
		fd = creat(name, 0644);
	else
		fd = open(name, flags);

	if (fd < 0 || !addfd(fd))
		close(fd);
	return fd;
}

intp
syswrite(intp fd, void *buf, intp len)
{
	return write(fd, buf, len);
}

intp
sysmknod(char *name, intp mode, intp dev)
{
	return mknod(name, mode, dev);
}

intp
sysunlink(char *name)
{
	return unlink(name);
}

intp
syslink(char *old, char *new)
{
	return link(old, new);
}

intp
sysmkdir(char *name)
{
	return mkdir(name, 0755);
}

intp
sysclose(intp fd)
{
	freefd(fd);
	return close(fd);
}
