#ifndef _GTEXT_H
#define _GTEXT_H been_here_before

struct font
{
  int tsize, tuvflags;
  float tscale;
  void *texture;
  short x[256], y[256], w[256];
};

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

void init_twiddletab();
void init_palette();
struct font *load_font(char *fn);
void draw_text(int x, int y, int w, unsigned char *text, struct font *font, int color);
void display_font(struct font *font);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _GTEXT_H */
