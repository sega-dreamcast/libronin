#include "soundcommon.h"
#include <stdarg.h>

#define AICA(n) ((volatile unsigned int *)(void*)(0x800000+(n)))

#define RING_BUF ((short *)(void *)(RING_BASE_ADDR))
#define SOUNDSTATUS ((volatile struct soundstatus *)(void *)(SOUNDSTATUS_ADDR))

static struct message_buffer *messages
  = ((struct message_buffer *)(MESSAGE_BASE_ADDR));

#ifdef MPEG_AUDIO
#include <mad.h>

static struct mad_stream _stream;
static struct mad_frame _frame;
static struct mad_synth _synth;

#define stream (&_stream)
#define frame (&_frame)
#define synth (&_synth)

static int current_mp ;

#define MPEG_BUFFER(N) ((struct mpeg_buffer *)(MPEG_BASE_ADDR+sizeof(struct mpeg_buffer)*(N)))
#define MIN(A,B) ((A)<(B)?(A):(B))
#endif

#define IDLE 0
#define OUTPUT 1
#define MOVE 2
#define READ 3
#define DECODE 4
#define SYNTH 5


static char *itoa(int x) 
{ 
  static char buf[30];
  int minus = 0;
  int ptr=29;
  buf[29]=0;

  if(!x) return "0";
  if( x < 0 )  {  minus=1;  x = -x; }
  while(x > 0) 
  { 
    buf[--ptr] = x%10 + '0'; 
    x/=10; 
  }
  if( minus ) buf[--ptr] = '-';
  return buf+ptr; 
}


static int report_begin()
{
  volatile int *lock = &(messages->lock);
  if( *lock == 1 )
    return 1;
  while( *lock == 2 ) ;
  *lock = 1;
  return 0;
}

static void report_end()
{
  volatile int *lock = &(messages->lock);
  *lock = 0;
}

void mm_putc( int c )
{
  if( messages->size < 1024 )
    messages->buffer[ messages->size++ ] = c;
}

void report(const char *str)
{
  int n = report_begin();
  while( *str )
    mm_putc( *(str++) );
  if( !n )
    report_end();
}

void reporthex( int d, int n )
{
  int e = report_begin();
  while( n-- )
  {
    int c = (d>>(n*4))&15;
    if( c > 9 ) mm_putc( 'a'+(c-10) );
    else mm_putc( '0'+c );
  }
  if( !e )
    report_end();
}

void reportf(const char *fmt, ...)
{
  int p/*, e*/;
  int ee = report_begin();
  va_list va;
  va_start(va, fmt);
  while((p = *fmt++))
    if(p=='%')
      switch(*fmt++) 
      {
       case '\0': --fmt;    break;
       case 's': report( va_arg(va, char *) );   break;
       case '%': mm_putc('%'); break;
       case 'd': report( itoa(va_arg(va, int)) ); break;
       case 'p': report("(void *)0x");
       case 'x': 
       {
         int n = va_arg( va, int );
	 reporthex( n, 8 );
         break;
       }
       case 'X': 
       {
         int n = va_arg( va, int );
	 reporthex( n, 2 );
         break;
       }
       case 'b':
       {
         char bits[33];
         int i, d = va_arg( va, int);
         bits[32]=0;
         for( i = 0; i<31; i++ )
           if( d & (1<<i) )
             bits[31-i] = '1';
           else
             bits[31-i] = '0';
         report( bits );
	 break;
       }
      }
    else
      mm_putc(p);
  if( !ee )
    report_end();
  va_end(va);
}

#ifdef MPEG_AUDIO
#define NEXT_MP() do{\
	mp->size = mpbp = 0;						\
	current_mp++;							\
	if( current_mp == NUM_MPEG_BUFFERS )				\
	  current_mp = 0;						\
	get_mpeg_bytes( d,n ); return on; } while(0)
  
static int get_mpeg_bytes( unsigned char *d, int n )
{
  int on = n;
  if( n )
  {
    struct mpeg_buffer *mp = MPEG_BUFFER(current_mp);
    static int mpbp;

    while( n )
    {
      *(d++) = mp->buffer[mpbp++];  n--;
      if( mpbp == mp->size )
	NEXT_MP();
    }
  }
  return on;
}


static signed short scale(mad_fixed_t sample)
{
  if (sample >= MAD_F_ONE)  return 32767;
  if (sample <= -MAD_F_ONE) return -32768;
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
    SOUNDSTATUS->message = IDLE;
  }  while( dist < 10 );
  SOUNDSTATUS->message = OUTPUT;

  ((signed short *)RING_BUF)[ writepos  ] = l;
  ((signed short *)RING_BUF)[ writepos + offset ] = r;
  
  if( ++writepos >= SOUNDSTATUS->ring_length )
    writepos = 0;
}

static void libmad_output_pcm( )
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
		     (nchannels<2?sample:scale( *right_ch++ )),
		     STEREO_OFFSET);
  }
}

static int first_frame;
static void init_mpeg()
{
  first_frame = 1;
  current_mp = 0;
  memset( MPEG_BUFFER(0), 0, sizeof( struct mpeg_buffer )*NUM_MPEG_BUFFERS );
  memset( &_stream, 0, sizeof(_stream ) );
  memset( &_frame,  0, sizeof(_frame )  );
  memset( &_synth,  0, sizeof(_synth )  );
  mad_stream_init(stream);
  mad_frame_init(frame);
  mad_synth_init(synth);
}

char *mad_error( int n )
{
  switch( n )
  {
    case 0x0001:
      return "input buffer too small (or EOF)";
    case 0x0002:
      return "invalid (null) buffer pointer";
    case 0x0031:
      return "not enough memory";
    case 0x0101:
      return "lost synchronization";
    case 0x0102:
      return "reserved header layer value";
    case 0x0103:
      return "forbidden bitrate value";
    case 0x0104:
      return "reserved sample frequency value";
    case 0x0105:
      return "reserved emphasis value";
    case 0x0201:
      return "CRC check failed";
    case 0x0211:
      return "forbidden bit allocation value";
    case 0x0221:
      return "bad scalefactor index";
    case 0x0231:
      return "bad frame length";
    case 0x0232:
      return "bad big_values count";
    case 0x0233:
      return "reserved block_type";
    case 0x0234:
      return "bad main_data_begin pointer";
    case 0x0235:
      return "bad main data length";
    case 0x0236:
      return "bad audio data length";
    case 0x0237:
      return "bad Huffman table select";
    case 0x0238:
      return "Huffman data overrun";
    case 0x0239:
      return "incompatible block_type for MS";
    default:
      return "Unkown error";
  }
}

#define BSIZE 1080
static void play_mpeg()
{
  int amnt = 0, err;
  static struct
  {
    int length;
    unsigned char data[BSIZE];
  } input;

  if( first_frame )
  {
    volatile struct mpeg_buffer *mp10 = MPEG_BUFFER(10);
    while( !mp10->size )   ;
    first_frame = 0;
  }

  SOUNDSTATUS->message = MOVE;
  if( _stream.next_frame )
    memmove(input.data, _stream.next_frame,
	    input.length =
	    (&input.data[input.length])-_stream.next_frame);

  SOUNDSTATUS->message = READ;
  if( BSIZE - input.length )
  {
    amnt = get_mpeg_bytes(input.data + input.length, BSIZE - input.length);
    input.length = BSIZE;
  }

  SOUNDSTATUS->samplepos++;
  mad_stream_buffer(stream, input.data, BSIZE);

  SOUNDSTATUS->message = DECODE;
  if( mad_frame_decode(frame, stream) )
  {
    if( _stream.error == 0x0101 )
      ;
    else
      reportf( "mad_frame_decode[%d]:  %s\n",
	       SOUNDSTATUS->samplepos, mad_error( _stream.error ) );
  }
  else
  {
    SOUNDSTATUS->message = SYNTH;
    mad_synth_frame(synth, frame);
    libmad_output_pcm(  ); /* write to device */
  }
}
#endif

void __gccmain() { }

static int freq_exp = 0;
static int freq_mantissa = 0;

static void aica_reset()
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

static void init_channel(int channel, int pan, void *data, int len)
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

static void do_command(int cmd)
{
  switch(cmd) {
   case CMD_SET_STEREO(0):
   case CMD_SET_STEREO(1):
     SOUNDSTATUS->stereo = cmd&1;
     break;
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
     if(SOUNDSTATUS->stereo)
       *AICA(0x80) = (*AICA(0x80) & ~0x4000) | 0x8000;
     SOUNDSTATUS->samplepos = 0;
     SOUNDSTATUS->mode = MODE_PAUSE;
     break;
   case CMD_SET_MODE(MODE_PLAY):
   case CMD_SET_MODE(MODE_MPEG):
     SOUNDSTATUS->message = 0;
     if(SOUNDSTATUS->stereo) {
       init_channel(0, 0x1f, RING_BUF,               SOUNDSTATUS->ring_length);
       init_channel(1, 0x0f, RING_BUF+STEREO_OFFSET, SOUNDSTATUS->ring_length);
     } else
       init_channel(0, 0x00, RING_BUF, SOUNDSTATUS->ring_length);
     SOUNDSTATUS->samplepos = 0;
     *AICA(0) |= 0xc000;
     if( SOUNDSTATUS->stereo )
       *AICA(0x80) |= 0xc000;
     *(unsigned char *)AICA(0x280d) = 0;
#ifdef MPEG_AUDIO
     if( cmd == CMD_SET_MODE(MODE_MPEG) )
     {
       SOUNDSTATUS->mode = MODE_MPEG;
       init_mpeg();
     }
     else
#endif
       SOUNDSTATUS->mode = MODE_PLAY;
     break;
  }
}

int main()
{
  /* int n = 1; */
  SOUNDSTATUS->mode = MODE_PAUSE;
  SOUNDSTATUS->samplepos = 0;
  SOUNDSTATUS->stereo = 0;
  freq_exp = FREQ_EXP;
  freq_mantissa = FREQ_MANTISSA;
  SOUNDSTATUS->freq = FREQ;
  SOUNDSTATUS->ring_length = RING_BUFFER_SAMPLES;

  aica_reset();

  for(;;) {

    if(SOUNDSTATUS->cmdstatus==1) {
      do_command(SOUNDSTATUS->cmd);
      SOUNDSTATUS->cmdstatus = 2;
    }

    if(SOUNDSTATUS->mode == MODE_PLAY)
      SOUNDSTATUS->samplepos = *AICA(0x2814);
#ifdef MPEG_AUDIO
    else if( SOUNDSTATUS->mode == MODE_MPEG )
      play_mpeg();
#endif
  }
}
