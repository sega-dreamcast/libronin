#ifndef _SOUNDCOMMON_H
#define _SOUNDCOMMON_H been_here_before


/*
 * This file is included both on ARM and SH
 */

/* stereo */
#define STEREO

struct soundstatus {
  int mode;
  int cmd;
  int cmdstatus;
  int samplepos;
  int stereo;
  int freq;
  int ring_length;
  int message;
};

struct mpeg_buffer
{
  int size;
  unsigned char buffer[2048];
};
#define NUM_MPEG_BUFFERS 128

struct message_buffer
{
  int size;
  int lock;
  unsigned char buffer[1024];
};

#define STACK_BASE_ADDR    0x7fff0
#define SOUNDSTATUS_ADDR   (STACK_BASE_ADDR+4)
#define HEAP_BASE_ADDR     (0x80000+48)
#define MPEG_BASE_ADDR     (0x100000)
#define MESSAGE_BASE_ADDR  (0x180000-sizeof(struct message_buffer))
#define RING_BASE_ADDR     (0x180000)



#define MODE_PAUSE 0
#define MODE_PLAY  1
#define MODE_MPEG  2

#define CMD_SET_MODE(n) (n)

#ifdef STEREO
#define CMD_SET_STEREO(n) (0x40|(n))
#endif

#define CMD_SET_FREQ(n) (0x50|(n))
#define CMD_SET_BUFFER(n) (0x60|(n))

/* This gives 11025 Hz */
#define FREQ_EXP      (-1)
#define FREQ_MANTISSA (0)
#define FREQ ((44100*(1024+FREQ_MANTISSA))>>(10-FREQ_EXP))

/* 44100 Hz */
#define FREQ1_EXP (0)
#define FREQ1_MANTISSA (0)

#define FREQ1 ((44100*(1024+FREQ1_MANTISSA))>>(10-FREQ1_EXP))


#define ADJUST_BUFFER_SIZE(n) ((n)&~31)

/* .5s buffer for menu */
#define RING_BUFFER_SAMPLES ADJUST_BUFFER_SIZE(FREQ1/2)

/* .25s buffer for game */
#define RING_BUFFER_SAMPLES0 ADJUST_BUFFER_SIZE(FREQ/4)

/* 16bit */
#define SAMPLE_MODE 0

/* #if SAMPLE_MODE == 0 */
#define SAMPLES_TO_BYTES(n) ((n)<<1)
/* #else */
/* #if SAMPLE_MODE == 1 */
/* #define SAMPLES_TO_BYTES(n) (n) */
/* #else */
/* #define SAMPLES_TO_BYTES(n) ((n)>>1) */
/* #endif */
/* #endif */

#define STEREO_OFFSET (RING_BUFFER_SAMPLES+256)

#endif //_SOUNDCOMMON_H
