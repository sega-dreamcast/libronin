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


CCFLAGS = $(OPTIMISE) $(CPUFLAGS) \
	-I. \

#-DNOSERIAL

CFLAGS=$(CCFLAGS)


# TYPE can be elf or srec
TYPE     = elf
OBJECTS  = serial.o report.o ta.o maple.o video.o c_video.o cdfs.o vmsfs.o time.o display.o sound.o
OBJECTS += notlibc.o 
EXAMPLES = examples/ex_serial.$(TYPE) \
	   examples/ex_video.$(TYPE)

all: libronin.a crt0.o

libronin.a: $(OBJECTS) Makefile
	$(AR) rcs $@ $(OBJECTS)

clean:
	rm -f *.o $(OBJECTS) libronin.a $(EXAMPLES)

examples: $(EXAMPLES)




#ARM sound code
dc/arm_sound_code.h : dc/arm_sound_code.bin
	dc/encode_armcode.pike < $< > $@

dc/arm_sound_code.bin : dc/arm_sound_code.elf
	arm-elf-objcopy -O binary $< $@

dc/arm_sound_code.elf : dc/arm_startup.o dc/arm_sound_code.o
	arm-elf-gcc -mcpu=arm7 -ffreestanding -Wl,-Ttext,0 -nostdlib -nostartfiles -o $@ $^ -lgcc

dc/arm_sound_code.o : dc/arm_sound_code.c dc/soundcommon.h
	arm-elf-gcc -c -mcpu=arm7 -ffreestanding -O4 -fomit-frame-pointer -o $@ $<

dc/arm_startup.o : dc/arm_startup.s
	arm-elf-as -marm7 -o $@ $<



.SUFFIXES: .o .cpp .c .cc .h .m .i .S .asm .elf .srec

.c.elf: libronin.a crt0.o Makefile
	$(CC) -Wl,-Ttext=0x8c020000 $*.c  $(LINK)

.c.srec: libronin.a crt0.o Makefile
	$(CC) -Wl,-Ttext=0x8c020000,--oformat,srec $*.c  $(LINK)

.cpp.o:
#	@echo Compiling $*.cpp
	$(CCC) $(INCLUDES) -c $(CCFLAGS) $*.cpp -o $@

.c.o:
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

