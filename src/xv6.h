enum {
	O_CREATE = 0x200,
};

// file types
enum {
	T_DIR = 1,  // directory
	T_FILE = 2, // regular file
	T_DEV = 3,  // device
};

#define ELF_PROG_LOAD 1

#define PGSIZE 4096
#define PGROUNDUP(x) (((x) + PGSIZE) & ~(PGSIZE - 1))

typedef struct Stat Stat;
typedef struct Dirent Dirent;

struct Stat {
	short type;
	int dev;
	uint ino;
	short nlink;
	uint size;
};

#define DIRSIZ 14

struct Dirent {
	ushort inum;
	char name[DIRSIZ];
};

#define NOFILE 16 // open files per process
#define MAXARG 32 // max exec arguments
