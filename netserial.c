#include "net/pci.h"
#include "net/ether.h"
#include "net/ip.h"
#include "net/udp.h"
#include <string.h>

static char putc_buf[1024], oob_buf[1024];
static int putc_pos = 0, oob_pos = 0;
static unsigned char bcast_hw[] = { ~0, ~0, ~0, ~0, ~0, ~0 };

static void low_send_putc(char *ptr, int l)
{
  udp_send_packet(bcast_hw, ~0, 1441, 1445, ptr, l);
}

static void low_send_oob(char *ptr, int l)
{
  udp_send_packet(bcast_hw, ~0, 1441, 1447, ptr, l);
}

void serial_init(int baudrate)
{
  unsigned int my_ip = 184658112 - 256 - (11-4);
  pci_setup();
  ether_setup();
  ip_set_my_ip(&my_ip);
  putc_pos = oob_pos = 0;
}

void serial_puts(const char *message)
{
  int l = strlen(message);
  if(putc_pos && l+putc_pos > sizeof(putc_buf)) {
    low_send_putc(putc_buf, putc_pos);
    putc_pos = 0;
  }
  if(l>sizeof(putc_buf))
    low_send_putc((char *)message, l);
  else {
    memcpy(putc_buf+putc_pos, message, l);
    putc_pos += l;
    if(putc_pos >= sizeof(putc_buf)) {
      low_send_putc(putc_buf, putc_pos);
      putc_pos = 0;
    }
  }
}

void oob_send_data(const char *data, int l)
{
  if(oob_pos && l+oob_pos > sizeof(oob_buf)) {
    low_send_oob(oob_buf, oob_pos);
    oob_pos = 0;
  }
  if(l>sizeof(oob_buf))
    low_send_oob((char *)data, l);
  else {
    memcpy(oob_buf+oob_pos, data, l);
    oob_pos += l;
    if(oob_pos >= sizeof(oob_buf)) {
      low_send_oob(oob_buf, oob_pos);
      oob_pos = 0;
    }
  }
}

void serial_putc(int c)
{
  putc_buf[putc_pos++] = c;
  if(putc_pos >= sizeof(putc_buf)) {
    low_send_putc(putc_buf, putc_pos);
    putc_pos = 0;
  }
}

void serial_flush()
{
  if(putc_pos) {
    low_send_putc(putc_buf, putc_pos);
    putc_pos = 0;
  }
}

void oob_flush()
{
  if(oob_pos) {
    low_send_oob(oob_buf, oob_pos);
    oob_pos = 0;
  }
}
