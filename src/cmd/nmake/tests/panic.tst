# ast nmake panic tests
#
# these tests may throw nmake into a panic
#
# panic messages still to be fixed are listed
# ad FIXME: instead of panic: so they show up
# as regression test failures

INCLUDE test.def

TEST 01 'local generated stdio.h'

	EXEC	--silent --regress --nojobs
		INPUT Makefile $'all : t.c stdio.h
stdio.h : stdio
	sleep 1
	cp $(*) $(<)
'
		INPUT t.c $'#include "/usr/include/stdio.h"'
		INPUT stdio

	EXEC	--silent --regress --nojobs
