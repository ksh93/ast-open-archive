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

match :TEST: testmatch.c testmatch.dat
	$(*:O=1) < $(*:O=2)

re :TEST: testre.c testre.dat
	$(*:O=1) $(TESTREFLAGS) < $(*:O=2)
