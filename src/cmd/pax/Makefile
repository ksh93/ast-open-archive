:PACKAGE: ast

LICENSE = since=1987,author=gsf

DEBUG == 1

pax :: RELEASE HISTORY PROMO.mm pax.1 pax.1.posix \
	pax.h options.h zip.h \
	bio.c convert.c copy.c pax.c \
	delta.c file.c format.c misc.c tv.c \
	-lardir -lodelta -lvdelta -lsum

"win32*" :NOOPTIMIZE: delta.c

nocom :: nocom.c

$(LIBDIR)/pax :INSTALLDIR: cia.fmt ppar.fmt

:INSTALLDIR: bax cpio tar testpax

bax :: bax.1 bax.sh

cpio :: cpio.sh

rpm2cpio :: rpm2cpio.sh

tar :: tar.sh

:TEST: pax

:TEST: testpax.sh pax

::	data/link.dat data/mode.dat \
	data/aha.dat data/ro.dat \
	data/star-all-fail-1.dat \
	data/star-gnu-fail-1.dat \
	data/star-gnu-fail-2.dat \
	data/star-long-1.dat \
	data/star-long-1.lst \
	data/star-long-2.dat \
	data/star-long-2.lst \
	data/star-test.dat \
	data/y2k.dat
