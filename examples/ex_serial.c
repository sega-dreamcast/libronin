#include "../ronin.h"

int main(int argc, char **argv)
{
  char name[20];
  int i;

  serial_init(57600);
  serial_puts("Hello World!\n");
  serial_flush();

#if 0
  serial_puts("What is your name? ");
  serial_flush();
  
  for(i=0; i<19; i++) {
    serial_puts(".\n");
    serial_flush();
    name[i] = serial_getc_blocking();
    if(name[i] == '\n' || name[i] == '\r') {
      name[i] = 0;
      break;
    }
  }
  name[19] = 0;
    
  serial_puts("\nHi there ");
  serial_puts(name);
  serial_putc('\n');
  serial_flush();
  
  serial_puts("Enter q to quit.\n");
  serial_flush();
  while(serial_getc() != 'q') {/* Busy loop */;}
#endif

  return 0;
}
