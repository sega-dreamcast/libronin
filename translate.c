#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/time.h>

#include "notlibc.h"
#include "cdfs.h"
#include "report.h"
//FIXME: Not funny...
//#include "menu.h"

#include "translate.h"
#define NUM_LANGUAGES 20
#define NUM_STRINGS  100
#define ENGLISH        0

static struct language
{
  char *name;
} languages[ NUM_LANGUAGES ];
static int next_language;



int current_language;
int set_language( int l )
{
  if( l >= next_language )
    return 0;
  current_language = l;
  return 1;
}

int set_next_language( )
{
  if( !set_language( current_language+1 ) )
    set_language( 0 );
  return current_language;
}

char *language_name( int id )
{
  if( id < next_language )
    return languages[ id ].name;
  return NULL;
}

static int language_id( char *what )
{
  int i;
  for( i = 0; i<next_language; i++ )
    if( !strcmp( languages[ i ].name, what ) )
      return i;
  languages[ next_language ].name = what;
  return next_language++;
}



static struct translation
{
  int key;
  int hval;
  struct translation *next;
  char  *t[ NUM_LANGUAGES ];
} translations[ NUM_STRINGS ];

static int next_translation;
static struct translation *hash[256];

int hashstr(const unsigned char *str)
{
  int ret,c;
  int maxn = 8; // number of characters.
  if(!(ret=str++[0]))
    return 0;
  for(; maxn>=0; maxn--)
  {
    c=str++[0];
    if(!c) break;
    ret ^= ( ret << 4 ) + c ;
    ret &= 0x7fffffff;
  }
  return ret;
}

static int translation_id( char *what, int nocreate )
{
  int hkey = hashstr( what );
  unsigned char hind = (unsigned char )((unsigned int)hkey & 255);
  struct translation *h = hash[ hind ];

  while( h ) {
    if( (h->hval == hkey) && !strcmp( h->t[ENGLISH], what ) )
      return h->key;
    h = h->next;
  }

  if( nocreate ) return -1;

  memset( &translations[ next_translation ], 0, sizeof( translations[ 0 ] ) );
  translations[ next_translation ].t[ ENGLISH ] = what;
  translations[ next_translation ].hval = hkey;
  translations[ next_translation ].key  = next_translation;
  translations[ next_translation ].next = hash[ hind ];
  hash[ (unsigned char )(hind & 255) ] = &translations[ next_translation ];
  return next_translation++;
}

unsigned char *___translate( char *what, int language )
{
  int tid;
  char *r;

  if( language == 0 )
    return what;

  tid = translation_id( what, 1 );

  if( tid < 0 )
    return what;

  if( (r = translations[tid].t[ language ]) )
    return r;
  return what;
}

static void add_translation( char *k, int language, char *translation )
{
  int id = translation_id( k, 0 );

#if 0
  reportf( "'%s' is '%s' in '%s' (%d)\n",k,translation,
	   language_name( language ),language);
#endif
  if( language != 0 )
    translations[ id ].t[ language ] = translation;
}

/*
 * #comment
 * lang languagename
 * int: string
 * int: string
 * ...
 * lang languagename
 * int: string
 * int: string
 * ...
 */


void read_translations( char *buffer, int len )
{
  int language = 0;
  int c;
  char *lp, *bp;
  void handle_line( char *line )
  {
    int i;
    if( line[0] == '#' )
      return;
    if( !strncmp( "lang ", line, 5 ) )
    {
      reportf( "New language '%s'\n", line+5 );
      language = language_id( line+5 );
      return;
    }
    for( i = 0; i<strlen(line); i++ )
      if( line[i] == ':' )
      {
	line[i++] = 0;
	while( line[i] == ' ' || line[i] == '\t' )
	  i++;
	add_translation( line, language, line+i );
      }
  };

  lp = buffer;
  bp = buffer;


  for( c = 0; c<len; c++,bp++ )
  {
    if( *bp == '\n' )
    {
      *bp = 0;
      handle_line( lp );
      lp = bp+1;
    }
  }
}


unsigned char *translate_from_iso( unsigned char *t, int *_l )
{
  int i, jisx0201=0, j=0;
  int l = *_l;
  for( i = 0; i<l; i++ )
    switch( t[i] )
    {
      case 0x1b:
	if(t[i+1]=='(' && (t[i+2]=='I' || t[i+2]=='B')) {
	  jisx0201 = (t[i+2]=='I');
	  i += 2;
	}
	break;
      case 0xc0 ... 0xdf: t[j++] = t[i]-0xc0;       break;
      case 0xe0 ... 0xff: t[j++] = t[i]-0xe0+0x80;  break;
      case 0x21 ... 0x7e: 
        if(jisx0201) 
        {
          t[j++] = t[i]+0x80; 
        }
        else 
        {
     default:
         t[j++] = t[i];
        }
    }
  *_l = j;
  return t;
}

/* BEGIN TRANSLATIONS */
#define TRANSLATIONS ""
/* END TRANSLATIONS */

void init_translations()
{
  struct dirent *e;
  DIR *dp;

  language_id( "English" );

  if( dp = opendir( "LOCALE/" ) )
    while ( e = readdir( dp ) )
      if( e->d_size && !strcmp( e->d_name+strlen( e->d_name )-4, ".TXT" ) )
      {
	char *p;
	char fn[64];
	int fd;
	strcpy( fn, "LOCALE/" );
	strcat( fn, e->d_name );
	fd = open( fn, O_RDONLY );
	if( (fd > 0)  && (p = malloc( e->d_size ))
	    && (read( fd, p, e->d_size ) == e->d_size) )
	  read_translations( translate_from_iso( p, &e->d_size ),
			     e->d_size );
      }

/*   read_translations( TRANSLATIONS, strlen(TRANSLATIONS) ); */
}
