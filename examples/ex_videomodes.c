#include "dc_time.h"
#include "video.h"
#include "serial.h"
#include "report.h"

//gtext
#include "gtext.h"
#include "cdfs.h"
#include "ta.h"
//end gtext

struct font *the_font;

void setup()
{
  serial_init(57600);
  usleep(1500000);

  dc_setup_ta();

  init_palette();
  init_twiddletab();

  report("Setting up GD drive ... ");
  cdfs_init();
  report("GD OK\r\n");
}

void showline(char *mode)
{
  reportf("%s\n", mode);
  ta_sync();
  ta_begin_frame();
  ta_commit_end();
  draw_text(140, 400, 640, mode, the_font, C_GREEN);
  draw_text(140, 420, 640, "Hello world!  --== G g a e E ==--", 
	    the_font, C_GREEN);
  ta_commit_frame();
  ta_sync();
}

void reinitiate(int res, int hz50, int pal, int voffset)
{
  //  dc_init_pvr();
  dc_clrscr(0xff);
  dc_init_video(dc_check_cable(), 0, 0, res, hz50, pal, voffset);
  dc_video_on();
}

int main (int argc, char **argv)
{
  char *pal50hz ="50Hz PAL";
  char *palm50hz ="50Hz PALM";
  //  char *paln50hz ="50Hz PALN";
  //  char *pal60hz ="60Hz PAL";
  //char *ntsc50hz="50Hz NTSC";
  //char *ntsc60hz="60Hz NTSC";
  int i;

  serial_init(57600);
  usleep(1500000);
  setup();
  the_font = load_font( "/GFX/DEFAULT.FNT" );

  showline("Uninitiated");
  usleep(2000000);


  reinitiate(2, 1, 1, 0);
  showline(pal50hz);
  usleep(4000000);

  report("Ending\n\n");
#if 0
  reinitiate(2, 1, 2, 0);
  showline(palm50hz);
  usleep(4000000);

  reinitiate(2, 1, 3, 0);
  showline(paln50hz);
  usleep(4000000);

  reinitiate(2, 0, 1, 0);
  showline(pal60hz);
  usleep(4000000);

  reinitiate(2, 0, 0, 0);
  showline(ntsc60hz);
  usleep(4000000);
  
  reinitiate(2, 1, 0, 0);
  showline(ntsc50hz);
  usleep(4000000);
  
  reinitiate(2, 0, 0, 0);
  showline(ntsc60hz);
  usleep(4000000);
#endif  

#if 0
  reinitiate(0, 0, 0, 0); //60Hz lowres 
  showline(hz60); 
  usleep(3000000); 
  
  reinitiate(0, 1, 0, 0); //50Hz lowres 
  showline(hz50); 
  usleep(3000000); 
#endif
  
  for(i=-18; i<255; i++) { 
    reinitiate(0, 1, 0, i); //50Hz lowres   
    usleep(50000); 
  } 

  //The slave doesn't reset registers properly, so go back into hires.
  report("Resetting screen to 60Hz NTSC and returning\n");
  reinitiate(2, 0, 0, 0); //60Hz hires

  return 0;
}
