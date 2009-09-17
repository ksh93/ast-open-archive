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

test.dt : testcdt dttest.h +last $$(*.ARGS:D=cdt:B:S=.c) .CLEARARGS
	$(*:O=1) -v $(*:O>1) $(CC) $(CCFLAGS) $(LDFLAGS)

date :TEST: date.c date.msk date.dat datey1k.dat datey2k.dat week.dat zone.dat
	for i in $(*:N=*.dat)
	do	DATEMSK=$(*:N=*.msk:/ /:/G) TZ=EST5EDT $(*:O=1) $(TESTDATEFLAGS) < $i
	done

.SOURCE : sfio
.SOURCE.h : ../../lib/libast/sfio ../../lib/libast

sfio :TEST: testsfio sftest.h +last \
	sfio/talarm.c sfio/talign.c sfio/tappend.c sfio/tatexit.c \
	sfio/tbadargs.c sfio/tclose.c sfio/tdisc.c sfio/terrno.c \
	sfio/texcept.c sfio/tflags.c sfio/tfmt.c sfio/tgetr.c \
	sfio/thole.c sfio/tleak.c sfio/tlocale.c sfio/tlongdouble.c \
	sfio/tmmap2read.c sfio/tmode.c sfio/tmove.c sfio/tmprdwr.c \
	sfio/tmpread.c sfio/tmprocess.c sfio/tmtsafe.c \
	sfio/tmultiple.c sfio/tmwrite.c sfio/tnoseek.c \
	sfio/tnotify.c sfio/topen.c sfio/tpipe.c sfio/tpipemove.c \
	sfio/tpkrd.c sfio/tpoll.c sfio/tpool.c sfio/tpopen.c \
	sfio/tpopenrw.c sfio/tprintf.c sfio/tpublic.c \
	sfio/tputgetc.c sfio/tputgetd.c sfio/tputgetl.c \
	sfio/tputgetm.c sfio/tputgetr.c sfio/tputgetu.c \
	sfio/trcrv.c sfio/treserve.c sfio/tresize.c sfio/tscanf.c \
	sfio/tscanf1.c sfio/tseek.c sfio/tsetbuf.c sfio/tsetfd.c \
	sfio/tsfstr.c sfio/tshare.c sfio/tsize.c sfio/tstack.c \
	sfio/tstatus.c sfio/tstdio.c sfio/tstkpk.c sfio/tstring.c \
	sfio/tswap.c sfio/tsync.c sfio/ttell.c sfio/ttmp.c \
	sfio/ttmpfile.c sfio/tungetc.c sfio/twalk.c sfio/twc.c \
	sfio/twchar.c sfio/twhole.c sfio/twrrd.c
		set +x; $(*:O=1) $(TESTSFIOFLAGS) $(*:O>1) $(CC) $(CCFLAGS) $(LDFLAGS)

test.sf : testsfio sftest.h +last $$(*.ARGS:D=sfio:B:S=.c) .CLEARARGS
	$(*:O=1) -v $(*:O>1) $(CC) $(CCFLAGS) $(LDFLAGS)

.SOURCE : vmalloc
.SOURCE.h : ../../lib/libast/vmalloc

vmalloc :TEST: testvmalloc vmtest.h +last \
	vmalloc/talign.c vmalloc/tcompact.c vmalloc/tlast.c vmalloc/tmalloc.c \
	vmalloc/tmrandom.c vmalloc/tpool.c vmalloc/trandom.c vmalloc/tresize.c \
	vmalloc/tsmall.c vmalloc/tstat.c vmalloc/twalk.c
		set +x; $(*:O=1) $(TESTVMALLOCFLAGS) $(*:O>1) $(CC) $(CCFLAGS) $(LDFLAGS)

test.vm : testvmalloc vmtest.h +last $$(*.ARGS:D=vmalloc:B:S=.c) .CLEARARGS
	$(*:O=1) -v $(*:O>1) $(CC) $(CCFLAGS) $(LDFLAGS)

:: RELEASE NOTES testdate.sh y2k.dat \
	strtof-6.37.38-15.307.308-15.307.308.rt   strtof-6.37.38-15.307.308-15.307.308.tst \
	strtof-6.37.38-15.307.308-18.4931.4932.rt strtof-6.37.38-15.307.308-18.4931.4932.tst \
	strtof-6.37.38-15.307.308-31.291.308.rt   strtof-6.37.38-15.307.308-31.291.308.tst \
	strtof-6.37.38-15.307.308-31.307.308.rt   strtof-6.37.38-15.307.308-31.307.308.tst \
	strtof-6.37.38-15.307.308-33.4931.4932.rt strtof-6.37.38-15.307.308-33.4931.4932.tst

install :
	: only tests in this directory : nothing to install :
