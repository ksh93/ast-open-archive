/*
 * interactive debug support
 *
 * @(#)debug (AT&T Research) 2003-11-24
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

for .S. $(.SUFFIX.c) $(.SUFFIX.C)

	%.i : %$(.S.) .ALWAYS $$(CCISCOPE)
		$(CC) $(CCIFLAGS) -E $(.CCDEFINITIONS.) $(>) > $(<)

	%.inc : %$(.S.) .ALWAYS $$(CCISCOPE)
		$(CPP) $(CCIFLAGS) -H $(>) > /dev/null 2> $(<)

	%.s : %$(.S.) .ALWAYS $$(CCISCOPE)
		$(CC) $(CCIFLAGS) -S $(>) > $(<)

end
