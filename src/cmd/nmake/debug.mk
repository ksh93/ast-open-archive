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

%.s : %.c .ALWAYS
	$(CC) $(CCFLAGS) -S $(>) > $(<)

%.s : %.C .ALWAYS
	$(CC) $(CCFLAGS) -S $(>) > $(<)

%.s : %.cc .ALWAYS
	$(CC) $(CCFLAGS) -S $(>) > $(<)
