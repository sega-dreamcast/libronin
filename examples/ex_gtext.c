#include "video.h"
#include "dc_time.h"
#include "gtext.h"
#include "report.h"
#include "serial.h"
#include "cdfs.h"
#include "ta.h"

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

int main(int argc, char **argv)
{
  struct font *the_font;

  setup();
  /* There needs to be a font on the CD */
  the_font = load_font( "/GFX/DEFAULT.FNT" );

  ta_sync();
  ta_begin_frame();
  ta_commit_end();
  draw_text(40, 40, 640, "Hello World", the_font, C_RED);
  draw_text(40, 100, 640, "Gazonk", the_font, C_RED);
  ta_commit_frame();
  ta_sync();

  report("Sleeping while showing text...\n");
  usleep(3*1000000);

  ta_sync();
  ta_begin_frame();
  ta_commit_end();
  display_font(the_font);
  ta_commit_frame();
  ta_sync();

  report("Sleeping while showing font...\n");
  usleep(3*1000000);

  return 0;
}
