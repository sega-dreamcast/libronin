#ifndef _RONIN_GTEXT_H
#define _RONIN_GTEXT_H been_here_before

#include "common.h"

struct font
{
  int tsize, tuvflags;
  float tscale;
  void *texture;
  short x[256], y[256];
  char cx[256], cy[256], cw[256], ch[256], adv[256];
};

START_EXTERN_C
void init_twiddletab();
void init_palette();
struct font *load_font(char *fn);
struct font *load_memfont(char *s);
void draw_text(int x, int y, int w, unsigned char *text, struct font *font, int color);
int text_width(unsigned char *text, struct font *font);
void display_font(struct font *font);
END_EXTERN_C

#endif /* _RONIN_GTEXT_H */
