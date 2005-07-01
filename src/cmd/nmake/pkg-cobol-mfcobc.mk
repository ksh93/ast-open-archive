/*
 * microfocus cobol package setup
 */

COBOLFLAGS = $(COBOLDIALECT) -O
COBOLLIBRARIES = -lmfcob

.COBOL.INIT : .MAKE .VIRTUAL .FORCE .IGNORE
	$(COBOLLIBRARIES) : .DONTCARE
	.CCLD.ORIGINAL := $(CCLD)
	CCLD = $(!:A=.SCAN.cob:@?$$(COBOL)?$$(.CCLD.ORIGINAL)?)
	LDFLAGS += $$(!:A=.SCAN.cob:@?$$(CC.EXPORT.DYNAMIC)??)
	LDLIBRARIES += $$(!:A=.SCAN.cob:@?$$(COBOLLIBRARIES)??)

for .S. $(.SUFFIX.cob)
	%.o : %$(.S.) (COBOL) (COBOLDIALECT) (COBOLFLAGS) .COBOL.INIT
		$(COBOL) -c $(COBOLFLAGS) $(>)
end
