/* Shuttle space for code beeing moved from DreamSNES. Should be
 *  treated as one big FIXME. 
 */

#include "dc_time.h"
#include "video.h"
#include "ta.h"
#include "gtext.h"
#include "misc.h"
#include "translate.h"

void show_file_error(struct font *font)
{
  ta_begin_frame();
  ta_commit_end();
  commit_dummy_transpoly();
  
  draw_text(100, 100, 640, _("One of DreamSNES datafiles is corrupt."), font, C_RED);
  draw_text(100, 120, 640, _("Check CD for scratches."), font, C_RED);

  ta_commit_frame();
  usleep(10000000);
}

/*! @decl void commit_dummy_transpoly()
 *!
 *! FIXME: It's used in two files, so it should have some doc I guess...
 */
void commit_dummy_transpoly()
{
  struct polygon_list mypoly;

  mypoly.cmd =
    TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_TRANSPARENT|TA_CMD_POLYGON_SUBLIST|
    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  mypoly.mode2 =
    TA_POLYMODE2_BLEND_SRC_ALPHA|TA_POLYMODE2_BLEND_DST_INVALPHA|
    TA_POLYMODE2_FOG_DISABLED|TA_POLYMODE2_ENABLE_ALPHA;
  mypoly.texture = 0;
  mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0;
  ta_commit_list(&mypoly);
}
