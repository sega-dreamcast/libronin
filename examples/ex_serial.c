#include "serial.h"

int main(int argc, char **argv)
{
  char name[20];
  int i;

  serial_init(57600);
  serial_puts("Hello World!\n");
  serial_flush(); //This is vital unless you aim for a hung application.

  return 0;
}
