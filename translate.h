#ifndef _TRANSLATE_H
#define _TRANSLATE_H been_here_before

#include "common.h"

START_EXTERN_C
char *language_name( int id );
int language_id( char *what );
int set_language( int language );
int set_next_language( );
int set_previous_language( );
extern int current_language;

unsigned char *___translate( char *what, int language );
void read_translations( char *data, int len );

void init_translations();
END_EXTERN_C


#define _(X)   ___translate( X, current_language )

#endif /* _TRANSLATE_H */
