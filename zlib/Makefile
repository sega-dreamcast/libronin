# Makefile for zlib
# Copyright (C) 1995-1998 Jean-loup Gailly.
# For conditions of distribution and use, see copyright notice in zlib.h 

# To compile and test, type:
#   ./configure; make test
# The call of configure is optional if you don't have special requirements
# If you wish to build zlib as a shared library, use: ./configure -s

# To install /usr/local/lib/libz.* and /usr/local/include/zlib.h, type:
#    make install
# To install in $HOME instead of /usr/local, use:
#    make install prefix=$HOME

CC = sh-elf-gcc

OPTIMISE=-O4 -ffreestanding -ffast-math -fschedule-insns2 -fomit-frame-pointer -fno-inline-functions -fno-rtti -fno-defer-pop -fforce-addr -fstrict-aliasing -fallow-single-precision -funroll-loops -fdelete-null-pointer-checks -fno-exceptions -fconserve-space

CPUFLAGS=-ml  -m4-single-only

CCFLAGS = $(OPTIMISE) $(CPUFLAGS)

CFLAGS=$(CCFLAGS)

# CRT0=../dc/crt0.o
# LINK=$(CPUFLAGS) -nostartfiles -nostdlib $(CRT0) -o $@ $(OBJS) -lgcc


LDFLAGS=-L. -lz 
EXTRALIBS = -L../ronin -nostartfiles -nostdlib -lronin -lstdc++ -lgcc -lm -lc -lgcc

LINK=$(CPUFLAGS) -nostartfiles -nostdlib $(CRT0) $(INCLUDES) -o $@  $(OBJECTS) $(EXTRAS_OBJECTS) $(GLIDEOBJS) $(LDLIBS) $(UNZIPLIBS) $(GLIDELIBS) $(GUILIBS)  $(MPEGOBJECTS) $(EXTRALIBS)

LDSHARED=$(CC)
CPP=$(CC) -E

VER=1.1.3
LIBS=libz.a
SHAREDLIB=libz.so

AR=sh-elf-ar rc
RANLIB=sh-elf-ranlib
SHELL=/bin/sh

prefix = /usr/local
exec_prefix = ${prefix}
libdir = ${exec_prefix}/lib
includedir = ${prefix}/include

OBJS = adler32.o compress.o crc32.o gzio.o uncompr.o deflate.o trees.o \
       zutil.o inflate.o infblock.o inftrees.o infcodes.o infutil.o inffast.o

all: example minigzip

libz.a: $(OBJS) $(OBJA)
	$(AR) $@ $(OBJS) $(OBJA)
	-@ ($(RANLIB) $@ || true) >/dev/null 2>&1

$(SHAREDLIB).$(VER): $(OBJS)
	$(LDSHARED) -o $@ $(OBJS)
	rm -f $(SHAREDLIB) $(SHAREDLIB).1
	ln -s $@ $(SHAREDLIB)
	ln -s $@ $(SHAREDLIB).1

example: example.o $(LIBS)
	$(CC) $(CFLAGS) -o $@ example.o $(LDFLAGS) $(LINK)
#	$(CC) -Wl,-Ttext=0x8c020000,-N $(LINK) -o $@ example.o

minigzip: minigzip.o $(LIBS)
	$(CC) $(CFLAGS) -o $@ minigzip.o $(LDFLAGS)

dontinstall: $(LIBS)
	-@if [ ! -d $(includedir)  ]; then mkdir $(includedir); fi
	-@if [ ! -d $(libdir) ]; then mkdir $(libdir); fi
	cp zlib.h zconf.h $(includedir)
	chmod 644 $(includedir)/zlib.h $(includedir)/zconf.h
	cp $(LIBS) $(libdir)
	cd $(libdir); chmod 755 $(LIBS)
	-@(cd $(libdir); $(RANLIB) libz.a || true) >/dev/null 2>&1
	cd $(libdir); if test -f $(SHAREDLIB).$(VER); then \
	  rm -f $(SHAREDLIB) $(SHAREDLIB).1; \
	  ln -s $(SHAREDLIB).$(VER) $(SHAREDLIB); \
	  ln -s $(SHAREDLIB).$(VER) $(SHAREDLIB).1; \
	  (ldconfig || true)  >/dev/null 2>&1; \
	fi
# The ranlib in install is needed on NeXTSTEP which checks file times
# ldconfig is for Linux

clean:
	rm -f *.o *~ example minigzip libz.a libz.so* foo.gz so_locations \
	   _match.s maketree

distclean:	clean


depend:
	makedepend -- $(CFLAGS) -- *.[ch]

.c.o:
	@echo Compiling $*.c
	@$(CC) -c $(CCFLAGS) $*.c -o $@

# DO NOT DELETE THIS LINE -- make depend depends on it.

adler32.o: zlib.h zconf.h
compress.o: zlib.h zconf.h
crc32.o: zlib.h zconf.h
deflate.o: deflate.h zutil.h zlib.h zconf.h
example.o: zlib.h zconf.h
gzio.o: zutil.h zlib.h zconf.h
infblock.o: infblock.h inftrees.h infcodes.h infutil.h zutil.h zlib.h zconf.h
infcodes.o: zutil.h zlib.h zconf.h
infcodes.o: inftrees.h infblock.h infcodes.h infutil.h inffast.h
inffast.o: zutil.h zlib.h zconf.h inftrees.h
inffast.o: infblock.h infcodes.h infutil.h inffast.h
inflate.o: zutil.h zlib.h zconf.h infblock.h
inftrees.o: zutil.h zlib.h zconf.h inftrees.h
infutil.o: zutil.h zlib.h zconf.h infblock.h inftrees.h infcodes.h infutil.h
minigzip.o:  zlib.h zconf.h 
trees.o: deflate.h zutil.h zlib.h zconf.h trees.h
uncompr.o: zlib.h zconf.h
zutil.o: zutil.h zlib.h zconf.h  
