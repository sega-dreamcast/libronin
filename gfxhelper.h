#ifndef _GFXHELPER_H
#define _GFXHELPER_H been_here_before

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
void commit_dummy_transpoly();
END_EXTERN_C

#endif /* _GFXHELPER_H */
