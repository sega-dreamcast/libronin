
void memmove( void *s, void *d, int n )
{
  char *_d = (char *)d;
  char *_s = (char *)s;
  while( n-- ) *(_d++) = *(_s++);
}

void *memcpy( void *s, void *d, int n )
{
  char *_d = (char *)d;
  char *_s = (char *)s;
  while( n-- ) *(_d++) = *(_s++);
  return d;
}


/* void memset( void *p, char d, int n ) */
/* { */
/*   while(n--) *p++=d; */
/* } */

char *membase = (char *)(512 * 1024);
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
  return (void *)(membase-n);
}

int free( void *n )
{
  /* not really */
  return 0;
}

void *calloc( int n, int m )
{
  void *r;
  memset( (r = malloc( n*m)), 0, n*m );
  return r;
}


void *memset( void *d, char b, int n )
{
  char *_d = (char *)d;
  while( n-- ) *(_d++) = b;
  return d;
}
