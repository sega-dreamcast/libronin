/* This is not an example per see. It's a test of malloc(3) and free(3) */

#include <stdlib.h>
#include "serial.h"
#include "report.h"
#include "dc_time.h"

int main(int argc, char **argv)
{
  void *foo, *bar, *gazonk, *flurp;

  serial_init(57600);
  usleep(20000);
  report("Malloc test starting!\n\n");

  foo = malloc(1024);
  report("1\n");
  free(foo);

  report("^-- *A1k F* \n\n");

  foo = malloc(1024);
  report("1\n");
  bar = malloc(1024);
  report("2\n");
  free(foo);
  report("3\n");
  gazonk = malloc(1024);

  report("^-- *A1k A1k F* A1k\n\n");

  flurp = malloc(0x8ffffff);

  report("^-- *A0x8ffffff\n\n");

  return 0;
}
