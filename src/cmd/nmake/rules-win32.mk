/*
 * win32 specific makerules
 */

.INSTALL.libast.a = .
.INSTALL.libast-g.a = .

RC = rc
RCFLAGS = -x

.SOURCE.%.SCAN.rc : $$(*.SOURCE.rc) $$(*.SOURCE) $$(*.SOURCE.h:D:X=mfc/include include)

.SCAN.rc : .SCAN
	$(@.SCAN.c)

.ATTRIBUTE.%.rc : .SCAN.rc

%.res : %.rc
	$(RC) $(RCFLAGS) -r -fo$(<:P=N) $(.INCLUDE. rc:P=N:/^/-I/) $(>:P=N)

%.def : %.sym
	{
	echo LIBRARY $(.DLL.NAME. $(%:B) $($(%:B).VERSION):B:F=%(upper)s)
	echo
	echo SECTIONS
	echo .data READ WRITE
	echo
	echo EXPORTS
	cat $(>)
	} > $(<)
