NETSERIAL = 0
NETCD = 0
CCC = sh-elf-c++ -fno-rtti -fconserve-space
CC = sh-elf-gcc -Wall
LD = sh-elf-ld -EL
AS = sh-elf-as -little
AR = sh-elf-ar


#Must be O4 to handle long jumps correctly.
OPTIMISE=-O4 -ffreestanding -ffast-math -fschedule-insns2 -fomit-frame-pointer -fno-inline-functions -fno-defer-pop -fforce-addr -fstrict-aliasing -funroll-loops -fdelete-null-pointer-checks -fno-exceptions
CPUFLAGS = -ml  -m4-single-only
INCLUDES = -Iinclude -Iinclude/ronin

# TYPE can be elf, srec or bin
TYPE = elf
EXTRALIBS += -lm
CRT0=lib/crt0.o
ifeq "$(TYPE)" "bin"
LINK=$(CPUFLAGS) -nostartfiles -nostdlib $(INCLUDES) -o $@ -Llib -lz -lronin-noserial -lgcc -lc -lgcc
else
LINK=$(CPUFLAGS) -nostartfiles -nostdlib $(INCLUDES) -o $@ -Llib -lz -lronin -lgcc -lc -lgcc
endif

EXAMPLEFLAGS = -DVMUCOMPRESS
CCFLAGS = $(OPTIMISE) $(CPUFLAGS) $(EXAMPLEFLAGS) -DDC -DDREAMCAST 

CFLAGS = $(CCFLAGS)

DISTHEADERS=cdfs.h common.h dc_time.h gddrive.h gfxhelper.h gtext.h maple.h matrix.h misc.h notlibc.h report.h ronin.h serial.h sincos_rroot.h soundcommon.h sound.h ta.h translate.h video.h vmsfs.h

# begin lwIP

LWCOREOBJS=lwip/core/mem.o lwip/core/memp.o lwip/core/netif.o \
	lwip/core/pbuf.o lwip/core/stats.o lwip/core/sys.o \
        lwip/core/tcp.o lwip/core/tcp_input.o \
        lwip/core/tcp_output.o lwip/core/udp.o 
LWCORE4OBJS=lwip/core/ipv4/icmp.o lwip/core/ipv4/ip.o \
	lwip/core/inet.o lwip/core/ipv4/ip_addr.o

LWAPIOBJS=lwip/api/api_lib.o lwip/api/api_msg.o lwip/api/tcpip.o \
	lwip/api/err.o lwip/api/sockets.o 

LWNETIFOBJS=lwip/netif/loopif.o \
	lwip/netif/tcpdump.o lwip/netif/arp.o

LWARCHOBJS=lwip/arch/dc/sys_arch.o lwip/arch/dc/thread_switch.o \
	lwip/arch/dc/netif/bbaif.o lwip/arch/dc/netif/rtk.o \
	lwip/arch/dc/netif/gapspci.o

LWIPOBJS=$(LWCOREOBJS) $(LWCORE4OBJS) $(LWAPIOBJS) $(LWNETIFOBJS) \
	$(LWARCHOBJS) lwip_util.o

# end lwIP

OBJECTS  = report.o ta.o maple.o video.o c_video.o vmsfs.o time.o display.o sound.o gddrive.o gtext.o translate.o misc.o gfxhelper.o malloc.o matrix.o

ifeq "$(NETSERIAL)" "1"
OBJECTS += netserial.o
else
OBJECTS += serial.o
endif
ifeq "$(NETCD)" "1"
OBJECTS += netcd.o
else
OBJECTS += cdfs.o
endif

OBJECTS += $(LWIPOBJS)

OBJECTS += notlibc.o 
EXAMPLES = examples/ex_serial.$(TYPE) \
	   examples/ex_video.$(TYPE) \
	   examples/ex_vmsfscheck.$(TYPE) \
	   examples/ex_gtext.$(TYPE) \
	   examples/ex_showpvr.$(TYPE) \
	   examples/ex_malloc.$(TYPE) \
	   examples/ex_purupuru.$(TYPE) \
	   examples/ex_compress.$(TYPE) \
	   examples/ex_videomodes.$(TYPE) \

ARMFLAGS=-mcpu=arm7 -ffreestanding  -O5 -funroll-loops

most: include lib/crt0.o lib/libronin.a lib/libz.a

all: include lib/crt0.o lib/libronin.a lib/libronin-noserial.a cleanish lib/libz.a

lib/crt0.o: crt0.s
	@test -d lib || mkdir lib
	$(AS) $< -o $@

lib/libronin.a: $(OBJECTS) arm_sound_code.h Makefile
	@test -d lib || mkdir lib
	rm -f $@ && $(AR) rs $@ $(OBJECTS)

noserial-dummy: $(OBJECTS) arm_sound_code.h Makefile
	@echo Dummy done.

lib/libronin-noserial.a: lib/libronin.a
	$(MAKE) cleanish
	rm -f lib/libronin-serial.a
	$(MAKE) CCFLAGS="$(CCFLAGS) -DNOSERIAL" CFLAGS="$(CCFLAGS) -DNOSERIAL" noserial-dummy
	rm -f $@ && $(AR) rs $@ $(OBJECTS)

lib/libz.a:
	@test -d lib || mkdir lib
	cd zlib; $(MAKE) libz.a
	@echo Making convenience links.
	rm -f lib/libz.a && ln -s ../zlib/libz.a lib/
	rm -f zlib.h && ln -s zlib/zlib.h .
	rm -f zconf.h && ln -s zlib/zconf.h .

include: $(DISTHEADERS) zlib/zlib.h zlib/zconf.h lwipopts.h lwip/include/lwip/*.h \
			lwip/include/netif/*.h lwip/include/ipv4/lwip/*.h \
			lwip/arch/dc/include/arch/*.h lwip/arch/dc/include/netif/*.h
	rm -rf include
	mkdir include
	mkdir include/ronin
	mkdir include/lwip
	mkdir include/netif
	mkdir include/arch
	for h in $(DISTHEADERS); do ln -s -f ../../$$h include/ronin/; done
	ln -s -f ../zlib/zlib.h ../zlib/zconf.h include/
	ln -s -f ../lwipopts.h include/
	cd include/lwip && ln -s -f ../../lwip/include/lwip/*.h ./
	cd include/lwip && ln -s -f ../../lwip/include/ipv4/lwip/*.h ./
	cd include/netif && ln -s -f ../../lwip/include/netif/*.h ./
	cd include/netif && ln -s -f ../../lwip/arch/dc/include/netif/*.h ./
	cd include/arch && ln -s -f ../../lwip/arch/dc/include/arch/*.h ./


cleanish:
	rm -f $(OBJECTS) $(EXAMPLES) \
	      arm_sound_code.h arm_sound_code.bin arm_sound_code.elf \
	      arm_sound_code.o arm_startup.o

clean: cleanish
	rm -f lib/crt0.o
	rm -f lib/libronin.a 
	rm -f lib/libronin-noserial.a 
	rm -f lib/libz.a
	cd zlib && $(MAKE) clean

examples: lib/libronin.a $(EXAMPLES)

ifeq "$(NETSERIAL)$(NETCD)" "00"
dist: $(DISTHEADERS) 
	@$(MAKE) clean && \
	$(MAKE) all && \
	if [ `ar -t lib/libronin-noserial.a|egrep 'net(cd|serial)'|wc -l` = 0 -a \
	     `ar -t lib/libronin.a | egrep 'net(cd|serial)' | wc -l` = 0 ]; then \
		mkdir disttmp && mkdir disttmp/ronin && mkdir disttmp/ronin/lib && \
		mkdir disttmp/ronin/include && mkdir disttmp/ronin/include/ronin && \
		cp lib/libronin.a disttmp/ronin/lib && \
		cp lib/libronin-noserial.a disttmp/ronin/lib && \
		cp lib/crt0.o disttmp/ronin/lib && \
		cp $(DISTHEADERS) disttmp/ronin/include/ronin && \
		cp README disttmp/ronin && \
		cp COPYING disttmp/ronin && \
		cp zlib/README disttmp/ronin/ZLIB_README && \
		cp zlib/libz.a disttmp/ronin/lib && \
		cp zlib/zlib.h disttmp/ronin/include && \
		cp zlib/zconf.h disttmp/ronin/include && \
		cp lwip/COPYING disttmp/ronin/LWIP_COPYING && \
		cp lwipopts.h disttmp/ronin/include && \
		cp -R lwip/include/lwip disttmp/ronin/include/ && \
		cp -R lwip/include/netif disttmp/ronin/include/ && \
		cp -R lwip/include/ipv4/lwip disttmp/ronin/include/ && \
		cp -R lwip/arch/dc/include/arch disttmp/ronin/include/ && \
		cp -R lwip/arch/dc/include/netif disttmp/ronin/include/ && \
		find disttmp -type d -name CVS | xargs rm -rf &&\
		(cd disttmp && tar cvf - ronin) | gzip -c > ronin-dist.tar.gz && \
		rm -rf disttmp/ronin && mkdir disttmp/ronin-src && \
		make clean && \
		mkdir disttmp/ronin-src/tools && \
		cp tools/ipupload.pike tools/ipconsole.pike tools/encode_armcode.pike disttmp/ronin-src/tools/ && \
		$(MAKE) arm_sound_code.h && \
		cp *.c *.s *.h disttmp/ronin-src && \
		cp -R lwip disttmp/ronin-src && \
		cp -R zlib disttmp/ronin-src && \
		cp Makefile disttmp/ronin-src && \
		cp README disttmp/ronin-src && \
		cp COMPILING disttmp/ronin-src && \
		cp COPYING disttmp/ronin-src && \
		cp zlib/README disttmp/ronin-src/ZLIB_README && \
		cp lwip/COPYING disttmp/ronin-src/LWIP_COPYING && \
		find disttmp -type d -name CVS | xargs rm -rf &&\
		(cd disttmp && tar cvf - ronin-src) | gzip -c > ronin-dist-src.tar.gz && \
		echo "remember to tag and bump version if you didn't already." && \
		rm -rf disttmp; \
	else \
		echo "Parts of NETCD/NETSERIAL found in libs!"; \
	fi;
else
dist:
	@echo "You must disable NETCD/NETSERIAL!"
endif

#RIS specific upload targets
test-vmufs: examples/ex_vmsfscheck.$(TYPE)
	/home/peter/hack/dreamsnes/dc/ipupload.pike < examples/ex_vmsfscheck.$(TYPE)

test-gtext: examples/ex_gtext.$(TYPE) 
	/home/peter/hack/dreamsnes/dc/ipupload.pike < examples/ex_gtext.$(TYPE)

test-showpvr: examples/ex_showpvr.$(TYPE) 
	/home/peter/hack/dreamsnes/dc/ipupload.pike < examples/ex_showpvr.$(TYPE)

test-clouds: examples/ex_clouds.elf
	/home/peter/hack/dreamsnes/dc/ipupload.pike < examples/ex_clouds.$(TYPE)

test-control: examples/ex_control.elf
	/home/peter/hack/dreamsnes/dc/ipupload.pike < examples/ex_control.$(TYPE)

test-malloc: examples/ex_malloc.elf
	/home/peter/hack/dreamsnes/dc/ipupload.pike < examples/ex_malloc.$(TYPE)

test-purupuru: examples/ex_purupuru.elf
	/home/peter/hack/dreamsnes/dc/ipupload.pike < examples/ex_purupuru.$(TYPE)

test-compress: examples/ex_compress.elf
	/home/peter/hack/dreamsnes/dc/ipupload.pike < examples/ex_compress.$(TYPE)

test-videomodes: examples/ex_videomodes.elf
	/home/peter/hack/dreamsnes/dc/ipupload.pike < examples/ex_videomodes.$(TYPE)

#ARM sound code
arm_sound_code.h: arm_sound_code.bin
	./tools/encode_armcode.pike < $< > $@

arm_sound_code.bin: arm_sound_code.elf
	arm-elf-objcopy -O binary $< $@

arm_sound_code.elf: arm_startup.o arm_sound_code.o
	arm-elf-gcc $(ARMFLAGS) -Wl,-Ttext,0 -nostdlib -nostartfiles -o $@ $^ -lgcc -lgcc

arm_sound_code.o: arm_sound_code.c soundcommon.h
	arm-elf-gcc -c -Wall $(ARMFLAGS) -Wundefined   -o $@ $<

arm_startup.o: arm_startup.s
	arm-elf-as -marm7 -o $@ $<

#Automatic extension conversion.
.SUFFIXES: .o .cpp .c .cc .h .m .i .S .asm .elf .srec .bin

.c.elf: lib/libronin.a $(CRT0) Makefile
	$(CC) -Wl,-Ttext=0x8c020000 $(CCFLAGS) $(CRT0) $*.c $(LINK) -lm

.c.bin: lib/libronin-noserial.a $(CRT0) Makefile
	$(CC) -Wl,-Ttext=0x8c010000,--oformat,binary -DNOSERIAL $(CCFLAGS) $(CRT0) $*.c $(LINK)

.c.srec: lib/libronin.a $(CRT0) Makefile
	$(CC) -Wl,-Ttext=0x8c020000,--oformat,srec $(CCFLAGS) $(CRT0) $*.c $(LINK)

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
malloc.o: Makefile
notlibc.o: Makefile
examples/ex_gtext.$(TYPE): lib/libronin.a
examples/ex_showpvr.$(TYPE): lib/libronin.a
examples/ex_cloud.$(TYPE): lib/libronin.a
examples/ex_malloc.elf: lib/libronin.a
