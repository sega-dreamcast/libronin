#ifndef _CDFS_H
#define _CDFS_H been_here_before
/*
 * ISO 9660 fs 
 */
#include "common.h"

typedef struct {
  int     dd_fd;
  int     dd_loc;
  int     dd_size;
  char    *dd_buf;
} DIR;

#define MAX_OPEN_FILES 64

typedef struct dirent {
  int      d_size;
  char     d_name[256];
} dirent_t;

#define O_RDONLY 0
#define O_DIR    4

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

START_EXTERN_C
int open(const char *path, int oflag, ...);
int close(int fd);
int pread(int fd, void *buf, unsigned int nbyte, unsigned int offset);
int read(int fd, void *buf, unsigned int nbyte);
long int lseek(int fd, long int offset, int whence);
DIR *opendir(const char *dirname);
int closedir(DIR *dirp);
int file_size( int fd );
int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **res);
struct dirent *readdir(DIR *dirp);
int chdir(const char *path);
void cdfs_init(void);
END_EXTERN_C

#endif //_CDFS_H
