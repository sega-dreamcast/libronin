/* example.c -- usage example of the zlib compression library
 * Copyright (C) 1995-1998 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */

/* @(#) $Id: example.c,v 1.1 2002-02-25 22:56:12 bash-peter Exp $ */

#define DC yep

#include <stdio.h>
#include "zlib.h"

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#else
   extern void exit  OF((int));
#endif

#define CHECK_ERR(err, msg) { \
    if (err != Z_OK) { \
        reportf("%s error: %d\n", msg, err); \
        exit(1); \
    } \
}

const char hello[] = "hello, hello!";
/* "hello world" would be more standard, but the repeated "hello"
 * stresses the compression code better, sorry...
 */

const char dictionary[] = "hello";
uLong dictId; /* Adler32 value of the dictionary */

void test_compress      OF((Byte *compr, uLong comprLen,
		            Byte *uncompr, uLong uncomprLen));
void test_gzio          OF((const char *out, const char *in, 
		            Byte *uncompr, int uncomprLen));
void test_deflate       OF((Byte *compr, uLong comprLen));
void test_inflate       OF((Byte *compr, uLong comprLen,
		            Byte *uncompr, uLong uncomprLen));
void test_large_deflate OF((Byte *compr, uLong comprLen,
		            Byte *uncompr, uLong uncomprLen));
void test_large_inflate OF((Byte *compr, uLong comprLen,
		            Byte *uncompr, uLong uncomprLen));
void test_flush         OF((Byte *compr, uLong *comprLen));
void test_sync          OF((Byte *compr, uLong comprLen,
		            Byte *uncompr, uLong uncomprLen));
void test_dict_deflate  OF((Byte *compr, uLong comprLen));
void test_dict_inflate  OF((Byte *compr, uLong comprLen,
		            Byte *uncompr, uLong uncomprLen));
int  main               OF((int argc, char *argv[]));

/* ===========================================================================
 * Test compress() and uncompress()
 */
void test_compress(compr, comprLen, uncompr, uncomprLen)
    Byte *compr, *uncompr;
    uLong comprLen, uncomprLen;
{
    int err;
    uLong len = strlen(hello)+1;

    err = compress(compr, &comprLen, (const Bytef*)hello, len);
    CHECK_ERR(err, "compress");

    strcpy((char*)uncompr, "garbage");

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");

    if (strcmp((char*)uncompr, hello)) {
        reportf("bad uncompress\n");
	exit(1);
    } else {
        reportf("uncompress(): %s\n", (char *)uncompr);
    }
}

int main (int argc, char **argv)
{
    Byte *compr, *uncompr;
    uLong comprLen = 10000*sizeof(int); /* don't overflow on MSDOS */
    uLong uncomprLen = comprLen;
    static const char* myVersion = ZLIB_VERSION;

#ifndef NOSERIAL
    serial_init(57600);
    report("Serial OK\r\n");
#endif

    if (zlibVersion()[0] != myVersion[0]) {
        reportf("incompatible zlib version\n");
        exit(1);

    } else if (strcmp(zlibVersion(), ZLIB_VERSION) != 0) {
        reportf("warning: different zlib version\n");
    }

    compr    = (Byte*)calloc((uInt)comprLen, 1);
    uncompr  = (Byte*)calloc((uInt)uncomprLen, 1);
    /* compr and uncompr are cleared to avoid reading uninitialized
     * data and to ensure that uncompr compresses well.
     */
    if (compr == Z_NULL || uncompr == Z_NULL) {
        report("out of memory\n");
	exit(1);
    }
    test_compress(compr, comprLen, uncompr, uncomprLen);

    goto_slave();
    exit(0);
    return 0; /* to avoid warning */
}
