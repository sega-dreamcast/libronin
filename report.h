/*
 * Functions used for debug output through the serial
 * interface. Dependent on the serial functions.
 */
#include "common.h"

START_EXTERN_C
void report(const char *str);
void reportf(const char *fmt, ...);
char *itoa(int x);
END_EXTERN_C
