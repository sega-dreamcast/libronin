/*
 * Sound
 */
#include "common.h"
#define DC_SOUND_BS 9216

struct buffer
{
  int read_pos;
  int write_pos;
  int full;

  short data[ DC_SOUND_BS ]; // enough, I hope.
};
//FIXME: This raises the namespace pollution to new levels.
extern struct buffer buff;

#define SOUNDSTATUS ((volatile struct soundstatus *)(void *)(0xa0800000+SOUNDSTATUS_ADDR))
#define MPEG_BUFF(X) \
((volatile struct mpeg_buffer *)(void *) \
 (0xa0800000+MPEG_BASE_ADDR+(sizeof(struct mpeg_buffer)*(X))))

extern int sound_device_open;
extern int fillpos;

START_EXTERN_C
void init_arm();
void write_samples( const short *samples, int nbytes );
void do_sound_command(int cmd);
int read_sound_int(volatile int *p);
void stop_sound();
void start_sound();
void ronin_process_sound_messages();
END_EXTERN_C



