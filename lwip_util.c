#include "lwip/debug.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/memp.h"
#include "lwip/tcpip.h"
#include "netif/bbaif.h" 
#include "netif/loopif.h"


static void tcpip_init_done(void *arg)
{
  sys_sem_t *sem;
  sem = arg;
  sys_sem_signal(*sem);
}

void lwip_init()
{
  static int lwip_inited = 0;
  sys_sem_t sem;
  struct ip_addr ipaddr, netmask, gw;

  if(lwip_inited) return;
  sys_init();
  mem_init();
  memp_init();
  pbuf_init();
  netif_init();
  sem = sys_sem_new(0);
  tcpip_init(tcpip_init_done, &sem);
  sys_sem_wait(sem);
  sys_sem_free(sem);

  IP4_ADDR(&gw, 127,0,0,1); 
  IP4_ADDR(&ipaddr, 127,0,0,1);  
  IP4_ADDR(&netmask, 255,0,0,0);
  netif_set_default(netif_add(&ipaddr, &netmask, &gw, loopif_init, 
			      tcpip_input));

  if(gapspci_probe_bba()) {

    IP4_ADDR(&gw, 192,168,42,1); 
    IP4_ADDR(&ipaddr, 192,168,42,5);
    IP4_ADDR(&netmask, 255,255,255,0); 
    netif_set_default(netif_add(&ipaddr, &netmask, &gw, bbaif_init,
				tcpip_input));

  }

  lwip_inited=1;
}
