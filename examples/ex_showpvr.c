#include "video.h"
#include "dc_time.h"
#include "gfxhelper.h"
#include "ta.h"
#include "cdfs.h"
#include "serial.h"
#include "gtext.h"

int main(int argc, char **argv)
{
  struct pvr *nonno;

  serial_init(57600);
  dc_setup_ta();

  cdfs_init();

  nonno = load_pvr("/GFX/NONNO.PVR");

  ta_begin_frame();  
  ta_commit_end();
  paste_pvr(nonno, 50, 50, 0);
  ta_commit_frame();
  ta_sync();

  return 0;
}
