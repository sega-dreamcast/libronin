CC =  arm-elf-gcc -Wall 
aS =  arm-elf-as
DEFINES= -DHAVE_CONFIG_H -I.. -I../.. -DASO_INTERLEAVE1 -DASO_IMDCT -DFPM_ARM

ARMFLAGS=-march=armv4m -ffreestanding -O2 -fomit-frame-pointer -ffast-math 

CFLAGS =  ${DEFINES} -Wall  ${ARMFLAGS}


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
