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
#ifdef STEREO
  int stereo;
#endif
  int freq;
  int ring_length;
};

struct mpeg_buffer
{
  int size;
  char buffer[2048];
};
#define NUM_MPEG_BUFFERS 128


#define SOUNDSTATUS_ADDR (0xff80)

#define RING_BASE_ADDR (0x180000)
#define MPEG_BASE_ADDR (0x100000)
#define MPEG_BUFF(X) ((volatile struct mpeg_buffer *)(void *)(MPEG_BASE_ADDR+(sizeof(struct mpeg_buffer)*(X))))

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

#if SAMPLE_MODE == 0
#define SAMPLES_TO_BYTES(n) ((n)<<1)
#else
#if SAMPLE_MODE == 1
#define SAMPLES_TO_BYTES(n) (n)
#else
#define SAMPLES_TO_BYTES(n) ((n)>>1)
#endif
#endif

#define STEREO_OFFSET (RING_BUFFER_SAMPLES+256)
