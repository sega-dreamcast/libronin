#include "soundcommon.h"

#define AICA(n) ((volatile unsigned int *)(void*)(0x800000+(n)))

#define SOUNDSTATUS ((volatile struct soundstatus *)(void *)(SOUNDSTATUS_ADDR))

static void __gccmain() { }

static int freq_exp = 0;
static int freq_mantissa = 0;

void aica_reset()
{
  int i, j;
  volatile unsigned int *hwptr = AICA(0);

  *AICA(0x2800) = 0;

  /* Reset all 64 channels to a silent state */
  for(i=0; i<64; i++) {
    hwptr[0] = 0x8000;
    hwptr[5] = 0x1f;
    hwptr[1] = 0;
    hwptr[2] = 0;
    hwptr[3] = 0;
    hwptr[4] = 0;
    for(j=6; j<32; j++)
      hwptr[j] = 0;
    hwptr += 32;
  }

  *AICA(0x2800) = 15;
}

void init_channel(int channel, int pan, void *data, int len)
{
  volatile unsigned int *hwptr = AICA(channel<<7);

  /* Set sample format and buffer address */
  hwptr[0] = 0x4200 | (SAMPLE_MODE<<7) | (((unsigned long)data)>>16);
  hwptr[1] = ((unsigned long)data) & 0xffff;
  /* Number of samples */
  hwptr[3] = len;
  /* Frequency */
  hwptr[6] = ((freq_exp&15)<<11)|(freq_mantissa&1023);
  /* Set volume, pan, and some other stuff */
  ((volatile unsigned char *)(hwptr+9))[4] = 0x24;
  ((volatile unsigned char *)(hwptr+9))[1] = 0xf;
  ((volatile unsigned char *)(hwptr+9))[5] = 0;
  ((volatile unsigned char *)(hwptr+9))[0] = pan;
  hwptr[4] = 0x1f;
}

void do_command(int cmd)
{
  switch(cmd) {
#ifdef STEREO
   case CMD_SET_STEREO(0):
   case CMD_SET_STEREO(1):
     SOUNDSTATUS->stereo = cmd&1;
     break;
#endif
   case CMD_SET_FREQ(0):
     freq_exp = FREQ_EXP;
     freq_mantissa = FREQ_MANTISSA;
     SOUNDSTATUS->freq = FREQ;
     break;
   case CMD_SET_FREQ(1):
     freq_exp = FREQ1_EXP;
     freq_mantissa = FREQ1_MANTISSA;
     SOUNDSTATUS->freq = FREQ1;
     break;
   case CMD_SET_BUFFER(0):
     SOUNDSTATUS->ring_length = RING_BUFFER_SAMPLES0;
     break;
   case CMD_SET_BUFFER(1):
     SOUNDSTATUS->ring_length = RING_BUFFER_SAMPLES;
     break;
   case CMD_SET_MODE(MODE_PAUSE):
     *AICA(0) = (*AICA(0) & ~0x4000) | 0x8000;
#ifdef STEREO
     if(SOUNDSTATUS->stereo)
       *AICA(0x80) = (*AICA(0x80) & ~0x4000) | 0x8000;
#endif
     SOUNDSTATUS->samplepos = 0;
     SOUNDSTATUS->mode = MODE_PAUSE;
     break;
   case CMD_SET_MODE(MODE_PLAY):
#ifdef STEREO
     if(SOUNDSTATUS->stereo) {
       init_channel(0, 0x1f, RING_BUF, SOUNDSTATUS->ring_length);
       init_channel(1, 0x0f, RING_BUF+STEREO_OFFSET, SOUNDSTATUS->ring_length);
     } else
#endif
     init_channel(0, 0x00, RING_BUF, SOUNDSTATUS->ring_length);
     SOUNDSTATUS->samplepos = 0;
     *AICA(0) |= 0xc000;
#ifdef STEREO
     if(SOUNDSTATUS->stereo)
       *AICA(0x80) |= 0xc000;
#endif
     *(unsigned char *)AICA(0x280d) = 0;
     SOUNDSTATUS->mode = MODE_PLAY;
     break;
  }
}

/*
void *memcpy(void *s1, const void *s2, unsigned int n)
{
  unsigned char *d = s1;
  const unsigned char *s = s2;
  while(n--)
    *d++ = *s++;
  return s1;
}
*/

int main()
{
  /* int n = 1; */

  SOUNDSTATUS->mode = MODE_PAUSE;
  SOUNDSTATUS->samplepos = 0;
#ifdef STEREO
  SOUNDSTATUS->stereo = 0;
#endif
  freq_exp = FREQ_EXP;
  freq_mantissa = FREQ_MANTISSA;
  SOUNDSTATUS->freq = FREQ;
  SOUNDSTATUS->ring_length = RING_BUFFER_SAMPLES;

  aica_reset();

  for(;;) {

    if(SOUNDSTATUS->cmdstatus==1) {
      /*      SOUNDSTATUS[n++] = *SOUNDSTATUS; */
      do_command(SOUNDSTATUS->cmd);
      SOUNDSTATUS->cmdstatus = 2;
      /*
      SOUNDSTATUS[n++] = *SOUNDSTATUS;
      SOUNDSTATUS[n].mode = 12345678;
      */
    }

    if(SOUNDSTATUS->mode == MODE_PLAY)
      SOUNDSTATUS->samplepos = *AICA(0x2814);
  }
}
