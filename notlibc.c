/* 
 * Rudimentary aproximations and stubs for various libc functions. 
 */

#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "dc_time.h"
#include "notlibc.h"
#include "report.h"

int errno = 0;

void _Console_Getc( char c )
{
}

void _Console_Putc( char c )
{
}

void exit(int rcode);

/* Stuff needed to make the binary smaller (some 2Mb or so...)
   remember to link in libronin before libc and libgcc to avoid
   multiple definition conflicts. */
static FILE _stderr;
FILE *stderr = &_stderr;
int atexit(void (*function)(void)){ return 0; }
void abort(){ report("aborted\n"); exit(1); }
int sprintf(char *str, const char *format, ...)
 {report("sprintf ignored\n");return 0;}
int fprintf(FILE *stream,  const  char  *format, ...)
 {report("fprintf ignored\n");return -1;}
int printf(const char  *format, ...){report("printf ignored\n");return -1;}
int fputs( const char *s, FILE *stream ){report("fputs ignored\n"); return -1;}
int __write(){report("__write ignored\n"); return -1;}
FILE *fopen(const char *f, const char *m){report("fopen ignored\n"); return 0;}
int __isnan(){} //Expect warning.
void __assert_fail(char *message){report("__asser_fail ignored\n");}
void __main(){}
void matherr( void *exp ){report("matherr ignored\n");}

/* Real implementations of functions normally found in libc */
void exit(int rcode)
{
  report("Exit called. Exiting to menu.\n");
  (*(void(**)())0x8c0000e0)(1);
  for(;;);
}

/* 8c 000000  notused         64Kb - vectors
 * 8c 010000  binary start    2.9 Mb
 *
 * 8c 280000  malloc start    
 * 8c efffff  malloc end       12 Mb
 *
 * 8c f00000  stack end       
 * 8c fffffc  stack start     1 Mb
 */
#define MEMSTART 0x8c300000
#define MEMEND   0x8cf00000

static int last;
static int mallocpointer=MEMSTART;
static int total_size;
void *malloc(size_t size)
{
  int keep;
  total_size += size;

  /* Point somewhere else next time... */
  keep=mallocpointer;
  mallocpointer += size + 32-size%32;

  reportf("malloc [%d %d %d -> %p]\r\n",
          size/1024,
          total_size/1024,
          (MEMEND-mallocpointer)/1024, 
          mallocpointer);

  if(mallocpointer > MEMEND) {
    report("Out of allocatable memory!\n");
    return 0;
  }

  return (void *)(last=keep);
}

void free(void *ptr)
{
  if( (int)ptr == last )
  {
    total_size -= mallocpointer-last;
    mallocpointer = last;
    last = 0;
  } else
    reportf("Free called with non-last block (%p). Wasting some memory.\n", ptr);
}

static unsigned int low_read_time()
{
  volatile unsigned int *rtc = (volatile unsigned int *)0xa0710000;
  return ((rtc[0]&0xffff)<<16)|(rtc[1]&0xffff);
}

time_t time(time_t *tloc)
{
  unsigned int old, new; //Expect stupid warning.
  time_t tmp;
  if(tloc == NULL)
    tloc = &tmp;
  old = low_read_time();
  for(;;) {
    int i;
    for(i=0; i<3; i++) {
      new = low_read_time();
      if(new != old)
	break;
    }
    if(i<3)
      old = new;
    else
      break;
  }
  return *tloc = new-631152000;
}

const unsigned short int __mon_yday[2][13] =
  {
    /* Normal years.  */
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    /* Leap years.  */
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
  };
