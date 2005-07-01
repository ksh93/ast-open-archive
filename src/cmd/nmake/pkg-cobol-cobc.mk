/*
 * open source cobol package setup
 */

COBOLFLAGS = -static $(COBOLDIALECT) -C -O
COBOLLIBRARIES = -lcob

.COBOL.INIT : .MAKE .VIRTUAL .FORCE .IGNORE
	$(COBOLLIBRARIES) : .DONTCARE
	LDFLAGS += $$(!:A=.SCAN.cob:@?$$(CC.EXPORT.DYNAMIC)??)
	LDLIBRARIES += $$(!:A=.SCAN.cob:@?$$(COBOLLIBRARIES)??)

for .S. $(.SUFFIX.cob)
	%.c %.c.h : %$(.S.) (COBOL) (COBOLDIALECT) (COBOLFLAGS) .COBOL.INIT
		$(COBOL) $(COBOLFLAGS) $(>)
end
