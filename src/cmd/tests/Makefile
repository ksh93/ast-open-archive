/*
 * regression tests for library functions
 * `tests' runs all tests visible from .. with output in tests.out
 *
 * test management is still in the design phase
 */

:PACKAGE: ast

LICENSE = since=1999,author=gsf

:TEST: astconf opt

date :TEST: date.c date.msk date.dat datey1k.dat datey2k.dat
	for i in $(*:N=*.dat)
	do	DATEMSK=$(*:N=*.msk:/ /:/G) TZ=EST5EDT $(*:O=1) $(TESTDATEFLAGS) < $i
	done

.SOURCE : sfio ../../lib/libast/sfio ../../lib/libast

sfio :TEST: testsfio sftest.h +last \
	talarm.c talign.c tappend.c tatexit.c tclose.c tdisc.c terrno.c \
	texcept.c tflags.c tgetr.c thole.c tleak.c tlongdouble.c tmmap2read.c \
	tmode.c tmove.c tmprocess.c tmtsafe.c tmultiple.c tmwrite.c \
	tnoseek.c tnotify.c topen.c tpipe.c tpipemove.c \
	tpkrd.c tpoll.c tpool.c tpopen.c tpopenrw.c tprintf.c \
	tpublic.c tputgetc.c tputgetd.c tputgetl.c tputgetm.c tputgetr.c \
	tputgetu.c trcrv.c treserve.c tscanf.c tseek.c \
	tsetbuf.c tsetfd.c tshare.c tsize.c tstack.c tstatus.c \
	tstdio.c tstkpk.c tstring.c tswap.c tsync.c ttell.c ttmp.c \
	ttmpfile.c tungetc.c twhole.c twrrd.c
		set +x; $(*:O=1) $(TESTSFIOFLAGS) $(*:O>1) $(CC) $(CCFLAGS)

test.sf : testsfio sftest.h +last $$(*.ARGS:D:B:S=.c) .CLEARARGS
	$(*:O=1) -v $(*:O>1) $(CC) $(CCFLAGS)

tests : settings tests.out

tests.out :
	rm -f $(MAKEFILE:B:S=.ml)
	(
		cd ..
		$(MAKE) $(-) 'dontcare test' recurse test
	) > $(<) 2>&1
	failed=$( egrep -c 'FAIL|failed|should fail| expecting ' $(<) || : )
	case $failed in
	1)	plural= ;;
	*)	plural=s ;;
	esac
	{
		echo
		echo TEST $failed error$plural
	} >> $(<)

settings : .MAKE .VIRTUAL .FORCE
	set keepgoing nowritestate

:: RELEASE y2k.dat

install :
	: only tests in this directory : nothing to install :
