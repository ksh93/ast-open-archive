/*
 * testlib library test harness operator
 *
 *	unit :TESTLIB: library [ testhdr*.h ] [ -ltestlib* ] test*.c
 *
 * *.[ch] assumed to be in ./unit
 */

":TESTLIB:" : .MAKE .OPERATOR
	eval
	.SOURCE : $$(<)
	.SOURCE.h : ../../lib/libast/$$(<) ../../lib/libast
	$(<) :TEST: testlib $(>:O>1:C,.*\.[ch]$,$(<)/&,)
		set +x; $$(*:O=1) $(>:O=1) $(<) $$(TESTCDTFLAGS) $$(*:O>1) $$(CC) $$(CCFLAGS) $$(LDFLAGS)
	test.$$(>:N=*.h:O=1:B:/\(..\).*/\1/:-$(<:B:/\(..\).*/\1/)) : testlib $$(>:V:O>1:N!=*.c) $$$(*.ARGS:D=$(<):B:S=.c) .CLEARARGS
		set +x; $$(*:O=1) -v $(>:O=1) $(<) -DINFO $$(TESTCDTFLAGS) $$(*:O>1) $$(CC) $$(CCFLAGS) $$(LDFLAGS)
	end
