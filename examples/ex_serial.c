#include "serial.h"
#include "dc_time.h"

int main(int argc, char **argv)
{
  serial_init(57600);
  usleep(1500000); //Give it no less than 1.5s to initiate before using it. 
                   //Important for IP-serial.
  serial_puts("Hello World!\n");
  serial_flush(); //This is vital unless you aim for a hung application.
  usleep(2000);   //Avoid trashing of the serial buffer.

  return 0;
}
