CCC = sh-elf-c++ -fno-rtti
CC = sh-elf-gcc -Wall
LD = sh-elf-ld -EL
AS = sh-elf-as -little
AR = sh-elf-ar


#Must be O4 to handle long jumps correctly.
OPTIMISE=-O4 -ffreestanding -ffast-math -fschedule-insns2 -fomit-frame-pointer -fno-inline-functions -fno-rtti -fno-defer-pop -fforce-addr -fstrict-aliasing -fallow-single-precision -funroll-loops -fdelete-null-pointer-checks -fno-exceptions -fconserve-space
CPUFLAGS = -ml  -m4-single-only -mhitachi
INCLUDES = -I.

# TYPE can be elf, srec or bin
TYPE = bin
EXTRALIBS += -lm
CRT0=crt0.o
ifeq "$(TYPE)" "bin"
LINK=$(CPUFLAGS) -nostartfiles -nostdlib $(CRT0) $(INCLUDES) -o $@ -L. -lronin-noserial -lgcc -lc
else
LINK=$(CPUFLAGS) -nostartfiles -nostdlib $(CRT0) $(INCLUDES) -o $@ -L. -lronin -lgcc -lc
endif

CCFLAGS = $(OPTIMISE) $(CPUFLAGS) -I.

CFLAGS = $(CCFLAGS)


# TYPE can be elf, srec or bin
TYPE     = bin
OBJECTS  = serial.o report.o ta.o maple.o video.o c_video.o cdfs.o vmsfs.o time.o display.o sound.o gddrive.o gtext.o translate.o misc.o 

OBJECTS += notlibc.o 
EXAMPLES = examples/ex_serial.$(TYPE) \
	   examples/ex_video.$(TYPE) \
	   examples/ex_vmsfscheck.$(TYPE) \
	   examples/ex_gtext.$(TYPE) \

ARMFLAGS=-mcpu=arm7 -ffreestanding  -O5 -funroll-loops

most: crt0.o libronin.a 

all: crt0.o libronin.a libronin-noserial.a cleanish

libronin.a: $(OBJECTS) arm_sound_code.h Makefile
	$(AR) rs $@ $(OBJECTS)

noserial-dummy: $(OBJECTS) arm_sound_code.h Makefile
	@echo Dummy done.

libronin-noserial.a: libronin.a
	$(MAKE) cleanish
	rm -f libronin-serial.a
	$(MAKE) CCFLAGS="$(CCFLAGS) -DNOSERIAL" CFLAGS="$(CCFLAGS) -DNOSERIAL" noserial-dummy
	$(AR) rs $@ $(OBJECTS)

cleanish:
	rm -f $(OBJECTS) crt0.o $(EXAMPLES) \
	      arm_sound_code.h arm_sound_code.bin arm_sound_code.elf \
	      arm_sound_code.o arm_startup.o

clean: cleanish
	rm -f libronin.a 


examples: libronin.a $(EXAMPLES)


#RIS specific upload targets
test-vmufs: examples/ex_vmsfscheck.$(TYPE)
	/home/peter/hack/dreamsnes/dc/ipupload.pike < examples/ex_vmsfscheck.$(TYPE)

test-gtext: examples/ex_gtext.$(TYPE) 
	/home/peter/hack/dreamsnes/dc/ipupload.pike < examples/ex_gtext.$(TYPE)

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
.SUFFIXES: .o .cpp .c .cc .h .m .i .S .asm .elf .srec .bin

.c.elf: libronin.a crt0.o Makefile
	$(CC) -Wl,-Ttext=0x8c020000 $(CCFLAGS) $*.c $(LINK)

.c.bin: libronin-noserial.a crt0.o Makefile
	$(CC) -Wl,-Ttext=0x8c010000,--oformat,binary -DNOSERIAL $(CCFLAGS) $*.c $(LINK)

.c.srec: libronin.a crt0.o Makefile
	$(CC) -Wl,-Ttext=0x8c020000,--oformat,srec $(CCFLAGS) $*.c $(LINK)

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

#Nice to have for special (libronin) development purposes.
cdfs.o: gddrive.h
examples/ex_gtext.$(TYPE): libronin.a gtext.c