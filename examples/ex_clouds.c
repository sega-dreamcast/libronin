#include "report.h"
#include "serial.h"
#include "video.h"
#include "dc_time.h"
#include "gfxhelper.h"
#include "ta.h"
#include "cdfs.h"
#include "gtext.h"
#include "sincos_rroot.h"
#include "maple.h"

//Note: sin() and cos() in mlib does not properly handle more than 360°

#define NUMLAY 2
#define STEPS (400/NUMLAY)

int is_pressed()
{
  int i,s;
  struct mapledev *pads;

  s = getimask(); //Save the interrupt mask
  
  setimask(15);   //Mask out all interrrupts
  pads = locked_get_pads();
  for(i=0; i<4; i++) 
  {
    if( pads[i].func & MAPLE_FUNC_CONTROLLER )
    {
      int buttons = pads[i].cond.controller.buttons;
      if(! (buttons & 4) ) // button A
      {
        report("A pressed.\n");
        setimask(s);
        return 1;
      }
    }
  }
  setimask(s); //Restore intrerrupt mask
  return 0;
}

int main(int argc, char **argv)
{
  struct pvr *cloud, *sky, *comic;
  int i, xo, yo, step, layer;
  float sinr, cosr;

  serial_init(57600);

  dc_setup_ta();

  ta_enable_irq();

  maple_init();

  cdfs_init();

  report("foo\n");

  usleep(1000000);
  report(".\n");
  cloud = load_pvr("/TEST/4444/FLUFF_1.PVR");
  report(".\n");
  sky   = load_pvr("/TEST/565/4902-TEXTURE512.PVR");
  report(".\n");
  comic = load_pvr("/TEST/COMIC565/CHICKEN-SONGS-1.PVR");
  

  report("bar\n");
  report("\n");

  /* Rotate Nonno around the x-axis, end with nice copperbars :) */
#if 0
  for(i=0; i<300; i++)
  {
    SINCOS( i*1024, sinr, cosr );

    reportf("%d,%d         \r",xo, yo);

    ta_begin_frame();  
    ta_commit_end();
    paste_pvr_scale(nonno, (320-i)-(i*sinr), 240-(i*0.75), 
                           (320-i)+(i*sinr), 240+(i*0.75), 0);
    ta_commit_frame();
    ta_sync();
    usleep(1000000/60);
  }

  /* Draw circle with nonni */
  for(i=0; i<1000; i++)
  {
    SINCOS( i*1024, sinr, cosr );
    xo = (int)(sinr * 100);
    yo = (int)(cosr * 100);
    
    ta_commit_end();
    paste_pvr_scale_a(sky, 250-i, 200-i, 390+i, 280+i, 0, 0.5);
    paste_pvr_scale_a(sky, 250-i*2, 200-i*2, 390+i*2, 280+i*2, 0, 0.5);
    paste_pvr_scale_a(cloud, 250-xo, 200-yo, 390-xo, 280-yo, 0, 0.5);
    ta_commit_frame();
    ta_sync();
  }
#endif

  while(! is_pressed() )
  {
    int partsize = STEPS;
    int xoffset = 40;
    int yoffset = 40;
    int xwind = 3;
    int ywind = 1;
    
    reportf("wind: %d, %d\n", xwind, ywind);

    for(step=0; step<STEPS; step+=1)
    {
      float progress = (float)step/(float)STEPS;

      ta_begin_frame();  
      ta_commit_end();
      paste_pvr_scale(comic, 0, 0, 639, 511, 0);      
      for(layer=0; layer<NUMLAY; layer++)
      {
        int offset=step+(layer*partsize);
        float alphaperlayer = 1.0/(float)(NUMLAY-1);
        float alpha = alphaperlayer*(NUMLAY-layer)-(alphaperlayer*progress);

        if(layer<(NUMLAY/2)) //Fade in from the back
          alpha = (1/alpha)*2;
        else
          alpha = alpha*2;

        paste_pvr_scale_a(sky, -offset*2.5, -offset, 640+offset*2.5, 512+offset,
                          0, alpha );

        if((xoffset += xwind) > 320+128) {
          xoffset = 40;
          xwind=rand()%4;
          reportf("wind: %d, %d\n", xwind, ywind);
        }
        if((yoffset += ywind) > 256+77) {
          yoffset = 40;
          ywind=rand()%4;
          reportf("wind: %d, %d\n", xwind, ywind);
        }
        
        paste_pvr_scale(cloud, 320+xoffset, 256+yoffset, 
                        320+xoffset+128*progress, 256+yoffset+77*progress, 0);

      }
      ta_commit_frame();
      ta_sync();
    }
  }

  return 0;
}
