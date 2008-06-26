/*
 * regression tests for library functions
 * `tests' runs all tests visible from .. with output in tests.out
 */

:PACKAGE: ast

LICENSE = since=1999,author=gsf

:TEST: astconf ip6 opt strelapsed strtoi strtof

base64 :TEST: base64.c

.SOURCE : cdt
.SOURCE.h : ../../lib/libast/cdt

cdt :TEST: testcdt dttest.h +last \
	cdt/tbags.c cdt/tevent.c cdt/tobag.c cdt/tqueue.c cdt/trenew.c \
	cdt/tsearch.c cdt/tshare.c cdt/tstack.c cdt/tview.c
		set +x; $(*:O=1) $(TESTCDTFLAGS) $(*:O>1) $(CC) $(CCFLAGS) $(LDFLAGS)

test.dt : testcdt dttest.h +last $$(*.ARGS:D:B:S=.c) .CLEARARGS
	$(*:O=1) -v $(*:O>1) $(CC) $(CCFLAGS) $(LDFLAGS)

date :TEST: date.c date.msk date.dat datey1k.dat datey2k.dat week.dat
	for i in $(*:N=*.dat)
	do	DATEMSK=$(*:N=*.msk:/ /:/G) TZ=EST5EDT $(*:O=1) $(TESTDATEFLAGS) < $i
	done

.SOURCE : sfio
.SOURCE.h : ../../lib/libast/sfio ../../lib/libast

sfio :TEST: testsfio sftest.h +last \
	talarm.c talign.c tappend.c tatexit.c tclose.c tdisc.c terrno.c \
	texcept.c tflags.c tgetr.c thole.c tleak.c tlocale.c tlongdouble.c \
	tmmap2read.c tmode.c tmove.c tmpread.c tmprocess.c tmtsafe.c \
	tmultiple.c tmwrite.c tnoseek.c tnotify.c topen.c tpipe.c tpipemove.c \
	tpkrd.c tpoll.c tpool.c tpopen.c tpopenrw.c tprintf.c \
	tpublic.c tputgetc.c tputgetd.c tputgetl.c tputgetm.c tputgetr.c \
	tputgetu.c trcrv.c treserve.c tscanf.c tseek.c \
	tsetbuf.c tsetfd.c tsfstr.c tshare.c tsize.c tstack.c tstatus.c \
	tstdio.c tstkpk.c tstring.c tswap.c tsync.c ttell.c ttmp.c \
	ttmpfile.c tungetc.c twchar.c twhole.c twrrd.c
		set +x; $(*:O=1) $(TESTSFIOFLAGS) $(*:O>1) $(CC) $(CCFLAGS) $(LDFLAGS)

test.sf : testsfio sftest.h +last $$(*.ARGS:D:B:S=.c) .CLEARARGS
	$(*:O=1) -v $(*:O>1) $(CC) $(CCFLAGS) $(LDFLAGS)

.SOURCE : vmalloc
.SOURCE.h : ../../lib/libast/vmalloc

vmalloc :TEST: testvmalloc vmtest.h +last \
	vmalloc/talign.c vmalloc/tcompact.c vmalloc/tlast.c vmalloc/tmalloc.c \
	vmalloc/tmrandom.c vmalloc/tpool.c vmalloc/trandom.c vmalloc/tresize.c \
	vmalloc/tsmall.c vmalloc/tstat.c vmalloc/twalk.c
		set +x; $(*:O=1) $(TESTVMALLOCFLAGS) $(*:O>1) $(CC) $(CCFLAGS) $(LDFLAGS)

test.vm : testvmalloc vmtest.h +last $$(*.ARGS:D:B:S=.c) .CLEARARGS
	$(*:O=1) -v $(*:O>1) $(CC) $(CCFLAGS) $(LDFLAGS)

:: RELEASE testdate.sh y2k.dat \
	strtof-6.128-15.1024-15.1024.rt strtof-6.128-15.1024-15.1024.tst \
	strtof-6.128-15.1024-18.16384.rt strtof-6.128-15.1024-18.16384.tst \
	strtof-6.128-15.1024-31.1024.rt strtof-6.128-15.1024-31.1024.tst \
	strtof-6.128-15.1024-33.16384.rt strtof-6.128-15.1024-33.16384.tst

install :
	: only tests in this directory : nothing to install :
