#ifndef _RONIN_GFXHELPER_H
#define _RONIN_GFXHELPER_H been_here_before

#include "common.h"

struct pvr
{
  unsigned int polymode2; /* really magic */
  unsigned int texturemode; /* really length */
  unsigned int attr;
  unsigned short xsize, ysize;
  unsigned short data[1];
};

START_EXTERN_C
struct pvr *load_pvr( char *fn );
void paste_image( struct pvr *i, int x, int y, int flags);
void paste_pvr( struct pvr *i, int x, int y, int flags);
void paste_pvr_part( struct pvr *i, int x, int y,
                     int tx1, int ty1, int tx2, int ty2, int flags);
void paste_pvr_scale( struct pvr *i, int x, int y, int x2, int y2, int flags);
void paste_pvr_scale_a( struct pvr *i, int x, int y, int x2, int y2,
                        int flags, float alpha);
void commit_dummy_transpoly();
END_EXTERN_C

#endif /* _RONIN_GFXHELPER_H */
