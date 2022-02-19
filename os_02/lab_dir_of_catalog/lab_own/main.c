#include <stdio.h>		/* for convenience */
#include <stdlib.h>		/* for convenience */
#include <stdarg.h>		/* for convenience */
#include <string.h>		/* for convenience */
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include "stack_t.h"
/* function type that is called for each filename */
typedef	int	Myfunc(const char *, const struct stat *, int);

static Myfunc	myfunc;
static void 	err_sys(const char *fmt, ...);
static void 	err_ret(const char *fmt, ...);
static void 	err_quit(const char *fmt, ...);
static void 	print_path(const char *data, const size_t level, const size_t inode);
static void 	print_name(node_t *node);
static int 		myftw_n(char *pathname);

static long	nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;

int main(int argc, char *argv[])
{
	int	ret;

	if (argc != 2)
		err_quit("usage:  ftw  <starting-pathname>");

	ret = myftw_n(argv[1]);		/* does it all */

	ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
	if (ntot == 0)
		ntot = 1;		/* avoid divide by 0; print 0 for all counts */
        
	printf("regular files  = %7ld, %5.2f %%\n", nreg,
	  nreg * 100.0 / ntot);
	printf("directories    = %7ld, %5.2f %%\n", ndir,
	  ndir * 100.0 / ntot);
	printf("block special  = %7ld, %5.2f %%\n", nblk,
	  nblk * 100.0 / ntot);
	printf("char special   = %7ld, %5.2f %%\n", nchr,
	  nchr * 100.0 / ntot);
	printf("FIFOs          = %7ld, %5.2f %%\n", nfifo,
	  nfifo * 100.0 / ntot);
	printf("symbolic links = %7ld, %5.2f %%\n", nslink,
	  nslink * 100.0 / ntot);
	printf("sockets        = %7ld, %5.2f %%\n", nsock,
	  nsock * 100.0 / ntot);
	exit(ret);
}


#define	FTW_F	1		/* file other than directory */
#define	FTW_D	2		/* directory */
#define	FTW_DNR	3		/* directory that can't be read */
#define	FTW_NS	4		/* file that we can't stat */

static char *alloc_str(const char *str, size_t size)
{
	char *temp = malloc(size * sizeof(char) + 1);
	if (temp) strcpy(temp, str);
	return temp;
}

char *join_path(const char *base, const char *const file)
{
    int len = strlen(base);
    char *path = malloc(len + strlen(file) + 2);

    if (!path)
        err_quit("malloc error");

    strcpy(path, base);
    path[len] = '/';
    strcpy(path + len + 1, file);

    return path;
}

static int proc_entry(stack_t *stk, const size_t level, const char *curr_path, const char *curr_file)
{
	struct stat statbuf;
	char *path = join_path(curr_path, curr_file);
	
	if (lstat(path, &statbuf) < 0) {
		myfunc(path, &statbuf, FTW_NS);
		fprintf(stderr, "lstat error");
		free(path);
		return EXIT_FAILURE;
	}

	if (S_ISDIR(statbuf.st_mode) == 0) {
		myfunc(path, &statbuf, FTW_F);
	} else {
		myfunc(path, &statbuf, FTW_D);
		push(stk, init_node(path, level, statbuf.st_ino));
		free(path);
		return EXIT_SUCCESS;
	}

	free(path);
	print_path(curr_file, level, statbuf.st_ino);
	return true;
}

static void proc_dir(stack_t *stk, const node_t *node, DIR *dp, struct dirent *dirp) {
	if ((dp = opendir(node->data)) != NULL) {
		while ((dirp = readdir(dp)) != NULL) {
			if (strcmp(dirp->d_name, ".") != 0 &&
		    	strcmp(dirp->d_name, "..") != 0)
				proc_entry(stk, node->level + 1, node->data, dirp->d_name);
		}
		if (closedir(dp) < 0)
			err_ret("closedir error");
	}
}

static int myftw_n(char *pathname)
{	
	struct dirent 	*dirp = NULL;
	DIR 			*dp = NULL;
	size_t			level = 0;
	char 			*full_path = alloc_str(pathname, 256);
	node_t			*node;
	stack_t			*stk = init_stack();

	struct stat statbuf;
	
	if (lstat(pathname, &statbuf) < 0) {
		myfunc(pathname, &statbuf, FTW_NS);
		fprintf(stderr, "lstat error");
		return EXIT_FAILURE;
	}
	push(stk, init_node(full_path, level, statbuf.st_ino));

	while (is_free(stk) == false)
	{
		node = pop(stk, true);
		char *file;

		if ((file = strrchr(node->data, '/'))) print_path(file + 1, node->level, node->inode);
		else print_name(node);

		proc_dir(stk, node, dp, dirp);
		free_node(node);

		if (dp != NULL && closedir(dp) < 0)
			err_ret("closedir error");
	}
	free(full_path);
	free_stack(stk);
	return true;
}

static void print_path(const char *data, const size_t level, const size_t inode)
{
	for (size_t i = 0; i < level; i++) printf("____");
	printf("[%zu]->%s \tst_ino->%zu\n", level, data, inode);
}

static void print_name(node_t *node)
{
	for (size_t i = 0; i < node->level; i++) printf("____");
	printf("[%zu]->%s\t %zu\n", node->level, node->data, node->inode);
}

static int myfunc(const char *pathname, const struct stat *statptr, int type)
{
	switch (type) {
	case FTW_F:
		switch (statptr->st_mode & S_IFMT) {
		case S_IFREG:	nreg++;		break;
		case S_IFBLK:	nblk++;		break;
		case S_IFCHR:	nchr++;		break;
		case S_IFIFO:	nfifo++;	break;
		case S_IFLNK:	nslink++;	break;
		case S_IFSOCK:	nsock++;	break;
		case S_IFDIR:	/* directories should have type = FTW_D */
			printf("for S_IFDIR for %s\n", pathname);
		}
		break;
	case FTW_D:
		ndir++;
		break;
	case FTW_DNR:
		err_ret("can't read directory %s", pathname);
		break;
	case FTW_NS:
		err_ret("stat error for %s", pathname);
		break;
	default:
		err_ret("unknown type %d for pathname %s", type, pathname);
	}
	return(0);
}

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
	char buf[4095];

	vsnprintf(buf, 4095, fmt, ap);
	if (errnoflag)
		snprintf(buf + strlen(buf), 40995 - strlen(buf) - 1, ": %s",
		  strerror(error));
	strcat(buf, "\n");
	fflush(stdout);		/* in case stdout and stderr are the same */
	fputs(buf, stderr);
	fflush(NULL);		/* flushes all stdio output streams */
}

void err_ret(const char *fmt, ...)
{
	va_list		ap;
	va_start(ap, fmt);

	err_doit(1, errno, fmt, ap);
	
	va_end(ap);
}

void err_sys(const char *fmt, ...)
{
	va_list		ap;
	va_start(ap, fmt);
	
	err_doit(1, errno, fmt, ap);
	
	va_end(ap);
	exit(1);
}

void err_quit(const char *fmt, ...)
{
	va_list		ap;
	va_start(ap, fmt);

	err_doit(0, 0, fmt, ap);
	
	va_end(ap);
	exit(1);
}