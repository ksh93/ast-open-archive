/*
 * standalone versions of -lcmd builtins
 */

:PACKAGE: ast cmd:space

LICENSE = since=1992,author=gsf+dgk

:ALL:

:: cmdlib.h builtin.c

BUILTIN ==
DYNAMIC ==
STANDALONE ==

LINKS = cp: ln mv

CAT = $(STDCAT|"cat")
CMP = $(STDCMP|"cmp") 2>/dev/null
CP = $(STDCP|"cp")
LN = $(STDLN|"ln")
RM = $(STDRM|"rm")

BUILTINS :COMMAND: cmd.h
	$(SED) -e '/^extern[ 	].*[ 	]b_/!d' -e 's/.*[ 	]b_\([a-z]*\).*/\1/' $(LINKS:N!=*[:]:C,.*,-e '/^&$/d',) $(*)

.INIT : .builtin

.builtin : .MAKE .VIRTUAL .FORCE
	local T I
	:ALL: $(BUILTINS)
	T = -lcmd
	if "$(PACKAGE_OPTIMIZE:N=space)" && "$(T:T=F)" == "-lcmd"
		T := $(BUILTINS:O=1)
		$(T) :: BUILTIN=1 DYNAMIC=1 builtin.c -lcmd -ldll
		for I $(BUILTINS:O>1)
			$(I) :LINK: $(T)
		end
		for I $(LINKS)
			if I != "*:"
				$(I) :LINK: $(T)
			end
		end
	else
		$(BINDIR) :INSTALLDIR: $(BUILTINS)
		$(BUILTINS) : .COMMAND $(&$("%.c":T=SM%)) BUILTIN=b_$$(<:B) builtin.c $(*$("%.c":T=SM%))
			$(CAT) $(*:N=*builtin.c) > $(<).c
			$(RM) $(RMFLAGS) $(<)
			$(@$("%.c":T=SM%):/.*builtin.c$/$(<).c/)
			$(RM) $(RMFLAGS) $(<).c
		for I $(LINKS)
			if I == "*:"
				T := $(I:/.$//)
			else
				$(I) :LINK: $(T)
			end
		end
	end

":BUILTIN:" : .MAKE .OPERATOR
	local T P
	for T $(>)
		if T == "*=*"
			$(P).o : $(T)
		elif T == "[-+]l*"
			$(P) : $(T)
		else
			eval
			$(T) :: STANDALONE=b_$(T) $(T).c
			end
			P := $(T)
		end
	end

:BUILTIN: dlls LICENSE=since=2002,author=gsf -ldll look mime \
		nl LICENSE=since=2003,author=dgk asa od pr \
		strings tr uudecode -luu uuencode -luu what who

:: PROMO.mm RELEASE

:TEST: asa chown cut date expr join look nl od paste tail tr uuencode
