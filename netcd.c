#include <stdio.h>
#include <string.h>

#include "cdfs.h"
#include "gddrive.h"

START_EXTERN_C
#include "net/ether.h"
#include "net/udp.h"
END_EXTERN_C

#define MAXFD 64

static struct {
  int serial;
  int result;
  int cmd;
  union {
    char filename[64];
    struct {
      int fd;
      int pos;
      int len;
    } rd;
  } data;
} cmd;

static int current_serial=4711;

static int readpos[MAXFD+1];
static char replybuf[2048];
static int replylen;

static void virtcdhdlr(char *pkt, int size, void *ip_pkt)
{
  if(size >= 8) {
    struct { int serial, result; } res;
    memcpy(&res, pkt, 8);
    if(res.serial == cmd.serial) {
      cmd.result = res.result;
      if(size > 8)
	memcpy(replybuf, ((char *)pkt)+8, size-8);
      replylen = size-8;
    }
  }
}

static int docmd(int command, const void *data, int sz)
{
  static unsigned char bcast_hw[] = { ~0, ~0, ~0, ~0, ~0, ~0 };
  cmd.serial = ++current_serial;
  cmd.result = -1;
  cmd.cmd = command;
  if(sz)
    memcpy(&cmd.data, data, sz);
  for(;;) {
    int i;
    udp_send_packet(bcast_hw, ~0, 1449, 1451, &cmd, 12+sz);
    for(i=0; i<1000000; i++) {
      ether_check_events();
      if(cmd.result != -1)
	return cmd.result;
    }
  }
}

EXTERN_C int open(const char *path, int mode, ...)
{
  int res;
  /*  printf("open(%s,%d)\n", path, mode); */
  res = docmd(1, path, strlen(path));
  /* printf("res = %d\n", res); */
  if(res>=0 && res<=MAXFD)
    readpos[res]=0;
  else
    res = -1;
  return res;
}

EXTERN_C int close(int fd)
{
  int res;
  /* printf("close(%d)\n", fd); */
  res = docmd(3, &fd, 4);
  /* printf("res = %d\n", res); */
  if(res>=0 && fd>=0 && fd<=MAXFD)
    readpos[fd]=-1;
  return res;
}

EXTERN_C int read(int fd, void *buf, unsigned int len)
{
  struct { int fd, pos, len; } cmd;
  int res;
  /* printf("read(%d,%p,%d)\n", fd, buf, len); */
  if(fd<0 || fd>MAXFD)
    return -1;
  while(len > sizeof(replybuf)) {
    res = read(fd, buf, sizeof(replybuf));
    if(res <= 0)
      return res;
    buf = ((char *)buf)+res;
    len -= res;
  }
  cmd.fd = fd;
  cmd.pos = readpos[fd];
  cmd.len = len;
  res = docmd(2, &cmd, 12);
  /* printf("res = %d\n", res); */
  if(res > 0) {
    readpos[fd] += res;
    memcpy(buf, replybuf, res);
  }
  return res;
}

EXTERN_C long lseek(int fd, long pos, int whence)
{
  /* printf("lseek(%d,%d,%d)\n", fd, pos, whence); */
  if(fd<0 || fd>MAXFD || readpos[fd]<0)
    return -1;
  switch(whence) {
  case SEEK_SET:
    readpos[fd] = pos;
    break;
  case SEEK_CUR:
    readpos[fd] += pos;
    break;
    /*
  case SEEK_END:
    readpos[fd] = maxpos[fd] + pos;
    break;
    */
  default:
    return -1;
  }
  /* printf("res = %d\n", readpos[fd]); */
  return readpos[fd];
}

static DIR g_dir;

EXTERN_C DIR *opendir(const char *path)
{
  int res;
  /* printf("opendir(%s)\n", path); */
  res = docmd(4, path, strlen(path));
  /* printf("res = %d\n", res); */
  if(res >= 0) {
    g_dir.dd_fd = res;
    g_dir.dd_loc = 0;
    return &g_dir;
  } else
    return 0;
}

EXTERN_C int closedir(DIR *dirp)
{
  int res;
  /* printf("closedir(%p)\n", dirp); */
  res = docmd(5, &dirp->dd_fd, sizeof(dirp->dd_fd));
  /* printf("res = %d\n", res); */
  return res;
}

static struct dirent g_entry;

EXTERN_C struct dirent *readdir(DIR *dirp)
{
  struct { int fd, pos; } cmd;
  int res;
  /*  printf("readdir(%p)\n", dirp); */
  cmd.fd = dirp->dd_fd;
  cmd.pos = dirp->dd_loc;
  res = docmd(6, &cmd, 8);
  /* printf("res = %d\n", res); */
  if(res >= 0) {
    dirp->dd_loc++;
    memcpy(&g_entry, replybuf, replylen);
    ((char *)&g_entry)[replylen] = '\0';
    return &g_entry;
  } else
    return 0;
}

EXTERN_C int chdir(const char *path)
{
  int res;
  printf("chdir(%s)\n", path);
  res = docmd(7, path, strlen(path));
  printf("res = %d\n", res);
  return res;
}

EXTERN_C void cdfs_init()
{
  register unsigned long p, x;
  udp_set_handler(virtcdhdlr);
  *((volatile unsigned long *)0xa05f74e4) = 0x1fffff;
  for(p=0; p<0x200000/4; p++)
    x = ((volatile unsigned long *)0xa0000000)[p];
  gdGdcInitSystem();  
}

EXTERN_C void cdfs_reinit()
{
}
