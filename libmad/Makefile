CC =  arm-elf-gcc -Wall 
aS =  arm-elf-as
DEFINES= -DHAVE_CONFIG_H -I.. -I../.. -DASO_INTERLEAVE1 -DASO_IMDCT -DFPM_ARM
OPTIMIZE=-O5 -Wall -ffast-math -Wundefined

# -Os -ffreestanding -ffast-math -fschedule-insns2 -fomit-frame-pointer -fno-inline-functions -fno-rtti -fno-defer-pop -fforce-addr -fstrict-aliasing -fallow-single-precision -funroll-loops -fdelete-null-pointer-checks -fno-exceptions -fconserve-space
CFLAGS =  ${DEFINES} ${OPTIMIZE}


OBJ=fixed.o frame.o huffman.o layer12.o layer3.o stream.o synth.o  \
    timer.o version.o bit.o armlib.o imdct_l_arm.o

all: libmad.a

.PHONY: clean cleandist dep 
clean:
	rm -f *.o *.a core

cleandist: clean
	rm -f *~

distclean: cleandist

libmad.a: $(OBJ) 
	arm-elf-ar cru libmad.a $(OBJ)

.c.o:
	$(CC) -c $(CFLAGS) $(GGDB) $(PG) $< 

.S.o:
	$(CC) -c $(CFLAGS) $(GGDB) $(PG) $< 

.c.s:
	$(CC) -S $(CFLAGS) $(GGDB) $(PG) $< 
