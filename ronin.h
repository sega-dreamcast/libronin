#ifndef _RONIN_H
#define _RONIN_H been_here_before

#include <ronin/serial.h>
#include <ronin/report.h>
#include <ronin/maple.h>
#include <ronin/notlibc.h>
#include <ronin/dc_time.h>
#include <ronin/cdfs.h>
#include <ronin/vmsfs.h>
#include <ronin/video.h>
#include <ronin/sound.h>
#include <ronin/ta.h>

//FIXME: Move this to somewhere else?
static __inline int getimask(void)
{
  register unsigned int sr;
  __asm__("stc sr,%0" : "=r" (sr));
  return (sr >> 4) & 0x0f;
}

static __inline void setimask(int m)
{
  register unsigned int sr;
  __asm__("stc sr,%0" : "=r" (sr));
  sr = (sr & ~0xf0) | (m << 4);
  __asm__("ldc %0,sr" : : "r" (sr));
}

#endif /* _RONIN_H */
