/*
 * :PACKAGE: tcl support
 */

TCLROOT = $(PACKAGE_tcl)

":TCL_LIBRARY:" : .MAKE .OPERATOR
	local A B L T
	A := $(<:O=1)
	if ! ( V = "$(<:O=2)" )
		V := 1.0
	end
	$(A) $(V) :LIBRARY: $(>)
	if L = "$(.DLL.NAME. $(A) $(V))"
		.INSTALL.$(.LIB.NAME. $(A) $(V)) := .
		B := $(A:/./&/U)
		T := $(B)$(L:/[^.]*//)
		$$(TCLROOT)/lib/$(B)/$(T) :INSTALL: $(L)
		if T == "*.$(V)"
			$$(TCLROOT)/lib/$(B)/$(T:/\.$(V)$//) :INSTALL: $(L)
		end
		$$(TCLROOT)/lib/$(B)/pkgIndex.tcl :INSTALL: $(B)Index.tcl
		eval
		$(B)Index.tcl :
			echo 'package ifneeded $(B) $(V) [list load [file join $dir $(T)] $(B)]' > $$(<)
		end
	end
