#include "config.h"
extern void reportf(const char *fmt, ...);

void memmove( void *d, void *s, int n )
{
  char *_d = (char *)d;
  char *_s = (char *)s;
  while( n-- ) *(_d++) = *(_s++);
}

void *memcpy( void *d, void *s, int n )
{
  char *_d = (char *)d;
  char *_s = (char *)s;
  while( n-- ) *(_d++) = *(_s++);
  return d;
}

#if 1
static char *membase = (char *)(512 * 1024 + 64);
/* room for a 256 Kbyte binary and a 256 KByte stack.
 *   <mem top>
 *     [ sample buffer l ]   64K   
 *     [ sample buffer r ]   64K   
 *
 *     [ mpeg buffer ]        512K  1M 512K
 *     [ heap ]               512K
 *     [ stack ]              256K
 *     [ binary ]             256K
 */
void *malloc( int n )
{
  membase += n;
  reportf( "ARM: malloc %d\n", n );
  return (void *)(membase-n);
}

int free( void *n )
{
  /* not really */
  reportf( "ARM: free %p\n", n );
  return 0;
}

void *calloc( int n, int m )
{
  void *r;
  memset( (r = malloc( n*m)), 0, n*m );
  return r;
}
#endif

void *memset( void *d, char b, int n )
{
  char *_d = (char *)d;
  while( n-- ) *(_d++) = b;
  return d;
}
