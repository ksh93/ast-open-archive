/*
 * interactive debug support
 *
 * @(#)debug (AT&T Research) 1990-12-11
 *
 * *.i from *.c
 */

%.i : %.c .ALWAYS
	$(CC) $(CCFLAGS) -E $(>) > $(<)

%.i : %.C .ALWAYS
	$(CC) $(CCFLAGS) -E $(>) > $(<)

%.i : %.cc .ALWAYS
	$(CC) $(CCFLAGS) -E $(>) > $(<)

%.inc : %.c .ALWAYS
	$(CPP) $(CCFLAGS) -H $(>) > /dev/null 2> $(<)

%.inc : %.C .ALWAYS
	$(CPP) $(CCFLAGS) -H $(>) > /dev/null 2> $(<)

%.inc : %.cc .ALWAYS
	$(CPP) $(CCFLAGS) -H $(>) > /dev/null 2> $(<)

%.s : %.c .ALWAYS
	$(CC) $(CCFLAGS) -S $(>) > $(<)

%.s : %.C .ALWAYS
	$(CC) $(CCFLAGS) -S $(>) > $(<)

%.s : %.cc .ALWAYS
	$(CC) $(CCFLAGS) -S $(>) > $(<)
