/* Shuttle space for code beeing moved from DreamSNES. Should be
 *  treated as one big FIXME. 
 */

#include "dc_time.h"
#include "video.h"
#include "ta.h"
#include "gtext.h"
#include "gfxhelper.h"
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
