/*
 * standalone versions of -lcmd builtins
 */

:PACKAGE: ast

LICENSE = since=1992,author=gsf+dgk

:ALL:

:: cmdlib.h builtin.c

BUILTIN ==
STANDALONE ==

LINKS = ln mv

CAT = $(STDCAT|"cat")
CMP = $(STDCMP|"cmp") 2>/dev/null
CP = $(STDCP|"cp")
LN = $(STDLN|"ln")

BUILTINS :COMMAND: cmd.h
	$(SED) -e '/^extern[ 	].*[ 	]b_/!d' -e 's/.*[ 	]b_\([a-z]*\).*/\1/' $(LINKS:C,.*,-e '/^&$/d',) $(*)

.INIT : .builtin

.builtin : .MAKE .VIRTUAL .FORCE
	local T
	:ALL: $(BUILTINS)
	$(BINDIR) :INSTALLDIR: $(BUILTINS)
	$(BUILTINS) : $(&$("%.c":T=SM%)) BUILTIN=b_$$(<:B) builtin.c -lcmd $(*$("%.c":T=SM%))
		$(CAT) $(*:N=*builtin.c) > $(<).c
		$(@$("%.c":T=SM%):/.*builtin.c$/$(<).c/)
		$(RM) $(RMFLAGS) $(<).c

":BUILTIN:" : .MAKE .OPERATOR
	local T P
	for T $(>)
		if T == "[-+]l*"
			$(P) : $(T)
		else
			eval
			$(T) :: STANDALONE=b_$(T) $(T).c
			end
			P := $(T)
		end
	end

:BUILTIN: look mime od pr strings tr uudecode -luu uuencode -luu what who

:INSTALL: $(BINDIR)/ln $(BINDIR)/mv

$(BINDIR)/ln :LINK: cp

$(BINDIR)/mv :LINK: cp

:: PROMO.mm RELEASE

:TEST: cut expr join od paste tail tr uuencode
