CCC = sh-elf-c++ -fno-rtti
CC = sh-elf-gcc
LD = sh-elf-ld -EL
AS = sh-elf-as -little
AR = sh-elf-ar


#Must be O4 to handle long jumps correctly.
OPTIMISE=-O4 -ffreestanding -ffast-math -fschedule-insns2 -fomit-frame-pointer -fno-inline-functions -fno-rtti -fno-defer-pop -fforce-addr -fstrict-aliasing -fallow-single-precision -funroll-loops -fdelete-null-pointer-checks -fno-exceptions -fconserve-space
CPUFLAGS = -ml  -m4-single-only -mhitachi
INCLUDES = -I.

EXTRALIBS += -lm
CRT0=crt0.o
LINK=$(CPUFLAGS) -nostartfiles -nostdlib $(CRT0) $(INCLUDES) -o $@ -L. -lronin -lgcc -lc


CCFLAGS = $(OPTIMISE) $(CPUFLAGS) -I. 

# -DNOSERIAL

CFLAGS=$(CCFLAGS)


# TYPE can be elf or srec
TYPE     = elf
OBJECTS  = serial.o report.o ta.o maple.o video.o c_video.o cdfs.o vmsfs.o time.o display.o sound.o gddrive.o
OBJECTS += notlibc.o 
EXAMPLES = examples/ex_serial.$(TYPE) \
	   examples/ex_video.$(TYPE)

ARMFLAGS=-mcpu=arm7 -ffreestanding  -O5 -funroll-loops

all: libronin.a crt0.o

libronin.a: $(OBJECTS) arm_sound_code.h Makefile
	$(AR) rs $@ $(OBJECTS)

clean:
	rm -f $(OBJECTS) libronin.a $(EXAMPLES) arm_sound_code.h \
	      arm_sound_code.bin arm_sound_code.elf arm_sound_code.o \
	      arm_startup.o


examples: $(EXAMPLES)




#ARM sound code
arm_sound_code.h: arm_sound_code.bin
	./encode_armcode.pike < $< > $@

arm_sound_code.bin: arm_sound_code.elf
	arm-elf-objcopy -O binary $< $@

arm_sound_code.elf: arm_startup.o arm_sound_code.o
	arm-elf-gcc $(ARMFLAGS) -Wl,-Ttext,0 -nostdlib -nostartfiles -o $@ $^ -lgcc -Llibmad -lmad -lgcc

arm_sound_code.o: arm_sound_code.c soundcommon.h
	arm-elf-gcc -c -I libmad -Wall $(ARMFLAGS) -Wundefined   -o $@ $<

# -DMPEG_AUDIO

arm_startup.o: arm_startup.s
	arm-elf-as -marm7 -o $@ $<

#Serial code that hangs
stella.elf: examples/ex_serial.c examples/ex_serial.o libronin.a serial.h Makefile
	$(CCC) -Wl,-Ttext=0x8c020000 examples/ex_serial.o $(LINK)


#Automatic extension conversion.
.SUFFIXES: .o .cpp .c .cc .h .m .i .S .asm .elf .srec

.c.elf: libronin.a crt0.o Makefile
	$(CC) -Wl,-Ttext=0x8c020000 $*.c $(LINK)

.c.srec: libronin.a crt0.o Makefile
	$(CC) -Wl,-Ttext=0x8c020000,--oformat,srec $*.c  $(LINK)

.cpp.o:
#	@echo Compiling $*.cpp
	$(CCC) $(INCLUDES) -c $(CCFLAGS) $*.cpp -o $@

.c.o: Makefile
#	@echo Compiling $*.c
	$(CC)  $(INCLUDES) -c $(CCFLAGS) $*.c -o $@

.cpp.S:
	$(CCC) $(INCLUDES) -S $(CCFLAGS) $*.cpp -o $@

.cpp.i:
	$(CCC) $(INCLUDES) -E $(CCFLAGS) $*.cpp -o $@

.S.o:
#	@echo Compiling $*.s
#	$(CCC) $(INCLUDES) -S $(CCFLAGS) $*.S -o $@
	$(AS) $*.S -o $@

.S.i:
	$(CCC) $(INCLUDES) -c -E $(CCFLAGS) $*.S -o $@

.s.o:
#	@echo Compiling $*.s
	$(AS) $*.s -o $@


#Extra dependencies.
sound.o: arm_sound_code.h