/*
 * zlib makefile
 */

:PACKAGE: ast

LICENSE = type=special

ID = z
VERSION = 1.0

CCFLAGS = -O $(CC.DLL)

$(ID) $(VERSION) :LIBRARY: zlib.h zconf.h zutil.h \
	deflate.h infblock.h infcodes.h inffast.h \
	inffixed.h inftrees.h infutil.h trees.h \
	adler32.c compress.c crc32.c gzio.c uncompr.c deflate.c \
	trees.c zutil.c inflate.c infblock.c inftrees.c infcodes.c \
	infutil.c inffast.c \
	sfdcgzip.c sfdclzw.c

$(INCLUDEDIR) :INSTALLDIR: zlib.h zconf.h

$(INCLUDEDIR) :INSTALLPROTO: sfdcgzip.h

:: RELEASE README INDEX ChangeLog algorithm.doc

:INSTALLDIR: example minigzip

example :: example.c -lz

minigzip :: minigzip.c -lz

test : example minigzip
	$(*:N=*example)
	echo hello world | $(*:N=*minigzip) | $(*:N=*minigzip) -d 
	rm -f foo.gz
