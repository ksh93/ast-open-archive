/*
 * interactive debug support
 *
 * @(#)debug (AT&T Research) 2003-02-28
 *
 * *.i from *.c
 */

CCIFLAGS = $(CC.ALTPP.FLAGS) $(CCFLAGS:N=-[DIU]*) $(&$(<:B:S=.o):T=D)
CCISCOPE =  $(~$(<:B:S=.o):N=*=*:Q)

%.i : %.c .ALWAYS $$(CCISCOPE)
	$(CC) $(CCIFLAGS) -E $(>) > $(<)

%.i : %.C .ALWAYS $$(CCISCOPE)
	$(CC) $(CCIFLAGS) -E $(>) > $(<)

%.i : %.cc .ALWAYS $$(CCISCOPE)
	$(CC) $(CCIFLAGS) -E $(>) > $(<)

%.inc : %.c .ALWAYS $$(CCISCOPE)
	$(CPP) $(CCIFLAGS) -H $(>) > /dev/null 2> $(<)

%.inc : %.C .ALWAYS $$(CCISCOPE)
	$(CPP) $(CCIFLAGS) -H $(>) > /dev/null 2> $(<)

%.inc : %.cc .ALWAYS $$(CCISCOPE)
	$(CPP) $(CCIFLAGS) -H $(>) > /dev/null 2> $(<)

%.s : %.c .ALWAYS $$(CCISCOPE)
	$(CC) $(CCIFLAGS) -S $(>) > $(<)

%.s : %.C .ALWAYS $$(CCISCOPE)
	$(CC) $(CCIFLAGS) -S $(>) > $(<)

%.s : %.cc .ALWAYS $$(CCISCOPE)
	$(CC) $(CCIFLAGS) -S $(>) > $(<)
