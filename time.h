/*
 * Time and timer-related functions.
 */
#include "common.h"
#define USEC_TO_TIMER(x) (((x)*100)>>11)
START_EXTERN_C
unsigned long Timer( );
void usleep( unsigned int usecs );
END_EXTERN_C
