#include <string.h> //FIXME: External dependecy.
#include "report.h"
#include "cdfs.h"
#include "notlibc.h"
#include "video.h"
#include "ta.h"
#include "misc.h"
#include "gtext.h"


/* setup a table for easy twiddling of texures.
   (palette based textures can't be non-twiddled) */

//FIXME: Move this to a more general video object. Used in lainblanker
//for the balls. Or perhaps a special twiddle object to avoid wasting
//1k in apps not using it.

int twiddletab[1024];

void init_twiddletab()
{
  int x;
  for(x=0; x<1024; x++)
    twiddletab[x] = (x&1)|((x&2)<<1)|((x&4)<<2)|((x&8)<<3)|((x&16)<<4)|
      ((x&32)<<5)|((x&64)<<6)|((x&128)<<7)|((x&256)<<8)|((x&512)<<9);
}

void init_palette()
{
  unsigned int (*palette)[4][256] = (unsigned int (*)[4][256])0xa05f9000;
  int n;

  for(n = 0; n<256; n++) {
    (*palette)[0][n] = (n<<24)|C_WHITE;
  }
}

static int convert_font(char *src, struct font *dst)
{
  short *index = (short *)src;
  int i, xx, yy, x=0, y=0, fontw = index[0];
  src += 512;
  for(i=0; i<256; i++) {
    int cp = (i==0? 0 : index[i]);
    int cw = (i==255? fontw : index[i+1])-cp;
    if(x+cw > dst->tsize) {
      x = 0;
      y += 24;
      if(y+24 > dst->tsize)
	return 0;
    }
    dst->x[i] = x;
    dst->y[i] = y;
    dst->w[i] = cw;
    for(xx=0; xx<cw; xx++)
      for(yy=0; yy<24; yy++) {
	char v = src[yy*fontw+cp+xx];
	char *p = ((char*)dst->texture)+
	  ((twiddletab[x+xx]<<1)|twiddletab[y+yy]);
	if(((unsigned int)p)&1)
	  *(short *)(p-1) |= v<<8;
	else
	  *(short *)p |= (unsigned char)v;
      }
    x += cw;
  }
  return 1;
}

/*! @decl font *load_font( char *filename )
 *!
 *! Loads a font and returns a pointer to the resulting texture.
 */
struct font *load_font(char *fn)
{
  struct font f;
  struct font *r;
  int i, fd;
  char *s;

  fd = open( fn, O_RDONLY );
  if( fd < 0 )
  {
    reportf( "FATAL: Failed to open font >%s< errno=%d\r\n", fn, fd );
    //FIXME: Use system font to write error.
    //FIXME: Implements atexit so that functions like goto_slave can be called.
    exit(1); 
  }

  s = (char *)malloc( file_size( fd ) );
  pread( fd, (void *)s, file_size( fd ), 0 );
  close( fd );

  for(i=5; i<11; i++) {
    f.tuvflags = (i-3)|((i-3)<<3);
    f.tsize = 1<<i;
    f.tscale = 1.0/f.tsize;
    f.texture = ta_txalloc(1<<(2*i));
    memset(f.texture, 0, 1<<(2*i));
    if(convert_font(s, &f))
      break;
    else
      ta_txfree(f.texture);
  }
  
  if(i>=11) {
    reportf( "FATAL: Font >%s< too large to fit in a texture\r\n", fn);
    //FIXME: Use system font to write error.
    //FIXME: Implements atexit so that functions like goto_slave can be called.
    exit(1);
  }

  free(s);
  r = (struct font *)malloc( sizeof(struct font) );
  *r = f;
  return r;
}

/*! @decl font *load_memfont( char *memfont )
 *!
 *! Loads a font already in memory and returns a pointer to the
 *! resulting texture.
 */
struct font *load_memfont(char *s)
{
  struct font f;
  struct font *r;
  int i;

  for(i=5; i<11; i++) {
    f.tuvflags = (i-3)|((i-3)<<3);
    f.tsize = 1<<i;
    f.tscale = 1.0/f.tsize;
    f.texture = ta_txalloc(1<<(2*i));
    memset(f.texture, 0, 1<<(2*i));
    if(convert_font(s, &f))
      break;
    else
      ta_txfree(f.texture);
  }
  
  if(i>=11) {
    reportf( "FATAL: Font too large to fit in a texture\r\n");
    //FIXME: Use system font to write error.
    //FIXME: Implements atexit so that functions like goto_slave can be called.
    exit(1);
  }

  r = (struct font *)malloc( sizeof(struct font) );
  *r = f;
  return r;
}

/*! @decl void draw_text(int x, int y, int w, unsigned char *text, 
 *!                      font *font, int color)
 *!
 *! Draws @[text] using @[font].
 */
void draw_text(int x, int y, int w, unsigned char *text, struct font *font, int color)
{
  struct polygon_list mypoly;
  struct packed_colour_vertex_list myvertex;
  float scale = font->tscale;
  int c;

#if 1
  /* setup font texture */
  mypoly.cmd =
    TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_TRANSPARENT|TA_CMD_POLYGON_SUBLIST|
    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_TEXTURED|TA_CMD_POLYGON_PACKED_COLOUR;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  mypoly.mode2 = TA_POLYMODE2_BLEND_SRC_ALPHA|TA_POLYMODE2_BLEND_DST_INVALPHA|
    TA_POLYMODE2_FOG_DISABLED|
    TA_POLYMODE2_TEXTURE_CLAMP_U|TA_POLYMODE2_TEXTURE_CLAMP_V|
    TA_POLYMODE2_BILINEAR_FILTER|TA_POLYMODE2_MIPMAP_D_1_00|
    TA_POLYMODE2_TEXTURE_MODULATE|font->tuvflags;
  mypoly.texture = TA_TEXTUREMODE_CLUT8|TA_TEXTUREMODE_ADDRESS(font->texture);
  mypoly.alpha = mypoly.red = mypoly.green = mypoly.blue = 1.0;
  ta_commit_list(&mypoly);  
  myvertex.colour = color;
  myvertex.ocolour = 0;
  myvertex.z = 0.125;

  while((c=*text++)) {
    int u = font->x[c], v = font->y[c], cw = font->w[c];

    /* plot character */
    if(cw)
    {
      if(cw > w)
	cw = w;

      myvertex.cmd = TA_CMD_VERTEX;
      myvertex.x = x;
      myvertex.y = y;
      myvertex.u = u*scale;
      myvertex.v = v*scale;
      ta_commit_list(&myvertex);

      myvertex.x = x+cw;
      myvertex.u = (u+cw)*scale;
      ta_commit_list(&myvertex);

      myvertex.x = x;
      myvertex.y = y+24;
      myvertex.u = u*scale;
      myvertex.v = (v+24)*scale;
      ta_commit_list(&myvertex);

      myvertex.x = x+cw;
      myvertex.u = (u+cw)*scale;
      myvertex.cmd |= TA_CMD_VERTEX_EOS;
      ta_commit_list(&myvertex);  
    }

    cw += (c==' '? 5:2);
    x += cw;
    if((w -= cw)<=0)
      break;
  }
#else
  struct sprite_list mysprite;

  /* setup font texture */
  mypoly.cmd =
    TA_CMD_SPRITE|TA_CMD_POLYGON_TYPE_TRANSPARENT|/*TA_CMD_POLYGON_SUBLIST|*/
    /*    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_TEXTURED| */
    TA_CMD_POLYGON_PACKED_COLOUR|TA_CMD_POLYGON_16BIT_UV;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  mypoly.mode2 = TA_POLYMODE2_BLEND_SRC_ALPHA|TA_POLYMODE2_BLEND_DST_INVALPHA|
    TA_POLYMODE2_FOG_DISABLED|
    TA_POLYMODE2_TEXTURE_CLAMP_U|TA_POLYMODE2_TEXTURE_CLAMP_V|
    TA_POLYMODE2_BILINEAR_FILTER|TA_POLYMODE2_MIPMAP_D_1_00|
    TA_POLYMODE2_TEXTURE_MODULATE|font->tuvflags;
  mypoly.texture = TA_TEXTUREMODE_CLUT8|TA_TEXTUREMODE_ADDRESS(font->texture);
  mypoly.alpha = mypoly.red = mypoly.green = mypoly.blue = 1.0;
  ta_commit_list(&mypoly);  
  mysprite.cmd = TA_CMD_VERTEX/*|TA_CMD_VERTEX_EOS*/;
  mysprite.dummy = 0;
  mysprite.az = mysprite.bz = mysprite.cz = 0.125;

  while((c=*text++)) {
    int u = font->x[c], v = font->y[c], cw = font->w[c];

    /* plot character */
    {
      mysprite.ax = x;
      mysprite.ay = y;

      mysprite.bx = x+cw;
      mysprite.by = y;

      mysprite.cx = x;
      mysprite.cy = y+24;

      mysprite.dx = x+cw;
      mysprite.dy = y+24;

      float u0 = u*scale, u1 = (u+cw)*scale;
      float v0 = v*scale, v1 = (v+24)*scale;

      mysprite.au = ((short *)&u0)[1];
      mysprite.av = ((short *)&v0)[1];

      mysprite.bu = ((short *)&u1)[1];
      mysprite.bv = ((short *)&v0)[1];

      mysprite.cu = ((short *)&u0)[1];
      mysprite.cv = ((short *)&v1)[1];

      ta_commit_list2(&mysprite);
    }

    x += cw;
    if(!(w -= cw))
      break;
  }
#endif
}

void display_font(struct font *font)
{
  struct polygon_list mypoly;
  struct packed_colour_vertex_list myvertex;

  /* setup font texture */
  mypoly.cmd =
    TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_TRANSPARENT|TA_CMD_POLYGON_SUBLIST|
    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_TEXTURED|TA_CMD_POLYGON_PACKED_COLOUR;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  mypoly.mode2 = TA_POLYMODE2_BLEND_SRC_ALPHA|TA_POLYMODE2_BLEND_DST_INVALPHA|
    TA_POLYMODE2_FOG_DISABLED|
    TA_POLYMODE2_TEXTURE_CLAMP_U|TA_POLYMODE2_TEXTURE_CLAMP_V|
    TA_POLYMODE2_BILINEAR_FILTER|TA_POLYMODE2_MIPMAP_D_1_00|
    TA_POLYMODE2_TEXTURE_MODULATE|font->tuvflags;
  mypoly.texture = TA_TEXTUREMODE_CLUT8|TA_TEXTUREMODE_ADDRESS(font->texture);
  mypoly.alpha = mypoly.red = mypoly.green = mypoly.blue = 1.0;
  ta_commit_list(&mypoly);  
  myvertex.colour = C_WHITE;
  myvertex.ocolour = 0;
  myvertex.z = 0.125;

  myvertex.cmd = TA_CMD_VERTEX;
  myvertex.x = 0;
  myvertex.y = 0;
  myvertex.u = 0.0;
  myvertex.v = 0.0;
  ta_commit_list(&myvertex);
  
  myvertex.x = 640;
  myvertex.u = 1.0;
  ta_commit_list(&myvertex);
  
  myvertex.x = 0;
  myvertex.y = 480;
  myvertex.u = 0.0;
  myvertex.v = 1.0;
  ta_commit_list(&myvertex);
  
  myvertex.x = 640;
  myvertex.u = 1.0;
  myvertex.cmd |= TA_CMD_VERTEX_EOS;
  ta_commit_list(&myvertex);  
}
