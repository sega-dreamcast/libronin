/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: delif.c,v 1.1 2003-03-17 21:57:54 marcus Exp $
 */

#include "lwip/debug.h"

#include <stdlib.h> 

#include "lwip/def.h"
#include "netif/delif.h"

#ifdef linux
#include "netif/tapif.h"
#else /* linux */
#include "netif/tunif.h"
#endif /* linux */

#include "lwip/sys.h"


#define DELIF_INPUT_DROPRATE 0.1
#define DELIF_OUTPUT_DROPRATE 0.1

#define DELIF_INPUT_DELAY  500      /* Miliseconds. */
#define DELIF_OUTPUT_DELAY 500      /* Miliseconds. */

#define DELIF_TIMEOUT 10

struct delif {
  err_t (* input)(struct pbuf *p, struct netif *inp);
  struct netif *netif;
};

struct delif_pbuf {
  struct delif_pbuf *next;
  struct pbuf *p;
  struct ip_addr *ipaddr;
  unsigned int time;
};

static struct delif_pbuf *input_list = NULL;
static struct delif_pbuf *output_list = NULL;
/*-----------------------------------------------------------------------------------*/
static void
delif_input_timeout(void *arg)
{
  struct netif *netif;
  struct delif *delif;
  struct delif_pbuf *dp;
  unsigned int timeout, now;

  timeout = DELIF_TIMEOUT;
  
  netif = arg;
  delif = netif->state;

  
  /* Check if there is anything on the input list. */
  dp = input_list;
  while(dp != NULL) {
    now = sys_now();
    
    if(dp->time <= now) {
      delif->input(dp->p, netif);
      if(dp->next != NULL) {
	if(dp->next->time > now) {
	  timeout = dp->next->time - now;
	} else {
	  timeout = 0;
	}
	DEBUGF(DELIF_DEBUG, ("delif_output_timeout: timeout %u.\n", timeout));

      }
      input_list = dp->next;
      free(dp);
      dp = input_list;
    } else {
      dp = dp->next;	
    }
  }
  
  sys_timeout(timeout, delif_input_timeout, arg);
}
/*-----------------------------------------------------------------------------------*/
static void
delif_output_timeout(void *arg)
{
  struct netif *netif;
  struct delif *delif;
  struct delif_pbuf *dp;
  unsigned int timeout, now;

  timeout = DELIF_TIMEOUT;

  netif = arg;
  delif = netif->state;
  
  /* Check if there is anything on the output list. */
  dp = output_list;
  while(dp != NULL) {
    now = sys_now();
    if(dp->time <= now) {
      DEBUGF(DELIF_DEBUG, ("delif_output_timeout: now %u dp->time %u\n",
	     now, dp->time));
      delif->netif->output(delif->netif, dp->p, dp->ipaddr);
      if(dp->next != NULL) {
	if(dp->next->time > now) {
	  timeout = dp->next->time - now;
	} else {
	  timeout = 0;
	}
	DEBUGF(DELIF_DEBUG, ("delif_output_timeout: timeout %u.\n", timeout));
	
      }
      pbuf_free(dp->p);
      
      output_list = dp->next;
      free(dp);
      dp = output_list;
    } else {
      dp = dp->next;	
    }
  }

  
  sys_timeout(timeout, delif_output_timeout, arg);
}
/*-----------------------------------------------------------------------------------*/
static err_t
delif_output(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr)
{
  struct delif_pbuf *dp, *np;
  struct pbuf *q;
  int i, j;
  char *data;

  DEBUGF(DELIF_DEBUG, ("delif_output\n"));
    
#ifdef DELIF_OUTPUT_DROPRATE
  if(((double)rand()/(double)RAND_MAX) < DELIF_OUTPUT_DROPRATE) {
    DEBUGF(DELIF_DEBUG, ("delif_output: Packet dropped\n"));
    return 0;
  }
#endif /* DELIF_OUTPUT_DROPRATE */

  
  DEBUGF(DELIF_DEBUG, ("delif_output\n"));


  dp = malloc(sizeof(struct delif_pbuf));
  data = malloc(p->tot_len);
    
  i = 0;
  for(q = p; q != NULL; q = q->next) {
    for(j = 0; j < q->len; j++) {
      data[i] = ((char *)q->payload)[j];
      i++;
    }
  }

  
  dp->p = pbuf_alloc(PBUF_LINK, 0, PBUF_ROM);
  dp->p->payload = data;
  dp->p->len = p->tot_len;
  dp->p->tot_len = p->tot_len;
  dp->ipaddr = ipaddr;
  dp->time = sys_now() + DELIF_OUTPUT_DELAY;
  dp->next = NULL;
  if(output_list == NULL) {
    output_list = dp;
  } else {
    for(np = output_list; np->next != NULL; np = np->next);
    np->next = dp;
  }
    

  return ERR_OK;
    

}
/*-----------------------------------------------------------------------------------*/
static err_t
delif_input(struct pbuf *p, struct netif *inp)
{
  struct delif_pbuf *dp, *np;

  DEBUGF(DELIF_DEBUG, ("delif_input\n"));
#ifdef DELIF_INPUT_DROPRATE
  if(((double)rand()/(double)RAND_MAX) < DELIF_INPUT_DROPRATE) {
    DEBUGF(DELIF_DEBUG, ("delif_input: Packet dropped\n"));
    pbuf_free(p);
    return ERR_OK;
  }
#endif /* DELIF_INPUT_DROPRATE */

  
  dp = malloc(sizeof(struct delif_pbuf));
  dp->p = p;
  dp->time = sys_now() + DELIF_INPUT_DELAY;
  dp->next = NULL;
  if(input_list == NULL) {
    input_list = dp;
  } else {
    for(np = input_list; np->next != NULL; np = np->next);
    np->next = dp;
  }
  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
void
delif_init(struct netif *netif)
{
  struct delif *del;
  
  del = malloc(sizeof(struct delif));
  netif->state = del;
  netif->name[0] = 'd';
  netif->name[1] = 'e';
  netif->output = delif_output;

  del->netif = malloc(sizeof(struct netif));  
#ifdef linux
  /*  tapif_init(del->netif);*/
  tunif_init(del->netif);
#else /* linux */
  tunif_init(del->netif);
#endif /* linux */
  del->input = netif->input;
  del->netif->input = delif_input;
  sys_timeout(DELIF_TIMEOUT, delif_input_timeout, netif);
  sys_timeout(DELIF_TIMEOUT, delif_output_timeout, netif);
}

/*-----------------------------------------------------------------------------------*/
static void 
delif_thread(void *arg)
{
  struct netif *netif = arg;
  struct delif *del;
  sys_sem_t sem;
  
  del = netif->state;
#ifdef linux
  tapif_init(del->netif);
#else /* linux */
  tunif_init(del->netif);
#endif /* linux */

  sys_timeout(DELIF_TIMEOUT, delif_input_timeout, netif);
  sys_timeout(DELIF_TIMEOUT, delif_output_timeout, netif);

  sem = sys_sem_new(0);
  sys_sem_wait(sem);

}
/*-----------------------------------------------------------------------------------*/
void
delif_init_thread(struct netif *netif)
{
  struct delif *del;

  DEBUGF(DELIF_DEBUG, ("delif_init_thread\n"));
  
  del = malloc(sizeof(struct delif));
  netif->state = del;
  netif->name[0] = 'd';
  netif->name[1] = 'e';
  netif->output = delif_output;

  del->netif = malloc(sizeof(struct netif));
  del->netif->ip_addr = netif->ip_addr;
  del->netif->gw = netif->gw;
  del->netif->netmask = netif->netmask;
  del->input = netif->input;
  del->netif->input = delif_input;
  sys_thread_new(delif_thread, netif);
}

/*-----------------------------------------------------------------------------------*/



