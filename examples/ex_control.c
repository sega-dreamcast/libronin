/* Wait for someone to press A on a controller and then exit. */

/*
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
*/

#include "report.h"
#include "serial.h"
#include "vmsfs.h"
#include "maple.h"
#include "dc_time.h"
#include "notlibc.h"

//gtext
#include "video.h"
#include "gtext.h"
//#include "cdfs.h"
#include "ta.h"


int main(int argc, char **argv)
{
  int i,s;
  struct mapledev *pads;

#ifndef NOSERIAL
  serial_init(57600);
#endif
  dc_setup_ta(); //We want the VBL IRQ.
  report("TA initiated.\n");

  maple_init();
  report("Maple BUS initiated.\n");

  ta_enable_irq();
  report("TA IRQ enabled.\n");

  setimask(0);    //Enable all interrupts. We need the VBL IRQ for the pads.
  s = getimask(); //Save the interrupt mask

  while(1) {
    setimask(s);
    usleep(15000*10);
    setimask(15);   //Mask out all interrrupts
    pads = locked_get_pads();
    for(i=0; i<4; i++) 
    {
      if( pads[i].func & MAPLE_FUNC_CONTROLLER )
      {
        int buttons = pads[i].cond.controller.buttons;
        // Exit to slave if A is pressed
        if(! (buttons & 4) )
        {
          report("A pressed.\n");
          setimask(s); //Do this before using the TA or it will stall.
          ta_sync();
          ta_disable_irq();
          return 0;
        }
      }
      else if(pads[i].func & MAPLE_FUNC_KEYBOARD)
      {
        unsigned char *key = pads[i].cond.kbd.key;
        // Exit to slave if Sysreq is pressed
        if((pads[i].cond.kbd.shift & 0x11) &&
           (key[0]==0x46 || key[1]==0x46 || key[2]==0x46 ||
            key[3]==0x46 || key[4]==0x46 || key[5]==0x46)) {
          report("Sysreq pressed.\n");
          setimask(s);
          ta_sync();
          ta_disable_irq();
          return 0;
        }
      }
    }
  }
  setimask(s); //Restore intrerrupt mask

  return 0;
}
