/*
 * interactive debug support
 *
 * @(#)debug (AT&T Research) 1990-12-11
 *
 * *.i from *.c
 */

CCIFLAGS = $(CCFLAGS:N=-[DIU]*) $(&$(<:B:S=.o):T=D)

%.i : %.c .ALWAYS
	$(CC) $(CCIFLAGS) -E $(>) > $(<)

%.i : %.C .ALWAYS
	$(CC) $(CCIFLAGS) -E $(>) > $(<)

%.i : %.cc .ALWAYS
	$(CC) $(CCIFLAGS) -E $(>) > $(<)

%.inc : %.c .ALWAYS
	$(CPP) $(CCIFLAGS) -H $(>) > /dev/null 2> $(<)

%.inc : %.C .ALWAYS
	$(CPP) $(CCIFLAGS) -H $(>) > /dev/null 2> $(<)

%.inc : %.cc .ALWAYS
	$(CPP) $(CCIFLAGS) -H $(>) > /dev/null 2> $(<)

%.s : %.c .ALWAYS
	$(CC) $(CCIFLAGS) -S $(>) > $(<)

%.s : %.C .ALWAYS
	$(CC) $(CCIFLAGS) -S $(>) > $(<)

%.s : %.cc .ALWAYS
	$(CC) $(CCIFLAGS) -S $(>) > $(<)
