#include "soundcommon.h"

#define AICA(n) ((volatile unsigned int *)(void*)(0x800000+(n)))

#if SAMPLE_MODE == 0
#define RING_BUF ((short *)(void *)(RING_BASE_ADDR))
#else
#define RING_BUF ((signed char *)(void *)(RING_BASE_ADDR))
#endif
#define SOUNDSTATUS ((volatile struct soundstatus *)(void *)(SOUNDSTATUS_ADDR))

#ifdef MPEGMUSIC
struct mpeg_buffer *mpeg_buffers = (struct mpeg_buffer *)MPEG_BASE_ADDR;

#include <mad.h>

static struct mad_stream _stream, *stream=&_stream;
static struct mad_frame _frame,   *frame=&_frame;
static struct mad_synth _synth,   *synth=&_synth;

struct mpeg_buffer *mp;
int current_mp ;

int get_mpeg_bytes( char *d, int n )
{
  while( n-- )
  {
    while( !mp->size )
    {
      current_mp++;
      if( current_mp >= NUM_MPEG_BUFFERS )
	current_mp = 0;
      mp = &(mpeg_buffers[ current_mp ]);
    }    
    *d++ = mp->buffer[ 2048-(mp->size--) ];
  }
}

static signed int scale(mad_fixed_t sample)
{
  /*   round  */
  /*   sample += (1L << (MAD_F_FRACBITS - 16)); */
  /* clip */
  if (sample >= MAD_F_ONE)
    return 32767;
  else if (sample <= -MAD_F_ONE)
    return -32768;
  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
};


static inline void output_sample_s( signed short l,
				    signed short r,
				    int offset )
{
  static int writepos  = 0;
  int dist;
  do
  {
    dist = *AICA(0x2814)-writepos;
    if( dist < 0 )
      dist += SOUNDSTATUS->ring_length;
  }  while( dist < 10 );

  ((signed short *)RING_BUF)[ writepos  ] = l;
#ifdef STEREO
  ((signed short *)RING_BUF)[ writepos + offset ] = r;
#endif
  
  if( ++writepos >= SOUNDSTATUS->ring_length )
    writepos = 0;
}

static inline void libmad_output_pcm( )
{
  struct mad_header *header = &_frame.header;
  struct mad_pcm *pcm =    &_synth.pcm;
  unsigned int nchannels, nsamples;
  mad_fixed_t const *left_ch, *right_ch;
  nchannels = MAD_NCHANNELS(header);
  nsamples  = pcm->length;
  left_ch   = pcm->samples[0];
  right_ch  = pcm->samples[1];
  while( nsamples-- )
  {
    signed int sample = scale( *left_ch );
    output_sample_s( sample,
#ifdef STEREO
		     (nchannels<2?sample:scale( *right_ch++ )),
#else
		     sample,
#endif
		     STEREO_OFFSET);
  }
}

static void play_mpeg()
{
  mp = &(mpeg_buffers[ 0 ]);
  current_mp = 0;
  memset( mpeg_buffers, 0, sizeof( struct mpeg_buffer )*NUM_MPEG_BUFFERS );
  memset( &_stream, 0, sizeof(_stream ) );
  memset( &_frame,  0, sizeof(_frame )  );
  memset( &_synth,  0, sizeof(_synth )  );
  memset( mpeg_buffers, 0, sizeof( mpeg_buffers ) );
  mad_stream_init(stream);
  mad_frame_init(frame);
  mad_synth_init(synth);

  while( !SOUNDSTATUS->cmdstatus )
  {
    int off = 0;
    static char buffer[ 2048 ];
  
    if (_stream.next_frame)
      memmove( buffer, _stream.next_frame,
	       2048-(off = (char *)_stream.next_frame - (char *)buffer) );

    get_mpeg_bytes( buffer+off, 2048-off );
    mad_stream_buffer( stream, buffer, 2048 );
    mad_frame_decode(frame, stream);
    mad_synth_frame(synth, frame);
    libmad_output_pcm(  ); /* write to device */
  }
}

#endif



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
   case CMD_SET_MODE(MODE_MPEG):
#ifdef MPEGMUSIC
#ifdef STEREO
     if(SOUNDSTATUS->stereo) {
       init_channel(0, 0x1f, RING_BUF,               SOUNDSTATUS->ring_length);
       init_channel(1, 0x0f, RING_BUF+STEREO_OFFSET, SOUNDSTATUS->ring_length);
     } else
#endif
     init_channel(0, 0x00, RING_BUF, SOUNDSTATUS->ring_length);
     SOUNDSTATUS->samplepos = 0;
     *AICA(0) |= 0xc000;
#ifdef STEREO
     if( SOUNDSTATUS->stereo )
       *AICA(0x80) |= 0xc000;
#endif
     *(unsigned char *)AICA(0x280d) = 0;
     SOUNDSTATUS->mode = MODE_MPEG;
     play_mpeg( );
#endif
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
