:PACKAGE: ast

LICENSE = since=1995,author=gsf

:ALL:

ed :: ed.c

red :LINK: ed

grep :: grep.c LICENSE=since=1995,author=gsf+doug

egrep fgrep :LINK: grep

sed :: sed.h sed0.c sed1.c sed2.c sed3.c LICENSE=since=1995,author=gsf+doug

:: README RELEASE ed.tst grep.tst sed.tst hog.dat badmatch.dat debugmatch.c

TESTREFLAGS = -c

:TEST: ed grep sed

glob :TEST: testglob.c testglob.dat
	for i in $(*:O>1)
	do	$(*:O=1) $(TESTREFLAGS) < $i
	done

fmt :TEST: testfmt.c testfmt.dat
	for i in $(*:O>1)
	do	$(*:O=1) $(TESTREFLAGS) < $i
	done

fnmatch :TEST: testfnmatch.c testfnmatch.dat testre.dat testmatch.dat
	for i in $(*:O>1)
	do	$(*:O=1) $(TESTREFLAGS) < $i
	done

match :TEST: testmatch.c testre.dat testmatch.dat
	for i in $(*:O>1)
	do	$(*:O=1) $(TESTREFLAGS) < $i
	done

re :TEST: testre.c testre.dat testmatch.dat testsub.dat \
		zero.dat regex++.dat iso8859-1.dat perl.dat minimal.dat \
		escape.dat group.dat
	for i in $(*:O>1)
	do	$(*:O=1) $(TESTREFLAGS) < $i
	done

test.regex : test.glob - test.fnmatch - test.match - test.re

re.tgz : README.re \
		testfnmatch.c testglob.c testmatch.c testre.c \
		testglob.dat testmatch.dat testre.dat
	for f in $(*)
	do	case $f in
		*READ*)	echo "^^^$f^README" ;;
		*.c)	echo "^^$(PROTO) $(PROTOFLAGS) -dp $f^$f^${f##*/}" ;;
		*.dat)	echo "^^$(PROTO) $(PROTOFLAGS) -dp -c'#' $f^$f^${f##*/}" ;;
		*)	echo "^^^$f^${f##*/}" ;;
		esac
	done |
	$(PAX) -x tgz -wvf $(<) --filter=-
