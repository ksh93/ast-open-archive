# ast nmake panic tests
#
# these tests throw nmake into a panic
#
# panic: messages are changed to FIXME:, forcing
# the tests to always fail with an ERROR mismatch

INCLUDE test.def

TEST 01 'local generated stdio.h'

	EXEC	--silent --regress --nojobs
		INPUT Makefile $'all : t.c stdio.h report
stdio.h : stdio
	sleep 1
	cp $(*) $(<)
report : .MAKE .VIRTUAL .REPEAT
	query t.c stdio.h /usr/include/stdio.h
'
		INPUT t.c $'#include "/usr/include/stdio.h"'
		INPUT stdio
		EXIT 253
		ERROR - $'
t.c : [current] .SCAN.c must=1 compiled regular scanned EXISTS

()t.c : [recent] .SCAN.c event=[current] compiled scanned state
 prerequisites: /usr/include/stdio.h 

stdio.h : [current] .LCL.INCLUDE must=2 target compiled regular triggered EXISTS
 prerequisites: stdio 
 action:
	 sleep 1
	 cp $(*) $(<)

()stdio.h : [current] .LCL.INCLUDE event=[current] force built compiled state
 prerequisites: stdio 
 action:
	 sleep 1
	 cp $(*) $(<)

make: FIXME: ("object.c", line 597) prerequisite /usr/include/stdio.h of ()t.c not compiled [No such file or directory]
make: Makefile.ms: state file not updated'
