#include "ta.h"
#include "cdfs.h"
#include "report.h"
#include "gfxhelper.h"
#include "matrix.h"

/* FIXME: There is a little bit to much redundancy going on between
   the diffrent texture pasting functions. Macros might be good. */

static struct pvr null_image = { 0,0,0,0,0, { 0 } };

/*! @decl pvr *load_pvr( char *filename )
 *!
 *! Loads a PVR and returns a pointer to the resulting texture.
 *!
 *! @seealso
 *!   @[paste_pvr()], @[paste_pvr_part()]
 */
struct pvr *load_pvr( char *fn )
{
  int usz, vsz;
  int size;
  struct pvr *p;
  
  int fd = open( fn, O_RDONLY );
  if( fd < 0 )
  {
    reportf( "Failed to open image  >%s< errno=%d\r\n", fn, fd );
    return &null_image;
  }

  size = file_size( fd );
  p = (struct pvr *)ta_txalloc( size );
  pread( fd, (void *)p, file_size( fd ), 0 );
  close( fd );

  for(usz=0; usz<7; usz++)
    if(p->xsize <= (8<<usz))
      break;
  for(vsz=0; vsz<7; vsz++)
    if(p->ysize <= (8<<vsz))
      break;

  p->polymode2 =
    (usz<<3)|(vsz<<0);
  p->texturemode =
    TA_TEXTUREMODE_ADDRESS(p->data);

  if(p->xsize == 640)
    p->texturemode |= TA_TEXTUREMODE_STRIDE;

  switch(p->attr & 0xff) {
   case 0x00: p->texturemode |= TA_TEXTUREMODE_ARGB1555; break;
   case 0x01: p->texturemode |= TA_TEXTUREMODE_RGB565; break;
   case 0x02: p->texturemode |= TA_TEXTUREMODE_ARGB4444; break;
   case 0x03: p->texturemode |= TA_TEXTUREMODE_YUV422; break;
   case 0x04: p->texturemode |= TA_TEXTUREMODE_BUMPMAP; break;
   case 0x05: p->texturemode |= TA_TEXTUREMODE_ARGB1555;
     p->polymode2 |= TA_POLYMODE2_DISABLE_TEXTURE_TRANSPARENCY; break;
   /* case 0x06: p->texturemode |= TA_TEXTUREMODE_ARGB8888; break; */
  }

  switch(p->attr & 0xff00) {
   case 0x100: p->texturemode |=
		 TA_TEXTUREMODE_TWIDDLED; break;
   case 0x200: p->texturemode |=
		 TA_TEXTUREMODE_TWIDDLED|TA_TEXTUREMODE_MIPMAP; break;
   case 0x300: p->texturemode |=
		 TA_TEXTUREMODE_VQ_COMPRESSION; break;
   case 0x400: p->texturemode |=
		 TA_TEXTUREMODE_VQ_COMPRESSION|TA_TEXTUREMODE_MIPMAP; break;
   case 0x500: p->texturemode |=
		 TA_TEXTUREMODE_CLUT4; break;
   case 0x600: p->texturemode |=
		 TA_TEXTUREMODE_CLUT4|TA_TEXTUREMODE_MIPMAP; break;
   case 0x700: p->texturemode |=
		 TA_TEXTUREMODE_CLUT8; break;
   case 0x800: p->texturemode |=
		 TA_TEXTUREMODE_CLUT8|TA_TEXTUREMODE_MIPMAP; break;
   case 0x900: p->texturemode |=
		 TA_TEXTUREMODE_NON_TWIDDLED; break;
   case 0xb00: p->texturemode |=
		 TA_TEXTUREMODE_NON_TWIDDLED|TA_TEXTUREMODE_STRIDE; break;
   case 0xd00: p->texturemode |=
		 TA_TEXTUREMODE_TWIDDLED; break;
  }

  return p;
}

/*! @decl void paste_pvr( struct pvr *, int x, int y, int flags)
 *!
 *! Paste texture to display by inserting the apropriate
 *! instructions into the tile accellerator list currently being built.
 *!
 *! @seealso
 *!  @[paste_pvr_part()], @[paste_image()], @[ta_begin_frame()], 
 *!  @[ta_commit_list()], @[ta_commit_end()], @[ta_commit_frame()]
 */
void paste_pvr( struct pvr *i, int x, int y, int flags)
{
  struct polygon_list mypoly;
  struct packed_colour_vertex_list myvertex;
  float w = (float)i->xsize, h = (float)i->ysize;
  float xf = (float)x, yf = (float)y;

  mypoly.cmd =
    TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_TRANSPARENT|TA_CMD_POLYGON_SUBLIST|
    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_TEXTURED|TA_CMD_POLYGON_PACKED_COLOUR;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  /*
     0x80  -  src * alpha
     0x40  -  min(src, dst)
     0x20  -  src
     0x10  -  dst * alpha
     0x08  -  min(src, dst)
     0x04  -  dst

     0x02  -  party
     0x01  -  ?
  */
  mypoly.mode2 = TA_POLYMODE2_BLEND_SRC_ALPHA|TA_POLYMODE2_BLEND_DST_INVALPHA|
    TA_POLYMODE2_FOG_DISABLED|
    TA_POLYMODE2_TEXTURE_CLAMP_U|TA_POLYMODE2_TEXTURE_CLAMP_V|
    TA_POLYMODE2_BILINEAR_FILTER|TA_POLYMODE2_MIPMAP_D_1_00|
    TA_POLYMODE2_TEXTURE_REPLACE| i->polymode2;
#if 1
  if( flags & 1 )
  {
    mypoly.cmd &= ~TA_CMD_POLYGON_TYPE_TRANSPARENT;
    mypoly.mode2 =
      TA_POLYMODE2_BLEND_SRC|TA_POLYMODE2_FOG_DISABLED|
      TA_POLYMODE2_TEXTURE_CLAMP_U|TA_POLYMODE2_TEXTURE_CLAMP_V|
      /*TA_POLYMODE2_BILINEAR_FILTER|*/TA_POLYMODE2_MIPMAP_D_1_00|
      TA_POLYMODE2_TEXTURE_REPLACE | i->polymode2;
  }
#endif
  mypoly.texture = i->texturemode;

  mypoly.alpha = mypoly.red = mypoly.green = mypoly.blue = 1.0;

  ta_commit_list(&mypoly);

  myvertex.cmd = TA_CMD_VERTEX;
  myvertex.colour = 0;
  myvertex.ocolour = 0;

  myvertex.x = xf;
  myvertex.y = yf;
  myvertex.u = 0.0;
  myvertex.v = 0.0;
  ta_commit_vertex(&myvertex, xf, yf, 0.0);

  myvertex.u = w/(8<<((i->polymode2>>3)&7));
  ta_commit_vertex(&myvertex, w+xf, yf, 0.0);

  myvertex.u = 0.0;
  myvertex.v = h/(8<<(i->polymode2&7));
  ta_commit_vertex(&myvertex, xf, h+yf, 0.0);

  myvertex.u = w/(8<<((i->polymode2>>3)&7));
  myvertex.cmd |= TA_CMD_VERTEX_EOS;
  ta_commit_vertex(&myvertex, w+xf, h+yf, 0.0);  
}

/* Paste untransparent PVR */
void paste_image( struct pvr *i, int x, int y, int flags)
{
  struct polygon_list mypoly;
  struct packed_colour_vertex_list myvertex;
  float w = (float)i->xsize, h = (float)i->ysize;
  float xf = (float)x, yf = (float)y;

  mypoly.cmd =
    TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_OPAQUE|TA_CMD_POLYGON_SUBLIST|
    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_TEXTURED|TA_CMD_POLYGON_PACKED_COLOUR;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS;
  mypoly.mode2 =
    TA_POLYMODE2_BLEND_DEFAULT|TA_POLYMODE2_FOG_DISABLED|
    TA_POLYMODE2_TEXTURE_CLAMP_U|TA_POLYMODE2_TEXTURE_CLAMP_V|
    TA_POLYMODE2_BILINEAR_FILTER|TA_POLYMODE2_MIPMAP_D_1_00|
    TA_POLYMODE2_TEXTURE_REPLACE|TA_POLYMODE2_U_SIZE_1024|TA_POLYMODE2_V_SIZE_1024;
  mypoly.texture =
    TA_TEXTUREMODE_RGB565|TA_TEXTUREMODE_NON_TWIDDLED|
    TA_TEXTUREMODE_STRIDE|TA_TEXTUREMODE_ADDRESS(i->data);
  mypoly.alpha = mypoly.red = mypoly.green = mypoly.blue = 1.0;

  ta_commit_list(&mypoly);

  myvertex.cmd = TA_CMD_VERTEX;
  myvertex.colour = 0;
  myvertex.ocolour = 0;

  myvertex.u = 0.0;
  myvertex.v = 0.0;
  ta_commit_vertex(&myvertex, xf, yf, 0.0);

  myvertex.u = w*(1.0/1024);
  ta_commit_vertex(&myvertex, w+xf, yf, 0.0);

  myvertex.u = 0.0;
  myvertex.v = h*(1.0/1024);
  ta_commit_vertex(&myvertex, xf, h+yf, 0.0);

  myvertex.x = w+xf;
  myvertex.u = w*(1.0/1024);
  myvertex.cmd |= TA_CMD_VERTEX_EOS;
  ta_commit_vertex(&myvertex, w+xf, h+yf, 0.0);
}

/*! @decl void paste_pvr_part( struct pvr *, int x, int y,
 *!              int tx1, int ty1, int tx2, int ty2, int flags)
 *!
 *! Paste a part of a texture to display by inserting the apropriate
 *! instructions into the tile accellerator list currently being built.
 *!
 *! @seealso
 *!  @[paste_pvr()], @[paste_image()], @[ta_begin_frame()], 
 *!  @[ta_commit_list()], @[ta_commit_end()], @[ta_commit_frame()]
 */
void paste_pvr_part( struct pvr *i, int x, int y,
                     int tx1, int ty1, int tx2, int ty2, int flags)
{
  struct polygon_list mypoly;
  struct packed_colour_vertex_list myvertex;
  float w, h, xf, yf, tx1f, ty1f, tx2f, ty2f;
  float ufoo, vfoo;

  //FIXME: Put this inside some debugging define later.
  if(tx1>tx2 || ty1>ty2 || tx1<0 || ty1<0
     || tx2>(i->xsize-1) || ty2>(i->ysize-1))
    reportf("Texture coordinates not valid! %p, %d, %d, %d, %d\n", 
            i, tx1, ty1, tx2, ty2);

  //Coordinates are not inclusive
  tx2++;
  ty2++;

  //  float w = (float)i->xsize, h = (float)i->ysize;
  w = (float)(tx2-tx1), h = (float)(ty1-ty2);
  xf = (float)x,        yf = (float)y;
  tx1f = (float)tx1,    ty1f = (float)ty1;
  tx2f = (float)tx2,    ty2f = (float)ty2;

  mypoly.cmd =
    TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_TRANSPARENT|TA_CMD_POLYGON_SUBLIST|
    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_TEXTURED|TA_CMD_POLYGON_PACKED_COLOUR;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  /*
     0x80  -  src * alpha
     0x40  -  min(src, dst)
     0x20  -  src
     0x10  -  dst * alpha
     0x08  -  min(src, dst)
     0x04  -  dst

     0x02  -  party
     0x01  -  ?
  */
  mypoly.mode2 = TA_POLYMODE2_BLEND_SRC_ALPHA|TA_POLYMODE2_BLEND_DST_INVALPHA|
    TA_POLYMODE2_FOG_DISABLED|
    TA_POLYMODE2_TEXTURE_CLAMP_U|TA_POLYMODE2_TEXTURE_CLAMP_V|
    TA_POLYMODE2_BILINEAR_FILTER|TA_POLYMODE2_MIPMAP_D_1_00|
    TA_POLYMODE2_TEXTURE_REPLACE| i->polymode2;
#if 1
  if( flags & 1 )
  {
    mypoly.cmd &= ~TA_CMD_POLYGON_TYPE_TRANSPARENT;
    mypoly.mode2 =
      TA_POLYMODE2_BLEND_SRC|TA_POLYMODE2_FOG_DISABLED|
      TA_POLYMODE2_TEXTURE_CLAMP_U|TA_POLYMODE2_TEXTURE_CLAMP_V|
      /*TA_POLYMODE2_BILINEAR_FILTER|*/TA_POLYMODE2_MIPMAP_D_1_00|
      TA_POLYMODE2_TEXTURE_REPLACE | i->polymode2;
  }
#endif
  mypoly.texture = i->texturemode;

  mypoly.alpha = mypoly.red = mypoly.green = mypoly.blue = 1.0;

  ta_commit_list(&mypoly);

  myvertex.cmd = TA_CMD_VERTEX;
  myvertex.colour = 0;
  myvertex.ocolour = 0;
  myvertex.z = 0.25;

  ufoo = 1.0/(8<<((i->polymode2>>3)&7));
  vfoo = 1.0/(8<<(i->polymode2&7));

  myvertex.u = tx1f*ufoo;
  myvertex.v = ty2f*vfoo;
  ta_commit_vertex(&myvertex, xf, yf, 0.0);

  myvertex.u = tx2f*ufoo;
  ta_commit_vertex(&myvertex, w+xf, yf, 0.0);

  myvertex.u = tx1f*ufoo;
  myvertex.v = ty1f*vfoo;
  ta_commit_vertex(&myvertex, xf, h+yf, 0.0);

  myvertex.u = tx2f*ufoo;
  myvertex.cmd |= TA_CMD_VERTEX_EOS;
  ta_commit_vertex(&myvertex, w+xf, h+yf, 0.0);
}

/*! @decl void paste_pvr_scale_a( 
            struct pvr *, int x, int y, int x2, int y2, int flags)
 *!
 *! Scale and paste texture to display by inserting the apropriate
 *! instructions into the tile accellerator list currently being built.
 *!
 *! @seealso
 *!  @[paste_pvr_part()], @[paste_image()], @[ta_begin_frame()], 
 *!  @[ta_commit_list()], @[ta_commit_end()], @[ta_commit_frame()]
 */
void paste_pvr_scale( struct pvr *i, int x, int y, int x2, int y2, int flags)
{
  struct polygon_list mypoly;
  struct packed_colour_vertex_list myvertex;
  float w = (float)i->xsize, h = (float)i->ysize;
  float xf = (float)x, yf = (float)y;
  float xf2 = (float)x2, yf2 = (float)y2;

  mypoly.cmd =
    TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_TRANSPARENT|TA_CMD_POLYGON_SUBLIST|
    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_TEXTURED|TA_CMD_POLYGON_PACKED_COLOUR;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  /*
     0x80  -  src * alpha
     0x40  -  min(src, dst)
     0x20  -  src
     0x10  -  dst * alpha
     0x08  -  min(src, dst)
     0x04  -  dst

     0x02  -  party
     0x01  -  ?
  */
  mypoly.mode2 = TA_POLYMODE2_BLEND_SRC_ALPHA|TA_POLYMODE2_BLEND_DST_INVALPHA|
    TA_POLYMODE2_FOG_DISABLED|
    TA_POLYMODE2_TEXTURE_CLAMP_U|TA_POLYMODE2_TEXTURE_CLAMP_V|
    TA_POLYMODE2_BILINEAR_FILTER|TA_POLYMODE2_MIPMAP_D_1_00|
    TA_POLYMODE2_TEXTURE_REPLACE| i->polymode2;
#if 1
  if( flags & 1 )
  {
    mypoly.cmd &= ~TA_CMD_POLYGON_TYPE_TRANSPARENT;
    mypoly.mode2 =
      TA_POLYMODE2_BLEND_SRC|TA_POLYMODE2_FOG_DISABLED|
      TA_POLYMODE2_TEXTURE_CLAMP_U|TA_POLYMODE2_TEXTURE_CLAMP_V|
      /*TA_POLYMODE2_BILINEAR_FILTER|*/TA_POLYMODE2_MIPMAP_D_1_00|
      TA_POLYMODE2_TEXTURE_REPLACE | i->polymode2;
  }
#endif
  mypoly.texture = i->texturemode;

  mypoly.alpha = mypoly.red = mypoly.green = mypoly.blue = 1.0;

  ta_commit_list(&mypoly);

  myvertex.cmd = TA_CMD_VERTEX;
  myvertex.colour = 0;
  myvertex.ocolour = 0;

  myvertex.u = 0.0;
  myvertex.v = 0.0;
  ta_commit_vertex(&myvertex, xf, yf, 0.0);

  myvertex.u = w/(8<<((i->polymode2>>3)&7));
  ta_commit_vertex(&myvertex, xf2, yf, 0.0);

  myvertex.u = 0.0;
  myvertex.v = h/(8<<(i->polymode2&7));
  ta_commit_vertex(&myvertex, xf, yf2, 0.0);

  myvertex.u = w/(8<<((i->polymode2>>3)&7));
  myvertex.cmd |= TA_CMD_VERTEX_EOS;
  ta_commit_vertex(&myvertex, xf2, yf2, 0.0);  
}

/*! @decl void paste_pvr_scale_a( 
            struct pvr *, int x, int y, int x2, int y2, int flags, float alpha)
 *!
 *! Scale and paste texture to display with specified @[alpha] by
 *! inserting the apropriate instructions into the tile accellerator
 *! list currently being built. An @[alpha] of 1.0 is opaque and 0.0 is 
 *! totally transparent.
 *!
 *! @seealso
 *!  @[paste_pvr_part()], @[paste_image()], @[ta_begin_frame()], 
 *!  @[ta_commit_list()], @[ta_commit_end()], @[ta_commit_frame()]
 */
void paste_pvr_scale_a( struct pvr *i, int x, int y, int x2, int y2, 
                        int flags, float alpha)
{
  struct polygon_list mypoly;
  struct packed_colour_vertex_list myvertex;
  float w = (float)i->xsize, h = (float)i->ysize;
  float xf = (float)x, yf = (float)y;
  float xf2 = (float)x2, yf2 = (float)y2;

  mypoly.cmd =
    TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_TRANSPARENT|TA_CMD_POLYGON_SUBLIST|
    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_TEXTURED|TA_CMD_POLYGON_INTENSITY;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  /*
     0x80  -  src * alpha
     0x40  -  min(src, dst)
     0x20  -  src
     0x10  -  dst * alpha
     0x08  -  min(src, dst)
     0x04  -  dst

     0x02  -  party
     0x01  -  ?
  */
  mypoly.mode2 = TA_POLYMODE2_BLEND_SRC_ALPHA|TA_POLYMODE2_BLEND_DST_INVALPHA|
    TA_POLYMODE2_FOG_DISABLED|
    TA_POLYMODE2_TEXTURE_CLAMP_U|TA_POLYMODE2_TEXTURE_CLAMP_V|
    TA_POLYMODE2_BILINEAR_FILTER|TA_POLYMODE2_MIPMAP_D_1_00|
    192| //TA_POLYMODE2_MODULATE_ALPHA
    TA_POLYMODE2_ENABLE_ALPHA| i->polymode2;
#if 1
  if( flags & 1 )
  {
    mypoly.cmd &= ~TA_CMD_POLYGON_TYPE_TRANSPARENT;
    mypoly.mode2 =
      TA_POLYMODE2_BLEND_SRC|TA_POLYMODE2_FOG_DISABLED|
      TA_POLYMODE2_TEXTURE_CLAMP_U|TA_POLYMODE2_TEXTURE_CLAMP_V|
      /*TA_POLYMODE2_BILINEAR_FILTER|*/TA_POLYMODE2_MIPMAP_D_1_00|
      TA_POLYMODE2_TEXTURE_REPLACE | i->polymode2;
  }
#endif
  mypoly.texture = i->texturemode;

  mypoly.red = mypoly.green = mypoly.blue = 1.0;
  mypoly.alpha = alpha;

  ta_commit_list(&mypoly);

  myvertex.cmd = TA_CMD_VERTEX;
  myvertex.colour = 0x3f800000; //0x80ff0000; //Multi with color
  myvertex.ocolour = 0; //Added to color

  myvertex.u = 0.0;
  myvertex.v = 0.0;
  ta_commit_vertex(&myvertex, xf, yf, 0.0);

  myvertex.u = w/(8<<((i->polymode2>>3)&7));
  ta_commit_vertex(&myvertex, xf2, yf, 0.0);

  myvertex.u = 0.0;
  myvertex.v = h/(8<<(i->polymode2&7));
  ta_commit_vertex(&myvertex, xf, yf2, 0.0);

  myvertex.u = w/(8<<((i->polymode2>>3)&7));
  myvertex.cmd |= TA_CMD_VERTEX_EOS;
  ta_commit_vertex(&myvertex, xf2, yf2, 0.0);  
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
