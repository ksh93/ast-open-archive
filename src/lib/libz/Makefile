/*
 * zlib makefile
 */

:PACKAGE: ast

LICENSE = zlib,since=1995,author="Jean-loup Gailly,Mark Adler"

ID = z
VERSION = 1.0

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

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

test : example example.out minigzip minigzip.out
	( $(*:N=*example); echo $? ) > test.tmp
	cmp -s test.tmp $(*:N=*example.out) || echo example test failed
	( echo hello world | $(*:N=*minigzip) | $(*:N=*minigzip) -d; echo $? ) > test.tmp
	cmp -s test.tmp $(*:N=*minigzip.out) || echo minigzip test failed
	rm -f foo.gz test.tmp
