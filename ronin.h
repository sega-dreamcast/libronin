#ifndef _RONIN_H
#define _RONIN_H been_here_before

#if defined(__cplusplus) || defined(c_plusplus)
# define EXTERN_C extern "C"
# define START_EXTERN_C extern "C" {
# define END_EXTERN_C }
#else
# define EXTERN_C extern
# define START_EXTERN_C
# define END_EXTERN_C
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

/* FIXME: Move them to a separe file. */

/*
 * Funktions for managing the DC serial port. Should not normally be
 * used in the final product, but only for debugging.
 */
START_EXTERN_C
void serial_init(int baudrate);
void serial_puts(const char *message);
void serial_putc(int c);
int  serial_getc();
int  serial_getc_blocking();
void serial_flush();
END_EXTERN_C

/*
 * Functions used for debug output through the serial
 * interface. Dependent on the serial functions.
 */
START_EXTERN_C
void report(const char *str);
void reportf(const char *fmt, ...);
char *itoa(int x);
END_EXTERN_C

/*
 * Maple
 */
#include "maple.h"

//FIXME: Move this to somewhere else!
static __inline int getimask(void)
{
  register unsigned int sr;
  __asm__("stc sr,%0" : "=r" (sr));
  return (sr >> 4) & 0x0f;
}

static __inline void setimask(int m)
{
  register unsigned int sr;
  __asm__("stc sr,%0" : "=r" (sr));
  sr = (sr & ~0xf0) | (m << 4);
  __asm__("ldc %0,sr" : : "r" (sr));
}


/*
 * Functions used to make life easier when there is no full featured
 * clib available.
 */
#include <sys/time.h>
START_EXTERN_C
void exit(int rcode) __THROW __attribute__ ((__noreturn__));
int __offtime(const long int *t, long int offset, struct tm *tp);
END_EXTERN_C

/*
 * Time and timer-related functions.
 */
#define USEC_TO_TIMER(x) (((x)*100)>>11)
START_EXTERN_C
unsigned long Timer( );
void usleep( unsigned int usecs );
END_EXTERN_C

/* 
 * Functions used to setup and operate on the framebuffer and tile
 * accellerator.
 */
START_EXTERN_C
void dc_init_pvr();
void dc_init_video(int cabletype, int mode, int tvmode, int res,
                   int hz50, int pal, int voffset);
void dc_video_on();
int  dc_check_cable();
void dc_waitvbl();
void dc_setup_ta();
void dc_reset_screen( int hires, int lace );

void dc_draw_string(int x, int y, const char *message, int color);
void dc_draw_char12(int x, int y, int c, int color);
void dc_clrscr(int color);
void dc_multibuffer_set(int show, int draw);
END_EXTERN_C

#define C_RED   0xff0000
#define C_GREEN 0x00ff00
#define C_BLUE  0x0000ff
#define C_MAGENTA (C_RED | C_BLUE)
#define C_YELLOW (C_RED | C_GREEN)
#define C_ORANGE (C_RED | 0x7f00)
#define C_WHITE 0xffffff
#define C_BLACK 0

#define C_DARK_ORANGE 0x7f3f00

#define C_GREY   0x7f7f7f

extern unsigned short *dc_current_draw_buffer;

#define VREG_BASE 0xa05f8000

// FIXME: Replace this with the fb_devconfig or something
struct _dispvar{
  int scnbot, center, overlay;
  unsigned int mode2;
};
extern struct _dispvar dispvar;
// END FIXME

#define vaddr_t void*
struct _fb_devconfig{
  vaddr_t dc_vaddr;		/* framebuffer virtual address */
  vaddr_t dc_paddr;		/* framebuffer physical address */
  int	dc_wid;			/* width of frame buffer */
  int	dc_ht;			/* height of frame buffer */
  int	dc_depth;		/* depth, bits per pixel */
  int	dc_rowbytes;		/* bytes in a FB scan line */
  vaddr_t dc_videobase;		/* base of flat frame buffer */
  int	dc_blanked;		/* currently has video disabled */
  int	dc_dispflags;		/* display flags */
  int	dc_tvsystem;		/* TV broadcast system */
  int	dc_voffset;		/* Vertical display offset */
  int   dc_fullscreen;          /* Render content streched to full screen */
  int   dc_hz50;                /* Refresh rate */

  //	struct rasops_info rinfo;
};
extern struct _fb_devconfig fb_devconfig;

/*
 * ISO 9660 fs 
 */
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
#define O_DIR    1

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

START_EXTERN_C
int open(const char *path, int oflag);
int close(int fd);
int pread(int fd, void *buf, unsigned int nbyte, unsigned int offset);
int read(int fd, void *buf, unsigned int nbyte);
long int lseek(int fd, long int offset, int whence);
DIR *opendir(const char *dirname);
int closedir(DIR *dirp);
int file_size( int fd );
int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **res);
struct dirent *readdir(DIR *dirp);
void cdfs_init(void);
END_EXTERN_C

/*
 * VMS fs
 */
struct timestamp {
  int year;   /* 0000-9999 */
  int month;  /* 1-12      */
  int day;    /* 1-31      */
  int hour;   /* 0-23      */
  int minute; /* 0-59      */
  int second; /* 0-59      */
  int wkday;  /* 0-6       */
};

struct vmsinfo {
  int port;
  int dev;
  int pt;
  unsigned long func;

  int partitions;
  int blocksz;
  int writecnt;
  int readcnt;
  int removable;

  int root_loc;
  int fat_loc;
  int fat_size;
  int dir_loc;
  int dir_size;
  int icon_shape;
  int num_blocks;
};

struct superblock {
  unsigned char root[8192];
  unsigned char fat[8192];
  int root_modified;
  int fat_modified;
  struct vmsinfo *info;
};

struct dir_iterator {
  unsigned char blk[8192];
  unsigned int this_blk, next_blk;
  int dcnt, blks_left;
  struct superblock *super;
};

struct dir_entry {
  unsigned char entry[0x20];
  unsigned int dblk;
  int dpos;
  struct dir_iterator *dir;
};

struct vms_file_header {
  char shortdesc[16];
  char longdesc[32];
  char id[16];
  unsigned short numicons;
  unsigned short animspeed;
  unsigned short eyecatchtype;
  unsigned short crc;
  unsigned long filesize;
  unsigned long reserved[5];
  unsigned short palette[16];
};

struct vms_file {
  int loc0, blks;
  int loc, offs, left;
  unsigned int size;
  struct superblock *super;
  struct vms_file_header header;
  unsigned char blk[8192];
};

START_EXTERN_C
void vmsfs_timestamp_to_bcd(unsigned char *bcd, struct timestamp *tstamp);
void vmsfs_timestamp_from_bcd(struct timestamp *tstamp, unsigned char *bcd);
int vmsfs_check_unit(int unit, int part, struct vmsinfo *info);
int vmsfs_beep(struct vmsinfo *info, int on);
int vmsfs_read_block(struct vmsinfo *info, unsigned int blk, unsigned char *ptr);
int vmsfs_write_block(struct vmsinfo *info, unsigned int blk, unsigned char *ptr);
int vmsfs_get_superblock(struct vmsinfo *info, struct superblock *s);
int vmsfs_sync_superblock(struct superblock *s);
unsigned int vmsfs_get_fat(struct superblock *s, unsigned int n);
void vmsfs_set_fat(struct superblock *s, unsigned int n, unsigned int l);
int vmsfs_count_free(struct superblock *s);
int vmsfs_find_free_block(struct superblock *s);
void vmsfs_open_dir(struct superblock *s, struct dir_iterator *i);
int vmsfs_next_dir_entry(struct dir_iterator *i, struct dir_entry *d);
int vmsfs_next_named_dir_entry(struct dir_iterator *i, struct dir_entry *d, char *name);
int vmsfs_next_empty_dir_entry(struct dir_iterator *i, struct dir_entry *d);
int vmsfs_write_dir_entry(struct dir_entry *d);
int vmsfs_open_file(struct superblock *super, char *name,
                    struct vms_file *file);
int vmsfs_read_file(struct vms_file *file, unsigned char *buf,
                    unsigned int cnt);
int vmsfs_create_file(struct superblock *super, char *name,
                      struct vms_file_header *header,
                      void *icons, void *eyecatch,
                      void *data, unsigned long datasize,
                      struct timestamp *tstamp);
END_EXTERN_C


/*
 * Sound
 */
#define DC_SOUND_BS 9216

struct buffer
{
  int read_pos;
  int write_pos;
  int full;

  short data[ DC_SOUND_BS ]; // enough, I hope.
};
//FIXME: This raises the namespace pollution to new levels.
extern struct buffer buff;

#define SOUNDSTATUS ((volatile struct soundstatus *)(void *)(0xa0800000+SOUNDSTATUS_ADDR))

extern int sound_device_open;
extern int fillpos;

START_EXTERN_C
void init_arm();
void write_samples( const short *samples, int nbytes );
void do_sound_command(int cmd);
int read_sound_int(volatile int *p);
void stop_sound();
void start_sound();
END_EXTERN_C



#endif /* _RONIN_H */
