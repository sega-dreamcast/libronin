/*
 * Functions used to make life easier when there is no full featured
 * clib available.
 */
#include <sys/time.h>
#include "common.h"
START_EXTERN_C
void exit(int rcode) __THROW __attribute__ ((__noreturn__));
void *malloc(size_t size);
int __offtime(const long int *t, long int offset, struct tm *tp);
END_EXTERN_C
