/*
 * Functions used to make life easier when there is no full featured
 * clib available.
 */
#include <sys/time.h>
START_EXTERN_C
void exit(int rcode) __THROW __attribute__ ((__noreturn__));
int __offtime(const long int *t, long int offset, struct tm *tp);
END_EXTERN_C
