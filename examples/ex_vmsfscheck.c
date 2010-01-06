#include <sys/types.h>
#include <sys/time.h>
#include <string.h>

#include "report.h"
#include "serial.h"
#include "vmsfs.h"
#include "maple.h"
#include "dc_time.h"
#include "notlibc.h"

//gtext
#include "video.h"
#include "gtext.h"
//#include "cdfs.h"
#include "ta.h"

#ifdef VMUCOMPRESS
#include "zlib.h"
#endif

#define SRAMSIZE 8192

static unsigned char icondata[] = {
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
 0x11, 0x00, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
 0x00, 0x00, 0x00, 0x01, 0x23, 0x33, 0x33, 0x33, 0x32, 0x01, 0x00, 
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x33, 0x33, 
 0x33, 0x33, 0x33, 0x32, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
 0x00, 0x12, 0x44, 0x44, 0x44, 0x44, 0x45, 0x33, 0x33, 0x32, 0x10, 
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x44, 0x55, 0x55, 0x44, 
 0x44, 0x45, 0x33, 0x32, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 
 0x55, 0x33, 0x33, 0x33, 0x33, 0x35, 0x44, 0x45, 0x21, 0x10, 0x00, 
 0x00, 0x00, 0x00, 0x00, 0x05, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 
 0x55, 0x44, 0x01, 0x22, 0x10, 0x00, 0x00, 0x00, 0x00, 0x53, 0x33, 
 0x33, 0x33, 0x33, 0x33, 0x32, 0x00, 0x25, 0x52, 0x33, 0x01, 0x00, 
 0x00, 0x00, 0x12, 0x33, 0x35, 0x33, 0x33, 0x22, 0x20, 0x11, 0x11, 
 0x00, 0x44, 0x33, 0x21, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x11, 
 0x11, 0x00, 0x11, 0x11, 0x22, 0x35, 0x54, 0x33, 0x32, 0x00, 0x00, 
 0x00, 0x20, 0x00, 0x22, 0x00, 0x00, 0x22, 0x00, 0x22, 0x33, 0x33, 
 0x35, 0x53, 0x32, 0x00, 0x00, 0x00, 0x33, 0x33, 0x53, 0x33, 0x33, 
 0x65, 0x33, 0x33, 0x33, 0x33, 0x33, 0x43, 0x33, 0x00, 0x00, 0x00, 
 0x35, 0x33, 0x53, 0x33, 0x33, 0x66, 0x33, 0x33, 0x33, 0x33, 0x35, 
 0x43, 0x33, 0x00, 0x00, 0x00, 0x36, 0x33, 0x63, 0x35, 0x55, 0x67, 
 0x33, 0x35, 0x66, 0x87, 0x97, 0x56, 0x33, 0x21, 0x00, 0x00, 0x01, 
 0x00, 0x00, 0x66, 0x55, 0x27, 0x63, 0x22, 0x88, 0x87, 0x88, 0x77, 
 0x33, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x88, 0x81, 0x99, 
 0x91, 0x8a, 0x87, 0x77, 0x79, 0x33, 0x32, 0x00, 0x00, 0x00, 0x00, 
 0x00, 0x02, 0x78, 0x81, 0x11, 0x11, 0x77, 0x77, 0x99, 0x79, 0x33, 
 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x01, 0x11, 0x91, 0x11, 
 0x11, 0x11, 0x99, 0x76, 0x33, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 
 0x02, 0x21, 0x11, 0x99, 0x11, 0x11, 0x19, 0x97, 0x33, 0x33, 0x33, 
 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x32, 0x11, 0x11, 0x11, 0x11, 
 0x99, 0x65, 0x33, 0x33, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 
 0x33, 0x11, 0x11, 0x11, 0x11, 0x99, 0x65, 0x33, 0x33, 0x32, 0x00, 
 0x00, 0x00, 0x00, 0x00, 0x03, 0x33, 0x33, 0x11, 0x11, 0x11, 0x99, 
 0x73, 0x33, 0x33, 0x32, 0x10, 0x00, 0x00, 0x00, 0x00, 0x03, 0x53, 
 0x33, 0x20, 0x11, 0x17, 0x99, 0x73, 0x33, 0x53, 0x33, 0x11, 0x00, 
 0x00, 0x00, 0x00, 0x02, 0x35, 0x33, 0x33, 0x28, 0x89, 0x99, 0x76, 
 0x33, 0x53, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x33, 
 0x33, 0x27, 0x99, 0x99, 0x02, 0x33, 0x55, 0x33, 0x22, 0x00, 0x00, 
 0x00, 0x00, 0x10, 0x25, 0x33, 0x33, 0x09, 0x10, 0x11, 0x11, 0x65, 
 0x55, 0x33, 0x53, 0x00, 0x00, 0x00, 0x00, 0x10, 0x23, 0x33, 0x55, 
 0x19, 0x11, 0x11, 0x11, 0xaa, 0x55, 0x33, 0x53, 0x00, 0x00, 0x00, 
 0x00, 0x00, 0x35, 0x54, 0x45, 0x99, 0x91, 0x11, 0x12, 0xbb, 0xba, 
 0x45, 0x35, 0x00, 0x00, 0x00, 0x00, 0x02, 0x54, 0x44, 0x46, 0x99, 
 0x99, 0x11, 0x08, 0xbb, 0xbb, 0xaa, 0x55, 0x00, 0x00, 0x00, 0x04, 
 0x44, 0x44, 0x44, 0x48, 0x99, 0x99, 0x11, 0x44, 0xab, 0xba, 0x44, 
 0x44, 0x00, 0x00, 0x00, 0x64, 0x44, 0x44, 0x44, 0x89, 0x99, 0x99, 
 0x90, 0x44, 0x4a, 0x44, 0x44, 0x44,
};
static unsigned short palette[] = { /* ARGB4 */
 0xfacc, 0xfddc, 0xf8ac, 0xf68c, 0xf02c, 0xf46c, 0xf688, 0xfa88, 
 0xf864, 0xfea8, 0xf224, 0xf004, 0xffff, 0xffff, 0xffff, 0xffff,
};

#include "encoded_font.h"

static int sram_save_unit = -1;
static struct vmsinfo info;
static struct superblock super;
static struct vms_file file;
static struct vms_file_header header;
static struct timestamp tstamp;
static struct tm tm;

unsigned char *SRAM;
unsigned char *RESTOREDRAM;

#define NUMLINES 20
#define LINEHEIGHT 22
static char *lines[NUMLINES];

struct font *the_font;

void gwrite(unsigned char *message, unsigned int color)
{
  int line;

  ta_sync();
  ta_begin_frame();
  ta_commit_end();
  for(line=0; line<NUMLINES; line++)
  {
    if(line==(NUMLINES-1))
      lines[line] = message;
    else
      lines[line] = lines[line+1];

    draw_text(40, line*LINEHEIGHT, 640, lines[line], the_font, color);
  }
  ta_commit_frame();
  ta_sync();
}

void init_lines()
{
  int line;
  for(line=0; line<NUMLINES; line++)
    lines[line] = "";
}

int status_vmu, status_vmu_time;
char *status_vmu_message;
int write_sram( unsigned char *data, int size )
{
  time_t t;
  int i;

  if(sram_save_unit<0)
    for(i=0; i<24; i++) {
      reportf("%d:", i);
      if(vmsfs_check_unit(i, 0, &info)) {
        //FIXME: Should check for enough space.
        sram_save_unit = i;
        reportf("Save unit is %d\n", i);
        break;
      }
    }

  if(sram_save_unit<0) {
    gwrite("1-", C_YELLOW);
    report("1-");
  } else
    reportf("sram_save_unit: %d\n", sram_save_unit);

  if(!vmsfs_check_unit(sram_save_unit, 0, &info)) {
    gwrite("2-", C_YELLOW);
    report("2-");
  }

  if(!vmsfs_get_superblock(&info, &super)) {
    gwrite("3 ", C_YELLOW);
    report("3 ");
  }

  vmsfs_errno = 0;
  if(sram_save_unit < 0 || !vmsfs_check_unit(sram_save_unit, 0, &info) ||
     !vmsfs_get_superblock(&info, &super))
  {
    if( sram_save_unit < 0 )
      status_vmu_message = "No memory unit found.";
    else
      status_vmu_message = vmsfs_describe_error();
    gwrite(status_vmu_message, C_RED);
    reportf("%s\n", status_vmu_message);
    return 0;
  }

  memset(&header, 0, sizeof(header));
  strncpy(header.shortdesc, "libronin test", 16);
  strncpy(header.longdesc, "Delete this file", 32);
  strncpy(header.id, "vmsfscheck", 16);
  header.numicons = 1;
  memcpy(header.palette, palette, sizeof(header.palette));
  time(&t);
  tm = *localtime(&t);
  tstamp.year = tm.tm_year+1900;
  tstamp.month = tm.tm_mon+1;
  tstamp.day = tm.tm_mday;
  tstamp.hour = tm.tm_hour;
  tstamp.minute = tm.tm_min;
  tstamp.second = tm.tm_sec;
  tstamp.wkday = (tm.tm_wday+6)%7;
  vmsfs_beep(&info, 1);

  usleep(50000);  //Might not be needed, but I'm covering my tracks in 
                  //lack of real review and testing right now.

#ifdef VMUCOMPRESS
  /* destination buffer must be at least 0.1% larger than
     sourceLen plus 12 bytes.  (0x20000*1.01+13) */
  Byte *compr = (Byte *)malloc(0x25000);
  uLong comprlen = 10000*sizeof(int); /* don't overflow on MSDOS */
  int err;
  
  err = compress2(compr, &comprlen, (const Bytef*)data, size, 1);
  
  if(err == Z_OK) {
    size = (int)comprlen;
    data = (unsigned char *)compr;
    reportf("compress successful, new size: %d\n", size);
  } else
    reportf("compress error: %d  Saving uncompressed\n", err);
#endif

  if(!vmsfs_create_file(&super, "vmsfschk.tmp", &header, icondata, 
                        NULL, data, size, &tstamp))
  {
    if(vmsfs_errno == 0)
      status_vmu_message = "Failed to save due to unknown error!";
    else
      status_vmu_message = vmsfs_describe_error();
    reportf("%s\n", status_vmu_message);
    gwrite(status_vmu_message, C_RED);
    vmsfs_beep(&info, 0);
    return 0;
  }
  vmsfs_beep(&info, 0);

#ifdef VMUCOMPRESS
  free(compr);
#endif

  return 1;
}

int restore_sram( )
{
  int i;
  unsigned int cards = 0;
#ifdef VMUCOMPRESS
  int err;
  uLong uncomprlen;
  char *comprtmp;
#endif

  memset( RESTOREDRAM, 0xaa, SRAMSIZE );

  sram_save_unit = -1;

  for(i=0; i<24; i++)
  {
    if(vmsfs_check_unit(i, 0, &info) && vmsfs_get_superblock(&info, &super)) {
      cards |= 1<<i;
      sram_save_unit = i;
      reportf("Save unit is %d\n", i);
      if(!vmsfs_open_file(&super, "vmsfschk.tmp", &file)) {
        //Don display this message in a real application.
        status_vmu_message = "No vmsfschk.tmp found on this unit.";
        reportf("%\n", status_vmu_message);
        gwrite(status_vmu_message, C_RED);
        continue;
      }
      if(strncmp(file.header.id, "vmsfscheck", 16)) {
        status_vmu_message = "Incorrect id.";
        reportf("%\n", status_vmu_message);
        gwrite(status_vmu_message, C_RED);
        continue;
      }
      if(strncmp(file.header.longdesc, "Delete this file", 32)) {
        status_vmu_message = "Incorrect id.";
        reportf("%\n", status_vmu_message);
        gwrite(status_vmu_message, C_RED);
        continue;
      }
      if(file.size > 0x10000) {
        status_vmu_message = "File to big.";
        reportf("%\n", status_vmu_message);
        gwrite(status_vmu_message, C_RED);
        continue;
      }        

      reportf("File size is %d\n", file.size);

      if(vmsfs_read_file(&file, RESTOREDRAM, file.size)) {
        sram_save_unit = i;
        reportf("Save unit is %d\n\n", i);
        gwrite("File restored.", C_GREEN);

#ifdef VMUCOMPRESS
	comprtmp = malloc(0x30000);
	err = uncompress(comprtmp, &uncomprlen, SRAM, file.size);

	if(err == Z_OK) {
	  memcpy(RESTOREDRAM, comprtmp, uncomprlen);
	  reportf("uncompress successful to %d bytes\n", (int)uncomprlen);
	} else
	  reportf("uncompress error: %d, loading as uncompressed\n", err);
#endif
	
	return 1;
      }
    }
  }
  return 0;
}

int main(int argc, char **argv)
{
  int i,x;

#ifndef NOSERIAL
  serial_init(57600);
#endif
  //gtext
  dc_setup_ta();

  init_palette();
  init_twiddletab();
  //end gtext
  init_lines();


  /* Normally you would load a font of CD, like this:
     report("Setting up GD drive ... ");
     cdfs_init();
     report("GD OK\r\n");
     gwrite("GD OK", C_GREEN);
     the_font = load_font( "/GFX/DEFAULT.FNT" );

     But when sending small test applications around a font encoded
     into the binary might be good: */

  the_font = load_memfont( cybersans );
  report("Font loaded, test starting.\n");
  gwrite("Font loaded, test starting ...", C_GREEN);

  maple_init();
  report("Maple BUS initiated.\n");
  gwrite("Maple BUS initiated.", C_GREEN);

  SRAM = malloc(SRAMSIZE);
  RESTOREDRAM = malloc(SRAMSIZE);
  i=0xF1;
  reportf("\nSaving %dB %d's ...\n", SRAMSIZE, i);
  gwrite("Saving.", C_YELLOW);
  memset(SRAM, i, SRAMSIZE);
  for(x=0; x<SRAMSIZE; x++) {
    if(SRAM[x] != i) {
      reportf("\nVerification of original failed at count %d!\n", x);
      gwrite("Verification of original failed.", C_RED);
      break;
    }
  }
  if(write_sram( SRAM, SRAMSIZE ))
  {
    report("Saved. Restoring ...\n");
    gwrite("Saved. Restoring ...", C_YELLOW);
    if(restore_sram())
    {
      report("\nRestored. Verifying ...\n");
      gwrite("Restored. Verifying ...", C_YELLOW);
      for(x=0; x<SRAMSIZE; x++) {
        if(RESTOREDRAM[x] != i) {
          reportf("\nVerification failed at count %d!\n", x);
          gwrite("Verification failed.", C_RED);
	  gwrite(itoa(x), C_RED);
          break;
        } else if(x==(SRAMSIZE-1)) {
          report("\nVerification successfull. A-OK!\n");
          gwrite("Verification successfull. A-OK!", C_GREEN);
        }
      }
    }
    free(SRAM);
    free(RESTOREDRAM);
  }

  usleep(15000*60*10); //Show the result for a while.
  return 0;
}
