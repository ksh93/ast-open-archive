/*
 * interactive debug support
 *
 * @(#)debug (AT&T Research) 2003-06-11
 *
 * *.i from *.c
 */

CCIFLAGS = $(CC.ALTPP.FLAGS) $(CCFLAGS:N=-[DIU]*) $(&$(<:B:S=.o):T=D)
CCISCOPE =  $(~$(<:B:S=.o):N=*=*:Q)

.CCDEFINITIONS. : .FUNCTION
	if ! "$(nativepp)" || "$(CC.DIALECT:N=LIBPP)"
		return -D-d
	end
	return $(CC.DIALECT:N=-dD)

%.i : %.c .ALWAYS $$(CCISCOPE)
	$(CC) $(CCIFLAGS) -E $(.CCDEFINITIONS.) $(>) > $(<)

%.i : %.C .ALWAYS $$(CCISCOPE)
	$(CC) $(CCIFLAGS) -E $(.CCDEFINITIONS.) $(>) > $(<)

%.i : %.cc .ALWAYS $$(CCISCOPE)
	$(CC) $(CCIFLAGS) -E $(.CCDEFINITIONS.) $(>) > $(<)

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
