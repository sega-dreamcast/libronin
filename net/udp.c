#include "udp.h"
#include "util.h"

static int udp_calc_checksum(unsigned short *pkt, int sz, void *ip_pkt)
{
  int i, csum = 0;

  if(sz & 1)
    ((unsigned char *)pkt)[sz] = 0;

  csum += (unsigned short)~((short *)ip_pkt)[6];
  csum += (unsigned short)~((short *)ip_pkt)[7];
  csum += (unsigned short)~((short *)ip_pkt)[8];
  csum += (unsigned short)~((short *)ip_pkt)[9];
  csum += (unsigned short)~(17<<8);
  csum += (unsigned short)~htons(sz);

  for(i=0; i<((sz+1)>>1); i++)
    if(i != 3)
      csum += (unsigned short)~((short *)pkt)[i];

  if((csum = ((unsigned short)csum)+(csum >> 16))>=0x10000)
    csum = (unsigned short)(csum + 1);

  return (csum? csum : 0xffff);
}

static void (*proto_got_packet)(char *, int, void *) = 0;

void udp_set_handler(void (*hn)(char *, int, void *))
{
  proto_got_packet = hn;
}

void udp_got_packet(void *pkt, int size, void *ip_pkt)
{
  int sz;
  if(size < 8)
    return;
  sz = htons(((unsigned short *)pkt)[2]);
  if(sz < size)
    size = sz;
  if(size < 8)
    return;
  switch(htons(((unsigned short *)pkt)[1])) {
    case 1449:
      if(proto_got_packet)
	proto_got_packet(((char *)pkt)+8, size-8, ip_pkt);
      break;
  }
}

void udp_send_packet(void *target_hw, unsigned int target_ip,
		     unsigned short src_port, unsigned short target_port,
		     void *pkt, unsigned size)
{
  static unsigned short udppkt[1024];
  udppkt[9] = htons(28+size);
  udppkt[12] = (17<<8)|1;
  udppkt[18] = htons(src_port);
  udppkt[19] = htons(target_port);
  udppkt[20] = htons(8+size);
  if(size)
    memcpy(udppkt+22, pkt, size);
  ip_get_my_ip((unsigned int *)&udppkt[14]);
  *(unsigned int *)&udppkt[16] = target_ip;
  udppkt[21] = 0/*udp_calc_checksum(udppkt+18, size, udppkt+8)*/;
  ip_send_packet(target_hw, udppkt+8);
}

