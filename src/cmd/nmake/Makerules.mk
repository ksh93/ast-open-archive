rules

/*
 * Glenn Fowler
 * AT&T Research
 *
 * default make base rules
 *
 * conventions:
 *
 *	internal make rule names match the RE \.[.A-Z][.A-Z0-9]*
 *	internal make variable names match the RE \.[.A-Z][.A-Z0-9]*\.
 *	internal make operator names match the RE :[A-Z]*:
 *	use $(tmp) (10 chars max) to generate shell tmp file names
 *	every (non-builtin shell) command should use a make variable
 *	the flags for command $(XYZ) are $(XYZFLAGS)
 */

.ID. = "@(#)$Id: Makerules (AT&T Research) 2004-02-29 $"

/*
 * handy attributes
 */

.FUNCTION : .USE .ATTRIBUTE .MAKE .FUNCTIONAL .VIRTUAL .FORCE .REPEAT

.MAKE.OPTIONS.SET. =

.MAKE.OPTIONS. : .FUNCTION
	local VAR VAL
	for VAR $(%)
		if VAR == "no*"
			VAR := $(VAR:/no//)
			VAL :=
		elif VAR == "*=*"
			VAL := $(VAR:/[^=]*=//)
			VAR := $(VAR:/=.*//)
		else
			VAL := 1
		end
		if "$(.MAKE.OPTIONS.SET.:N=$(VAR))" || ! "$($(VAR))"
			.MAKE.OPTIONS.SET. += $(VAR)
			$(VAR) := $(VAL)
		end
	end

/*
 * made when the base rules are loaded
 */

.MAKERULES.LOAD : .MAKE .VIRTUAL .FORCE .IMMEDIATE
	: $(.MAKE.OPTIONS. $(MAKE_OPTIONS))
	if viewverify && ! "$(*.VIEW:O=2)"
		error $(viewverify) viewpath not set
	end
	if "$(-mam:N=(regress|static)*)"
		.MAMROOT. := $(PWD:C@.*/src/@/src/@:C@/[^/]*@/..@G:C@/@@)
		.MAKE : .MAM.LOAD
	end
	PAXFLAGS &= $$(MAKEPATH:C@:@ @G:N!=...*:C@.*@-s\@&/*\@\@@)
	.MAKE : .PROBE.LOAD

/*
 * binding directory lists
 */

PKGDIRS = $(LIBDIR) $(*.VIEW:X=$(VROOT)/$(LIBDIR:B:S)) $(MAKERULESPATH:/:/ /G::D)
LCLDIRS = /usr/local/arch/$(_hosttype_):/usr/common:/usr/local
OPTDIRS = $(INSTALLROOT)/opt:/usr/add-on:/usr/addon:/usr/contrib:$(LCLDIRS):/opt:/home
STDDIRS = /:/usr
USRDIRS = $(LCLDIRS):$(STDDIRS)

/*
 * common directories
 */

INSTALLROOT = $(HOME)

BINDIR = $(INSTALLROOT)/bin
DLLDIR = $(CC.DLL.DIR)
ETCDIR = $(INSTALLROOT)/etc
FUNDIR = $(INSTALLROOT)/fun
INCLUDEDIR = $(INSTALLROOT)/include
LIBDIR = $(INSTALLROOT)/lib
LOCALEDIR = $(INSTALLROOT)/lib/locale
MANDIR = $(INSTALLROOT)/man/man
SHAREDIR = $(INSTALLROOT)/share
TMPDIR = /usr/tmp

/*
 * common files
 */

CATALOG = $(.CATALOG.NAME.)
HTMLINITFILES = 2HTML:$(HOME)/.2html
LICENSE =
LICENSEFILE = LICENSE
LICENSEFILES = $(LICENSEFILE):$(.PACKAGE.:D=$(LIBDIR)/package:B:S=.lic)

/*
 * recursion defaults
 */

MAKESKIP = *-*

/*
 * language processor suffix equivalences
 */

.SUFFIX.c = .c
.SUFFIX.HEADER.c = .h
.SUFFIX.C = .C .cc .cpp .cxx .c++
.SUFFIX.cob = .cob .COB .cbl .CBL
.SUFFIX.HEADER.cob = .cpy .CPY
.SUFFIX.f = .f .F
.SUFFIX.r = .r .R

.COMMAND.CC = g++ CC c++ cxx

/*
 * makerules control variables
 */

.make.options. =

allstatic = 1
ancestor =
ancestor_list = $(ancestor_source) .SOURCE.a lib .SOURCE.h include
arclean =
cctype =
cleanignore =
clobber =
compare = 1
debug =
force_shared =
instrument =
ldscript =
link =
nativepp =
official_out = OFFICIAL
output = $(PWD:N=*[0-9].[0-9]*|*-$(VERSION):?$$(PWD:B:S:/---*\([^-]*\)/-\1/)?$$(VERSION:Y%$$(PWD:B:S)-$$(VERSION)%$$(PWD:B)%)?)
package_local = win32.*
physical =
prefixinclude =
preserve = $(CC.SUFFIX.SHARED:?$(CC.PREFIX.SHARED)*$(CC.SUFFIX.SHARED).*??)|$(CC.SUFFIX.DYNAMIC:?$(CC.PREFIX.DYNAMIC)*$(CC.SUFFIX.DYNAMIC)??)
profile =
recurse = 1
recurse_enter =
recurse_exit =
select =
skeleton =
static =
strip =
threads =
tmp = ${COTEMP}
variants =
viewverify =

/*
 * $(sh echo hello) may be handier than :COMMAND:
 * $(.sh. ...) will not be frozen in .mo
 */

.sh. : .SPECIAL .VIRTUAL .FUNCTIONAL .FORCE .REPEAT .ALWAYS .LOCAL
	set -
	$(%)

/*
 * CC.* are candidates for probe
 * the ones here are currently the same for all systems
 */

CC.LIB.TYPE = $(-mam:N=static*,port*:??$(instrument:@Y%$(instrument:B:/\(...\).*/-\1/)%%) $(CCFLAGS:N=-p|-pg) $(LDFLAGS:N=-O[1-9]) $(CCFLAGS:N=-g*([0-9]))?)

/*
 * $(INSTRUMENT_$(instrument:O=1:B)) is eval'd in .MAKEINIT
 */

INSTRUMENT_app = : $(.INSTRUMENT.app)
INSTRUMENT_insight = command=CC root=INSIGHT bin=bin.$(_hosttype_)
INSTRUMENT_purecov = command=CCLD root=PURE_HOME CCFLAGS=$(CC.DEBUG)
INSTRUMENT_purify = command=CCLD root=PURE_HOME
INSTRUMENT_quantify = command=CCLD root=PURE_HOME CCFLAGS=$(CC.DEBUG)
INSTRUMENT_sentinel = command=CCLD root=SE_HOME CCFLAGS=$(CC.DEBUG)

.INSTRUMENT.app : .FUNCTION
	if ! ( APP = "$(PATH:/:/ /G:X=../lib/app/cpp:P=X:O=1)" )
		error 3 app not found
	end
	CPP = $(APP)
	CPPFLAGS = $(CCFLAGS:N=-[DIU]*)
	LDLIBRARIES += -lapp
	.SOURCE.a : $(APP:D:D)

.INSTRUMENT.notfound : .MAKE .FUNCTIONAL .VIRTUAL .FORCE
	error 3 $(instrument) not found

/*
 * standard versions to avoid ETXTBSY during installation
 */

for .X. cat cmp cp ln mv rm
	eval
	STD$(.X.:F=%(upper)s) := $(STDDIRS:/:/ /G:X=bin/$(.X.):P=X:O=1)
	end
end
.X. := $(STDLN)
STDLN := $$(_feature_:N=ln:?$(.X.)?$$(STDCP)?)

/*
 * action related symbols
 */

AR = $(CC.AR)
ARFLAGS = r
AS = as
if "$(PATH:/:/ /G:X=awk:P=X)"
AWK = awk
elif "$(PATH:/:/ /G:X=nawk:P=X)"
AWK = nawk
elif "$(PATH:/:/ /G:X=gawk:P=X)"
AWK = gawk
else
AWK = awk
end
BISON = bison
BISONFLAGS = -d
cc = cc
CC = cc
CCFLAGS = $(CC.OPTIMIZE)
CCLD = $(CC)
CCLDFLAGS = $(CCFLAGS:N!=-[DIU]*:@C@$(CC.ALTPP.FLAGS)@@) $(LDFLAGS)
CHGRP = chgrp
CHMOD = chmod
CHOWN = chown
CMP = cmp
CMPFLAGS = -s
COBOL = cobc
COBOLFLAGS = -static -std=mvs -C
COBOLLIBRARIES = -lcob
COBOLMAIN = -fmain
CP = cp
CPIO = cpio
CPP = $(MAKEPP)
CPPFLAGS = $(CCFLAGS:N=-[DIU]*)
CPR = cp
CPRFLAGS = -pr

if "$(PATH:/:/ /G:X=ctags:P=X)"
	CTAGS = ctags
end

DIFF = diff
DIFFFLAGS = -u
EGREP = egrep
F77 = f77
FLEX = flex
FLEXFLAGS = -l
GREP = grep
HOSTCC = $(CC)
IFFE = iffe
IFFESRCDIR = features
IFFEGENDIR = FEATURE
IFFEFLAGS = -v
IFFECC = $(CC)
IFFECCFLAGS = $(CCFLAGS)
IFFELDFLAGS = $(LDFLAGS)
IGNORE = ignore
LD = $(CC.LD)
LDFLAGS =
LDSHARED = $(CC.SHARED.LD|CC.LD)
LDRUNPATH =
if ! "$(PATH:/:/ /G:X=lex:P=X)" && "$(PATH:/:/ /G:X=flex:P=X)"
LEX = $(FLEX)
LEXFLAGS = $(FLEXFLAGS)
else
LEX = lex
LEXFLAGS =
end

.T. = $(USRDIRS:/:/ /G:X=lib/lint) $(USRDIRS:/:/ /G:X=lib/cmplrs/cc) $(USRDIRS:/:/ /G:X=ccs/lib)
if .T. = "$(.T.:X=lint1:P=X:O=1)"
LINTLIB := $(.T.:D)
else
LINTLIB = /usr/lib
end
LINTLIBRARIES = libc$(CC.SUFFIX.ARCHIVE)
LINT1 = $(LINTLIB)/lint1
LINT2 = $(LINTLIB)/lint2
if ! "$(_release_:N=V)"
LINTFLAGS = -bh
end

LN = $(_feature_:N=ln:?ln?$(CP)?)
LPR = lpr

if "$(PATH:/:/ /G:X=lprof:P=X)"
LPROF = lprof
end

LS = ls
M4 = m4

if "$(PATH:/:/ /G:X=mcs:P=X)"
MCS = mcs
MCSFLAGS = -d
end

MKDIR = mkdir
MM2HTML = mm2html
MM2HTMLINFO = $(LICENSEFILES:/:/ /G:T=F:O=1)
MM2HTMLINIT = $(HTMLINITFILES:/:/ /G:T=F:O=1)
MM2HTMLFLAGS = $(MM2HTMLINFO:@Q:/^/-l /) $(MM2HTMLINIT:@Q:/^/-h /) $(LICENSE:@/^/-o /:@Q)
MV = mv
if "$(PATH:/:/ /G:X=nawk:P=X)"
NAWK = nawk
elif "$(PATH:/:/ /G:X=gawk:P=X)"
NAWK = gawk
else
NAWK = awk
end
NM = $(CC.NM)
NMEDIT = $(CC.NMEDIT) -e '/^$(CC.PREFIX.SYMBOL)_STUB_/d' -e '/$(CC.PREFIX.SYMBOL)_already_defined$/d'
NMFLAGS = $(CC.NMFLAGS)
PACKAGE =
PACKAGE_IGNORE =
PACKAGE_PATH = $(PACKAGE)
PACKAGE_LOCAL = $(CC.HOSTTYPE:N=$(package_local:/^0$/*/):??$(.PACKAGE.:O=1)_?)
PAX = pax
PPCC = $(MAKERULESPATH:/:/ /G:X=ppcc:P=X:O=1)
PPCCFLAGS =
PR = pr
PROTO = proto
PROTOEDIT =
PROTOFLAGS = -s $(LICENSEFILES:/:/ /G:T=F:O=1:$(PROTOEDIT):/^/-l /) $(LICENSE:@/^/-o /:@Q)
RANLIB = $(IGNORE) ranlib
REGRESS = regress
REGRESSFLAGS =
RM = rm
RMFLAGS = -f
RMRECURSEFLAGS = -r
SED = sed
SHAR = shar
SHELLMAGIC = $(CC.SHELLMAGIC)
SILENT = silent
SORT = sort
STRIP = $(CC.STRIP)
STRIPFLAGS = $(CC.STRIP.FLAGS)
TAR = tar
TARFLAGS = v
VARIANT =
VARIANTID =

if "$(PATH:/:/ /G:X=vgrind:P=X)"
VGRIND = vgrind
end

if ! "$(PATH:/:/ /G:X=yacc:P=X)" && "$(PATH:/:/ /G:X=bison:P=X)"
YACC = $(BISON)
YACCFLAGS = -y $(BISONFLAGS)
else
YACC = yacc
YACCFLAGS = -d
end

/*
 * special symbols
 */

.BUILT. = $(...:T=XU:T=F:P=L:N!=/*$(.INSTALL.LIST.:@/ /|/G:/^./|&/)$(VROOT:?|$(VROOT)/*??)$(-global:@/:/|/G:/^./|&/)|$(CATALOG).msg:T=G)
.CLOBBER. = $(".":L=*.([it]i|l[hn])) core
.FILES. = $(LICENSEFILE)
.MANIFEST.FILES. = $(*.COMMON.SAVE:T=F) $(.SELECT.:A!=.ARCHIVE|.COMMAND|.OBJECT)
.MANIFEST. = $(.MANIFEST.FILES.:P=C:H=U)
.SOURCES. = $(.SELECT.:A=.REGULAR:A!=.ARCHIVE|.COMMAND|.OBJECT)

/*
 * stop expansion of certain state variables
 */

(AR) (ARFLAGS) (AS) (ASFLAGS) (CPP) (CC) (CCFLAGS) (CCLD) \
	(CCLDFLAGS) (COATTRIBUTES) (COBOL) (COBOLFLAGS) (F77) (F77FLAGS) \
	(IFFE) (IFFEFLAGS) (LD) (LDFLAGS) (LDLIBRARIES) (LDSHARED) \
	(LEX) (LEXFLAGS) (M4) (M4FLAGS) (SHELLMAGIC) (YACC) (YACCFLAGS) \
	: .PARAMETER

/*
 * mark actions that operate on built objects
 */

.ONOBJECT : .ATTRIBUTE

/*
 * file attributes
 */

.OBJECT : .ATTRIBUTE

include "Scanrules.mk"

/*
 * pattern association attributes
 */

.INSTALL. :
.INSTALL.%.COMMAND : $$(BINDIR)
.INSTALL.%.ARCHIVE : $$(LIBDIR)

/*
 * naming conventions
 */

.CATALOG.NAME. : .FUNCTION
	local N
	if ID == "+([A-Za-z0-9_])"
		N := $(ID)
		if PWD == "*/lib/*"
			N := lib$(N)
		end
	elif "$(PWD:B)" == "cc-*"
		N := $(PWD:D::B)
	elif "$(PWD:D::B)" == "?*lib"
		N := $(PWD:D::B:/lib$//)$(PWD:B)
	else
		N := $(PWD:B)
	end
	return $(N)

.DLL.NAME. : .FUNCTION .PROBE.INIT
	local L
	if "$(CC.DLL)"
		if "$(.SHARED.DEF.:A=.TARGET)"
			L := $($(.SHARED.DEF.) - $(%))
		else
			L := $(CC.PREFIX.SHARED)$(%:O=1)$(CC.SUFFIX.SHARED)
			if "$(%:O=2)" == "[0-9]*"
				L := $(L).$(%:O=2)
			end
		end
	end
	return $(L)

.LIB.NAME. : .FUNCTION .PROBE.INIT
	return $(CC.PREFIX.ARCHIVE)$(%:O=1)$(CC.LIB.TYPE:O=1)$(CC.SUFFIX.ARCHIVE)

/*
 * bind overrides
 */

+ldl : .VIRTUAL

-ldl -lm : .DONTCARE

.ARPROFILE. : .FUNCTION
	local P T
	if ! .NO.ARPROFILE
		for P $(CC.LIB.TYPE)
			/* libX-P.a or libX_P.a or libP/libX.a */
			T := $(CC.PREFIX.ARCHIVE)$(%)$(P)$(CC.SUFFIX.ARCHIVE)
			if ! "$(T:A=.TARGET)" && ! "$(T:T=F)"
				T := $(CC.PREFIX.ARCHIVE)$(%)(P:/-/_/)$(CC.SUFFIX.ARCHIVE)
				if ! "$(T:A=.TARGET)" && ! "$(T:T=F)"
					T := lib$(P:/-//)/$(CC.PREFIX.ARCHIVE)$(%)$(CC.SUFFIX.ARCHIVE)
				end
			end
			if "$(T:A=.TARGET)" || "$(T:T=F)"
				return $(T)
			end
		end
	end

.BIND.+l% : .FUNCTION
	local A B L
	B := $(%:/+l//)
	A := $(CC.PREFIX.ARCHIVE)$(B)$(CC.SUFFIX.ARCHIVE)
	if "$(-mam:N=static*,port*)"
		if L = "$(A:A=.TARGET)"
			return $(L)
		end
		L := -l$(B)
		if "$(%:A=.DONTCARE)"
			$(L) : .DONTCARE
		end
		return + $(L:T=F)
	end
	if ( L = "$(.ARPROFILE. $(B))" )
		return $(L)
	end
	if ( L = "$(A:A=.TARGET)" )
		return $(L)
	end
	if ! "$(.LIBRARY.LIST.)" || "$(.PACKAGE.LIBRARY. $(B))" != "-l"
		if ( L = "$(A:T=F)" )
			return $(L)
		end
	end
	A := $(%:/+l/-l/)
	if ! ( L = "$(A:T=F)" )
		if "$(A:A=.DONTCARE)"
			$(%) : .DONTCARE
		end
	end
	return $(L)

.BIND.-l% : .FUNCTION
	local A B D T V
	B := $(%:/[-+]l//)
	if "$(-mam:N=static*,port*)" && ! .BIND.REAL.
		if "$(%)" != "-l+([a-zA-Z0-9_])"
			$(%) : .DONTCARE
			return + $(%)
		end
		.BIND.REAL. = 1
		A := $(.BIND.-l% $(%):T=F)
		.BIND.REAL. =
		V := ${mam_lib$(B)}
		if "$(%:A=.DONTCARE)" && "$(A:N=/*)"
			print -um bind -l$(B) dontcare
			return + $(V)
		elif T = "$(%:/-l\(.*\)/$(CC.PREFIX.ARCHIVE)\1$(CC.SUFFIX.ARCHIVE)/:A=.TARGET)"
			return $(T)
		else
			$(V) : .DONTCARE
			return $(V:T=F)
		end
	end
	if ( T = "$(.ARPROFILE. $(B))" )
		return $(T)
	end
	if ( static || "$(.PACKAGE.LIBRARY. $(B))" == "+l" ) && "$(<<)" != ".BIND.+l%"
		V := $(CC.PREFIX.ARCHIVE)$(B)$(CC.SUFFIX.ARCHIVE)
		if ( T = "$(V:A=.TARGET)" )
			return $(T)
		end
		if ( T = "$(V:T=F)" ) && "$(T:D)" != "$(CC.STDLIB:/ /|/G)" && "$(T:D)" != "/usr/($(.PACKAGE.:/ /|/G))"
			return $(T)
		end
	end
	if "$(CC.DIALECT:N=DYNAMIC)" && ( "$(CCLDFLAGS:N=$(CC.DYNAMIC))" || ! "$(CCLDFLAGS:N=$(CC.STATIC))" )
		if CC.SUFFIX.DYNAMIC && CC.SUFFIX.SHARED
			T := $(*$(B):N=*$(CC.SUFFIX.DYNAMIC))
			if "$(CC.PREFIX.DYNAMIC)" != "$(CC.PREFIX.SHARED)"
				if CC.PREFIX.DYNAMIC
					T := $(T:D:B=$(T:B:/$(CC.PREFIX.DYNAMIC)/$(CC.PREFIX.SHARED)/):S=$(CC.SUFFIX.SHARED))
				else
					T := $(T:D:B=$(T:B:/^/$(CC.PREFIX.SHARED)/):S=$(CC.SUFFIX.SHARED))
				end
			else
				T := $(T:D:B:S=$(CC.SUFFIX.SHARED))
			end
			if "$(T:A=.TARGET)"
				return $(T)
			end
			if ( T = "$(*$(B):N=-l$(B):/-l//)" )
				T := $(T)$($(B).VERSION:/[^0-9]//G)
				if ( T = "$(T:B:S=$(CC.SUFFIX.SHARED):A=.TARGET)" )
					return $(T)
				end
			end
		end
		if ( T = "$(%:/-l\(.*\)/$(CC.PREFIX.ARCHIVE)\1$(CC.SUFFIX.ARCHIVE)/:A=.TARGET)" )
			if "$(PACKAGE_OPTIMIZE:N=space)" && "$(CC.SUFFIX.SHARED)"
				if ( V = "$(*$(B):N=*$(CC.SUFFIX.SHARED)*:A=.TARGET)" )
					A := $(V:/\(.*$(CC.SUFFIX.SHARED)\).*/\1/)
					if "$(A)" != "$(V)"
						$(A) : $(V)
							$(LN) -f $(*) $(<)
						V := $(A)
					end
					.MAKE : $(V)
					$(%) : .NULL
					return $(%)
				end
			end
			return $(T)
		else
			local H I J L P S
			L := $(%)
			V :=
			if P = "$(PACKAGE_$(B)_VERSION)"
				P := ?([-.])$(P)
			end
			if T = "$(PACKAGE_$(B)_LIB)"
				if ! "$(.PACKAGE.stdlib:N=$(T))"
					.SOURCE.a : $(T)
				end
			end
			for J $(CC.PREFIX.SHARED) ''
				for I $(P) ''
					if "$(CC.SUFFIX.DYNAMIC)"
						if T = "$(*.SOURCE.%.ARCHIVE:L>$(CC.PREFIX.ARCHIVE)$(B)$(I)$(CC.SUFFIX.ARCHIVE)|$(J)$(B)$(I)$(CC.SUFFIX.SHARED))"
							if "$(CC.SUFFIX.SHARED)"
								if T != "*$(CC.SUFFIX.SHARED)" && T == "*$(CC.SUFFIX.ARCHIVE)"
									if S = "$(T:D:B=$(J)$(T:B:/$(CC.PREFIX.ARCHIVE)//):S=$(CC.SUFFIX.SHARED):T=F)"
										T := $(S)
									end
								end
							end
							T := $(T:T=F)
							$(%) $(T) : .ARCHIVE $(force_shared:@??.IGNORE?)
							return $(T)
						end
					elif "$(J)" || ! "$(CC.PREFIX.SHARED)"
						if "$(CC.SUFFIX.SHARED)"
							S = |$(CC.SUFFIX.SHARED)
							if "$(CC.SUFFIX.SHARED)" != "$(CC.SUFFIX.OBJECT)"
								H = -
								if "$(CC.DIALECT:N=VERSION)"
									S := $(S)$$(V)*
								end
							end
						end
						if "$(CC.SUFFIX.STATIC)"
							S := $(S:V)|$(CC.SUFFIX.STATIC)
						end
						while 1
							T := $(*.SOURCE.%.ARCHIVE:L>$(L:/-l\(.*\)/$(CC.PREFIX.ARCHIVE)\1/)$(I)@($(V)$(CC.SUFFIX.ARCHIVE)$(S)))
							if T
								if ! "$(CC.SUFFIX.SHARED)" || T != "*$(CC.SUFFIX.SHARED)"
									if T == "*$(CC.SUFFIX.ARCHIVE)"
										return $(T)
									end
									if "$(CC.SUFFIX.STATIC)" && T == "*$(CC.SUFFIX.STATIC)"
										H = -
									end
								end
								$(%) $(T) : .ARCHIVE $(force_shared:@??.IGNORE?)
								return $(H) $(T:T=F)
							end
							if ! "$(L:N=*+(.+([0-9])))"
								break
							end
							T := $(L:/\.[0-9]*$//)
							V := $(L:/$(T)//)$(V)
							L := $(T)
						end
					end
				end
			end
		end
	else
		T := $(CC.PREFIX.ARCHIVE)$(B)$(CC.SUFFIX.ARCHIVE)
		if "$(%:A=.DONTCARE)"
			$(T) : .DONTCARE
		end
		return $(T)
	end
	if ! "$(.BOUND.$(B))"
		.BOUND.$(B) := 1
		if T = "$(CC.REQUIRE.$(B))"
			$(%) : .DONTCARE
			return $(T)
		elif T = "$(B:B:S=.req:T=F)"
			$(%) : .DONTCARE
			return $(T:T=I)
		else
			T := lib/$(B)
			$(T) : .ARCHIVE
			if ! "$(T:T=F)"
				T := $(MAKELIB:D)/$(T)
			end
			if "$(T:T=F)"
				T := $(T:T=I)
				if ! "$(T:N=[-+]l$(B))"
					$(%) : .DONTCARE
				end
				return $(T)
			end
		end
	end

.BIND.-u% : .FUNCTION
	$(%) : .NULL

.BIND.$"{"mam_lib%"}" : .FUNCTION
	if "$(-mam:N=static*,port*)"
		local L T
		L := $(%:/\${mam_lib\(.*\)}/\1/)
		T := $(CC.PREFIX.ARCHIVE)$(L)$(CC.SUFFIX.ARCHIVE)
		if ( T = "$(T:T=F)" )
			print -um bind -l$(L)
			return - $(T)
		else
			-l$(L) : .DONTCARE
			return + $(%)
		end
	end

/*
 * .REQUIRE.-l% helper that includes pkg-*.mk of lib and prereqs
 * before any -l bindings -- needed because the package lib is
 * probably linked last but the pkg-*.mk rules may give binding
 * hints for prereq libs
 */

"{" "}" : .VIRTUAL .IGNORE .DONTCARE

.REQUIRE.RULES. : .FUNCTION
	local B R T D DL DR
	B := $(%:/-l//)
	if "$(-mam:N=static*)" && "$(%)" != "-l+([a-zA-Z0-9_])"
		return
	end
	if "$(.REQUIRED.$(B))"
		return
	end
	for D 1 2
		if ( T = "$(CC.REQUIRE.$(B))" )
			if "$(-mam:N=static*)"
				return
			end
			return $(T)
		end
		if ( T = "$(PACKAGE_$(B)_LIB)" )
			if T == "$(%:T=F:P=D)"
				T := $(T)/lib/$(B)
				if ( T = "$(T:T=F)" )
					return $(T:T=I)
				end
				return $(%)
			end
		end
		if "$(.PACKAGE.$(B).rules)"
			break
		end
		R := pkg-$(B).mk
		if R = "$(R:T=F)"
			include + $(R)
			.PACKAGE.$(B).rules := $(R)
		else
			.PACKAGE.$(B).rules := -
			break
		end
	end
	if ! ( T = "$(%:/-l\(.*\)/\1.req/:T=F)" )
		R := lib/$(B)
		$(R) : .ARCHIVE
		if ! ( T = "$(R:T=F)" )
			R := $(MAKELIB:D)/$(R)
			if ! ( T = "$(R:T=F)" )
				return $(%)
			end
		end
		if "$(-mam:N=static*,port*)"
			return ${mam_lib$(B)}
		end
		/* req must be seen at or before the lib */
		DL := $(%:T=WF:P=D)
		DR := $(T:P=D)
		if DL != "$(DR)"
			for D $(*.SOURCE.%.ARCHIVE)
				if D == "$(DR)"
					break
				end
				if D == "$(DL)"
					return $(%)
				end
			end
		end
	elif "$(-mam:N=static*,port*)"
		return ${mam_lib$(B)}
	end
	return $(T:T=I)

.REQUIRE.-l% : .FUNCTION
	local B H L R S
	R := $(.REQUIRE.RULES. $(%))
	for L $(R)
		B := $(L:/-l//)
		if ! "$(.PACKAGE.$(B).rules)"
			H := pkg-$(B).mk
			if H = "$(H:T=F)"
				include + $(H)
				.PACKAGE.$(B).rules := $(H)
			else
				.PACKAGE.$(B).rules := -
			end
		end
	end
	B := $(%:/[-+]l//)
	if "$(.PACKAGE.LIBRARY. $(B))" == "+l"
		if allstatic
			H =
			for L $(R)
				S := $(L:/-l/+l/)
				if "$(S:A=.VIRTUAL)"
					H += $(L)
				else
					H += $(S)
				end
			end
			return { $(H) }
		end
		return { $(R:/-l$(B)/+l$(B)/) }
	end
	return { $(R) }

.REQUIRE.+l% : .FUNCTION
	local A B L S
	B := $(%:/+l//)
	L := $(.REQUIRE.-l% -l$(B))
	if allstatic
		for B $(L)
			S := $(B:/-l/+l/)
			if "$(S:A=.VIRTUAL)"
				A += $(B)
			else
				A += $(S)
			end
		end
		return $(A)
	end
	return $(L:/-l$(B)/+l$(B)/)

.SOURCE.%.ARCHIVE : .FORCE $$(*.SOURCE.a) $$(*.SOURCE)

/*
 * some actions cannot operate concurrently
 */

.LEX.SEMAPHORE .YACC.SEMAPHORE : .SEMAPHORE

/*
 * target property metarules
 */

.LD.KEEP. = $(CC.SUFFIX.LD) $(ldscript)
.ARCHIVE.OMIT. = $(CC.LD.STATIC)|$(CC.LD.DYNAMIC)$(.LD.KEEP.:/^/|*/:/ //G)

.ARCHIVE.o : .USE .ARCHIVE (AR) (ARFLAGS) .ARPREVIOUS .ARUPDATE .ARCLEAN
	$(^:?$$(CP) $$(^) $$(<)$$("\n")??)$(.ARPREVIOUS.$(<:B:S):@?$(IGNORE) $$(AR) d $$(<) $$(.ARPREVIOUS.$$(<:B:S))$$("\n")??)$(>:N!=$(.ARCHIVE.OMIT.):K=$(AR) $(ARFLAGS) $(<))

.ARPREVIOUS : .MAKE .VIRTUAL .FORCE .REPEAT .IGNORE
	eval
	$(<).$(<<:B:S) = $(*$(<<:T=SR):N!=$(**:@/ /|/G))
	end

.ARUPDATE : .AFTER .IGNORE .VIRTUAL .FORCE .REPEAT
	$(<<:T=A)

.ARCOPY : .AFTER .IGNORE .VIRTUAL .FORCE .REPEAT
	$(**:O=1:P=I=$(<<):?: $(<<) linked to $(**:O=1)?$(<<:T=AF)?)

.ARCLEAN.LIST. : .FUNCTION
	local I V
	if ! .AR.RETAIN
		for I $(***:T=F:T=G:P=L:A!=.ARCHIVE|.TERMINAL:N!=$(.ARCHIVE.OMIT.):$(arclean))
			if ! "$(~$(I):A=.FORCE:A!=.IGNORE)"
				V += $(I)
			end
		end
	end
	return $(V)

.ARCLEAN : .AFTER .IGNORE .VIRTUAL .FORCE .REPEAT
	$(.ARCLEAN.LIST.:K=$(RM) $(RMFLAGS))

.COMMAND.o : .USE .COMMAND (CCLD) (CCLDFLAGS) $$(LDLIBRARIES)
	$(CCLD) $(CCLDFLAGS) $(&:T=D:N!=-[DIUl]*) -o $(<) $(.SHARED.LIST. $(.SHARED.LIST.LIBS.))

.OBJECT.o : .USE (LD) (CCFLAGS) (LDFLAGS)
	$(LD) -r $(LDFLAGS) -o $(<) $(*)

/*
 * match-all metarules
 */

for .S. $(.SUFFIX.c) $(.SUFFIX.C)
	% : %$(.S.) (CC) (CCFLAGS) (LDFLAGS) $$(LDLIBRARIES)
		$(CC) $(CCFLAGS) $(LDFLAGS:N!=-[DIU]*) $(CCLDFLAGS) -o $(<) $(*)
end

for .S. $(.SUFFIX.f) $(.SUFFIX.r)
	% : %$(.S.) (F77) (F77FLAGS) (LDFLAGS) $$(LDLIBRARIES)
		$(F77) $(F77FLAGS) $(LDFLAGS) -o $(<) $(*)
end

% : %.fql (F77) (CCFLAGS) (F77FLAGS) (LDFLAGS) $$(LDLIBRARIES)
	$(F77) $(CCFLAGS) $(F77FLAGS) $(LDFLAGS) -o $(<) $(*)

% : %.s (ASFLAGS) (LDFLAGS) $$(LDLIBRARIES)
	$(CC) $(ASFLAGS) $(LDFLAGS) -o $(<) $(*)

% : %.sh (SHELLMAGIC)
	case $(-mam:N=static*:/:.*//):$OPTIND:$RANDOM in
	?*:*:*|*::*|*:*:$RANDOM)
		;;
	*)	if	ENV= x= $SHELL -nc ': ${list[level]} $$(( 1 + $x )) !(pattern)' 2>/dev/null
		then	ENV= $SHELL -n $(>)
		fi
		;;
	esac
	case '$(SHELLMAGIC)' in
	"")	case $(&:T=E:@O!) in
		0)	$(CP) $(>) $(<)
			;;
		*)	{
			i=`(read x; echo $x) < $(>)`
			case $i in
			'#!'*|*'||'*|':'*|'":"'*|"':'"*)	echo "$i" ;;
			esac
			cat - $(>) <<'!'
	$(&:T=E)
	!
			} > $(<)
			;;
		esac
		;;
	*)	cat - $(>) > $(<) <<'!'
	$(SHELLMAGIC)
	$(&:T=E)
	!
		;;
	esac
	$(CHMOD) u+w,+x $(<)

/*
 * double suffix metarules
 */

for .S. $(.SUFFIX.c) $(.SUFFIX.C)
	%.o : %$(.S.) (CC) (CCFLAGS)
		$(CC) $(CCFLAGS) -c $(>)
end

.COBOL.INIT : .MAKE .VIRTUAL .FORCE .IGNORE
	$(COBOLLIBRARIES) : .DONTCARE
	LDFLAGS += $$(!:A=.SCAN.cob:@?$$(CC.EXPORT.DYNAMIC)??)
	LDLIBRARIES += $$(!:A=.SCAN.cob:@?$$(COBOLLIBRARIES)??)

for .S. $(.SUFFIX.cob)
	%.c %.c.h : %$(.S.) (COBOL) (COBOLFLAGS) .COBOL.INIT
		$(COBOL) $(COBOLFLAGS) $(>)
end

for .S. $(.SUFFIX.f) $(.SUFFIX.r)
	%.o : %$(.S.) (F77) (F77FLAGS)
		$(F77) $(F77FLAGS) -c $(>)
end

%.o : %.s (AS) (ASFLAGS)
	$(AS) $(ASFLAGS) -o $(<) $(>)

%.o : %.S (CC) (CCFLAGS)
	$(CC) $(CCFLAGS) -c $(>)

%.c %.h : %.y .YACC.SEMAPHORE (YACC) (YACCFLAGS)
	if	silent $(YACC) --version >/dev/null 2>&1
	then	$(YACC) $(YACCFLAGS) -o$(<:N=*.c) $(YACCFIX.$(%):?-p$(YACCFIX.$(%))??) $(>)
	else	$(YACC) $(YACCFLAGS) $(>)$(YACCFIX.$(%):?$("\n")$(STDED) $(STDEDFLAGS) y.tab.c <<!$("\n")g/yytoken/s//yy_token/g$("\n")g/yy/s//$(YACCFIX.$(%))/g$("\n")g/YY/s//$(YACCFIX.$(%):F=%(invert)s)/g$("\n")w$("\n")q$("\n")!??)$(YACCHDR.$(%):?$("\n")$(STDED) $(STDEDFLAGS) y.tab.c <<!$("\n")1i$("\n")#include "$(YACCHDR.$(%))"$("\n").$("\n")w$("\n")q$("\n")!??)
		$(MV) y.tab.c $(%).c
		if	$(SILENT) test -s y.tab.h
		then	$(STDED) $(STDEDFLAGS) y.tab.h <<'!'
	1i
	$("#")ifndef _$(%:F=%(upper)s)_H
	$("#")define _$(%:F=%(upper)s)_H
	.
	$a
	$("#")endif $("/")* _$(%:F=%(upper)s)_H *$("/")
	.
	w
	q
	!
			$(YACCFIX.$(%):?$(STDED) $(STDEDFLAGS) y.tab.h <<!$("\n")g/yytoken/s//yy_token/g$("\n")g/yy/s//$(YACCFIX.$(%))/g$("\n")g/YY/s//$(YACCFIX.$(%):F=%(invert)s)/g$("\n")w$("\n")q$("\n")!$("\n")??)if	$(SILENT) $(CMP) $(CMPFLAGS) y.tab.h $(%).h
			then	$(RM) $(RMFLAGS) y.tab.h
			else	$(MV) y.tab.h $(%).h
			fi
		fi
		if	$(SILENT) test -f y.output
		then	$(MV) y.output $(%).grammar
		fi
	fi

%.c : %.l .LEX.SEMAPHORE (LEX) (LEXFLAGS) (CC)
	if	silent $(LEX) --version >/dev/null 2>&1
	then	$(LEX) $(LEXFLAGS) -o$(<) $(LEXFIX.$(%):?-P$(LEXFIX.$(%))??) $(>)
	else	$(LEX) $(LEXFLAGS) $(>)$(LEXFIX.$(%):?$("\n")$(STDED) $(STDEDFLAGS) lex.yy.c <<!$("\n")g/yy/s//$(LEXFIX.$(%))/g$("\n")g/YY/s//$(LEXFIX.$(%):F=%(invert)s)/g$("\n")w$("\n")q$("\n")!??)$(LEXHDR.$(%):?$("\n")$(STDED) $(STDEDFLAGS) lex.yy.c <<!$("\n")1i$("\n")#include "$(LEXHDR.$(%))"$("\n").$("\n")w$("\n")q$("\n")!??)
		$(MV) lex.yy.c $(<)
	fi

%.mo : %.mk
	$(MAKE) $(-) --base --compile --file=$(>) $(CCFLAGS:N=-[I][!-]*) $(&:T=E)

/*
 * library req support
 */

-L% : "" .MAKE .DONTCARE .VIRTUAL .FORCE .REPEAT .IGNORE
	/* we should be able to do just .SOURCE.a here */
	.SOURCE.%.ARCHIVE .SOURCE.a : $(<:/-L//)
	if CC.SUFFIX.DYNAMIC
		.SOURCE.%$(CC.SUFFIX.SHARED) : $(<:/-L//)
	end

/*
 * C alternate flags support
 */

cc-% : "" .ALWAYS .LOCAL .FORCE .RECURSE.SEMAPHORE
	set -
	if	test -d $(<:V:Q)
	then	$(-silent:Y%%echo $(-errorid:C%$%/%)$(<:V:Q): >&2%)
		cd $(<:V:Q)
		$(MAKE) --file=$(MAKEFILE) --keepgoing $(-) --errorid=$(<:V:Q) .ATTRIBUTE.$(IFFEGENDIR)/%:.ACCEPT MAKEPATH=..:$(MAKEPATH) $(=:V:N!=MAKEPATH=*) $(.RECURSE.ARGS.:N!=.CC-*:/^\.INSTALL$/.CC-INSTALL/) $(%:Y!$$(INSTRUMENT_$$(%:/,.*//):@?instrument=$$(%:/,.*//)?CCFLAGS=-$$(%:V:/$$(%:V:N=*~*:?~?,?)/ /G:@/-W\(.\) /-W\1,/G:@Q)?)!!) $(.VARIANT.$(<))
	fi

/*
 * feature test support -- the magic is in $(IFFE)
 */

$(IFFEGENDIR)/% : $(IFFESRCDIR)/%.c .SCAN.c (IFFE) (IFFEFLAGS)
	$(IFFE) $(IFFEFLAGS) run $(>)

$(IFFEGENDIR)/% : $(IFFESRCDIR)/%.sh .SCAN.c (IFFE) (IFFEFLAGS)
	$(IFFE) $(IFFEFLAGS) run $(>) $(?$(>):T=F)

$(IFFEGENDIR)/% : $(IFFESRCDIR)/% .SCAN.c (IFFE) (IFFEFLAGS)
	$(IFFE) $(IFFEFLAGS) run $(>)

$(IFFEGENDIR)/% : %.iffe .SCAN.c (IFFE) (IFFEFLAGS)
	$(IFFE) $(IFFEFLAGS) run $(>)

$(IFFEGENDIR)/% : "" .SCAN.c (IFFE) (IFFEFLAGS)
	$(IFFE) $(IFFEFLAGS) def $(%)

%.h : %.iffe .SCAN.c (IFFE) (IFFEFLAGS)
	$(IFFE) -o $(<) $(IFFEFLAGS) run $(>)

.IFFE.REF. : .FUNCTION
	local T
	T := $(...:N=*.req:A=.TARGET)
	.MAKE : $(T)
	T := $(T:T=F:T=I:/[[:space:]][[:space:]]*/ /G:N!=-l($(T:B:C, ,|,G)))
	T += $(LDLIBRARIES)
	.R. : .CLEAR .MAKE $(T)
		: $(*)
	.MAKE : .R.
	T := $(*.SOURCE.%.STD.INCLUDE:N=*/($(.PACKAGE.build:A!=.TARGET:/ /|/G)):T=F:U!)
	T := $(T:/^/-I/) $(T:D:U:/^/-I/) $(*.R.:N!=$(<:T=M:@/ /|/G))
	if T
		if "$(-mam:N=static*,port*)"
			return ref $(*.SOURCE.%.ARCHIVE:I=$$(T:N=${mam_lib+([a-zA-Z0-9_])}:P=D):$(.CC.NOSTDLIB.):/.*/${mam_cc_L+-L&}/) $(T) :
		else
			return ref $(*.SOURCE.%.ARCHIVE:I=$$(T:N=-l*:P=D):$(.CC.NOSTDLIB.):P=A:/^/-L/) $(T) :
		end
	end

/*
 * C information abstractor support rules
 */

.SOURCE.db : $$(CIAFLAGS:N=-d*:/-d//)

CIA = $(CC.ALTPP.ENV) $(CC.DIALECT:N=C++:?CIA?cia?)
CIALD = $(CIA)
CIADBFLAGS =
CIAFLAGS =
CIAFILES = $(CC.DIALECT:N=C++:?Ref.db Sym.db?reference.db symbol.db?)
DOT = dot
DOTFLAGS = -Tps
DAGGER = $(CC.DIALECT:N=C++:?Dagger?dagger?)
DAGGERFLAGS =

(CIA) (CIADBFLAGS) (CIAFLAGS) : .PARAMETER

%.A : %.c (CIA) (CIAFLAGS)
	$(CIA) $(CIAFLAGS) $(CPPFLAGS) -c $(>)

%.A : %.C (CIA) (CIAFLAGS)
	$(CIA) $(CIAFLAGS) $(CPPFLAGS) -c $(>)

.CIADB.AGAIN =

.CIADB : .MAKE .VIRTUAL .FORCE .ONOBJECT .REPEAT .PROBE.INIT
	local A S T U X
	for S $(.SUFFIX.c) $(.SUFFIX.C)
		T += $(.SOURCES.:G=%$(S):B:S=$(CC.SUFFIX.OBJECT))
	end
	if T
		.SOURCE.c : $(IFFESRCDIR)
		.CIA.REBIND : .MAKE .VIRTUAL .FORCE .AFTER .FOREGROUND
			.REBIND : $(CIAFILES)
		for U $(T)
			X =
			for S $(.SUFFIX.c) $(.SUFFIX.C)
				X += $(*$(U):G=%$(S))
			end
			if ( X = "$(X:A!=.LCL.INCLUDE|.STD.INCLUDE)" )
				$(U:B:S=.A) : .IMPLICIT $(X) $(~$(U):A=.STATEVAR)
				A += $(U:B:S=.A)
			end
		end
		$(CIAFILES:O=1) : (CIADBFLAGS) (CIAFLAGS) $(A) .CIA.REBIND
			$(CIALD) $(CIAFLAGS) $(*)
		.MAKE : $(CIAFILES:O=1)
	elif .CIADB.AGAIN
		error 3 ciadb: no source to generate database
	else
		.CIADB.AGAIN = 1
		.ARGS : .INSERT $(<)
		.MAKE : .ALL
	end

.CIADOT : .CIADB
	$(DAGGER) $(DAGGERFLAGS) | $(DOT) $(DOTFLAGS)

/*
 * .USE action scripts
 */

.DO.COPY : .USE
	$(CMP) $(CMPFLAGS) $(*:O=1) $(<) || { $(RM) $(RMFLAGS) $(<); $(SILENT) test -d $(<:D) || $(MKDIR) $(<:D); $(CP) $(*:O=1) $(<); }

.DO.INSTALL.OPT. = $(~:N=user=*:@C,.*,\&\& { $(CHOWN) & $(<) || true ;},:@C, user=, ,G) \
		   $(~:N=group=*:@C,.*,\&\& { $(CHGRP) & $(<) || true ;},:@C, group=, ,G) \
		   $(~:N=mode=*:@C,.*,\&\& $(CHMOD) & $(<),:@C, mode=\([-+]\), ugo\1,G:@C, mode=, ,G)

.DO.INSTALL.OLD. : .FUNCTION
	if "$(preserve)" == "1" || "$(preserve:N=*/*:?$$(%:N=$$(preserve))?$$(%:B:S:N=$$(preserve))?)"
		return $(SILENT) test -d $(%:D:B=ETXTBSY) || $(MKDIR) $(%:D:B=ETXTBSY); $(MV) $(%) $(%:D:B=ETXTBSY)/$(%:B)#$(%:P=I)
	elif "$(clobber)" == "1" || "$(clobber:N=*/*:?$$(%:N=$$(clobber))?$$(%:B:S:N=$$(clobber))?)"
		return $(RM) $(RMFLAGS) $(%)
	else
		return $(MV) $(%) $(%).old
	end

.DO.INSTALL : .USE $$(<:N=*$$(CC.SUFFIX.ARCHIVE):?.ARCOPY??)
	if	$(SILENT) test '' != "$(*:O=1)"
	then	if	$(SILENT) test -d "$(*:O=1)"
		then	$(CPR) $(CPRFLAGS) $(*:O=1) $(<:D)
		else	$(*:O=1:P=I=$(<):?: $(<) linked to $(*:O=1) ||?$$(compare:/^0$//:Y@$$$(SILENT) $$$(CMP) $$$(CMPFLAGS) $$$(*:O=1) $$$(<) ||@@)?)
			{
			if	$(SILENT) test -f "$(<)"
			then	$(.DO.INSTALL.OLD. $(<))
			fi
			$(link:/^0$//:?$$(<:B:$$(<:A=.COMMAND:Y@@S@):N=$$(link:/^1$/*/):Y@$$$(LN) $$$(_feature_:N=ln|ln-s:O=2:Y%-s $$$(*:O=1:D=$$$(<:D:P=R=$$$(*:O=1:D)):B:S)%$$$(*:O=1)%) $$$(<) || @@)??)$(IGNORE) $(CP) $(*:O=1) $(<) $(.DO.INSTALL.OPT.)
			}
		fi
	fi

.DO.INSTALL.DIR : .USE .PLACEHOLDER
	set -
	if	test ! -d $(<) 
	then	$(-silent:??set -x && ?)$(MKDIR) -p $(<)$(.DO.INSTALL.OPT.)
	fi

.PLACEHOLDER : .VIRTUAL .IGNORE

.DO.LINK.OLD. : .FUNCTION
	if "$(clobber)" == "1" || "$(clobber:N=*/*:?$$(%:N=$$(clobber))?$$(%:B:S:N=$$(clobber))?)"
		return $(RM) $(RMFLAGS) $(%)
	else
		return $(CP) $(%) $(%).old$("\n")$(RM) $(RMFLAGS) $(%)
	end

.DO.LINK : .USE .ACCEPT /* .ACCEPT until dual-time implementation */
	if	$(SILENT) test -f "$(<)"
	then	$(.DO.LINK.OLD. $(<))
	fi
	$(LN) $(*:O=1) $(<)

.DO.NOTHING : .USE .NULL

.NOOPTIMIZE.c .CC.NOOPTIMIZE /* drop .CC.* in 2004 */ : .MAKE .LOCAL
	CCFLAGS := $(.MAM.CC.FLAGS|CCFLAGS:VP:N!=-O*|$(CC.OPTIMIZE)|$\(CC.OPTIMIZE\))
	if "$(-mam)"
		CCFLAGS := ${mam_cc_FLAGS} ${debug?1?${mam_cc_DEBUG} -D_BLD_DEBUG??}
	elif debug
		CCFLAGS := $(CC.DEBUG) $(CCFLAGS:VP:N!=-g|$(CC.DEBUG)|$\(CC.DEBUG\))
	end

.READONLY. : .FUNCTION
	local ( .ROSRC. .ROOBJ. ... RO ) $(%)
	if RO == "-S.idat"
		.ROSED. = s/^\([ 	]*[.a-zA-Z0-9_$:]*[ 	]*\.*\)idat/\1code/
		return $(@.DO.READONLY.c.sed)
	elif RO == "-S.data"
		.ROSED. = s/^\([ 	]*\.*\)data/\\1text/
		return $(@.DO.READONLY.c.sed)
	elif RO == "-S.data"
		.ROSED. = s/^\([ 	]*\.*\)data/\1rdata/
		return $(@.DO.READONLY.c.sed)
	end
	return $$(CC) $$(CCFLAGS:N!=$(CC.DEBUG)) $(RO) -c $(.ROSRC.)

.READONLY.c : .USE .IMPLICIT
	$(.READONLY. $(>) $(<) $(CC.READONLY))

.DO.READONLY.c.sed : .USE
	$(CC) $(CCFLAGS:N!=$(CC.DEBUG)) -S $(.ROSRC.)
	$(.ROSRC.:B:N=$(<:B):?$(":") convert $(<:B:S=.c) data to readonly text in $(<:B:S)$("\n")??)set -
	$(SED)	-e '$(.ROSED.)' \
		-e 's/^\([ 	]*[.a-zA-Z0-9_$:]*[ 	]*\.*\)zero[ 	][ 	]*/\1set	.,.+/' \
		-e 's/^\([ 	]*[.a-zA-Z0-9_$:]*[ 	]*\.*\)space[ 	][ 	]*1/\1byte 0/' \
		-e 's/^\([ 	]*[.a-zA-Z0-9_$:]*[ 	]*\.*\)space[ 	][ 	]*2/\1byte 0,0/' \
		-e 's/^\([ 	]*[.a-zA-Z0-9_$:]*[ 	]*\.*\)space[ 	][ 	]*3/\1byte 0,0,0/' \
		-e 's/^\([ 	]*[.a-zA-Z0-9_$:]*[ 	]*\.*\)space[ 	][ 	]*4/\1byte 0,0,0,0/' \
		$(.ROSRC.:B:S=.s) > 1.$(tmp).s
	$(AS) $(ASFLAGS) -o $(.ROOBJ.) 1.$(tmp).s
	$(RM) $(RMFLAGS) $(.ROSRC.:B:S=.s) 1.$(tmp).s

.READONLY.l : .USE .IMPLICIT
	$(".YYSUF.=.l":R)$(@.DO.READONLY.ly)

.READONLY.y : .USE .IMPLICIT
	$(".YYSUF.=.y":R)$(@.DO.READONLY.ly)

.DO.READONLY.ly : .USE
	: convert $(<:B:S=.c) tables to readonly text in $(<:B:S)
	set -
	$(CP) $(<:B:S=.c) 1.$(tmp).c
	$(STDED) $(STDEDFLAGS) 1.$(tmp).c > 2.$(tmp).c <<!
	$(@.YYSCRIPT$(.YYSUF.))
	w
	q
	!
	$(".ROSRC.=2.$(tmp).c":R)$(".ROOBJ.=2.$(tmp)$(CC.SUFFIX.OBJECT)":R)$(@.DO.READONLY.c.$(CC.READONLY))
	$(CC) $(CCFLAGS) -c 1.$(tmp).c
	$(LD) -r $(LDFLAGS) $(&:T=D:N=-[!DIOUl]*) -o $(<:B:S) 1.$(tmp)$(CC.SUFFIX.OBJECT) 2.$(tmp)$(CC.SUFFIX.OBJECT)
	$(CHMOD) -x $(<:B:S)
	$(RM) $(RMFLAGS) [12].$(tmp).[cos]

.YYSCRIPT.l : .USE
	/struct yysvf[ 	]*{/;.,/}/p
	/define[ 	][ 	]*YYTYPE/p
	/struct yywork[ 	]*{/s/^.*}/&;\\
	struct yywork /
	-p
	$(@.YYFIX:@/YY/yycrank/G)
	$(@.YYFIX:@/YY/yyvstop/G)
	$(@.YYFIX:@/YY/yysvec/G)
	$(@.YYFIX:@/YY/yymatch/G)
	$(@.YYFIX:@/YY/yyextra/G)

.YYSCRIPT.y : .USE .IMPLICIT
	g/typedef.*yyt[a-z]*;/p
	$(@.YYFIX:@/YY/yyexca/G)
	$(@.YYFIX:@/YY/yyact/G)
	$(@.YYFIX:@/YY/yypact/G)
	$(@.YYFIX:@/YY/yypgo/G)
	$(@.YYFIX:@/YY/yyr1/G)
	$(@.YYFIX:@/YY/yyr2/G)
	$(@.YYFIX:@/YY/yychk/G)
	$(@.YYFIX:@/YY/yydef/G)
	$(@.YYFIX.release)

.YYFIX :
	/^\(.*\)YY[ 	]*\[]/s//extern \1 YY[];\\
	\1 YY []/
	.ka
	/}/kb
	'a,'bp
	'a,'bd

if "$(_release_:N=V)"
.YYFIX.release :
	/^#[ 	]*ifndef[ 	][ 	]*YYDEBUG/;/^#[ 	]*endif/p
	/^#[ 	]*if[ 	][ 	]*YYDEBUG/p
	$(@.YYFIX:@/YY/yytoks/G)
	$(@.YYFIX:@/YY/yyreds/G)
	/^#[ 	]*endif/p
end

.RECURSE : .MAKE .VIRTUAL .FORCE .NULL

.NORECURSE : .MAKE .VIRTUAL .FORCE .NULL

.RECURSE.SEMAPHORE : .VIRTUAL .FORCE .IGNORE .NULL

.RECURSE.ARGS. : .FUNCTION
	local A V
	for A $(.ORIGINAL.ARGS.)
		if ! "$(A:A=.TARGET)" && ( T = "$(A:/.*/.&/U:A=.TARGET)" )
			A := $(T)
		end
		if "$(A:A!=.ACTIVE:N!=.RECURSE)" || "$(A:A=.ONOBJECT)"
			V += $(A)
		end
	end
	if "$(~.ARGS:A=.ONOBJECT)"
		V := .RECURSE $(V)
	end
	return $(V)

.RECURSE.INIT. : .FUNCTION
	local D P
	if ! ( D = "$(%)" )
		D = .
	end
	if recurse == "prereqs"
		print $(D:W=P=$(.RECURSE.ARGS.:A!=.ONOBJECT:N!=.RECURSE))
		exit 0
	end
	if D == "."
		D := $(D:W=R=$(.RECURSE.ARGS.:A!=.ONOBJECT:N!=.RECURSE))
	end
	if recurse == "list"
		print $(D:/ /$("\n")/G)
		exit 0
	end
	$(D) : .OBJECT
	P := $(D:B:S:/ /|/G)
	if P
		P := $(P)|
	end
	P := $(P)recurse|.RECURSE
	.ORIGINAL.ARGS. := $(.ORIGINAL.ARGS.:N!=$(P))
	.ARGS : .CLEAR $(~.ARGS:N!=$(P))
	P := $(D:N!=-|.CC-*|cc-*)
	$(P:T!=FR) : .TERMINAL .RECURSE.DIR
	$(P:T=FR) : .TERMINAL .RECURSE.FILE
	return $(D)

.RECURSE.DIR : .USE .ALWAYS .LOCAL .FORCE .RECURSE.SEMAPHORE
	set -
	if	$(physical:?$$(*.VIEW:O=2:N=...:Y%2d%%)??) test -d $(<) $(skeleton:?|| mkdir $(<)??)
	then	$(-silent:Y%%echo $(recurse_enter) $(.RWD.:?$(<:N!=/*:?$(.RWD.)/??)??)$(<): >&2%)
		cd $(<)
		$(MAKE) $(-) --errorid=$(<:Q) $(=:V:N!=MAKEPATH=*|VPATH=*) .RWD.=$(.RWD.:C%$%/%)$(<) $(.RECURSE.ARGS.)
		$(recurse_exit:Y,$$(-silent:Y%%echo $$(recurse_exit) $$(.RWD.:?$$(<:N!=/*:?$$(.RWD.)/??)??)$$(<): >&2%),,)
	elif	test '' = '$(physical)'
	then	echo $(<): cannot recurse on virtual directory >&2
	fi

.RECURSE.FILE : .USE .ALWAYS .LOCAL .FORCE .RECURSE.SEMAPHORE
	set -
	$(-silent:Y%%echo $(.RWD.:?$(<:N!=/*:?$(.RWD.)/??)??)$(<): >&2%)
	$(MAKE) $(-) --errorid=$(<:B:Q) --file=$(<) $(=) .RWD.=$(.RWD.) $(.RECURSE.ARGS.)

/*
 * source dependency operator
 */

"::" : .MAKE .OPERATOR .PROBE.INIT
	if ! "$(<)"
		$(>:N=[!-.]*|.[!A-Z]*) : .SPECIAL
		.FILES. += $(>:N=[!-.]*|.[!A-Z]*)
	elif "$(<:O=2)"
		error 2 $(<:O=2): only one target expected
	else
		local T0 T1 T2 T3 T4 TA TP TS OBJ STATIC
		eval
			.FILES.$(<:B:S) = $(>:V:N=[!-.]*|.[!A-Z]*)
			.RHS.$(<:B:S) = $(>:V)
		end
		.FILES. += $$(.FILES.$(<:B:S))
		if "$(.INSTALL.$(<:B:S):V)" != "." && ! "$(.NO.INSTALL.)"
			.ALL : $(<)
		end
		if ! .MAIN.TARGET.
			.MAIN.TARGET. := $(<:B:S)
		end
		if "$(>:V:G=%$(CC.SUFFIX.OBJECT))"
			OBJ = 1
		end
		if ! "$(<:N=*$(CC.SUFFIX.OBJECT))"
			$(<) : .COMMAND
		end
		T0 := $(.LD.KEEP.:/^/%/)
		STATIC = 0
		for T1 $(>:V:T=X)
			if "$(T1:V:A=.ARCHIVE)" || T1 == "[-+]l*"
				$(T1:V) : .ARCHIVE .MULTIPLE
				if T1 == "$(CC.LD.STATIC)"
					STATIC = 1
					if CC.SUFFIX.DYNAMIC == ".dll"
						continue
					end
				elif T1 == "$(CC.LD.DYNAMIC)"
					STATIC = 0
					if CC.SUFFIX.DYNAMIC == ".dll"
						continue
					end
				end
				TP += $(T1:V)
			elif T2 = "$(T1:G=$(T0))"
				TP += $(T2)
			elif T1 == ".[a-zA-Z]*|[!-+]*=*|$\(*\)" || "$(T1:A=.ATTRIBUTE|.STATE)" || "$(T1:A=.MAKE:A=.LOCAL)"
				TP += $(T1:V)
				if "$(T1:A!=.USE)"
					TA += $(T1:V)
				end
			elif T1 == "[-+]?*"
				.OPTIONS.$(<) += $(T1)
			elif T1 == "-"
				T4 = 1
			elif OBJ && ( T2 = "$(T1:G=%$(CC.SUFFIX.OBJECT))" )
				if CC.SUFFIX.OBJECT == "$(T1:S)"
					T2 := $(T1)
				else
					if "$(-targetcontext)"
						T2 := $(T2:D=$(T1:D):B:S)
					end
					$(T2) : .SPECIAL .IMPLICIT $(T1)
					$(T1) : .SPECIAL .TERMINAL
				end
				if T2 != "$(<)?($(CC.SUFFIX.COMMAND))"
					if STATIC
						$(T2) : .SPECIAL CC.DLL= CC.DLLBIG=
						if CC.SUFFIX.DYNAMIC == ".dll"
							TS += $(T2)
							continue
						end
					end
					TP += $(T2)
				end
				T3 += $(T2)
			elif "$(T1:N=*.*:G=%)" && T1 != "$(<)?($(CC.SUFFIX.COMMAND))"
				if "$(T1:G=$(<))" || "$(T1:G=$(<)$(CC.SUFFIX.COMMAND))"
					TP += .IMPLICIT
				end
				TP += $(T1)
			elif T4
				TP += $(T1)
			end
		end
		T0 := $(<)
		if "$(<:A=.ARCHIVE)" || "$(TP:V:A=.ATTRIBUTE:A=.ARCHIVE)"
			TP += -COMMAND
			if ! "$(@:V)" && ! "$(~:A=.USE)"
				TP += .ARCHIVE$(CC.SUFFIX.OBJECT)
			end
		else
			TA += $(.LIBRARY.ONLY.)
			if ! "$(T3:N=$(<)?($(CC.SUFFIX.COMMAND)))"
				if OBJ || "$(>:V:A=.ARCHIVE)"
					if "$(T0:N=*$(CC.SUFFIX.OBJECT))"
						TP += .OBJECT$(CC.SUFFIX.OBJECT)
					else
						if ! "$(<:S)" && CC.SUFFIX.COMMAND
							$(T0) : .VIRTUAL $(T0)$(CC.SUFFIX.COMMAND)
							T0 := $(T0)$(CC.SUFFIX.COMMAND)
						end
						TP += .COMMAND$(CC.SUFFIX.OBJECT)
					end
				else
					TP += .OBJECT
				end
			end
		end
		if T3
			$(T3) : .SPECIAL $(TA:V:Q)
			if "$(.OPTIONS.$(<))"
				$(T3) : .SPECIAL (.OPTIONS.$(<))
			end
		end
		if TS
			.LIBRARY.STATIC.$(<:B:/^$(CC.PREFIX.ARCHIVE)//) : $(CC.LD.STATIC) $(TS) $(CC.LD.DYNAMIC)
		end
		$(T0) : $(TP:V:Q)
		if "$(@:V:?1??)"
			eval
				$$(T0) :
					$(@:V)
			end
		end
		if "$(<:N=*_*)" && ! "$(@$(<):V)" && ( T1 = "$(*$(<):@N=$(<:/.*_//).*)" )
			T1 := $(T1:S)
			T2 := %$(T1)>%
			if "$(@$(T2):V)"
				eval
				$$(<) :
					$(@$(T2):V:/$$(>)/$$(*:N=*$(T1):O=1)/G)
				end
			end
		end
		if ! "$(.NO.INSTALL.)"
			if ! ( T1 = "$(.INSTALL.$(<:B:S):V)" )
				for T2 $(.INSTALL.MAPS.)
					if "$(<:$(T2))"
						T1 = .
						break
					end
				end
				if ! T1
					if ( T1 = "$(<:A<=.INSTALL.)" )
						T1 := $(*$(T1):V)
					end
				end
				if T1 && T1 != "." && T1 != "$(T0:V:D)"
					$(T1:V) :INSTALLDIR: $(T0)
				end
			end
			let T1 = 1
			while T1 <= 9
				if T2 = "$(>:V:N=*.$(T1)?([A-Za-z]))"
					if ! "$(*$(T0):V:N=$(T2:V))"
						$$(MANDIR)$(T1) :INSTALLDIR: $(T2:V)
					end
				end
				let T1 = T1 + 1
			end
		end
	end

/*
 * if no rhs then make all by default
 * otherwise add rhs to all
 */

":ALL:" : .MAKE .OPERATOR
	if "$(>:V)"
		.ALL : $(>:V)
		if ! "$(~.MAIN)"
			.MAIN : .ALL
		end
	else
		.MAIN : .CLEAR .ALL
	end

/*
 * DEPRECATED
 * lhs builds rhs as a side effect
 */

":BUILD:" : .MAKE .OPERATOR
	$(<) : $(>)
	eval
	$$(>) : .JOINT .FORCE
		$(@:V)
	end

/*
 * DEPRECATED -- use $(sh ...) instead
 * lhs is a variable whose value is the result
 * of shell command substitution on the action
 * wrt prerequisites on the rhs
 */

":COMMAND:" : .MAKE .OPERATOR
	.RETAIN : $(<)
	eval
	$$(<) : .SPECIAL .VIRTUAL .FUNCTIONAL $$(-mam:N=static*:?.ALWAYS .LOCAL??) $$(>)
		$(@:V)
	end

/*
 * lhs is a copy of rhs
 */

":COPY:" : .MAKE .OPERATOR
	$(<:V) : .SPECIAL .DO.COPY $(>:V)

/*
 * make dll via cc- if not already made at this level
 */

.RECURSE.DLL : .MAKE .USE
	local DIR
	if .DLL.LIST. && ! "$(-mam:N=static*,port*)" && ! "$(.SHARED.ON.)" && ( CC.SUFFIX.DYNAMIC || CC.SUFFIX.SHARED )
		DIR := $(<:/^.DLL.//)
		if ! "$(DIR:T=F)"
			{ test -d $(DIR) || $(MKDIR) $(DIR) }
		end
		.ORIGINAL.ARGS. := .DLL.$(.ORIGINAL.ARGS.:N=install:@?INSTALL?ALL?)
		make $(DIR)
	end

":DLL:" : .MAKE .OPERATOR
	local DIR OPT
	if "$(PWD:B)" != "cc-*"
		if ! ( OPT = "$(>:N=-*)" )
			OPT := $(CC.OPTIMIZE)
		end
		if "$(>:N=big|BIG)"
			OPT += $(CC.DLLBIG)
		else
			OPT += $(CC.DLL)
		end
		DIR := cc$(OPT:/ /,/G)
		.DLL.LIST. += $$(.LIBRARY.LIST.)
		.VARIANT.$(DIR) := VARIANT=DLL
		.ALL : .DLL.$(DIR)
		.DLL.$(DIR) : .RECURSE.DLL
	end

.DLL.ALL : $$(.DLL.LIST.)

.DLL.INSTALL. : .FUNCTION
	local L R
	for L $(.DLL.LIST.)
		R += $(*.INSTALL:N=*/$(.DLL.NAME. $(L))*)
	end
	return $(R)

.DLL.INSTALL : $$(.DLL.INSTALL.)

.DLL.CHECK : .USE .MAKE .VIRTUAL .FORCE
	local B L
	B := $(<:/.DLL.CHECK.//)
	if ! .DLL.LIST. && ! "$(-mam:N=static*,port*)" && ! "$(.SHARED.ON.)" && ! "$(PWD:B:N=cc-*)" && ( L = "$(.DLL.NAME. $(B))" )
		$(L) : .ARCHIVE
		if L = "$(L:T=F)"
			error 1 $(B): generating static library but dynamic $(L) already exists
		end
	end

/*
 * lhs is a function
 */

":FUNCTION:" : .MAKE .OPERATOR
	eval
	$$(<:V) : .SPECIAL .FUNCTION $$(>:V)
		$(@:V)
	end

/*
 * install lhs from rhs
 */

":INSTALL:" : .MAKE .OPERATOR
	local D O R S T
	if ! "$(<:V)" && ! "$(>:V)" && ! "$(@:V)"
		.NO.INSTALL. = 1
	else
		if O = "$(>:N=*=*)"
			$(O) : .SPECIAL .VIRTUAL .DONTCARE
		end
		if T = "$(>:N=-*)"
			error 1 use {user,group,mode}=... instead of -{u,g,m}...
			T := $(T:/-u/user=/:/-g/group=/:/-m/mode=/)
			$(T) : .SPECIAL .VIRTUAL .DONTCARE
			O += $(T)
		end
		S := $(>:T=XS)
		R := $(>:N!=-*|*=*:T!=XS)
		if ! "$(<:V)"
			if ! "$(R:V)"
				eval
				.DO.INSTALL : .USE
					$(@:V)
				end
			else
				.INSTALL : $(R)
				if "$(@:V)"
					eval
					$$(R) :
						$(@:V)
					end
				end
			end
		elif "$(.INSTALL.$(R:B:S))" != "."
			if $(R:O) == 1
				eval
				.INSTALL.$(R:B:S) = $(<:V:D)
				end
			end
			if "$(<:V:D)"
				$(<:V:D) : .SPECIAL .DO.INSTALL.DIR
				D := .DO.INSTALL.DIR.$(<:V:D)
				$(D:V) : .VIRTUAL .IGNORE $(<:V:D)
				.INSTALL : $(<:V:D)
			end
			.INSTALL : $(<:V)
			if "$(@:V)"
				eval
				$$(<:V) : .SPECIAL .SCAN.IGNORE $$(D:V) $(S) $(R)
					$(@:V)
				end
			elif "$(R)"
				$(<:V) : .SPECIAL .SCAN.IGNORE $(D:V) $$("$(R)":N!=$$(<)|$$(<:P=U):@?$(O) $(S) $(R) .DO.INSTALL??)
			else
				$(<:V) : .SPECIAL .SCAN.IGNORE $(D:V) $$(<:B:S:N!=$$(<):@?$(O) $(S) $(R) $$(<:B:S) .DO.INSTALL??)
			end
		end
	end

/*
 * install rhs into lhs dir
 */

":INSTALLDIR:" : .MAKE .OPERATOR
	local D O R T
	if O = "$(>:N=*=*)"
		$(O) : .SPECIAL .VIRTUAL .DONTCARE
	end
	if T = "$(>:N=-*)"
		error 1 use {user,group,mode}=... instead of -{u,g,m}...
		T := $(T:/-u/user=/:/-g/group=/:/-m/mode=/)
		$(T) : .SPECIAL .VIRTUAL .DONTCARE
		O += $(T)
	end
	O += $(>:T=XS)
	R := $(>:N!=-*|*=*:T!=XS)
	if ! "$(<:V)" || "$(<:V)" == "."
		for T $(R)
			eval
			.INSTALL.$(T:B:S) = .
			end
		end
	elif ! "$(R)"
		$(<:V) : .SPECIAL .DO.INSTALL.DIR $(>)
		.INSTALL : $(<:V)
	else
		for T $(R)
			eval
			.INSTALL.$(T:B:S) =
			end
			for D $(<:V)
				eval
				$(T:D=$(D:V):B:S) :INSTALL: $(O) $(T)
					$(@:V)
				end
			end
		end
	end

/*
 * install in lhs dir using rhs pattern to select 
 */

":INSTALLMAP:" : .MAKE .OPERATOR
	if "$(<)"
		local D T
		T := $(.GENSYM.)
		.INSTALL : $(<) $(T)
		$(<) : .SPECIAL .DO.INSTALL.DIR
		D := .DO.INSTALL.DIR.$(<)
		$(D) : .VIRTUAL .IGNORE $(<)
		.INSTALL.MAPS. += $(>)
		eval
		$$(T) : .MAKE .FORCE .IGNORE
			local T
			for T $$(...:$(>):A!=.ATTRIBUTE|.FUNCTIONAL|.MAKE|.VIRTUAL:T=F:N!=$(<)/*)
				$$(T:D=$(<):B:S) : $$(T) .SPECIAL .SCAN.IGNORE $(D) .DO.INSTALL
				.INSTALL : $$(T:D=$(<):B:S)
			end
		end
	end

/*
 * install $(PROTO) output of rhs in lhs dir
 */

":INSTALLPROTO:" : .MAKE .OPERATOR
	local A P T
	T := $(<:O=1)
	if "$(T)" == "*.h"
		A = :INSTALL:
	else
		A = :INSTALLDIR:
	end
	if P = "$(<:O=2)"
		P := -f -e $(P)
	end
	eval
	$(T) $(A) $(>) (PROTO) (PROTOFLAGS)
		$$(PROTO) -p $$(PROTOFLAGS) $(P) $$(*) > 1.$(tmp).x
		if	$$(CMP) $$(CMPFLAGS) $$(<) 1.$(tmp).x
		then	$$(RM) $$(RMFLAGS) 1.$(tmp).x
		else	$$(MV) 1.$(tmp).x $$(<)
		fi
	end

/*
 * action jointly builds all targets on lhs wrt prerequisites on rhs
 */

":JOINT:" : .MAKE .OPERATOR
	eval
	$$(<) : .JOINT $$(>)
		$(@:V)
	end

/*
 * <name> [<major.minor> [option ...]] :LIBRARY: <source> -[lL]*
 */

":LIBRARY:" : .MAKE .OPERATOR .PROBE.INIT
	local A B L P R S T V
	P := $(.PACKAGE.plugin)
	for T $(<:O>2)
		if T == "DLL*"
			.DLL.LIST. += $(B)
			.ALL : .$(B)
		elif T == "plugin=*"
			P := $(T:/plugin=//)
		elif T == "static"
			A = 1
		end
	end
	B := $(<:O=1)
	if "$(B:A=.TARGET)"
		T := .ALL
	else
		T := $(B)
		.ALL : $(B)
	end
	.LIBRARY.LIST. += $(B)
	if ! .MAIN.TARGET.
		.MAIN.TARGET. := $(B:B:S)
	end
	L := $(.LIB.NAME. $(P)$(B) $(<:O=2))
	$(L) : .ARCHIVE$(CC.SUFFIX.OBJECT)
	if P
		local X Y
		/* drop the .LIBRARY.CLEANUP.$(B) code in 2005 */
		X := $(CC.DLL.DIR)/$(.DLL.NAME. $(P)$(B) $(<:O=2)) $(CC.DLL.DIR)/$(.DLL.NAME. $(P)$(B))
		Y := $(X:B:S)
		X += $(X:N=*$(CC.SUFFIX.SHARED).*:C%\$(CC.SUFFIX.SHARED)\.%.oo.%)
		X += $(X:N=*$(CC.SUFFIX.SHARED):C%\$(CC.SUFFIX.SHARED)%.oo%)
		if ! A
			X += $(LIBDIR)/$(.LIB.NAME. $(P)$(B) $(<:O=2))
			:INSTALLDIR: $(L)
			X += $(LIBDIR)/lib/$(P)$(B) $(P)$(B).req
		end
		X += $(Y)
		if ! "$(>:N=[-+]l$(B))"
			X += $(.LIB.NAME. $(B) $(<:O=2))
		end
		if X = "$(X:U:T=F)"
			.LIBRARY.CLEANUP.$(B) : $(X)
			.INSTALL : $$(*.LIBRARY.CLEANUP.$(B):T=F)
			$(X) : .SPECIAL
				$(RM) $(RMFLAGS) $(<)
		end
		X := $(.DLL.NAME. $(B) $(<:O=2):B:C%\..*%%)
		if CC.SHARED.REGISTRY
			.CC.SHARED.REGISTRY.$(X) := $(LIBDIR)/$(P)/registry.ld
			$(.DLL.NAME. $(B) $(<:O=2)) : .CC.DLL.DIR.INIT
			.CC.DLL.DIR.INIT : .VIRTUAL .IGNORE $(LIBDIR)/$(P)
			$(LIBDIR)/$(P) : .DO.INSTALL.DIR
		end
		.CC.DLL.DIR.$(X) := $(LIBDIR)/$(P)
		.INSTALL.$(X) := .
	end
	eval
	$(L) :: $(>:V:N!=[-+][lL]*)
		$(@:V)
	end
	.INSTALL : .DLL.CHECK.$(B)
	.DLL.CHECK.$(B) : .DLL.CHECK
	if ! ( V = "$(<:O=2)" )
		V = 1.0
	elif V == "-"
		V =
	elif ! VERSION
		VERSION := $(V)
	end
	$(B).VERSION := $(V)
	D := $(>:V:N=-L*)
	if ! P || A
		R := $(P)$(B)
		S := $(R) $(>:V:N=[-+]l*:/[-+]l//:N!=$(R)) $(.PACKAGE.LIBRARIES. $(.PACKAGE.build:A!=.TARGET):/^[-+]l//:N!=$(R)) $(LDLIBRARIES:N=[-+]l*:/^[-+]l//:N!=$(R))
		if ! "$(S:N=$(R))"
			S := $(R) $(S)
		end
		eval
		if ! "$(.NO.INSTALL.)"
			$$(LIBDIR)/lib/$(R) :INSTALL: $(R).req
		end
		.REQUIRE.$(R) = $(S:U)
		(.REQUIRE.$(R)) : .PARAMETER
		if "$(-mam:N=static*,port*)"
			$(R).req : (CC) (CCFLAGS) (LDFLAGS) (.REQUIRE.$(R))
				set -
				echo 'main(){return(0);}' > 1.$(tmp).c
				$$(CC) $$(CCFLAGS) -c 1.$(tmp).c &&
				x=`$$(CC) $$(CCFLAGS) $$(LDFLAGS) -o 1.$(tmp).x 1.$(tmp)$(CC.SUFFIX.OBJECT) -l'*' 2>&1 | $(SED) -e 's/[][()+@?]/#/g' || :` &&
				{
				case "$(D)" in
				*?)	echo " $(D)" ;;
				esac
				for i in $$(.REQUIRE.$(R))
				do	case $i in
					"$(R)"$(...:A=.ARCHIVE:A=.TARGET:N=$(CC.PREFIX.ARCHIVE)*$(CC.SUFFIX.ARCHIVE):/^$(CC.PREFIX.ARCHIVE)\(.*\)$(CC.SUFFIX.ARCHIVE)/|\1/:@/ //G))
						;;
					*)	if	test ! -f $$(LIBDIR)/$(CC.PREFIX.ARCHIVE)$i$(CC.SUFFIX.ARCHIVE)
						then	case `{ $$(CC) $$(CCFLAGS) $$(*.SOURCE.%.ARCHIVE:$$(.CC.NOSTDLIB.):N=*/$(CC.PREFIX.ARCHIVE)*:P=L:/^/-L/) $$(LDFLAGS) -o 1.$(tmp).x 1.$(tmp)$(CC.SUFFIX.OBJECT) $(D) -l$i 2>&1 || echo '' $x ;} | $(SED) -e 's/[][()+@?]/#/g' || :` in
							*$x*)	case `{ $$(CC) $$(CCFLAGS) $$(LDFLAGS) -o 1.$(tmp).x 1.$(tmp)$(CC.SUFFIX.OBJECT) $(D) -l$i 2>&1 || echo '' $x ;} | $(SED) -e 's/[][()+@?]/#/g' || :` in
								*$x*) continue ;;
								esac
								;;
							esac
						fi
						;;
					esac
					echo " -l$i"
				done
				} > $$(<)
				$$(RM) $$(RMFLAGS) 1.$(tmp).*
		else
			$(R).req : (CC) (.REQUIRE.$(R)) .PREQUIRE
				echo "" $$(.REQ. $$(.REQUIRE.$(R))) > $$(<)
		end
		$(L) : .INSERT $(R).req.REQUIRE
		$(R).req.REQUIRE : .VIRTUAL .IGNORE .NULL $(R).req
		end
	end
	eval
	_BLD_$(B:B:S:/[^a-zA-Z0-9_]/_/G) == 1
	end
	.LIBRARY.ONLY. += _BLD_$(B:B:S:/[^a-zA-Z0-9_]/_/G)=
	$(T) : $(L) $(.SHARED. $(L) $(B) $(V|"-") $(>:V:N=[!-+]*=*) $(>:V:N=[-+]l*))

.REQ. : .FUNCTION
	local I Q R
	for I $(%)
		I := $(I:/^[-+]l//)
		if "$(.REQUIRE.$(I))"
			R += -l$(I)
		else
			I := -l$(I)
			if Q = "$(.REQUIRE.-l% $(I))"
				if ! "$(Q:N=$(I))" && ! "$(MAKE_QUESTIONABLE_require)"
					continue
				end
			end
			if "$(I:T=F)"
				R += $(I)
			end
		end
	end
	return $(R)

.PREQUIRE : .MAKE .IGNORE
	local B P R
	B := $(<<:B)
	P := $((.PREQUIRE.$(B)))
	R := $(.REQ. $(.REQUIRE.$(B)))
	if "$(R)" != "$(P)"
		(.PREQUIRE.$(B)) := $(R)
		force $(<<)
	end

.SHARED.DEF. = .SHARED.DEF$(CC.SUFFIX.SHARED)
.SHARED.USE. = .SHARED$(CC.SUFFIX.SHARED)
.SHARED.FLAGS. = : $(CCFLAGS) :

.SHARED.ON. : .FUNCTION
	if ! "$(static)" && ! "$(CC.LIB.DLL:N=broken)" && "$(CC.DLL)" && ! "$(-mam:N=static*,port*)" && ( "$(.SHARED.FLAGS.:@N=* ($(CC.DLL)|$(CC.DLLBIG)) *)" || "$(.SHARED.FLAGS.:V:@N=* ($\(CC.DLL\)|$\(CC.DLLBIG\)) *)" )
		return 1
	end

.SHARED.LIST.LIBS. : .FUNCTION
	if $(MAKEVERSION:@/.* //:/-//G) >= 20030609
		return $(~~:VFU)
	end
	return $(~~:A!=.USE)

.SHARED.LIST. : .FUNCTION
	local N=0 L D X W G A R
	for L $(%)
		if L == "{"
			let N = N + 1
		elif L == "}"
			let N = N - 1
		elif L == "[-+]l*|*$(CC.SUFFIX.ARCHIVE)"
			if N <= 0
				G := $(L) $(G)
			elif N > 1
				W := $(L) $(W)
			else
				X := $(X) $(L)
			end
		else
			X := $(X) $(L)
		end
	end
	A := $(X) $(W) $(G)
	A := $(A:U)
	for L $(G) $(W)
		if L == "+l*"
			if ! "$(X:N=$(L))"
				D := $(L:/+/-/)
				if "$(A:N=$(D))" || "$(CC.STDLIB:N=$(D:T=F:P=D))"
					L := $(D)
				end
			end
		end
		if ! "$(R:N=$(L))"
			R := $(L) $(R)
		end
	end
	R := $(X) $(R)
	return $(R:T=F)

.SHARED. : .FUNCTION
	local A B D L S T
	if "$(.SHARED.ON.)"
		if "$(.SHARED.DEF.:A=.TARGET)"
			return $($(.SHARED.DEF.) $(%))
		end
		A := $(%:O=1)
		B := $(CC.PREFIX.SHARED)$(%:O=2)$(CC.SUFFIX.SHARED)
		L := $(%:O>3:N=[-+]l*)
		$(L) : .DONTCARE
		if "$(%:O=3)" != "[0-9]*"
			S := $(B)
		else
			S := $(B).$(%:O=3)
		end
		$(S) : .SHARED.o $(%:N=[!-+]*=*) $(A) $$(.SHARED.BIND. $(L))
		if ! "$(.INSTALL.$(S))" && ! "$(.NO.INSTALL.)"
			if ! ( D = "$(.CC.DLL.DIR.$(S:C%\..*%%))" )
				D = $(DLLDIR)
			end
			$(D:V) :INSTALLDIR: $(S)
				$(LD_PRELOAD:N=$(<:C%\$(CC.SUFFIX.SHARED)\..*%$(CC.SUFFIX.SHARED)%):?LD_PRELOAD=""; _RLD_LIST=DEFAULT;?)if	$(SILENT) test -f $(<:C%\$(CC.SUFFIX.SHARED)\.%.oo.%)
				then	$(STDRM) $(RMFLAGS) $(<:C%\$(CC.SUFFIX.SHARED)\.%.oo.%)
				fi
				if	$(SILENT) test -f $(<)
				then	$(STDMV) $(<) $(<:C%\$(CC.SUFFIX.SHARED)\.%.oo.%)
				fi
				$(STDCP) $(<:B:S) $(<)
				if	$(SILENT) test "$(<)" != "$(<:C%\$(CC.SUFFIX.SHARED)\..*%$(CC.SUFFIX.SHARED)%)"
				then	if	$(SILENT) test -f $(<:C%\$(CC.SUFFIX.SHARED)\..*%$(CC.SUFFIX.SHARED)%)
					then	$(STDRM) $(RMFLAGS) $(<:C%\$(CC.SUFFIX.SHARED)\..*%$(CC.SUFFIX.SHARED)%)
					fi
					$(STDLN) $(<) $(<:C%\$(CC.SUFFIX.SHARED)\..*%$(CC.SUFFIX.SHARED)%)
				fi
				chmod -w $(<)
		end
	end
	return $(S)

.SHARED.BIND. : .FUNCTION
	local L P S
	if ! "$(%)" || ! CC.SUFFIX.STATIC
		.NO.ARPROFILE = 1
		for L $(%)
			if "$(CC.SHARED)"
				if L == "+l*|-ldl|-liconv" /* XXX: probe!!! */
					S += $(L)
				else
					L := $(L:T=F)
					if L == "-l*"
						S += $(L)
					end
				end
			else
				L := $(L:T=F)
				if L == "*$(CC.SUFFIX.ARCHIVE)"
					S += $(L)
				end
			end
		end
		.NO.ARPROFILE =
	else
		for L $(%:T=WF:P=B:/\(.*\)\$(CC.SUFFIX.SHARED)\(\.[0-9.]*\)$/\1$(CC.SUFFIX.STATIC)\2)
			while 1
				if "$(L:T=F)"
					S += $(L)
					break
				end
				if "$(L)" != "*.+([0-9])"
					break
				end
				L := $(L:/\.[0-9]*$//)
			end
		end
	end
	return $(.SHARED.LIST. $(S))

.SHARED.o : .USE (LDSHARED) (LDFLAGS) $$(LDLIBRARIES)
	$(LDSHARED) $(LDFLAGS) $(CC.SHARED) -o $(<) $(.CC.LIB.DLL.$(CC.LIB.DLL) $(.SHARED.LIST. $(.SHARED.LIST.LIBS.:$(CC.SHARED:@??:T=F:N=*$(CC.SUFFIX.ARCHIVE)?)))) $(CC.DLL.LIBRARIES)

.SHARED.DEF.dll.a .SHARED.DEF.lib .SHARED.DEF.x : .FUNCTION
	local A B D I L S X Y Z W
	Y := $(%:O=2)
	B := $(Y)$(%:O=3:/[^0-9]//G)$(dll.custom:?_$(dll.custom)??)
	D := $(CC.PREFIX.DYNAMIC)$(B:B)$(CC.SUFFIX.DYNAMIC)
	if "$(%:O=1)" != "-"
		L := $(CC.PREFIX.SHARED)$(Y:B)$(CC.SUFFIX.SHARED)
		S := $(CC.PREFIX.SHARED)$(B:B)$(CC.SUFFIX.SHARED)
		Z := $(%:O>3:N=+l*)
		X := $(Y) $(Z:/+l//)
		Z += $(%:O>3:/.l//:N!=$(X:/ /|/):/^/-l/)
		$(Z) : .DONTCARE
		if CC.DLL.DIR == "$\(BINDIR)"
			W := $(Y:B:S=.so)
			D := $(W)/$(D)
			S := $(W)/$(S)
			if "$(*.LIBRARY.STATIC.$(Y))"
				Z += .LIBRARY.STATIC.$(Y)
				.LIBRARY.STATIC.$(Y) : .VIRTUAL
			end
			if CC.HOSTTYPE == "win32.*" && ! "$(.NO.INSTALL.)" && "$(.INSTALL.$(S))" != "." && "$(ARFLAGS)" != "*I*"
				ARFLAGS := $(ARFLAGS)I
			end
		end
		A := $(%:O=1)
		$(D) $(S) : .JOINT $(<:/DEF.//) $(*$(A):N=*@($(.LD.KEEP.:/ /|/G:/|$//))) $(A) $(Z)
		.ALL : $(D) $(S)
		if ! "$(.NO.INSTALL.)"
			X := $(D:B:C%\..*%%)
			if ! "$(.INSTALL.$(D))"
				if ! ( I = "$(.CC.DLL.DIR.$(X))" )
					I = $(DLLDIR)
				end
				$(I:V) :INSTALLDIR: $(D)
			end
			if ! "$(.INSTALL.$(S))" && ! "$(.INSTALL.$(X))"
				$$(LIBDIR)/$(L) :INSTALL: $(S)
			end
		end
	end
	return $(D)

.SHARED.REF.dll.a .SHARED.REF.lib : .FUNCTION
	local K L
	K := *@($(.LD.KEEP.:/ /|/G:/|$//))
	L := $(%:N=*$(CC.SUFFIX.ARCHIVE):O=1)
	return $(CC.LIB.ALL) $(L) $(CC.LIB.UNDEF) $(%:N=$(K)) $(*.LIBRARY.STATIC.$(L:B:/^$(CC.PREFIX.ARCHIVE)//):T=*) $(.SHARED.LIST. $(%:N!=*$(L)|$(K)))

.SHARED.dll.a .SHARED.lib : .USE (LDSHARED) (LDFLAGS) $$(LDLIBRARIES)
	$(SILENT) test -d $(<:O=1:D) || mkdir $(<:O=1:D)
	$(LDSHARED) $(LDFLAGS) $(CCFLAGS:N=-[gG]*) $(CC.SHARED) -o $(<:O=1) $(.SHARED.REF.lib $(.SHARED.LIST.LIBS.)) $(CC.DLL.LIBRARIES)

.SHARED.REF.x : .FUNCTION
	local L
	L := $(%:N=*$(CC.SUFFIX.ARCHIVE):O=1)
	return $(.CC.LIB.DLL.symbol $(L)) $(.SHARED.LIST. $(*.LIBRARY.STATIC.$(L:B:/^$(CC.PREFIX.ARCHIVE)//):T=*) $(%:N!=$(L)|*$(CC.SUFFIX.OBJECT)))

.SHARED.x : .USE (LDSHARED) (LDFLAGS) $$(LDLIBRARIES)
	$(LDSHARED) $(LDFLAGS) $(CCFLAGS:N=-[gG]*) $(CC.SHARED) -o $(<:O=1:B:S) $(.SHARED.REF.x $(.SHARED.LIST.LIBS.)) $(CC.DLL.LIBRARIES)

/*
 * link lhs to rhs
 */

":LINK:" : .MAKE .OPERATOR
	local D T U
	if "$(<:N=*/*)"
		.ALL : $(<:D) $(<)
		$(<:D) : .SPECIAL .DO.INSTALL.DIR
		D := .DO.INSTALL.DIR.$(<:D)
		$(D) : .VIRTUAL .IGNORE $(<:D)
		$(<) : .DO.LINK $(D) $(>)
	else
		if U = "$(<:N!=/*)"
			.ALL : $(U)
			$(U) : .SPECIAL .DO.LINK $(>)
		end
		if ( T = "$(.INSTALL.$(>:B:S):V)" ) != "." && "$(>:A=.ARCHIVE|.COMMAND)"
			if ! T
				if "$(>:A=.ARCHIVE)"
					T = $(LIBDIR)
				else
					T = $(BINDIR)
				end
			end
			.INSTALL : $(U:D=$(T:V):B:S) $(<:N=/*)
			$(U:D=$(T:V):B:S) $(<:N=/*) : .SPECIAL .SCAN.IGNORE .DO.LINK $(>:D=$(T:V):B:S)
		end
	end

/*
 * if rhs is dir then make recursively, else just make
 * rhs name=value are .EXPORT for sub-makes
 */

":MAKE:" : .MAKE .OPERATOR
	local ATT EXP LHS RHS
	if ! ( LHS = "$(<)" )
		LHS = .RECURSE
		.ALL .MAIN : $(LHS)
	end
	ATT := $(>:A=.ATTRIBUTE)
	for EXP $(>:N=*=*)
		eval
		$(EXP)
		end
		export $(EXP:/=.*//)
	end
	RHS = $(>:N!=*=*:A!=.ATTRIBUTE)
	eval
		$$(<) :
			$(@:V)
	end
	$(LHS) : $(ATT) $$(.RECURSE.INIT. $(RHS))

/*
 * rhs compilation with no optimization
 * lhs is CC.HOSTTYPE match pattern list to match
 * activated for all architectures if lhs omitted
 */

":NOOPTIMIZE:" : .MAKE .OPERATOR .PROBE.INIT
	local T P
	.P. : .CLEAR $(<)
	P := $(*.P.:/ /|/G)
	if ! "$(P)" || "$(-mam)" == "static*" || CC.HOSTTYPE == "$(P)"
		for T $(>)
			if "$(@.NOOPTIMIZE$(T:S):V)"
				$(T:B:S=$(CC.SUFFIX.OBJECT)) : .SPECIAL .NOOPTIMIZE$(T:S)
			else
				error 1 :NOOPTIMIZE: ignored for suffix $(T:S)
			end
		end
	end

/*
 * force default and the usual suspects to do nothing
 */

":NOTHING:" : .MAKE .OPERATOR
	.ALL .INSTALL .MAIN all cc- install : .CLEAR .DO.NOTHING

/*
 * var :OPTIONAL: src ...
 *
 *	lhs var set to rhs src and rhs asserted on ::
 */

":OPTIONAL:" : .MAKE .OPERATOR
	$(<) := $(>)
	:: $(>)

/*
 * rhs are package names used in compilation
 * a package consists of a library and include directory
 * the most recent :PACKAGE: assertion takes highest precedence
 * .PACKAGE. is the ordered package list used to alter
 * .SOURCE.h, INCLUDEDIR, LDLIBRARIES and ancestor
 * .PACKAGE.build is the package list to build against
 * $(PACKAGE_<package>_INCLUDE) explicit include for <package>
 * $(PACKAGE_<package>_LIB) explicit lib dir for <package>
 * $(PACKAGE_<package>) root dir for include|lib for <package>
 * $(PACKAGE_PATH) default root dirs for all packages
 * _PACKAGE_<package>==1 state var defined for <package>
 * the following options either control all packages (: starts token)
 * or individual packages (: appended to package name)
 *
 *	:version=xxx:	attempt version xxx first
 *	:noinstall:	do not redefine INCLUDEDIR
 *	:nolibrary:	do not link with library
 *	:static:	attempt static library first
 *	:dynamic:	attempt dynamic library first
 *	:insert:	insert into package list
 *	:noregistry:	do not link dll against address registry
 */

.PACKAGE. =
.PACKAGE.build =
.PACKAGE.install =
.PACKAGE.libraries =
.PACKAGE.plugin =
.PACKAGE.registry = 1
.PACKAGE.stdlib = $(*.SOURCE.a) $(CC.STDLIB) /usr/lib /lib
.PACKAGE.strip =

.PACKAGE.LIBRARY. : .FUNCTION
	local P R
	if P = "$(<:T=M:A=.COMMAND:O=1)"
		if R = "$(.PACKAGE.$(%).library.$(P))"
			return $(R)
		end
		if R = "$(.PACKAGE.$(%).library.weak.$(P))"
			return $(R)
		end
	end
	return $(.PACKAGE.$(%).library)

.PACKAGE.LIBRARIES. : .FUNCTION
	local L P R
	for L $(%)
		if P = "$(.PACKAGE.LIBRARY. $(L))"
			if ! "$(**:B:S:N=[-+]l$(L)|$(CC.PREFIX.ARCHIVE)$(L)$(CC.LIB.TYPE:?*($(CC.LIB.TYPE:/ /|/G))??)$(CC.SUFFIX.ARCHIVE))"
				$(P)$(L) : .DONTCARE
				R += $(P)$(L)
			end
		end
	end
	.UNBIND : $(R)
	R += $(.PACKAGE.libraries)
	$(R) : .ARCHIVE .MULTIPLE
	return $(R)

/*
 * initialize the package args
 * on return the following are set if found for package $(P)
 *	PACKAGE_$(P)_INCLUDE	package include dir
 *	PACKAGE_$(P)_LIB	package lib dir
 *	.PACKAGE.$(P).found	1 if at least one package lib or header found
 * the canonicalized last package name is returned
 */

.PACKAGE.CONFIG. = lib/pkgconfig
.PACKAGE.GLOBAL. =
.PACKAGE.LOCAL. =

.PACKAGE.INIT. : .FUNCTION .PROBE.INIT
	local T1 T4 T5 T6 T7
	local B D G H I K L N P Q T V W X Z IP LP LPL LPV PFX SFX FOUND
	if ! .PACKAGE.GLOBAL.
		.PACKAGE.GLOBAL. := $(PATH:/:/ /G:D) $(OPTDIRS:/:/ /G)
		.PACKAGE.GLOBAL. := $(.PACKAGE.GLOBAL.:N!=$(PACKAGE_IGNORE):T=F:U)
		.PACKAGE.CONFIG. := $(.PACKAGE.GLOBAL.:X=$(.PACKAGE.CONFIG.):T=F:U)
	end
	for P $(%)
		FOUND = 0
		I := $(PACKAGE_$(P)_INCLUDE)
		IP := $(.PACKAGE.$(P).include)
		if IP == "/*"
			if ! I
				I := $(IP)
			end
			IP =
		end
		if ! IP
			IP = include
		end
		L := $(PACKAGE_$(P)_LIB)
		LP := $(.PACKAGE.$(P).lib)
		if LP == "/*"
			if ! L
				L := $(LP)
			end
			LP =
		end
		if ! LP
			LP = lib
		end
		if ( I && L )
			FOUND = 1
		else
			if T1 = "$(.PACKAGE.CONFIG.:L!~?(lib)($(P)|$(P)-*).pc:O=1)"
				T1 := $(T1:T=I:/${\([^}]*\)}/$$(\1)/G)
				local $(T1:N=+([[:alnum:]_[:space:]])=*:/=.*//)
				$(T1:N=+([[:alnum:]_[:space:]])[:]*:/:.*//) : .CLEAR
				: $(T1:V:R)
				if !I
					I := $(~Cflags:N=-I*:/-I//)
					PACKAGE_$(P)_INCLUDE := $(I)
				end
				if !L
					L := $(~Libs:N=-L*:/-L//)
					PACKAGE_$(P)_LIB := $(L)
				end
				CC.REQUIRE.$(P) := $(~Libs:N=[-+]l*)
				PACKAGE_$(P) := /
				FOUND = 1
			end
			if ( !I && !L )
				T1 = $(INSTALLROOT)/$(IP)/$(P)
				if "$(T1:P=X)"
					for K SHARED ARCHIVE
						T1 = $(INSTALLROOT)/$(LP)/$(CC.PREFIX.$(K))$(P)$(CC.SUFFIX.$(K))
						if "$(T1:P=X)"
							eval
							PACKAGE_$(P) = $(INSTALLROOT)
							end
							break
						end
					end
				end
			end
			if ( !I || !L ) && ! ( T1 = "$(PACKAGE_$(P))" )
				T4 := $(PACKAGE_PATH:/:/ /G) $(.PACKAGE.DIRS.) $(.PACKAGE.GLOBAL.) $(.PACKAGE.GLOBAL.:/:/ /G:C%$%/$(P)%)
				if P == "*[!0-9]+([0-9])"
					T4 += $(.PACKAGE.GLOBAL.:/:/ /G:C%$%/$(P:C,[0-9]*$,,)%)
				end
				V =
				if T5 = "$(PACKAGE_$(P)_VERSION)"
					N := $(P)-$(T5) $(P)$(T5) $(P)R$(T5) $(P)r$(T5)
					B =
					for T5 $(T5:/\./ /G)
						if V
							V := $(V)*(?(.)$(T5)
							B := $(B))
						else
							V := $(T5)
						end
					end
					V := $(V)$(B)
				elif P == "*[0-9][rR]+([0-9])"
					N := $(P)
					P := $(N:/[rR][0-9]*$//)
				else
					N :=
				end
				if B = "$(P:N=[!0-9]*+([0-9.]):/[0-9]*$//)"
					B := @($(P)|$(B))
				else
					B := $(P)
				end
				N += $(P)
				if P == "*[!0-9]+([0-9])"
					N += $(P:C,[0-9]*$,,)
				end
				LPL := $(CC.STDLIB:B) $(LP)
				LPL := $(LPL:U)
				T4 := $(T4:N!=//*:T=F:U)
				T7 := $(T4:X=$(IP)/$(P):N!=//*:T=F:D:D)
				T1 =
				for K SHARED ARCHIVE
					SFX := $(CC.SUFFIX.$(K))
					PFX := $(CC.PREFIX.$(K))
					for D $(N) /
						T5 := $(T7) $(T4:N=*/$(D):D) $(T4)
						T5 := $(T5:U)
						if ! V
							if ! .PACKAGE.LOCAL.
								.PACKAGE.LOCAL. := $(PKGDIRS:T=F:P=A:U)
							end
							for X $(.PACKAGE.LOCAL.)
								X := $(X)/$(PFX)$(P)$(SFX)
								if ( T1 = "$(X:P=X:O=1:D:D)" )
									break 3
								end
							end
						end
						for LPV $(LPL)
							X := $(T5:C%$%/$(D)/$(LPV)/$(P)$(P)$(SFX)%:C%^//%/%)
							if ( T1 = "$(X:N!=//*:P=X:O=1:D:D)" )
								break 3
							end
							if T6 = "$(X:D:N!=//*:P=X:O=1)"
								if Z = "$(T6:L>=?(lib)$(B)*$(SFX)*([0-9.]))"
									if V
										if W = "$(Z:N=*$(V)*:O=1)"
											T1 := $(T6:D)
											break 3
										end
									end
									T1 := $(T6:D)
									break 3
								end
							end
						end
					end
				end
				if "$(T1)"
					FOUND = 1
				elif ! ( T1 = "$(T4:C%$%/$(IP)/$(P)%:N!=//*:P=X:O=1:D:D)" )
					T1 := $(T4:N=*/$(P):C%$%/$(IP)%:N!=//*:P=X:O=1:D)
				end
				if T1 == "/"
					T1 = /usr
				end
				eval
				PACKAGE_$(P) = $(T1:P=C)
				end
			end
			if T1
				.PACKAGE.DIRS. += $(T1)
				if !I
					I := $(T1)/$(IP)
					H := $(I)/$(P)/.
					if "$(H:P=X)"
						I := $(I)/$(P)
					end
					eval
					PACKAGE_$(P)_INCLUDE = $(I)
					end
				end
				if !L
					L := $(T1)/$(LP)
					eval
					PACKAGE_$(P)_LIB = $(L)
					end
					if ! "$(.PACKAGE.stdlib:N=$(L))"
						.SOURCE.a : $(L)
					end
				end
				eval
				_PACKAGE_$(P) $(.INITIALIZED.:?=?==?) 1
				end
				if ! FOUND
					I := $(I)/$(P:/[0-9]*$//)*.h*
					if "$(I:P=G:O=1:P=X)"
						FOUND = 1
					end
				end
			end
		end
		.PACKAGE.$(P).found := $(FOUND)
	end
	return $(P)

/*
 * evaluate a logical expression of package ids
 * used by :VARIANT:
 */

PACKAGES : .SPECIAL .FUNCTION
	local P M X
	if ! "$(%)"
		return 1
	end
	M := $(.PACKAGE.:/ /|/G)
	for P $(%:/[^a-zA-Z0-9_.]/ /G)
		if "$(P)" != "$(M)"
			: $(.PACKAGE.INIT. $(P))
		end
	end
	X := $(%)
	for P $(.PACKAGE.)
		X := $(X:/\<$(P)\>/1/G)
	end
	return $(X:/\([A-Za-z_.][A-Za-z0-9_.]*\)/"$$(.PACKAGE.\1.found)"=="1"/G:@/"  *"/" \&\& "/G:E)

":PACKAGE:" : .MAKE .OPERATOR
	local A H I T N P V version insert=0 install=1 library=-l
	if "$(<)"
		/* a separate include handles package definitions */
		eval
		$$(<:V) :package: $$(>:V)
			$(@:V)
		end
		return
	end
	if ! "$(ancestor)"
		/* please convert me to long op names */
		let ancestor = $(".":P=L=*:C,[^/],,G:O!:H=N:O=1) - 2
		if ancestor > 3
			ancestor = 3
		elif ancestor < 0
			ancestor = 0
		end
	end
	for T $(>)
		if T == "{" || T == "}"
			.PACKAGE.build += $(T)
		elif T == "+"
			break
		elif T == "-"
			install = 0
		elif T == "-*"
			.PACKAGE.libraries += $(T)
		elif T == ":*"
			for N $(T:/:/ /G)
				if N == "no*"
					V := 0
					N := $(N:/no//)
				else
					V := 1
				end
				if N == "*=*"
					V := $(N:/[^=]*=//)
					N := $(N:/=.*//)
				end
				S =
				if N == "optimize"
					if "$(PACKAGE_OPTIMIZE:N=space)"
						N = dynamic
						.PACKAGE.strip = $(CC.LD.STRIP)
					elif "$(PACKAGE_OPTIMIZE:N=time)"
						N = static
					else
						N =
					end
				elif N == "space"
					if ! "$(PACKAGE_OPTIMIZE:N=time)"
						N = dynamic
						.PACKAGE.strip = $(CC.LD.STRIP)
					else
						N =
					end
				elif N == "time"
					if ! "$(PACKAGE_OPTIMIZE:N=space)"
						N = static
					else
						N =
					end
				end
				if N == "dynamic"
					library := -l
				elif N == "static"
					if V
						library := +l
					else
						library := -l
					end
				elif N == "debug|insert|install|profile|threads|version"
					$(N) := $(V)
				elif N == "registry"
					.PACKAGE.$(N) := $(V)
				elif N == "ignore"
					if PACKAGE_IGNORE
						PACKAGE_IGNORE := $(PACKAGE_IGNORE)|$(V)
					else
						PACKAGE_IGNORE := $(V)
					end
				end
			end
		elif T != "*:(command|force|prereq)"
			if T == "*:*"
				P := $(T:/:.*//)
				T := $(T:/[^:]*//)
			else
				P := $(T)
				T :=
			end
			I = pkg-$(P).mk
			V := $(version)
			while 1
				if H = "$(I:T=F)"
					.PACKAGE.$(P).rules := $(N)
					break
				elif P == "*[0-9][rR]+([0-9.])"
					N := $(P:/[rR][0-9.]*$//)
					V := $(P:/$(N).\([0-9.]*\)$/\1/)
					P := $(N)
				elif P == "*?(-)+([0-9.])"
					N := $(P:/-*[0-9.]*$//)
					V := $(P:/$(N)-*\([0-9.]*\)$/\1/)
					P := $(N)
				else
					break
				end
			end
			if ! "$(.PACKAGE.$(P).rules)"
				.PACKAGE.$(P).rules := -
			end
			.PACKAGE.$(P).library := $(library)
			if "$(V)"
				PACKAGE_$(P)_VERSION := $(V)
			end
			I := $(insert)
			A := 0
			for N $(T:/:/ /G)
				if N == "no*"
					V := 0
					N := $(N:/no//)
				else
					V := 1
				end
				if N == "*=*"
					V := $(N:/[^=]*=//)
					N := $(N:/=.*//)
				end
				if N == "optimize"
					if "$(PACKAGE_OPTIMIZE:N=space)"
						N = dynamic
					elif "$(PACKAGE_OPTIMIZE:N=time)"
						N = static
					else
						N =
					end
				elif N == "space"
					if ! "$(PACKAGE_OPTIMIZE:N=time)"
						N = dynamic
					else
						N =
					end
				elif N == "time"
					if ! "$(PACKAGE_OPTIMIZE:N=space)"
						N = static
					else
						N =
					end
				end
				if N == "dontcare"
					if V
						.PACKAGE.$(P).dontcare := 1
					end
				elif N == "dynamic"
					.PACKAGE.$(P).library := -l
				elif N == "static"
					.PACKAGE.$(P).library := +l
				elif N == "library"
					if V
						.PACKAGE.$(P).library := -l
					else
						.PACKAGE.$(P).library :=
					end
				elif N == "include|lib"
					.PACKAGE.$(P).$(N) := $(V)
				elif N == "version"
					PACKAGE_$(P)_VERSION := $(V)
				elif N == "install"
					$(N) := $(V)
				elif N == "insert"
					I := 1
				elif N == "attributes"
					A := $(V)
				elif N == "plugin"
					A = 1
					.PACKAGE.$(N) := $(P)
				end
			end
			if ! A
				if ! "$(.PACKAGE.$(P).library)"
					-l$(P) : .VIRTUAL
				end
				eval
				_PACKAGE_$(P) $(.INITIALIZED.:?=?==?) 1
				end
				if install && ! .PACKAGE.install
					if "$(INCLUDEDIR:V)" == "\$\(INSTALLROOT\)/include"
						.PACKAGE.install = 1
						.PACKAGE.$(P).dontcare := 1
						INCLUDEDIR := $(INCLUDEDIR:V)/$(P)
					end
				end
				if ! "$(.PACKAGE.:N=$(P))"
					if I
						.PACKAGE. := $(P) $(.PACKAGE.)
						.PACKAGE.build := $(P) $(.PACKAGE.build)
					else
						.PACKAGE. += $(P)
						.PACKAGE.build += $(P)
					end
				end
				if "$(H)"
					include + "$(H)"
				end
			end
		end
	end

/*
 * :PACKAGE_INIT: foo.c bar.c ...
 *
 *	rhs built and copied to $(BINDIR) if not already there
 *	rhs appear in $(BINDIR) for .LIST.PACKAGE.BINARY
 *	for source that compiles either standalone or with richer libraries
 *	lhs for pre-installed $(BINDIR) files listed by .LIST.PACKAGE.BINARY
 */

":PACKAGE_INIT:" : .MAKE .OPERATOR
	local I B
	.LIST.PACKAGE.BINARY : .LIST.PACKAGE.INIT
	eval
	.LIST.PACKAGE.INIT : .MAKE
		local I
		for I $$(*)
			print ;;;$$(I:T=F:P=A);$$(BINDIR)/$$(I:B)
		end
		for I $(<)
			if I != "/*"
				I := $$(I:D=$$(BINDIR):B:S)
			end
			if I = "$$(I:T=F)"
				print ;;;$$(I:P=A)
			end
		end
	end
	for I $(>)
		B := $(I:B)
		if ! "$(.NO.INSTALL.)"
			$(BINDIR) :INSTALLDIR: $(B)
				if	test ! -f $(<)
				then	$(CP) $(*) $(<)
				fi
		end
		$(B) :: $(I)
		.LIST.PACKAGE.INIT : $(B)
	end

/*
 * rhs compilation to place tables and/or data in
 * readonly text object section -- should go away
 * when C const is used/implemented to do same
 * lhs is CC.HOSTTYPE match pattern to match
 * activated for all architectures if lhs omitted
 */

":READONLY:" : .MAKE .OPERATOR .PROBE.INIT
	local T
	if ! "$(<)" || "$(-mam)" == "static*" || CC.HOSTTYPE == "$(<)"
		for T $(>)
			if "$(@.READONLY$(T:S):V)"
				$(T:B:S=$(CC.SUFFIX.OBJECT)) : .SPECIAL .READONLY$(T:S)
			else
				error 1 :READONLY: ignored for suffix $(T:S)
			end
		end
	end

/*
 * rhs are generated but still saved by save common actions
 */

":SAVE:" : .MAKE .OPERATOR
	.COMMON.SAVE : $(>)

/*
 * rhs are metarule patterns that do not generate %
 */

":TERMINAL:" : .MAKE .OPERATOR
	.METARULE.X. : $(>)

/*
 * cc-* variants
 *
 *	id :VARIANT: [ package-expr ]
 *		n1 = v1
 *		n2 = v2
 *		...
 *
 *	:VARIANT: [ arch:suffix:option ]
 */

":VARIANT:" : .MAKE .OPERATOR
	if "$(<)"
		.VARIANTS. += cc-$(<)
		.CLOBBER. += cc-$(<)
		eval
		cc-$$(<) : .ALWAYS .LOCAL .FORCE .SPECIAL .RECURSE.SEMAPHORE
			set -
			if	test "0" != "$$(PACKAGES $(>))"
			then	if	test ! -d $$(<)
				then	mkdir $$(<)
				fi
				$$(-silent:Y%%echo $$(.RWD.:C%$%/%)$$(<): >&2%)
				cd $$(<)
				$$(MAKE) --file=$$(MAKEFILE) --keepgoing $$(-) --errorid=$$(<) _BLD_$$(<:/cc-//:/[^a-zA-Z0-9_]/_/G)==1 VARIANT=$$(<:/cc-//) VARIANTID=$$(<:/cc-//:N=[a-zA-Z]*:?-??)$$(<:/cc-//) .ATTRIBUTE.$(IFFEGENDIR)/%:.ACCEPT MAKEPATH=..:$$(MAKEPATH) $$(=:V:N!=MAKEPATH=*) $$(.RECURSE.ARGS.:N!=.CC-*) $(@:V:@Q)
			fi
		end
	else
		local V
		for V $(>|CC.DLL.VARIANTS)
			local ( A S O ... ) $(V:/:/ /G)
			eval
			-$(S) :VARIANT:
				CC = cc $(O)
			end
		end
	end

/*
 * external distribution workarounds
 *
 *	target :WORKAROUND:		target is empty
 *	target :WORKAROUND: -		target is .TERMINAL
 *	target :WORKAROUND: prereq	target copied from prereq
 *	target :WORKAROUND: prereq	target from action applied to prereq
 *		action
 */

.WORKAROUND.COVERED. :FUNCTION:
	local T
	if T = "$(^^)"
		return $(T)
	end
	if T = "$(<<:P=L=*:O=2)"
		return $(T)
	end

":WORKAROUND:" : .MAKE .OPERATOR
	if "$(>)" == "-"
		$(<) : .TERMINAL
	else
		eval
		$$(>) : .SCAN.NULL
		$$(<) : -TERMINAL $$(>)
			$(@:V:@?$$(>:V:@Y%%test "" != "$$$$(.WORKAROUND.COVERED.)" && { $$$$(SILENT) test -d $$$$(<:D) || $$$$(MKDIR) $$$$(<:D); $$$$(CP) $$$$(.WORKAROUND.COVERED.) $$$$(<) ;}$$("\n")%)$$(@:V)?$$(>:@Y%$$$$(SILENT) test -d $$$$(<:D) || $$$$(MKDIR) $$$$(<:D); $$$$(CP) $$$$(*)%: >%) $$$(<)?)
		end
	end

/*
 * :YYPREFIX: [prefix] [header.h] a.y b.l
 */

":YYPREFIX:" : .MAKE .OPERATOR
	local H P F
	for F $(>)
		if "$(F)" != "*.*"
			P := $(F)
		elif "$(F)" == "*.h"
			H := $(F)
		elif "$(F)" == "*.l"
			LEXFIX.$(F:B) := $(P)
			LEXHDR.$(F:B) := $(H)
		elif "$(F)" == "*.y"
			YACCFIX.$(F:B) := $(P)
			YACCHDR.$(F:B) := $(H)
		end
	end
	
/*
 * rhs are compiled using $(cc) rather than $(CC)
 * use like ::
 * NOTE: case ignorant filesystems make this a little tricky
 */

":cc:" : .MAKE .OPERATOR .PROBE.INIT
	if "$(CC)" != "$(cc)"
		local O S T
		for S $(.SUFFIX.$(CC.DIALECT:N=C++:?c?C?))
			T := $(>:G=%$(S))
			if T == "*$(S)" || S == ".c" && T != "*.C" || S == ".C" && T != "*.c"
				O += $(T:B:S=$(CC.SUFFIX.OBJECT))
			end
		end
		O := $(O:U)
		.CLOBBER. += null.mo null.ms
		:: $(>)
		$(<) :: $(O)
		$(O) : .CLEAR .JOINT .DO.cc
	end

.DO.cc : .USE .ALWAYS .LOCAL .FORCE
	$(-exec:?silent ??)$(MAKE) --file=/dev/null --file=$(MAKEFILE) $(-) --errorid=$(cc:N!=*=*:B) '.null : $(<)' .null $(=) CC=$(cc:@Q) CCFLAGS=$(CCFLAGS:VP:@Q)

/*
 * make scripts
 */

.CLEARARGS : .MAKE .VIRTUAL .FORCE
	.ARGS : .CLEAR

.SELECT. : .MAKE .VIRTUAL .FORCE .FUNCTIONAL
	local T X
	.UNION. : .CLEAR
	if T = "$(*.ARGS)"
		.ARGS : .CLEAR
		.MAIN.TARGET. := $(T:O=1:B:S)
		for X $(T)
			T := $(.FILES.$(X:B:S):T=F:T!=G)
			$(T:N!=[-+]l*) : -ARCHIVE -COMMAND -OBJECT
			.UNION. : $(T) $(?$(X:B:S):T=F:P=S:T!=G)
		end
	else
		.UNION. : $(.FILES.:T=F:T!=G)
		$(*.UNION.:N!=[-+]l*) : -ARCHIVE -COMMAND -OBJECT
		.UNION. : $(...:T!=XS:T=F:A=.REGULAR:P=S:T!=G)
		.UNION. : $(...:T=XSFA:T=F:A=.REGULAR:P=S:T!=G)
	end
	return $(*.UNION.:$(select))

.GENSYM.COUNT. = 0

.RETAIN : .GENSYM.COUNT.

.GENSYM. : .FUNCTION
	let .GENSYM.COUNT. = .GENSYM.COUNT. + 1
	return .GENSYM.$(.GENSYM.COUNT.).

.INSTALL.LIST. : .FUNCTIONAL .FORCE .MAKE 
	set noalias /* don't tell dmr */
	: $(!.INSTALL)
	.MAKE : $(~.INSTALL:A=.MAKE)
	return $(*.INSTALL:N=$(INSTALLROOT)/*) $(*.INSTALL:N=$(INSTALLROOT)/*$(CC.SUFFIX.SHARED).+([0-9.]):/\.[0-9.]*$//)

.PROBE.LOAD : .MAKE .VIRTUAL .FORCE
	.PROBE.SPECIAL. = CC.HOSTTYPE CC.LD.DYNAMIC CC.LD.STATIC
	$(.PROBE.SPECIAL.) : .FUNCTION
		$(.PROBE.SPECIAL.) : -FUNCTIONAL
		make .PROBE.INIT
		return $($(<))

.PROBE.INIT : .MAKE .VIRTUAL .FORCE
	local I
	if "$(.COMMAND.CC:N=$(CC))"
		if ! "$(PATH:/:/ /G:X=$(CC):P=X)"
			for I $(.COMMAND.CC:N!=$(CC))
				if "$(PATH:/:/ /G:X=$(I):P=X)"
					CC := $(I)
					break
				end
			end
		end
	end
	$(.PROBE.SPECIAL.) : -FUNCTIONAL
	if "$(-base)"
		cctype =
		.CC.PROBE. =
	else
		if ! cctype
			cctype := $(CC:N!=*=*)
			if ! cctype
				cctype = cc
			end
		end
		cctype := $(cctype:O=1:P=C) $(cctype:O>1)
		if ! ( .CC.PROBE. = "$(CC.PROBE)" )
			.CC.PROBE. := $(cctype|"cc":@P=P=C)
		end
		.CC.PROBE. : .VIRTUAL $(.CC.PROBE.)
		$(.CC.PROBE.) : .SPECIAL .ACCEPT .DONTCARE
		(AR) (AS) (CPP) (CC) (LD) : .CC.PROBE.
		if .CC.PROBE.
			include "$(.CC.PROBE.)"
			if ! "$(CC.HOSTTYPE)"
				CC.HOSTTYPE := $(_hosttype_)
			end
		else
			if cctype
				error 1 $(cctype): C probe failed -- default assumed
			end
			if ! CC.STDINCLUDE
				CC.STDINCLUDE = /usr/include
			end
			if ! CC.MAKE.OPTIONS
				CC.MAKE.OPTIONS = nativepp=-1
			end
		end
	end
	: $(.MAKE.OPTIONS. $(CC.MAKE.OPTIONS))
	$(CC.LD.DYNAMIC) $(CC.LD.STATIC) : .MULTIPLE .ARCHIVE .IGNORE .DO.NOTHING
	if ! CC.SUFFIX.ARCHIVE
		CC.SUFFIX.ARCHIVE = .a
	end
	.ATTRIBUTE.%$(CC.SUFFIX.ARCHIVE) : .ARCHIVE
	if CC.SUFFIX.STATIC
		.ATTRIBUTE.%$(CC.SUFFIX.STATIC) : .ARCHIVE
	end
	if CC.SUFFIX.DYNAMIC && CC.SUFFIX.SHARED
		.ATTRIBUTE.%$(CC.SUFFIX.SHARED) : .ARCHIVE .SCAN.IGNORE
		.SOURCE.%$(CC.SUFFIX.SHARED) : .FORCE $$(*.SOURCE.%.ARCHIVE)
	end
	if ! CC.SUFFIX.OBJECT
		CC.SUFFIX.OBJECT = .o
	end
	.ATTRIBUTE.%$(CC.SUFFIX.OBJECT) : .OBJECT
	if CC.SHARED.REGISTRY
		CC.SHARED += $$(.CC.SHARED.REGISTRY.)
	end

	/*
	 * this is a workaround hack to help packages with broken compilers
	 * don't rely on this hook
	 *
	 * sco.i386 may need
	 *	export _MAKE_PROBE_WORKAROUND_='CC.LIB.DLL=broken static=1'
	 */

	if "$(_MAKE_PROBE_WORKAROUND_)"
		local I
		for I $(_MAKE_PROBE_WORKAROUND_)
			eval
			$(I)
			end
		end
	end

.MAKEINIT : .MAKE .VIRTUAL .FORCE
	local T1 T2 T3 T4 T5 T6 TI

	/*
	 * .SOURCE.mk gets bound long before the first user makefile is read
	 * this assertion ensures that any user specified dirs appear
	 * before the internal defaults set in the initdynamic[] script
	 */

	.SOURCE.mk : .CLEAR . $(*.SOURCE.mk:N!=$(*.SOURCE.mk.INTERNAL:/ /|/G)) $(*.SOURCE.mk.INTERNAL)
	.MAKE : .PROBE.INIT
	if "$(instrument)"
		if ! ( instrument.root = "$(instrument:O=1:D:N!=.:T=F)" )
			T1 = $(PATH):$(OPTDIRS)
			instrument.root := $(T1:/:/ /G:X=$(instrument:O=1:B)/.:P=X:D)
			if ! instrument.root
				instrument.root = $(.INSTRUMENT.notfound)
			end
		end
		T2 := $(instrument:O=1:B)
		T1 := $(INSTRUMENT_$(T2))
		if ! T1 || ! "$(T1:N!=*=*)"
			T5 =
			T6 =
			for T3 $(T1:N=*=*:/command=/TI=/:/root=/T5=/:/bin=/T6=/)
				eval
				$(T3)
				end
			end
			if ! TI
				TI = CC
				CCFLAGS := $(CCFLAGS:N!=-g|-O*|-p*|$(CC.DEBUG)|$(CC.OPTIMIZE)) $(CC.DEBUG)
			end
			T1 := $(TI) = $($(TI):N=*=*)
			if ! T5
				T2 := $(T2:F=%(upper)s)
				T5 = $(T2)HOME
				for T3  $(T2) $(T2)HOME $(T2)_HOME $(T2)ROOT $(T2)_ROOT
					if "$($(T3):T=F)"
						T5 := $(T3)
						break
					end
				end
			end
			if "$($(T5))"
				T5 := $($(T5))
			else
				T1 += $(T5)=$(instrument.root)
				T5 := $(instrument.root)
			end
			if ! "$(PATH:/:/ /G:X=$(instrument):P=X)"
				if T6
					T5 := $(T5)/$(T6)
				end
				T1 += PATH=$(PATH):$(T5)
			end
			T1 += $(instrument) $($(TI):N!=*=*)
		end
		if T1
			CC.READONLY =
			set expandview
			eval
			$(T1:V)
			end
		end
	end
	if "$(-mam:N=static*)"
		nativepp = -1
	end
	if "$(debug)" && ! "$(CCFLAGS:N=$(CC.DEBUG))"
		CCFLAGS := $(CCFLAGS:V:N!=-O*|$(CC.OPTIMIZE)|$\(CC.OPTIMIZE\)) $(CC.DEBUG)
	end
	if "$(profile)" && ! "$(CCFLAGS:N=$(CC.PROFILE))"
		CCFLAGS += $(CC.PROFILE)
	end
	T1 :=
	T2 :=
	if ! "$(-mam:N=static*,port*)"
		if "$(instrument)"
			T1 := $(instrument:B:/\(...\).*/-\1/)
		end
		if "$(threads)"
			T2 := t$(T2)
		end
		if "$(debug)" || "$(CCFLAGS:N=$(CC.DEBUG))"
			T2 := $(T2)g
		end
		if ! "$(T2)"
			if T3 = "$(CCFLAGS:N=-p|-pg)"
				T2 := $(T3:O=1:/-//)
			elif T3 = "$(LDFLAGS:N=-O[1-9])"
				T2 := $(T3:O=1:/-//)
			end
		end
		if T2
			T2 := -$(T2)
		end

		/*
		 * insert default flags
		 */

		for T6 $(...:T=XQSV:N=\(*FLAGS\):/[()]//G)
			if "$($(T6)_DEFAULT:V)"
				$(T6) := $$($(T6)_DEFAULT) $($(T6):V)
			end
		end
	end
	CC.LIB.TYPE := $(T1)$(T2)
	if ! CC.LIB.THREADS
		CC.LIB.THREADS = -lpthreads
	end
	$(CC.LIB.THREADS) : .DONTCARE
	if ancestor
		if "$(*.VIEW:O=2:P=I=..)"
			let ancestor = ancestor + 1
		end
		if "$(-mam:N=static*)"
			T1 = ..
			let T2 = 1
			while T2 < ancestor
				let T2 = T2 + 1
				T1 := ../$(T1)
			end
			print -um setv INSTALLROOT $(T1)
			T4 =
			for T3 $(ancestor_list)
				if T4
					$(T4) : $(T1)/$(T3)
					T4 =
				else
					T4 := $(T3)
				end
			end
		end
		T1 = ..
		let T2 = 0
		while T2 < ancestor
			let T2 = T2 + 1
			T4 =
			for T3 $(ancestor_list)
				if T4
					$(T4) : $(T1)/$(T3)
					T4 =
				else
					T4 := $(T3)
				end
			end
			T1 := ../$(T1)
		end
	elif "$(-mam:N=static*)"
		print -um setv INSTALLROOT $(INSTALLROOT:N=..*(/*):?$(INSTALLROOT)?$HOME?)
	end
	for T3 $(ancestor_list)
		if T4
			$(T4) : $($(T3:F=%(upper)s)DIR)
			T4 =
		else
			T4 := $(T3)
		end
	end
	if "$(-mam:N=dynamic*)"
		print -um setv INSTALLROOT $(INSTALLROOT:N=$(HOME):?$HOME?$(INSTALLROOT)?)
	end
	.MAMEDIT. =
	if "$(INSTALLROOT:N=..*(/*))"
		.MAMROOT. := $(INSTALLROOT)
	else
		.MAMROOT. := $(PWD:C@.*/src/@/src/@:C@/[^/]*@/..@G:C@/@@)
	end
	.SOURCE.h : $(*.SOURCE.c:N!=.) $(*.SOURCE:N!=.)
	.SOURCE .SOURCE.a .SOURCE.c .SOURCE.h : .INSERT .
	if CC.PREROOT && "$(CC.PREROOT:P!=I=/)"
		T1 := $(CC.PREROOT) /
		CC.STDINCLUDE := $(T1:X=$$(CC.STDINCLUDE:C@^/@@))
		CC.STDLIB := $(T1:X=$$(CC.STDLIB:C@^/@@))
	end
	if T1 = "$(.PACKAGE.build:A!=.TARGET)"
		local B D H I L N P Q S T V W X Z K=0 K.0=0 KP IP LP
		LDLIBRARIES += $$(.PACKAGE.LIBRARIES. $(T1))
		T2 =
		for P $(T1)
			if P == "{"
				if ! "$(-mam:N=static*)"
					if K
						KP := $(K.$(K))
					else
						KP = 0
					end
					let K = K + 1
					local K.$(K)=$(KP)
					local L.$(K)
				end
				continue
			elif P == "}"
				if ! "$(-mam:N=static*)" && K
					if ! $(K.$(K))
						error 3 at least one of the packages { $(L.$(K)) } must exist
					end
					let K = K - 1
				end
				continue
			elif $(K.$(K)) > 0
				continue
			end
			if K
				L.$(K) += $(P)
			end
			P := $(.PACKAGE.INIT. $(P))
			I := $(PACKAGE_$(P)_INCLUDE)
			L := $(PACKAGE_$(P)_LIB)
			if "$(.PACKAGE.$(P).found)" == "1"
				if K
					K.$(K) := 1
				end
			elif ! K && ! "$(.PACKAGE.$(P).dontcare)"
				error 3 $(P): package not found
			end
			T2 += $(P)
			T4 =
			if "$(-mam:N=static*)"
				if "$(INCLUDEDIR:D:N=$(INSTALLROOT))" || "$(INCLUDEDIR:D:D:N=$(INSTALLROOT))"
					T4 += P
				end
				if I && ( "$(I:D)" == "$(L:D)" || "$(I:D:D)" == "$(L:D)" )
					if "$(I:D)" == "$(L:D)"
						H := $(I:D)
					else
						H := $(I:D:D)
					end
					T := $(I:C,^$(H)/,,)
					print -um setv PACKAGE_$(P) $(T4:N=P:?${INSTALLROOT}?$(H)?)
					print -um setv PACKAGE_$(P)_INCLUDE ${PACKAGE_$(P)}/$(T)
					print -um setv PACKAGE_$(P)_LIB ${PACKAGE_$(P)}/$(L:B:S)
					T4 += I L
				end
			end
			if I
				if ! "$(CC.STDINCLUDE:N=$(I))"
					.SOURCE.h : $(I)
					if "$(I:B)" == "$(P)" && ! "$(CC.STDINCLUDE:N=$(I:D))"
						.SOURCE.h : $(I:D)
					end
				elif "$(-mam:N=static*)"
					.SOURCE.h : $(I)
					CC.STDINCLUDE := $(CC.STDINCLUDE:N!=$(I))
					if "$(I:B)" == "$(P)"
						.SOURCE.h : $(I:D)
						CC.STDINCLUDE := $(CC.STDINCLUDE:N!=$(I:D))
					end
				end
				if "$(-mam:N=static*)"
					.MAMPACKAGE. += PACKAGE_$(P)_INCLUDE
					if ! "$(T4:N=I)"
						print -um setv PACKAGE_$(P)_INCLUDE $(T4:N=P:?${INSTALLROOT}/$(I:B:S)?$(I)?)
					end
				end
			end
			if L && L != "/lib|/usr/lib"
				if ! "$(CC.STDLIB:N=$(L))"
					.SOURCE.a : $(L)
				elif "$(-mam:N=static*)"
					.SOURCE.a : $(L)
					CC.STDLIB := $(CC.STDLIB:N!=$(L))
				end
				if "$(-mam:N=static*)"
					.MAMPACKAGE. += PACKAGE_$(P)_LIB
					if ! "$(T4:N=L)"
						print -um setv PACKAGE_$(P)_LIB $(T4:N=P:?${INSTALLROOT}/$(L:B:S)?$(L)?)
					end
				end
			end
		end
		T3 := $(*.SOURCE.h)
		.SOURCE.h : $(CC.STDINCLUDE) $(LCLDIRS:/:/ /G:C,$,/include,)
		T4 := $(*.SOURCE.a)
		.SOURCE.a : $(CC.STDLIB)
		if T2
			T2 += .
			T1 := $(*.SOURCE.h:C,.*,$$(T2:C%.*%&/\&%),)
			.SOURCE.h : .CLEAR . $(T1:P=C)
			T1 := $(*.SOURCE.a:C,.*,$$(T2:C%.*%&/\&%),)
			.SOURCE.a : .CLEAR . $(T1:P=C)
		end
	else
		T3 := $(*.SOURCE.h)
		.SOURCE.h : $(CC.STDINCLUDE)
		T4 := $(*.SOURCE.a)
		.SOURCE.a : $(CC.STDLIB)
	end
	if "$(-mam:N=static*)"
		.CC.NOSTDINCLUDE. := $(CC.STDINCLUDE:V:N!=*[()]*|$(T3:@C@ @|@G):C@ @|@G:C@^@N!=@)
		.CC.NOSTDLIB. := $(CC.STDLIB:V:N!=*[()]*|$(T4:@C@ @|@G):C@ @|@G:C@^@N!=@)
	else
		.CC.NOSTDINCLUDE. := $(CC.STDINCLUDE:N!=$(T3:@C@ @|@G):C@ @|@G:C@^@N!=@)
		.CC.NOSTDLIB. := $(CC.STDLIB:N!=*/local/*:N!=$(T4:@C@ @|@G):C@ @|@G:C@^@N!=@)
	end
	T3 = $(PACKAGE_PATH:/:/ /G) $(.PACKAGE.DIRS.) $(.PACKAGE.GLOBAL.)
	.SOURCE.a : $(T3:X=lib:N!=$(.PACKAGE.stdlib:/ /|/G):T=F)
	T3 =
	if ! "$(CC.DIALECT:N=ANSI)"
		stdarg.h : .SPECIAL .NULL .TERMINAL .DONTCARE .IGNORE /* courtesy to proto(1) */
	end
	if "$(CC.REPOSITORY)"
		.PTR.PREREQS = $(!) $(!.PTR.CHECK)
		if ! "$(*.SOURCE.ptr)"
			.SOURCE.ptr : $(CC.REPOSITORY)
		end
		T3 += $$(.PTR.OPTIONS.)
	end
	if nativepp == "" && "$(CC.DIALECT:N=-I-)" && !CC.ALTPP.FLAGS && !CC.ALTPP.ENV
		nativepp = 1
	end
	if ! nativepp
		if CC.CC && ! ( T4 = "$(CC.PROBEPP)" )
			T4 := $(cctype:@P=P=C,pp)
		end
		if ! "$(CC.DIALECT:N=LIBPP)" || CPP != "$\(MAKEPP\)"
			if CPP != "$\(MAKEPP\)"
				prefixinclude = 1
			end
			(CC) : (CPP)
			if CC.ALTPP.FLAGS || CC.ALTPP.ENV
				T5 := $(CC.ALTPP.FLAGS)
				T3 += $(T5:V)
				if CC.ALTPP.ENV
					.BIND : (CC)
					CC := $$(CC.ALTPP.ENV) $(CC:V)
				end
			else
				.BIND : (CC)
				if "$(CCLD:V)" == "$$(CC)"
					CCLD := $(CC:V)
				end
				CC := $(CC:N=*=*) $$(PPCC) $$(PPCCFLAGS) $$(CC.DIALECT:/^/-o /) $$(CPP) $(TI:@N=CC:?$(instrument)?$(cctype)?)
			end
			if T4
				T5 := -I-D$(T4)
				T3 += $(T5:V)
			end
		end
	elif ! "$(CC.DIALECT:N=-I-)"
		prefixinclude = 1
	end
	if "$(CCFLAGS:N=-I-)"
		prefixinclude = 1
	end
	if prefixinclude
		T3 += $$(*:A=.SCAN.c:@?$$$(*.SOURCE.%.LCL.INCLUDE:I=$$$$(!$$$$(*):A=.LCL.INCLUDE|.STD.INCLUDE:P=D):$(.CC.NOSTDINCLUDE.):/^/-I/)??) $$(&:T=D)
		if nativepp == "[1-9]*"
			error 1 local include files may be ignored by the native C preprocessor
		end
	else
		prefixinclude = 0
		T3 += $$(*:A=.SCAN.c:@?$$$(*.SOURCE.%.LCL.INCLUDE:I=$$$$(!$$$$(*):A=.LCL.INCLUDE:P=D):$(.CC.NOSTDINCLUDE.):/^/-I/) -I- $$$(*.SOURCE.%.STD.INCLUDE:I=$$$$(!$$$$(*):A=.STD.INCLUDE:P=D):$(.CC.NOSTDINCLUDE.):/^/-I/)??) $$(&:T=D)
	end
	if "$(CC.DIALECT:N=TOUCHO)"
		.TOUCHO : .MAKE .VIRTUAL .FORCE .REPEAT .AFTER .FOREGROUND
			.REBIND : $(*$(<<):N=*$(CC.SUFFIX.OBJECT))
		.APPEND.%.COMMAND : .TOUCHO
	end
	if "$(CCFLAGS:N=-[gG]|$(CC.DEBUG))"
		_BLD_DEBUG == 1
	end
	if "$(instrument)"
		_BLD_INSTRUMENT == 1
	end
	if T3
		CCFLAGS &= $$(-targetcontext:?$$$(!$$$(*):A=.PFX.INCLUDE:@Y%$$$(<:P=U:D:T=*:P=L=*:/^/-I/)%%)??) $(T3:V)
	end
	T3 =
	T4 =
	if "$(CC.REPOSITORY)"
		T3 += $(T5:V) $$(CCFLAGS:N=-[D]*) $$(.PTR.PREREQS:A=.SCAN.c:@?$$$(*.SOURCE.%.LCL.INCLUDE:I=$$$$(.PTR.PREREQS:A=.LCL.INCLUDE:P=D):/^/-I/) $(CC.DIALECT:N=-I-) $$$(*.SOURCE.%.LCL.INCLUDE:I=$$$$(.PTR.PREREQS:A=.LCL.INCLUDE|.STD.INCLUDE:P=D):$(.CC.NOSTDINCLUDE.):/^/-I/)??) $$(&.PTR.CHECK:T=D)
		.APPEND.%.COMMAND : .PTR.CHECK .PTR.UPDATE
		T3 := $$(@:V:N=*$\(CC*(LD)\)*:@?$(T3:V)??)
	end
	if "$(CC.ARFLAGS)"
		.ARCHIVE.o : .CLEAR .USE .ARPREVIOUS (CC) (AR)
			$(.ARPREVIOUS.$(<:B:S):@?$(IGNORE) $$(AR) d $$(<) $$(.ARPREVIOUS.$$(<:B:S))$$("\n")??)$(CC) $(CC.ARFLAGS) -o $(<) $(*)
		.SHARED.o : .CLEAR .USE (LDSHARED)
			$(LDSHARED) $(CC.SHARED) -o $(<) $(*$(**):N!=*$(CC.SUFFIX.ARCHIVE))
		.ATTRIBUTE.%.a : -ARCHIVE
	end
	COBOLFLAGS &= $$(.INCLUDE. cob -I) $$(&:T=D)
	IFFEFLAGS += -c '$$(IFFECC) $$(IFFECCFLAGS) $$(IFFELDFLAGS)' $$(-mam:N=static*:??-S '$$(CC.STATIC)')
	if "$(-cross)" || "$(CC.EXECTYPE)" && "$(CC.HOSTTYPE)" != "$(CC.EXECTYPE)"
		set cross
		IFFEFLAGS += -x $(CC.HOSTTYPE)
	end
	.BIND : (IFFEFLAGS)
	IFFEFLAGS += $$(.IFFE.REF.)
	if ! IFFE_huh
		/* can't figure out why this .PARAMETER must be repeated */
		(IFFEFLAGS) : .PARAMETER
	end
	if "$(-mam:N=static*)"
		LDFLAGS &= $(T3:V)
	else
		if "$(CC.LD.RUNPATH:V)"
			T4 += $$(.CC.LD.RUNPATH.)
		end
		LDFLAGS &= $$(*.SOURCE.%.ARCHIVE:I=$$$(*:N=-l*:P=D):$(.CC.NOSTDLIB.):P=A:/^/-L/) $(T3:V)
	end
	if "$(CC.LD.ORIGIN:V)"
		T4 += $$(CC.LD.ORIGIN)
	end
	if T4
		CCLDFLAGS &= $(T4:V)
	end
	if "$(CC.LD.STRIP:V)"
		if "$(strip)" || "$(PACKAGE_OPTIMIZE:N=space)"
			.PACKAGE.strip = $(CC.LD.STRIP)
		end
		CCLDFLAGS += $$(.PACKAGE.strip)
	end
	M4FLAGS &= $$(*.SOURCE.%.M4.INCLUDE:I=$$$(!$$$(*):P=D):/^/-I/) $$(&:T=D)

	/*
	 * state var defaults
	 */

	if ERROR_CATALOG == ""
		ERROR_CATALOG == "$(CATALOG)"
	end
	if USAGE_LICENSE == ""
		USAGE_LICENSE == "$(LICENSEFILES:/:/ /G:T=F:O=1:P=W=$(LICENSE),type=usage)$(CATALOG:N!=$(ID):Y%[--catalog?$(CATALOG)]%%)"
	end

	/*
	 * map unknown command line targets to common actions
	 */

	for T3 .ARGS .MAIN
		T2 := $(~$(T3):V)
		eval
		.ORIGINAL$(T3). = $(T2:V)
		end
		$(T3) : .CLEAR .VIRTUAL
		for T1 $(T2)
			if "$(T1:A!=.IMMEDIATE|.TARGET)"
				T2 := $(".$(T1:F=%(upper)s)":A=.IMMEDIATE|.TARGET:A!=.ATTRIBUTE)
				if T2
					T1 := $(T2)
				end
			end
			$(T3) : $(T1:V)
		end
	end

	/*
	 * pure recursion makefiles (only :MAKE:) make .RECURSE first by default
	 * .NORECURSE inhibits the default .RECURSE first
	 */

	if "$(~.MAIN:V)" == ".RECURSE" && "$(~.ALL:V)" == ".RECURSE" && ! "$(~.ARGS:V:N=.RECURSE|.NORECURSE)" && ! "$(~.ARGS:V:O=1:A=.IMMEDIATE)"
		.ARGS : .INSERT .RECURSE
	end

	/*
	 * check make recursion limits
	 */

	if recurse > 0
		T1 = 0
		T2 =
		while T1 < recurse
			T2 += .SEMAPHORE
			let T1 = T1 + 1
		end
		.RECURSE.SEMAPHORE : $(T2)
	end
	.INITIALIZED. = 1

.PTR.LOCAL =

.PTR.OPTIONS. : .CLEAR .FUNCTION .PTR.INIT
	if ! "$(.PTR.LOCAL)"
		.PTR.LOCAL := $(*.SOURCE.ptr:N!=.:O=1)
	elif "$(.PTR.LOCAL)" != "$(*.SOURCE.ptr:N!=.:O=1)"
		.UNBIND : $(.PTR.LOCAL)
		.BIND : $(.PTR.LOCAL)
	end
	.UNION : .CLEAR $(*.SOURCE.ptr:N!=.:O=1) $(*.SOURCE.ptr:N!=.:O>1:T=F)
	return $(*.UNION:@N!=$(CC.REPOSITORY):$(CC.DIALECT:N=PTRIMPLICIT:?D??):/^/-ptr/)

.PTR.DONTCARE : .MAKE .VIRTUAL .IGNORE .FORCE
	.DONTCARE.% : .FUNCTION
		if "$(<:T=M:N=.PTR.CHECK)"
			return ok
		end

.PTR.CHECK : .VIRTUAL .PTR.INIT .PTR.DONTCARE $$(.PTR.LIST.)

.PTR.INIT : .MAKE .VIRTUAL .FORCE .IGNORE
	if "$(CC.DIALECT:N=PTRMKDIR)"
		local X
		.PTR.DIR. := $(~.SOURCE.ptr:N!=.:O=1)
		$(.PTR.DIR.) : .MAKE .IGNORE
			if "$(^)"
				$(^) : -MAKE
				if "$(CC.DIALECT:N=PTRCOPY)"
					X := $("defmap":D=$(^):B:S:T=F)
				end
				.MAKE :
					$(MKDIR) $(<) 2>/dev/null $(X:?&& $(CP) -p $(X) $(<))
			end
		.MAKE : $(.PTR.DIR.)
		$(.PTR.DIR.) : -MAKE
	end

.PTR.UPDATE : .MAKE .VIRTUAL .FORCE .REPEAT .AFTER .FOREGROUND
	.PTR.CHECK : .CLEAR .VIRTUAL .IGNORE $$(.PTR.LIST.)
	.UNBIND : $(*.PTR.CHECK)
	.MAKE : .PTR.CHECK

.PTR.LIST. : .FUNCTION
	local D
	.UNION : .CLEAR
	.UNBIND : $(*.SOURCE.ptr)
	for D $(*.SOURCE.ptr:N!=.)
		.UNION : $(D:T=F:L<=*@(.[cChH]*|$(CC.SUFFIX.SOURCE)|$(CC.SUFFIX.OBJECT)|$(CC.SUFFIX.ARCHIVE)):D=$(D):B:S)
	end
	$(*.UNION) : .TERMINAL
	for D $(*.UNION:N=*.c)
		$(D:D:B:S=$(CC.SUFFIX.OBJECT)) : $(D)
			$(<<<:N=.PTR.UPDATE:??$$(<:T=F:D:P=L:Y%$$(RM) $$(RMFLAGS) $$(<)%%)?)
	end
	return $(*.UNION)

.NOSTATEFILE : .MAKE .VIRTUAL .FORCE
	set nowritestate

.SETARGS : .MAKE .VIRTUAL .FORCE
	if ! "$(*.ARGS)"
		.ARGS : $(*.MAIN)
	end

.MAMACTION. : .FUNCTION
	if ! .MAMEDIT.
		local P
		for P $(.MAMPACKAGE.)
			.MAMEDIT. := $(.MAMEDIT.)@C%$($(P))%$("$"){$(P)}%G:
		end
		if P = "$(.MAMROOT.:C@\.@\\.@G)"
			.MAMEDIT. := $(.MAMEDIT.)@C%$(P)/\.\./\.\.%$("$"){PACKAGEROOT}%G:
			.MAMEDIT. := $(.MAMEDIT.)@C%$(P)%$("$"){INSTALLROOT}%G:
		end
		for P $(*.VIEW:N=/*)
			.MAMEDIT. := $(.MAMEDIT.)@C%$(P)/%%G:
		end
	end
	return $(%:$(.MAMEDIT.))

.MAMNAME. : .FUNCTION
	if "$(%)" == ".[A-Z]*" && "$(%)" != ".INIT|.DONE"
		return $(%:/.//:F=%(lower)s)
	end
	if "$(%)" == "/*|$(.MAMROOT.)/*)"
		return $(.MAMACTION. $(%))
	end
	if "$(%)" == "-l+([a-zA-Z0-9_])" && "$(-mam:N=*,port*)"
		return $(%:/-l\(.*\)/${mam_lib\1}/)
	end

/*
 * common actions
 *
 *	if the first command line target is not a known atom then
 *	the function .UNKNOWN translates the atom name to upper
 *	case preceded by a dot in an attempt to yield one of the
 *	common actions described below
 *
 *	$(clobber)	clobber old files if non-null
 *	$(output)	save action output D:B name
 *	$(select)	edit op source file selector
 *
 *	.COMMON.SAVE	prerequisite of all common save actions
 *
 *	$(.BUILT.)	list of built targets
 *	$(.CLOBBER.)	list of additional files to clobber
 *	$(.FILES.)	all :: source files
 *	$(.FILES.targ)	source files for :: target targ
 *	$(.MAIN.TARGET.)first :: target name
 *	$(.MANIFEST.)	all source and miscellaneous files
 *	$(.ORIGINAL.ARGS.) original command line args
 *	$(.ORIGINAL.MAIN.) original .MAIN prereqs
 *	$(.RWD.)	current directory via .RECURSE
 *	$(.SELECT.)	list of all or selected source files
 *	$(.SOURCES.)	non-generated source files in $(.SELECT.)
 */

test : .SPECIAL .DONTCARE .ONOBJECT

.ALL : .VIRTUAL

.RETAIN : .BASE.

.BASE.DATE.FORMAT. = %Y%m%d

.BASE.VERIFY : .MAKE .VIRTUAL .FORCE .REPEAT
	if ! "$(.BASE.)"
		error 3 $(output): no base archive for delta
	end

.BASE.UPDATE : .MAKE .VIRTUAL .FORCE
	local B N
	N := $("":T=R:F=%($(.BASE.DATE.FORMAT.))T)
	if ( B = "$(N:T=F)" )
		error $(-force:?1?3?) $(B): base archive already generated
	end
	.BASE. := $(N)

.BASE : .BASE.UPDATE .COMMON.SAVE
	$(PAX) -w -f $(.BASE.) -z - $(PAXFLAGS) $(.MANIFEST.)

.DELTA : .BASE.VERIFY .COMMON.SAVE
	$(PAX) -w -f $("":T=R:F=%($(.BASE.DATE.FORMAT.))T).$(.BASE.) -z $(.BASE.) $(PAXFLAGS) $(.MANIFEST.)

/*
 * believe generated files and clean up archive intermediates
 */

.BELIEVE : .MAKE .VIRTUAL .ONOBJECT
	local A O
	set ignorelock keepgoing touch
	make .INSTALL
	set notouch
	for A $(...:A=.TARGET:A=.ARCHIVE.o)
		O += $(*$(A):N=*$(CC.SUFFIX.OBJECT):T=F)
	end
	if "$(O)"
		make { $(RM) $(RMFLAGS) $(O) }
	end

.CC- .VARIANTS : .ONOBJECT $$(*.SOURCE:L<=cc-*:$$(variants:@?N=$$(variants)??)) $$(.VARIANTS.:$$(variants:@?N=$$(variants)??))

.CC-INSTALL : .ONOBJECT .ALL $$(*.INSTALL:N=*-*$$(CC.SUFFIX.ARCHIVE))

.CC.LD.RUNPATH. : .FUNCTION
	if LDRUNPATH && CC.LD.RUNPATH && "$(CC.DIALECT:N=DYNAMIC)" && ( "$(CCLDFLAGS:V:N=$(CC.DYNAMIC)|$\(CC.DYNAMIC\))" || ! "$(CCLDFLAGS:V:N=$(CC.STATIC)|$\(CC.STATIC\))" )
		local T
		T := $(LDRUNPATH:N!=.) $(*.SOURCE.%.ARCHIVE:I=$$(**:N=-l*:P=D):N!=.:$(.CC.NOSTDLIB.):P=A:N!=$(LIBDIR))
		if T = "$(T:@/ /:/G)"
			return $(CC.LD.RUNPATH)$(T)
		end
	end

.CC.LIB.DLL. : .FUNCTION
	return $(%)

.CC.LIB.DLL.export : .FUNCTION
	local F J
	J := $(+jobs) /* XXX: multiple :LIBRARY: may get botched with jobs>0 */
	set nojobs
	F := _$(%:O=1:B:/^$(CC.PREFIX.ARCHIVE)//:/$(CC.LIB.TYPE:O=1)$//:/[^a-zA-Z0-9_]/_/G)_
	F := $(F:/___*/_/G)
	$(F).exp : .FORCE $(%:O=1)
		: generate $(*) export symbols in $(<)
		set -
		$(NM) $(NMFLAGS) $(*) |
		$(SED) $(NMEDIT) > $(<)
	.MAKE : $(F).exp
	set $(J)
	return $(F).exp $(%)

.CC.LIB.DLL.object : .FUNCTION
	local F J
	J := $(+jobs) /* XXX: multiple :LIBRARY: may get botched with jobs>0 */
	set nojobs
	F := _$(%:O=1:B:/^$(CC.PREFIX.ARCHIVE)//:/$(CC.LIB.TYPE:O=1)$//:/[^a-zA-Z0-9_]/_/G)_
	F := $(F:/___*/_/G)
	$(F) : .FORCE $(%:O=1)
		: extract $(*) objects in $(<)
		set -
		$(RM) $(RMFLAGS) $(RMRECURSEFLAGS) $(<)
		$(MKDIR) $(<)
		cd $(<)
		$(AR) x $(*:P=A)
	.MAKE : $(F)
	.DONE : .DONE.$(F)
	.DONE.$(F) : .FORCE
		$(RM) $(RMFLAGS) $(RMRECURSEFLAGS) $(<:/.DONE.//)
	set $(J)
	return $(F)/* $(%:O>1)

.CC.LIB.DLL.option : .FUNCTION
	return $(CC.LIB.ALL) $(%:O=1) $(CC.LIB.UNDEF) $(%:O>1)

.CC.LIB.DLL.symbol : .FUNCTION
	local F J
	J := $(+jobs) /* XXX: multiple :LIBRARY: may get botched with jobs>0 */
	set nojobs
	F := _$(%:O=1:B:/^$(CC.PREFIX.ARCHIVE)//:/$(CC.LIB.TYPE:O=1)$//:/[^a-zA-Z0-9_]/_/G)_
	F := $(F:/___*/_/G)
	$(F).c : .FORCE $(%:O=1)
		: generate $(*) export symbols in $(<)
		set -
		$(NM) $(NMFLAGS) $(*) |
		$(SED) $(NMEDIT) -e '/^[ 	]*$/d' $(CC.PREFIX.SYMBOL:?-e 's/^$(CC.PREFIX.SYMBOL)//'??) |
		$(SORT) -u > $(<:B:S=.i)
		{
			echo '/* $(*) export symbols */'
			echo
			echo 'typedef int (*Export_t)();'
			echo
			echo '#if defined(__cplusplus)'
			echo 'extern "C" {'
			echo '#endif'
			echo
			while	read sym
			do	echo "extern int $sym();"
			done < $(<:B:S=.i)
			echo
			echo 'extern int $(<:B)();'
			echo 'extern Export_t $(<:B)export[];'
			echo
			echo '#if defined(__cplusplus)'
			echo '}'
			echo '#endif'
			echo
			echo 'Export_t $(<:B)export[] ='
			echo '{'
			while	read sym
			do	echo "	$sym,"
			done < $(<:B:S=.i)
			echo '};'
			echo
			echo 'int $(<:B)() { return 0; }'
			$(RM) $(RMFLAGS) $(<:B:S=.i)
		} > $(<)
	.MAKE : $(F)$(CC.SUFFIX.OBJECT)
	set $(J)
	return $(F)$(CC.SUFFIX.OBJECT) $(%)

.CC.LIB.DLL.undef : .FUNCTION
	return `$(NM) $(NMFLAGS) $(%:O=1) | $(SED) $(NMEDIT) -e "s/^/-u /"` $(%)

.CC.SHARED.REGISTRY. : .FUNCTION
	local R
	if ( R = "$(<<:B:C%\..*%%)" ) && .PACKAGE.registry && "$(CC.DIALECT:N=DYNAMIC)" && ( "$(CCLDFLAGS:V:N=$(CC.DYNAMIC)|$\(CC.DYNAMIC\))" || ! "$(CCLDFLAGS:V:N=$(CC.STATIC)|$\(CC.STATIC\))" )
		if R = "$(.CC.SHARED.REGISTRY.$(R))"
			local CC.SHARED.REGISTRY.PATH
			CC.SHARED.REGISTRY.PATH := $(R)
		end
		return $(CC.SHARED.REGISTRY)
	end

.COMMON.SAVE : .NULL .VIRTUAL .IGNORE .FOREGROUND

.CLEAN : .CLEARARGS .ONOBJECT
	$(IGNORE) $(RM) $(RMFLAGS) $(.BUILT.:A=.REGULAR:A!=.ARCHIVE|.COMMAND:N!=*.(m[klos]|db$(CC.SUFFIX.SHARED:?|$(CC.SUFFIX.SHARED:/\.//).*??))$(cleanignore:?|($(cleanignore))??))

.CLOBBER : .CLEARARGS .ONOBJECT .NOSTATEFILE
	$(IGNORE) $(RM) $(RMFLAGS) $(RMRECURSEFLAGS) $(.CLOBBER.:T=F:P=L) $(.BUILT.:A=.REGULAR)

.CLOBBER.INSTALL : .ONOBJECT
	$(IGNORE) $(RM) $(RMFLAGS) $(RMRECURSEFLAGS) $(.INSTALL.LIST.:T=F:P=L:A=.REGULAR)

if CPIO
.CPIO : .COMMON.SAVE
	echo $(.MANIFEST.) | tr ' ' '\012' | $(CPIO) -o $(CPIOFLAGS) > $(output).cpio
end

if CTAGS
.CTAGS :
	$(CTAGS) $(CTAGSFLAGS) $(.SOURCES.:N=*.[cfhly])
end

if VGRIND
.GRIND : $(CTAGS:@?.GRINDEX?.COMMON.SAVE?)
	$(VGRIND) $(VGRINDFLAGS) $(.SOURCES.:N=*.[chly]|*.mk|$(MAKEFILE)) $(LPR:@?| $$(LPR) $$(LPRFLAGS)??)

if CTAGS
.GRINDEX : .COMMON.SAVE
	$(CTAGS) $(CTAGSFLAGS) -v $(.SOURCES.:N=*.[chly]) | $(SORT) -f >index
	$(VGRIND) $(VGRINDFLAGS) -x index $(LPR:@?| $$(LPR) $$(LPRFLAGS)??)
end
end

.INSTALL : .ALL .ONOBJECT

.LINT : .MAKE .VIRTUAL .FORCE .SETARGS
	local T1 T2 LINT1ARGS
	set nojobs nowritestate override
	if "$(_release_:N=research|V)"
		if "$(_release_:N=V)"
			LINT1FLAGS += -H$$(<<:B:S=.lh)
		else
			LINT1FLAGS += -S$$(<<:B:S=.lh)
		end
		LINT1ARGS = $(*$(<):G=%$(<:S):O=2:??$(>)?O)
	end
	.LINTLIBRARIES. = $(*:A=.ARCHIVE) $(LINTLIBRARIES)
	.SOURCE.ln : $(LINTLIB)
	.INSERT.%.ARCHIVE .INSERT.%.COMMAND : .DO.NOTHING $$(<:A=.TARGET:?.LINT0??)
	.LINT0 : .VIRTUAL .FORCE
		$(SILENT) $(RM) $(RMFLAGS) $(<<:B:S=.l[hn])
	for T1 % %.o
		for T2 $(.METARULE.I.$(T1))
			$(T1) : $(T2) .NULL
		end
	end
	eval
	%.o : %.c .FORCE
		set -
		$(_release_:N=V:??echo $$$(>):$$$("\n")?){ $$(CPP) $$(CPPFLAGS) -C -Dlint $$(>) | $$(LINT1) $$(LINT1FLAGS) $$(LINTFLAGS) $(LINT1ARGS:V) >> $$(<<:B:S=.ln) ;} 2>&1
	end
	.ARCHIVE.o : .CLEAR .USE
		$(SILENT) $(RM) $(RMFLAGS) $(<:B:S=.lh)
	if "$(_release_:N=V)"
	.COMMAND.o :
		set -
		cat $(<:B:S=.ln) $(.LINTLIBRARIES.:B:S=.ln:/$(CC.PREFIX.ARCHIVE)\(.*\)/& llib-l\1/:T=F) > $(TMPDIR)/l$$.ln
		$(LINT2) -T$(TMPDIR)/l$$.ln $(<:B:S=.lh:T=F:/^/-H/) $(LINTFLAGS)
		$(RM) $(RMFLAGS) $(TMPDIR)/l$$.ln $(<:B:S=.l[hn])
	elif "$(_release_:N=research)"
	.COMMAND.o :
		set -
		echo $(<)::
		cat $(<:B:S=.ln) $(.LINTLIBRARIES.:B:S=.ln:/$(CC.PREFIX.ARCHIVE)\(.*\)/& llib-l\1/:T=F) > $(TMPDIR)/l$$.ln
		$(LINT2) $(TMPDIR)/l$$.ln -S$(<:B:S=.lh) $(LINTFLAGS)
		$(RM) $(RMFLAGS) $(TMPDIR)/l$$.ln $(<:B:S=.l[hn])
	else
	.COMMAND.o :
		set -
		echo $(<)::
		cat $(<:B:S=.ln) $(.LINTLIBRARIES.:B:S=.ln:/$(CC.PREFIX.ARCHIVE)\(.*\)/& llib-l\1/:T=F) > $(TMPDIR)/l$$.ln
		$(LINT2) $(TMPDIR)/l$$.ln $(LINTFLAGS)
		$(RM) $(RMFLAGS) $(TMPDIR)/l$$.ln $(<:B:S=.l[hn])
	end

.LIST.GENERATED : .ONOBJECT .MAKE
	print $(.INSTALL.LIST.:T=F:P=L:A=.REGULAR:/$/$("\n")/G) $(.CLOBBER.:T=F:P=L:/$/$("\n")/G) $(.BUILT.:A=.REGULAR:/ /$("\n")/G)

.LIST.INSTALL : .ONOBJECT .MAKE
	print $(.INSTALL.LIST.:$(INSTALLROOT:N=.:?T=F?N=$(INSTALLROOT)/*:C%$(INSTALLROOT)/%%):C% %$("\n")%G)

.LIST.INSTALLED : .ONOBJECT .MAKE
	print $(.INSTALL.LIST.:$(INSTALLROOT:N=.:?T=F?N=$(INSTALLROOT)/*:T=F:C%$(INSTALLROOT)/%%):C% %$("\n")%G)

.LIST.MANIFEST : .ONOBJECT .COMMON.SAVE .MAKE
	print $(.MANIFEST.:/ /$("\n")/G)

.LIST.PACKAGE.EDIT. = $(VROOT:T=F:P=L*:C%.*%C,^&/,,%:C% %:%G)

.LIST.PACKAGE.LOCAL : .ONOBJECT .COMMON.SAVE .MAKE
	local I E
	E := $(.LIST.PACKAGE.EDIT.)
	for I $(.MANIFEST.:P=A)
		print ;;;$(I);$(I:$(E))
	end

.LIST.PACKAGE.BINARY : .ONOBJECT .MAKE
	local I E
	.UNION : .CLEAR $(.INSTALL.LIST.:N=$(INSTALLROOT)/*:T=F:P=A)
	E := $(.LIST.PACKAGE.EDIT.)
	if package.strip
		for I $(*.UNION:$(PACKAGE_OPTIMIZE:N=space:Y%:N=$(INSTALLROOT)/(bin|fun|lib)/*:N!=*$(CC.SUFFIX.ARCHIVE)|$(INSTALLROOT)/lib/lib?(/*)%%))
			if "$(I:T=Y)" == "*/?(x-)(dll|exe)"
				print ;;filter $(STRIP) $(STRIPFLAGS) $(I);$(I);$(I:$(E))
			else
				print ;;;$(I);$(I:$(E))
			end
		end
	else
		for I $(*.UNION)
			print ;;;$(I);$(I:$(E))
		end
	end

.LIST.PACKAGE.SOURCE : .ONOBJECT .COMMON.SAVE .MAKE
	local I E
	PROTOEDIT = P=A
	E := $(.LIST.PACKAGE.EDIT.)
	for I $(.MANIFEST.:P=A)
		if I == "*.{1,3}(?)"
			print ;;$(PROTO) $(PROTOFLAGS) -c '' -dp $(I);$(I);$(I:$(E))
		else
			print ;;;$(I);$(I:$(E))
		end
	end

.LIST.SOURCE.TGZ : .ONOBJECT .COMMON.SAVE .MAKE
	local E F P
	PROTOEDIT = P=A
	E := $(.LIST.PACKAGE.EDIT.)
	for F $(.MANIFEST.)
		P := $(F:T=F)
		if F == "*.{1,3}(?)"
			print ;;$(PROTO) $(PROTOFLAGS) -c '' -dp $(P);$(P);$(F:$(E))
		else
			print ;;;$(P);$(F:$(E))
		end
	end

.LIST.SHIP : .ONOBJECT .MAKE
	.UNION : .CLEAR $(.INSTALL.LIST.:N=$(INSTALLROOT)/*:T=F:C%$(INSTALLROOT)/%%)
	print $(*.UNION:/ /$("\n")/G)

.LIST.SHIP.BIN : .ONOBJECT .MAKE
	.UNION : .CLEAR $(.INSTALL.LIST.:N=$(INSTALLROOT)/*:T=F:C%$(INSTALLROOT)/%%:N!=include/*|lib/lib*.a|man/man[!18]?(/*))
	print $(*.UNION:/ /$("\n")/G)

.LIST.SYMBOLS : .ALWAYS
	lib="$(...:A=.ARCHIVE:A=.TARGET:T=F:N!=*[-/]*)"
	for lib in $lib
	do	$(NM) $(NMFLAGS) $lib | $(SED) $(NMEDIT) $(CC.PREFIX.SYMBOL:?-e 's/^$(CC.PREFIX.SYMBOL)//'??) | $(SORT) -u
	done

if LPROF
.LPROF : .MAKE .VIRTUAL .FORCE .ONOBJECT .SETARGS
	local T1 T2
	set force noscan nowritestate override
	.INSERT.%.COMMAND : .LPROF0 .LPROF1 $$(<:B:S=.cnt)
	.LPROF0 : .MAKE .VIRTUAL .FORCE
		.LPROFFILES. =
	.LPROF1 : .AFTER .IGNORE $$(<<:B:S=.cnt)
		$(LPROF) $(LPROFFLAGS) -r "$(.LPROFFILES.:/^ //)" -o $(<<) -c $(*)
	for T1 % %.o
		for T2 $(.METARULE.I.$(T1))
			$(T1) : $(T2) .NULL
		end
	end
	%.o : %.c .MAKE
		.LPROFFILES. += $(>)
	.ARCHIVE.o .COMMAND.o : .NULL
end

.OFFICIAL : .ONOBJECT
	$(*.VIEW:O=2:@?$$(.MANIFEST.:P=L:N!=[-/]*:C@.*@{ $$(DIFF) $$(DIFFFLAGS) $$(*.VIEW:O=2)/& & || true; } >> $(official_out:D=$$(*.VIEW:O=2):B:S); $$(MV) & $$(*.VIEW:O=2)/&;@)?: no lower view?)

.PAX : .COMMON.SAVE $$(*.RECURSE:@?.PAX.RECURSE?.PAX.LOCAL)

.PAX.LOCAL : .COMMON.SAVE
	$(PAX) -w -f $(output).pax $(PAXFLAGS) $(.MANIFEST.)

.PAX.RECURSE : .COMMON.SAVE
	$(MAKE) --noexec --file=$(MAKEFILE) $(-) recurse list.manifest |
	$(SORT) -u |
	$(PAX) -w -f $(output).pax $(PAXFLAGS)

.PRINT : .COMMON.SAVE
	$(PR) $(PRFLAGS) $(.SOURCES.:N!=*.[0-9]*([!./])) $(LPR:@?| $(LPR) $(LPRFLAGS)??)

.SAVE : .COMMON.SAVE
	$(PAX) -w -f $(output).$("":T=R:F=%($(.BASE.DATE.FORMAT.))T) $(PAXFLAGS) $(.MANIFEST.)

.SHAR : .COMMON.SAVE
	$(SHAR) $(SHARFLAGS) $(.MANIFEST.) > $(output).shar

.TAR : .COMMON.SAVE
	$(TAR) cf$(TARFLAGS) $(output).tar $(.MANIFEST.)

.TARBALL : .COMMON.SAVE
	$(PAX) -w -f $(output).tgz -x tar:gzip $(PAXFLAGS) -s ',^[^/],$(PWD:B:/-.*//)$(VERSION:?-$(VERSION)??)/,' $(.MANIFEST.)

.TGZ : .COMMON.SAVE $$(*.RECURSE:@?.TGZ.RECURSE?.TGZ.LOCAL)

.TGZ.LOCAL : .COMMON.SAVE
	$(PAX) -w -f $(output).tgz -x tar:gzip $(PAXFLAGS) $(.MANIFEST.)

.TGZ.RECURSE : .COMMON.SAVE
	$(MAKE) --noexec --file=$(MAKEFILE) $(-) recurse list.manifest |
	$(SORT) -u |
	$(PAX) -w -f $(output).tgz -x tar:gzip $(PAXFLAGS)

/*
 * make abstract machine support
 */

.MAM.LOAD : .MAKE .VIRTUAL .FORCE
	set noreadstate reread strictview
	if "$(-mam:N=static*)"
		set readonly
		INSTALLROOT = $(.MAMROOT.)
		PACKAGEROOT = $(.MAMROOT.)/../..
		set noreadonly
	end
	.MAKEINIT : .MAM.INIT

.MAM.INIT : .MAKE .VIRTUAL .FORCE .AFTER
	local T
	LICENSEFILES := $(LICENSEFILES:/:/ /G:T=F:O=1)
	if "$(-mam:N=*,port*)"
		CC.DEBUG = ${mam_cc_DEBUG}
		CC.DIALECT =
		CC.DLL = ${mam_cc_DLL}
		CC.DLLBIG = ${mam_cc_DLLBIG}
		CC.DYNAMIC =
		CC.EXPORT.DYNAMIC = ${mam_cc_EXPORT_DYNAMIC}
		CC.HOSTTYPE = ${mam_cc_HOSTTYPE}
		CC.OPTIMIZE = ${mam_cc_OPTIMIZE}
		CC.PIC = ${mam_cc_PIC}
		CC.PICBIG = ${mam_cc_PICBIG}
		CC.PREFIX.DYNAMIC = ${mam_cc_PREFIX_DYNAMIC}
		CC.PREFIX.SHARED = ${mam_cc_PREFIX_SHARED}
		CC.SHARED =
		CC.SHELLMAGIC = ${mam_cc_SHELLMAGIC}
		CC.STATIC =
		CC.SUFFIX.DYNAMIC = ${mam_cc_SUFFIX_DYNAMIC}
		CC.SUFFIX.SHARED = ${mam_cc_SUFFIX_SHARED}
		_hosttype_ = ${mam_cc_HOSTTYPE}
	end
	print -um setv PACKAGEROOT $(PACKAGEROOT)
	PACKAGEROOT = ${PACKAGEROOT}
	print -um setv AR ar
	AR = ${AR}
	print -um setv ARFLAGS cr
	ARFLAGS = cr
	print -um setv AS as
	AS = ${AS}
	print -um setv ASFLAGS
	ASFLAGS = ${ASFLAGS}
	print -um setv CC $(-mam:N=static*,port*:?$(cctype:O=1:B:S)?$(CC)?)
	CC = ${CC}
	.MAM.CC.FLAGS := $(CCFLAGS:VP:N!=-O*|$(CC.OPTIMIZE)|$\(CC.OPTIMIZE\)|-g|$(CC.DEBUG)|$\(CC.DEBUG\))
	print -um setv mam_cc_FLAGS $(.MAM.CC.FLAGS)
	T := $(CCFLAGS:VP) ${strip?1?${mam_cc_LD_STRIP}??}
	if "$(T:N=-O*|$(CC.OPTIMIZE))"
		print -um setv CCFLAGS ${debug?1?${mam_cc_DEBUG} -D_BLD_DEBUG?${mam_cc_OPTIMIZE}?}
	elif "$(T:N=-g|$(CC.DEBUG))"
		print -um setv CCFLAGS ${mam_cc_DEBUG}
	else
		print -um setv CCFLAGS ${debug?1?${mam_cc_DEBUG} -D_BLD_DEBUG??}
	end
	CCFLAGS = ${mam_cc_FLAGS} ${CCFLAGS}
	CC.NATIVE = ${CC}
	CMP = cmp 2>/dev/null
	print -um setv COTEMP $$
	print -um setv CPIO cpio
	CPIO = ${CPIO}
	print -um setv CPIOFLAGS
	CPIOFLAGS = ${CPIOFLAGS}
	print -um setv CPP "${CC} -E"
	CPP = ${CPP}
	print -um setv F77 f77
	F77 = ${F77}
	print -um setv HOSTCC ${CC}
	print -um setv IGNORE
	INSTALLROOT = ${INSTALLROOT}
	print -um setv LD ld
	LD = ${LD}
	print -um setv LDFLAGS $(LDFLAGS:VP:@Y&"$(LDFLAGS:VP)"&&)
	LDFLAGS = ${LDFLAGS}
	if "$(-mam:N=*,port*)"
		LDFLAGS &= $$(*.SOURCE.%.ARCHIVE:I=$$$(*:N=${mam_lib+([a-zA-Z0-9_])}:P=D):$(.CC.NOSTDLIB.):/.*/${mam_cc_L+-L&}/)
	end
	print -um setv LEX lex
	LEX = ${LEX}
	print -um setv LEXFLAGS
	LEXFLAGS = ${LEXFLAGS}
	print -um setv LPR lpr
	LPR = ${LPR}
	print -um setv LPRFLAGS
	LPRFLAGS = ${LPRFLAGS}
	M4 = ${M4}
	print -um setv M4FLAGS $(M4FLAGS:VP:@?"$(M4FLAGS:VP)"??)
	M4FLAGS = ${M4FLAGS}
	print -um setv NMAKE nmake
	MAKE := $(-never:?${NMAKE}?$$$(<:A=.ALWAYS:@Y%$(MAKE)%${NMAKE}%)?)
	print -um setv NMAKEFLAGS
	NMAKEFLAGS = ${NMAKEFLAGS}
	print -um setv PR pr
	PR = ${PR}
	print -um setv PRFLAGS
	PRFLAGS = ${PRFLAGS}
	print -um setv SHELL /bin/sh
	SHELL = ${SHELL}
	print -um setv SILENT
	STDCMP = ${STDCMP} 2>/dev/null
	STDCP = ${STDCP}
	STDED = ${STDED}
	STDEDFLAGS = ${STDEDFLAGS}
	STDLN = ${STDLN}
	STDMV = ${STDMV}
	STDRM = ${STDRM}
	print -um setv TAR tar
	TAR = ${TAR}
	print -um setv YACC yacc
	YACC = ${YACC}
	print -um setv YACCFLAGS -d
	YACCFLAGS = ${YACCFLAGS}
	.ARUPDATE .ARCOPY :
		$(<<:N=*$(CC.SUFFIX.ARCHIVE):?(ranlib $$(<<)) >/dev/null 2>&1 || true??)
	.ARCLEAN : .NULL
	.DO.INSTALL :
		test '' = '$(*)' || $(STDCMP) $(CMPFLAGS) $(*) $(<) || { $(STDMV) $(<) $(<).old 2>/dev/null || true; $(STDCP) $(*) $(<) $(.DO.INSTALL.OPT.) ;}
	for T .READONLY.*
		$(T) : .CLEAR .NULL .VIRTUAL
	end

	/*
	 * force some make...prev
	 * bindfile() should probably do this
	 * but right now its too noisy
	 */

	make (USAGE_LICENSE) $(LICENSEFILES)

/*
 * miscellaneous support
 */

include - "msgcat.mk"

/*
 * local installation info
 */

include "Makeinstall.mk"

/*
 * architecture specific additions
 */

include - "rules-$(_hosttype_).mk"

if _hosttype_ != "$(_hosttype_:B)"
include - "rules-$(_hosttype_:B).mk"
end

/*
 * local additions
 */

include - "Localrules.mk"
