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

.ID. = "@(#)$Id: Makerules (AT&T Research) 2001-10-20 $"

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

PKGDIRS = $(LIBDIR) $(*.VIEW:X=$(VROOT)/$(LIBDIR:B:S)) $(MAKELIB:D)
LCLDIRS = /usr/local/arch/$(_hosttype_):/usr/common:/usr/local
OPTDIRS = $(INSTALLROOT)/opt:/usr/add-on:/usr/addon:/usr/contrib:$(LCLDIRS):/opt:/home
STDDIRS = /:/usr
USRDIRS = $(LCLDIRS):$(STDDIRS)

/*
 * common directories
 */

INSTALLROOT = $(HOME)

BINDIR = $(INSTALLROOT)/bin
DLLDIR = $(CC.SUFFIX.SHARED:N=.lib:?$(BINDIR)?$(LIBDIR)?)
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

CATALOG = $(ID:N=+([A-Za-z0-9_]):?$(PWD:N=*/lib/*:Y,lib,,)$(ID)?$(PWD:$(PWD:B:N=cc-*:Y,D,,)::B)?)
HTMLINITFILES = 2HTML:$(HOME)/.2html
LICENSE =
LICENSEFILE = LICENSE
LICENSEFILES = $(LICENSEFILE):$(.PACKAGE.:D=$(LIBDIR)/package:B:S=.lic)

/*
 * recursion defaults
 */

MAKEDIRS = cmd:contrib:etc:lib
MAKESKIP = *-*

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
output = $(PWD:M=.*[0-9]\.[0-9]:?$$(PWD:B:S:/-*\([0-9]\.[0-9]\)/-\1/)?$$(VERSION:Y%$$(PWD:B:S)-$$(VERSION)%$$(PWD:B)%)?)
package_local = win32.*
physical =
prefixinclude = 1
preserve = $(CC.SUFFIX.SHARED:?$(CC.PREFIX.SHARED)*$(CC.SUFFIX.SHARED).*??)|$(CC.SUFFIX.DYNAMIC:?$(CC.PREFIX.DYNAMIC)*$(CC.SUFFIX.DYNAMIC)??)
profile =
recurse = 1
recurse_enter =
recurse_exit =
select =
skeleton =
static =
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
INSTRUMENT_purecov = command=CCLD root=PURE_HOME CCFLAGS=-g
INSTRUMENT_purify = command=CCLD root=PURE_HOME
INSTRUMENT_quantify = command=CCLD root=PURE_HOME CCFLAGS=-g
INSTRUMENT_sentinel = command=CCLD root=SE_HOME CCFLAGS=-g

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
	STD$(.X.:F=%(upper)S) := $(STDDIRS:/:/ /G:X=bin/$(.X.):P=X:O=1)
	end
end
.X. := $(STDLN)
STDLN := $$(_feature_:N=ln:?$(.X.)?$$(STDCP)?)

/*
 * action related symbols
 */

AR = ar
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
CCFLAGS = -O
CCLD = $(CC)
CCLDFLAGS = $(CCFLAGS:N!=-[DIU]*:@C@$(CC.ALTPP.FLAGS)@@) $(LDFLAGS)
CHGRP = chgrp
CHMOD = chmod
CHOWN = chown
CMP = cmp
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
if ! "$(PATH:/:/ /G:X=ed:P=X)" && "$(PATH:/:/ /G:X=ex:P=X)"
ED = ex
else
ED = ed
end
EDFLAGS = -
EGREP = egrep
F77 = f77
FLEX = flex
FLEXFLAGS = -l
GREP = grep
HOSTCC = cc
IFFE = iffe
IFFESRCDIR = features
IFFEGENDIR = FEATURE
IFFEFLAGS = -v
IGNORE = ignore
LD = $(CC.LD)
LDRUNPATH =
LEX = lex
LEXFLAGS =

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
PACKAGE_PATH = $(PACKAGE)
PACKAGE_LOCAL = $(CC.HOSTTYPE:N=$(package_local:/^0$/*/):??$(.PACKAGE.:O=1)_?)
PAX = pax
PPCC = $(MAKELIB)/ppcc
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

.BUILT. = $(...:T=XU:T=F:P=L:N!=/*$(.INSTALL.LIST.:@/ /|/G:/^./|&/)$(VROOT:?|$(VROOT)/*??)$(-global:@/:/|/G:/^./|&/):T=G)
.CLOBBER. = $(".":L=*.([it]i|l[hn])) core
.MANIFEST.FILES. = $(*.COMMON.SAVE:T=F) $(.SELECT.:A!=.ARCHIVE|.COMMAND|.OBJECT)
.MANIFEST. = $(.MANIFEST.FILES.:$(.RECURSE.OFFSET.:?C,^\([^/]\),$(.RECURSE.OFFSET.)/\1,??):P=C:H!)
.SOURCES. = $(.SELECT.:A=.REGULAR:A!=.ARCHIVE|.COMMAND|.OBJECT)

/*
 * stop expansion of certain state variables
 */

(AR) (ARFLAGS) (AS) (ASFLAGS) (CPP) (CC) (CCFLAGS) (CCLD) \
	(CCLDFLAGS) (COATTRIBUTES) (F77) (F77FLAGS) (IFFE) \
	(IFFEFLAGS) (LD) (LDFLAGS) (LDLIBRARIES) (LEX) (LEXFLAGS) \
	(M4) (M4FLAGS) (SHELLMAGIC) (YACC) (YACCFLAGS) : .PARAMETER

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
 * library naming conventions
 */

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
		return + "${mam_cc_static} $(L:T=F) ${mam_cc_dynamic}"
	end
	if ( L = "$(.ARPROFILE. $(B))" )
		return $(L)
	end
	if ( L = "$(A:A=.TARGET)" )
		return $(L)
	end
	if ! "$(.LIBRARY.LIST.)" || "$(.PACKAGE.$(B).library)" != "-l"
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
	if ( static || "$(.PACKAGE.$(B).library)" == "+l" ) && "$(<<)" != ".BIND.+l%"
		V := $(CC.PREFIX.ARCHIVE)$(B)$(CC.SUFFIX.ARCHIVE)
		if ( T = "$(V:A=.TARGET)" )
			return $(T)
		end
		if ( T = "$(V:T=F)" ) && "$(T:D)" != "$(CC.STDLIB:/ /|/G)" && "$(T:D)" != "/usr/($(.PACKAGE.:/ /|/G))"
			return $(T)
		end
	end
	if "$(CC.DIALECT:N=DYNAMIC)" && ( "$(CCLDFLAGS:N=$(CC.DYNAMIC))" || ! "$(CCLDFLAGS:N=$(CC.STATIC))" )
		if "$(CC.SUFFIX.DYNAMIC)" && "$(CC.SUFFIX.SHARED)"
			if ( T = "$(*$(B):N=*$(CC.SUFFIX.DYNAMIC):D:B:S=$(CC.SUFFIX.SHARED):A=.TARGET)" )
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
			for J $(CC.PREFIX.SHARED) ''
				for I $(P) ''
					if "$(CC.SUFFIX.DYNAMIC)"
						if T = "$(*.SOURCE.%.ARCHIVE:L>$(CC.PREFIX.ARCHIVE)$(B)$(I)$(CC.SUFFIX.ARCHIVE)|$(J)$(B)$(I)$(CC.SUFFIX.SHARED))"
							if "$(T)" == "*$(CC.SUFFIX.ARCHIVE)"
								if S = "$(T:D:B=$(J)$(T:B:/$(CC.PREFIX.ARCHIVE)//):S=$(CC.SUFFIX.SHARED):T=F)"
									return $(S:T=F)
								end
							end
							return $(T:T=F)
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
								if T == "*$(CC.SUFFIX.ARCHIVE)"
									return $(T)
								end
								if "$(CC.SUFFIX.STATIC)" && T == "*$(CC.SUFFIX.STATIC)"
									H = -
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
	if "$(CC.REQUIRE.$(B))"
		$(%) : .DONTCARE
	elif T = "$(B:B:S=.req:T=F)"
		return $(T:T=I)
	else
		T := lib/$(B)
		$(T) : .ARCHIVE
		if ! "$(T:T=F)"
			T := $(MAKELIB:D)/$(T)
		end
		if "$(T:T=F)"
			$(%) : .DONTCARE
			return $(T:T=I)
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

.REQUIRE.-l% : .FUNCTION
	local B R T D DL DR
	B := $(%:/-l//)
	if "$(-mam:N=static*)" && "$(%)" != "-l+([a-zA-Z0-9_])"
		return
	end
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
			return
		end
	end
	if ! ( T = "$(%:/-l\(.*\)/\1.req/:T=F)" )
		R := lib/$(B)
		$(R) : .ARCHIVE
		if ! ( T = "$(R:T=F)" )
			R := $(MAKELIB:D)/$(R)
			if ! ( T = "$(R:T=F)" )
				return
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
					return
				end
			end
		end
	elif "$(-mam:N=static*,port*)"
		return ${mam_lib$(B)}
	end
	return $(T:T=I)

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
	$(<<:T=AF)

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
	$(.ARCLEAN.LIST.:K=$(RM) -f)

.COMMAND.o : .USE .COMMAND (CCLD) (CCLDFLAGS) $$(LDLIBRARIES)
	$(CCLD) $(CCLDFLAGS) $(&:T=D:N!=-[DIUl]*) -o $(<) $(*)

.OBJECT.o : .USE (LD) (CCFLAGS) (LDFLAGS)
	$(LD) -r $(LDFLAGS) -o $(<) $(*)

/*
 * match-all metarules
 */

% : %.c (CC) (CCFLAGS) (LDFLAGS) $$(LDLIBRARIES)
	$(CC) $(CCFLAGS) $(LDFLAGS:N!=-[DIU]*) $(CCLDFLAGS) -o $(<) $(*)

% : %.f (F77) (F77FLAGS) (LDFLAGS) $$(LDLIBRARIES)
	$(F77) $(F77FLAGS) $(LDFLAGS) -o $(<) $(*)

% : %.r (F77) (F77FLAGS) (LDFLAGS) $$(LDLIBRARIES)
	$(F77) $(F77FLAGS) $(LDFLAGS) -o $(<) $(*)

% : %.F (F77) (CCFLAGS) (F77FLAGS) (LDFLAGS) $$(LDLIBRARIES)
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
	"")	case '$(&:T=E)' in
		"")	$(CP) $(>) $(<)
			;;
		*)	{
			i=`(read x; echo $x) < $(>)`
			case $i in
			'#!'*|*'||'*|':'*|'":"'*|"':'"*)	echo $i ;;
			esac
			echo $(&:T=E)
			cat $(>)
			} > $(<)
			;;
		esac
		;;
	*)	{
		echo '$(SHELLMAGIC)'
		case '$(&:T=E)' in
		?*)	echo $(&:T=E) ;;
		esac
		cat $(>)
		} > $(<)
		;;
	esac
	$(CHMOD) u+w,+x $(<)

/*
 * double suffix metarules
 */

%.o : %.c (CC) (CCFLAGS)
	$(CC) $(CCFLAGS) -c $(>)

%.o : %.r (F77) (F77FLAGS)
	$(F77) $(F77FLAGS) -c $(>)

%.o : %.f (F77) (F77FLAGS)
	$(F77) $(F77FLAGS) -c $(>)

%.o : %.F (F77) (F77FLAGS)
	$(F77) $(F77FLAGS) -c $(>)

%.o : %.s (AS) (ASFLAGS)
	$(AS) $(ASFLAGS) -o $(<) $(>)

%.o : %.S (CC) (CCFLAGS)
	$(CC) $(CCFLAGS) -c $(>)

%.c %.h : %.y .YACC.SEMAPHORE (YACC) (YACCFLAGS)
	$(YACC) $(YACCFLAGS) $(>)$(YACCFIX.$(%):?$("\n")$(ED) $(EDFLAGS) y.tab.c <<!$("\n")g/yy/s//$(YACCFIX.$(%))/g$("\n")g/YY/s//$(YACCFIX.$(%):F=%(invert)S)/g$("\n")w$("\n")q$("\n")!??)$(YACCHDR.$(%):?$("\n")$(ED) $(EDFLAGS) y.tab.c <<!$("\n")1i$("\n")#include "$(YACCHDR.$(%))"$("\n").$("\n")w$("\n")q$("\n")!??)
	$(MV) y.tab.c $(%).c
	if	$(SILENT) test -s y.tab.h
	then	$(ED) $(EDFLAGS) y.tab.h <<'!'
	1i
	$("#")ifndef _$(%:F=%(upper)S)_H
	$("#")define _$(%:F=%(upper)S)_H
	.
	$a
	$("#")endif $("/")* _$(%:F=%(upper)S)_H *$("/")
	.
	w
	q
	!
		$(YACCFIX.$(%):?$(ED) $(EDFLAGS) y.tab.h <<!$("\n")g/yy/s//$(YACCFIX.$(%))/g$("\n")g/YY/s//$(YACCFIX.$(%):F=%(invert)S)/g$("\n")w$("\n")q$("\n")!$("\n")??)if	$(SILENT) $(CMP) -s y.tab.h $(%).h
		then	$(RM) $(RMFLAGS) y.tab.h
		else	$(MV) y.tab.h $(%).h
		fi
	fi
	if	$(SILENT) test -f y.output
	then	$(MV) y.output $(%).grammar
	fi

%.c : %.l .LEX.SEMAPHORE (LEX) (LEXFLAGS) (CC)
	$(LEX) $(LEXFLAGS) $(>)$(LEXFIX.$(%):?$("\n")$(ED) $(EDFLAGS) lex.yy.c <<!$("\n")g/yy/s//$(LEXFIX.$(%))/g$("\n")g/YY/s//$(LEXFIX.$(%):F=%(invert)S)/g$("\n")w$("\n")q$("\n")!??)$(LEXHDR.$(%):?$("\n")$(ED) $(EDFLAGS) lex.yy.c <<!$("\n")1i$("\n")#include "$(LEXHDR.$(%))"$("\n").$("\n")w$("\n")q$("\n")!??)
	$(MV) lex.yy.c $(<)

%.o : %.C (CC) (CCFLAGS)
	$(CC) $(CCFLAGS) -c $(>)

%.o : %.cc (CC) (CCFLAGS)
	$(CC) $(CCFLAGS) -c $(>)

%.o : %.cpp (CC) (CCFLAGS)
	$(CC) $(CCFLAGS) -c $(>)

%.o : %.cxx (CC) (CCFLAGS)
	$(CC) $(CCFLAGS) -c $(>)

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
		$(MAKE) --file=$(MAKEFILE) --keepgoing $(-) --errorid=$(<:V:Q) .ATTRIBUTE.$(IFFEGENDIR)/%:.ACCEPT MAKEPATH=..:$(MAKEPATH) $(=:N!=MAKEPATH=*) $(.RECURSE.ARGS.:N!=.CC-*:/^\.INSTALL$/.CC-INSTALL/) $(%:Y!$$(INSTRUMENT_$$(%:/,.*//):@?instrument=$$(%:/,.*//)?CCFLAGS=-$$(%:V:/$$(%:V:N=*~*:?~?,?)/ /G:@/-W\(.\) /-W\1,/G:@Q)?)!!) $(.VARIANT.$(<))
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
	T := $(T:T=F:T=I:/$("\n")/ /G:N!=-l($(T:B:C, ,|,G)))
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
	local A T U X
	T := $(.SOURCES.:G=%.c:B:S=$(CC.SUFFIX.OBJECT)) $(.SOURCES.:G=%.C:B:S=$(CC.SUFFIX.OBJECT)) $(.SOURCES.:G=%.cxx:B:S=$(CC.SUFFIX.OBJECT))
	if T
		.SOURCE.c : $(IFFESRCDIR)
		.CIA.REBIND : .MAKE .VIRTUAL .FORCE .AFTER .FOREGROUND
			.REBIND : $(CIAFILES)
		for U $(T)
			X := $(*$(U):G=%.c) $(*$(U):G=%.C) $(*$(U):G=%.cxx)
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
	$(CMP) -s $(*:O=1) $(<) || { $(RM) -f $(<); $(SILENT) test -d $(<:D) || $(MKDIR) $(<:D); $(CP) $(*:O=1) $(<); }

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
		else	$(*:O=1:P=I=$(<):?: $(<) linked to $(*:O=1) ||?$$(compare:/^0$//:Y@$$$(SILENT) $$$(CMP) -s $$$(*:O=1) $$$(<) ||@@)?)
			{
			if	$(SILENT) test -f "$(<)"
			then	$(.DO.INSTALL.OLD. $(<))
			fi
			$(link:/^0$//:?$$(<:B:$$(<:A=.COMMAND:Y@@S@):N=$$(link:/^1$/*/):Y@$$$(LN) $$$(_feature_:N=ln|ln-s:O=2:Y%-s $$$(<:D=$$$(<:D:P=R=$$$(*:O=1:D)):B:S)%$$$(*:O=1)%) $$$(<) || @@)??)$(IGNORE) $(CP) $(*:O=1) $(<) $(.DO.INSTALL.OPT.)
			}
		fi
	fi

.DO.INSTALL.DIR : .USE .PLACEHOLDER
	set -
	if	test ! -d $(<) 
	then	$(-silent:??set -x && ?)$(MKDIR) -p $(<) 2>/dev/null$(-silent:?? && set -?) ||
		{
			test -d ./-p && $(RM) -Rf ./-p
			p=
			for d in $(-mam:N=static*:?`echo $$(<) | $$(SED) -e 's%\\(.\\)/%\\1 %g'`?$$(".":P=R=$(<):C%/% %G)?)
			do	case $p in
				"")	p=$d ;;
				*)	p=$p/$d ;;
				esac
				case $d in
				.|..)	;;
				*)	if	test ! -d $p
					then	$(RM) $(RMFLAGS) $p
						$(-silent:??set -x && ?)$(MKDIR) $p$(-silent:?? && set -?)
					fi
					;;
				esac
			done
		} $(-silent:??&& set -x?)$(.DO.INSTALL.OPT.)
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

.NOOPTIMIZE.c .CC.NOOPTIMIZE /* drop .CC.* in 2001 */ : .MAKE .LOCAL
	CCFLAGS := $(.MAM.CCFLAGS|CCFLAGS:VP:N!=-O*|$\(CC.OPTIMIZE\))

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
	return $$(CC) $$(CCFLAGS:N!=-g) $(RO) -c $(.ROSRC.)

.READONLY.c : .USE .IMPLICIT
	$(.READONLY. $(>) $(<) $(CC.READONLY))

.DO.READONLY.c.sed : .USE
	$(CC) $(CCFLAGS:N!=-g) -S $(.ROSRC.)
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
	$(ED) $(EDFLAGS) 1.$(tmp).c > 2.$(tmp).c <<!
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

.RECURSE.OFFSET. = $(.RWD.)

.RECURSE : .MAKE .VIRTUAL .FORCE
	.RECURSE.OFFSET. :=

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
	local P X M I T
	if ! ( P = "$(%)" )
		if "$(PWD:B)" == "src|source"
			P = ($(MAKEDIRS:/:/|/G))/!($(MAKESKIP))
		elif "$(PWD:D::B)" == "src|source"
			P = !(.*|$(MAKESKIP))
		elif "$(PWD:C,$,/src,:P=X)"
			P = src/($(MAKEDIRS:/:/|/G))/!(.*|$(MAKESKIP))
		else
			P = *
		end
	end
	M := ($(MAKEFILES:/:/|/G))
	T := $(.RECURSE.ARGS.:A!=.ONOBJECT:N!=.RECURSE)
	X := $(P:N!=-:C,$,/$(M),:P=G)
	$(X) : .OBJECT
	X := $(X:W=O=$(T))
	M := $(X:B:S:/ /|/G)
	if M
		M := $(M)|
	end
	M := $(M)recurse|.RECURSE
	.ORIGINAL.ARGS. := $(.ORIGINAL.ARGS.:N!=$(M))
	.ARGS : .CLEAR $(~.ARGS:N!=$(M))
	$(X:N!=-|.CC-*|cc-*) : .RECURSE.DIR
	if recurse == "list"
		print $(X:/ /$("\n")/G)
		exit 0
	end
	return $(X)

.RECURSE.DIR : .USE .ALWAYS .LOCAL .FORCE .RECURSE.SEMAPHORE
	set -
	if	$(physical:?$$(*.VIEW:O=2:N=...:Y%2d%%)??) test -d $(<) $(skeleton:?|| mkdir $(<)??)
	then	$(-silent:Y%%echo $(recurse_enter) $(.RWD.:?$(<:N!=/*:?$(.RWD.)/??)??)$(<): >&2%)
		cd $(<)
		$(MAKE) $(-) --errorid=$(<:Q) $(=:N!=MAKEPATH=*|VPATH=*) .RWD.=$(.RWD.:C%$%/%)$(<) $(.RECURSE.ARGS.)
		$(recurse_exit:Y,$$(-silent:Y%%echo $$(recurse_exit) $$(.RWD.:?$$(<:N!=/*:?$$(.RWD.)/??)??)$$(<): >&2%),,)
	elif	test '' = '$(physical)'
	then	echo $(<): cannot recurse on virtual directory >&2
	fi

.RECURSE.FILE : .USE .ALWAYS .LOCAL .FORCE .RECURSE.SEMAPHORE
	set -
	$(-silent:Y%%echo $(.RWD.:?$(<:N!=/*:?$(.RWD.)/??)??)$(<): >&2%)
	$(MAKE) $(-) --errorid=$(<:B:Q) --file=$(<) $(=) .RWD.=$(.RWD.) $(.RECURSE.ARGS.)

.UNIQ. : .FUNCTION
	.UNION : .CLEAR $(%)
	return $(~.UNION)

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
					if CC.SUFFIX.SHARED == ".lib"
						continue
					end
				elif T1 == "$(CC.LD.DYNAMIC)"
					STATIC = 0
					if CC.SUFFIX.SHARED == ".lib"
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
						$(T2) : .SPECIAL CC.DLL= CC.DLLBIG= CC.PIC=
						if CC.SUFFIX.SHARED == ".lib"
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
		if T3
			$(T3) : .SPECIAL $(TA:V:Q)
			if "$(.OPTIONS.$(<))"
				$(T3) : .SPECIAL (.OPTIONS.$(<))
			end
		end
		if TS
			.LIBRARY.STATIC.$(<:B:/^$(CC.PREFIX.ARCHIVE)//) : $(CC.LD.STATIC) $(TS) $(CC.LD.DYNAMIC)
		end
		T0 := $(<)
		if "$(<:A=.ARCHIVE)" || "$(TP:V:A=.ATTRIBUTE:A=.ARCHIVE)"
			TP += -COMMAND
			if ! "$(@:V)" && ! "$(~:A=.USE)"
				TP += .ARCHIVE$(CC.SUFFIX.OBJECT)
			end
		elif ! "$(T3:N=$(<)?($(CC.SUFFIX.COMMAND)))"
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

":DLL:" : .MAKE .OPERATOR
	local DIR OPT
	.DLL.LIST. += $$(.LIBRARY.LIST.)
	if ! ( OPT = "$(>:N=-*)" )
		OPT := $(CC.OPTIMIZE)
	end
	if "$(>:N=big|BIG)"
		.ALL : .DLLBIG
		OPT += $(CC.DLLBIG)
	else
		.ALL : .DLL
		OPT += $(CC.DLL)
	end
	DIR := cc$(OPT:/ /,/G)
	.VARIANT.$(DIR) := VARIANT=DLL

.DLL .DLLBIG : .MAKE
	local DIR
	if .DLL.LIST. && ! "$(-mam:N=static*,port*)" && ! "$(.SHARED.ON.)" && ( CC.SUFFIX.DYNAMIC || CC.SUFFIX.SHARED )
		if "$(<)" == ".DLLBIG"
			DIR := cc-g,$(CC.DLLBIG:/ /,/G)
		else
			DIR := cc-g,$(CC.DLL:/ /,/G)
		end
		if ! "$(DIR:T=F)"
			{ test -d $(DIR) || $(MKDIR) $(DIR) }
		end
		.ORIGINAL.ARGS. := .DLL.$(.ORIGINAL.ARGS.:N=install:@?INSTALL?ALL?)
		make $(DIR)
	end

.DLL.ALL : $$(.DLL.LIST.)

.DLL.INSTALL : $$(*.INSTALL:N=*$$(.DLL.LIST.:/ /|/G)*)

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
	local O R S T
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
				.INSTALL : $(<:V:D)
			end
			.INSTALL : $(<:V)
			if "$(@:V)"
				eval
				$$(<:V) : .SPECIAL .SCAN.IGNORE $(S) $(R)
					$(@:V)
				end
			elif "$(R)"
				$(<:V) : .SPECIAL .SCAN.IGNORE $$("$(R)":N!=$$(<)|$$(<:P=U):@?$(O) $(S) $(R) .DO.INSTALL??)
			else
				$(<:V) : .SPECIAL .SCAN.IGNORE $$(<:B:S:N!=$$(<):@?$(O) $(S) $(R) $$(<:B:S) .DO.INSTALL??)
			end
		end
	end

/*
 * install rhs into lhs dir
 */

":INSTALLDIR:" : .MAKE .OPERATOR
	local O R T
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
			$(T:D=$(<:V):B:S) :INSTALL: $(O) $(T)
				$(@:V)
			end
		end
	end

/*
 * install in lhs dir using rhs pattern to select 
 */

":INSTALLMAP:" : .MAKE .OPERATOR
	if "$(<)"
		local T
		T := $(.GENSYM.)
		.INSTALL : $(<) $(T)
		$(<) : .SPECIAL .DO.INSTALL.DIR
		.INSTALL.MAPS. += $(>)
		eval
		$$(T) : .MAKE .FORCE .IGNORE
			local T
			for T $$(...:$(>):A!=.ATTRIBUTE|.FUNCTIONAL|.MAKE|.VIRTUAL:T=F:N!=$(<)/*)
				$$(T:D=$(<):B:S) : $$(T) .SPECIAL .SCAN.IGNORE .DO.INSTALL
				.INSTALL : $$(T:D=$(<):B:S)
			end
		end
	end

/*
 * install $(PROTO) output of rhs in lhs dir
 */

":INSTALLPROTO:" : .MAKE .OPERATOR
	local A
	if "$(<)" == "*.h"
		A = :INSTALL:
	else
		A = :INSTALLDIR:
	end
	eval
	$(<) $(A) $(>) (PROTO) (PROTOFLAGS)
		$$(PROTO) -p $$(PROTOFLAGS) $$(*) > 1.$(tmp).x
		if	$$(CMP) -s $$(<) 1.$(tmp).x
		then	$$(RM) -f 1.$(tmp).x
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
	local A B D L S T V
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
	L := $(.LIB.NAME. $(<))
	$(L) : .ARCHIVE$(CC.SUFFIX.OBJECT)
	if "$(.SHARED.ON.)"
		if ! "$(CC.DLL:N=-D_BLD_DLL)"
			_BLD_DLL == 1
		end
		if CC.SUFFIX.DYNAMIC == ".dll" && CC.SUFFIX.SHARED != ".x"
			.INSTALL.$(L) := .
		end
	else
		if ! "$(CC.DLL:N=-D_BLD_DLL)"
			_BLD_DLL ==
		end
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
	S := $(B) $(>:V:N=[-+]l*:/[-+]l//) $(.PACKAGE.LIBRARIES. $(.PACKAGE.build:A!=.TARGET):/-l//:N!=$(B))
	D := $(>:V:N=-L*)
	if S
		if ! "$(S:N=$(B))"
			S := $(B) $(S)
		end
		eval
		if ! "$(.NO.INSTALL.)"
			$$(LIBDIR)/lib/$(B) :INSTALL: $(B).req
		end
		.REQUIRE.$(B) = $(S)
		(.REQUIRE.$(B)) : .PARAMETER
		if "$(-mam:N=static*,port*)"
			$(B).req : (CC) (CCFLAGS) (LDFLAGS) (.REQUIRE.$(B))
				set -
				echo 'main(){return(0);}' > 1.$(tmp).c
				$$(CC) $$(CCFLAGS) -c 1.$(tmp).c &&
				x=`$$(CC) $$(CCFLAGS) $$(LDFLAGS) -o 1.$(tmp).x 1.$(tmp)$(CC.SUFFIX.OBJECT) -l'*' 2>&1 | $(SED) -e 's/[][()+@?]/#/g' || :` &&
				{
				case "$(D)" in
				*?)	echo " $(D)" ;;
				esac
				for i in $$(.REQUIRE.$(B))
				do	case $i in
					"$(B)"$(...:A=.ARCHIVE:A=.TARGET:N=$(CC.PREFIX.ARCHIVE)*$(CC.SUFFIX.ARCHIVE):/^$(CC.PREFIX.ARCHIVE)\(.*\)$(CC.SUFFIX.ARCHIVE)/|\1/:@/ //G))
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
			$(B).req : (CC) (.REQUIRE.$(B))
				echo "" $$(.REQ. $$(.REQUIRE.$(B))) > $$(<)
		end
		$(L) : .INSERT $(B).req.REQUIRE
		$(B).req.REQUIRE : .VIRTUAL .IGNORE .NULL $(B).req
		end
	end
	eval
	_BLD_$(B:B:S:/[^a-zA-Z0-9_]/_/G) == 1
	end
	$(T) : $(L) $(.SHARED. $(L) $(B) $(V|"-") $(>:V:N=[!-+]*=*) $(>:V:N=[-+]l*))
	for T $(<:O>2)
		if T == "DLL*"
			.DLL.LIST. += $(B)
			.ALL : .$(B)
		end
	end

.REQ. : .FUNCTION
	local I Q R
	for I $(%)
		I := $(I:/^[-+]l//)
		if "$(.REQUIRE.$(I))"
			R += -l$(I)
		else
			I := -l$(I)
			if Q = "$(.REQUIRE.-l% $(I))"
				if "$(Q:N=$(I))"
					R += $(I)
				end
			elif "$(I:T=F)"
				R += $(I)
			end
		end
	end
	return $(R)

.SHARED.DEF. = .SHARED.DEF$(CC.SUFFIX.SHARED)
.SHARED.USE. = .SHARED$(CC.SUFFIX.SHARED)
.SHARED.FLAGS. = : $(CCFLAGS) :

.SHARED.ON. : .FUNCTION
	if ! "$(static)" && ! "$(CC.LIB.DLL:N=broken)" && "$(CC.DLL)" && ! "$(-mam:N=static*,port*)" && ( "$(.SHARED.FLAGS.:@N=* ($(CC.DLL)|$(CC.DLLBIG)) *)" || "$(.SHARED.FLAGS.:V:@N=* ($\(CC.DLL\)|$\(CC.DLLBIG\)) *)" )
		return 1
	end

.SHARED. : .FUNCTION
	local B L S T
	if "$(.SHARED.ON.)"
		if "$(.SHARED.DEF.:A=.TARGET)"
			return $($(.SHARED.DEF.) $(%))
		end
		B := $(%:O=2)
		L := $(%:O>3:N=[-+]l*)
		$(L) : .DONTCARE
		B := $(CC.PREFIX.SHARED)$(%:O=2)$(CC.SUFFIX.SHARED)
		if "$(%:O=3)" != "[0-9]*"
			S := $(B)
		else
			S := $(B).$(%:O=3)
		end
		$(S) : .SHARED.o $(%:N=[!-+]*=*) $(CC.PREFIX.ARCHIVE)$(%:O=2)$(CC.LIB.TYPE:O=1)$(CC.SUFFIX.ARCHIVE) $$(.SHARED.BIND. $(L))
		if ! "$(.INSTALL.$(S))" && ! "$(.NO.INSTALL.)"
			$$(LIBDIR) :INSTALLDIR: $(S)
				$(LD_PRELOAD:N=$(<:D:B:D:B:D:B:S=$(CC.SUFFIX.SHARED)):?LD_PRELOAD=""; _RLD_LIST=DEFAULT;?)if	silent test -f $(<:C%\$(CC.SUFFIX.SHARED)\.%.oo.%)
				then	$(STDRM) -f $(<:C%\$(CC.SUFFIX.SHARED)\.%.oo.%)
				fi
				if	silent test -f $(<)
				then	$(STDMV) $(<) $(<:C%\$(CC.SUFFIX.SHARED)\.%.oo.%)
				fi
				$(STDCP) $(<:B:S) $(<)
				if	silent test -f $(<:D:B:D:B:D:B:S=$(CC.SUFFIX.SHARED))
				then	$(STDRM) -f $(<:D:B:D:B:D:B:S=$(CC.SUFFIX.SHARED))
				fi
				$(STDLN) $(<) $(<:D:B:D:B:D:B:S=$(CC.SUFFIX.SHARED))
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
		for L $(%:T=F:P=B:/\(.*\)\$(CC.SUFFIX.SHARED)\(\.[0-9.]*\)$/\1$(CC.SUFFIX.STATIC)\2)
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
	return $(.UNIQ. $(S))

.SHARED.o : .USE (LD) (LDFLAGS) $$(LDLIBRARIES)
	$(LD) $(LDFLAGS) $(CC.SHARED) -o $(<) $(.CC.LIB.DLL.$(CC.LIB.DLL) $(.UNIQ. $(*:$(CC.SHARED:@??:T=F:N=*$(CC.SUFFIX.ARCHIVE)?)))) $(CC.DLL.LIBRARIES)

.SHARED.DEF.lib .SHARED.DEF.x : .FUNCTION
	local B D L S X Y Z W
	Y := $(%:O=2)
	B := $(Y)$(%:O=3:/[^0-9]//G)$(dll.custom:?_$(dll.custom)??)
	D := $(CC.PREFIX.DYNAMIC)$(B:B:S=$(CC.SUFFIX.DYNAMIC))
	if "$(%:O=1)" != "-"
		L := $(Y:B:S=$(CC.SUFFIX.SHARED))
		S := $(B:B:S=$(CC.SUFFIX.SHARED))
		Z := $(%:O>3:N=+l*)
		X := $(Y) $(Z:/+l//)
		Z += $(%:O>3:/.l//:N!=$(X:/ /|/):/^/-l/)
		$(Z) : .DONTCARE
		if CC.SUFFIX.SHARED == ".lib"
			W := $(L:B:S=.so)
			D := $(W)/$(D)
			S := $(W)/$(S)
			if "$(*.LIBRARY.STATIC.$(Y))"
				Z += .LIBRARY.STATIC.$(Y)
				.LIBRARY.STATIC.$(Y) : .VIRTUAL
			end
		end
		$(D) $(S) : .JOINT $(<:/DEF.//) $(%:O=1) $(Z)
		.ALL : $(D) $(S)
		if ! "$(.NO.INSTALL.)"
			if ! "$(.INSTALL.$(D))"
				$(DLLDIR) :INSTALLDIR: $(D)
			end
			if ! "$(.INSTALL.$(S))"
				$(LIBDIR)/$(L) :INSTALL: $(S)
				if CC.SUFFIX.SHARED == ".lib"
					eval
					.NO.STATIC.$(Y) : .AFTER
						if	$(SILENT) test -f $(LIBDIR)/$(CC.PREFIX.ARCHIVE)$(Y)$(CC.SUFFIX.ARCHIVE)
						then	$(MV) $(LIBDIR)/$(CC.PREFIX.ARCHIVE)$(Y)$(CC.SUFFIX.ARCHIVE) $(LIBDIR)/$(CC.PREFIX.ARCHIVE)$(Y)-static$(CC.SUFFIX.ARCHIVE)
						fi
					end
					$(LIBDIR)/$(L) : .NO.STATIC.$(Y)
				end
			end
		end
	end
	return $(D)

.SHARED.REF.lib : .FUNCTION
	local L
	L := $(%:N=*$(CC.SUFFIX.ARCHIVE):O=1)
	return $(*$(L):N=*@($(CC.SUFFIX.LD:/ /|/G))) $(CC.LIB.ALL) $(L) $(CC.LIB.UNDEF) $(*.LIBRARY.STATIC.$(L:B:/^$(CC.PREFIX.ARCHIVE)//)) $(%:N!=*$(L))

.SHARED.lib : .USE $$(LDLIBRARIES)
	$(SILENT) test -d $(<:O=1:D) || mkdir $(<:O=1:D)
	$(LD) $(LDFLAGS) $(CCFLAGS:N=-[gG]*) $(CC.SHARED) -o $(<:O=1) $(.SHARED.REF.lib $(*)) $(CC.DLL.LIBRARIES)

.SHARED.REF.x : .FUNCTION
	local L
	L := $(%:N=*$(CC.SUFFIX.ARCHIVE):O=1)
	return $(.CC.LIB.DLL.symbol $(L)) $(*.LIBRARY.STATIC.$(L:B:/^$(CC.PREFIX.ARCHIVE)//)) $(%:N!=*$(CC.SUFFIX.OBJECT))

.SHARED.x : .USE $$(LDLIBRARIES)
	$(CC) $(LDFLAGS) $(CCFLAGS:N=-[gG]*) $(CC.SHARED) -o $(<:O=1:B:S) $(.SHARED.REF.x $(*)) $(CC.DLL.LIBRARIES)

/*
 * link lhs to rhs
 */

":LINK:" : .MAKE .OPERATOR
	local T U
	if "$(<:N=*/*)"
		.ALL : $(<:D) $(<)
		$(<:D) : .SPECIAL .DO.INSTALL.DIR
		$(<) : .DO.LINK $(>)
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
 * lhs is CC.HOSTTYPE match pattern to match
 * activated for all architectures if lhs omitted
 */

":NOOPTIMIZE:" : .MAKE .OPERATOR .PROBE.INIT
	local T
	if ! "$(<)" || "$(-mam)" == "static*" || CC.HOSTTYPE == "$(<)"
		for T $(>)
			if "$(@.NOOPTIMIZE$(T:S):V)"
				$(T:B:S=$(CC.SUFFIX.OBJECT)) : .SPECIAL .NOOPTIMIZE$(T:S)
			else
				error 1 :NOOPTIMIZE: ignored for suffix $(T:S)
			end
		end
	end

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
 */

.PACKAGE. =
.PACKAGE.build =
.PACKAGE.install =
.PACKAGE.libraries =

.PACKAGE.LIBRARIES. : .FUNCTION
	local L P R
	for L $(%)
		if P = "$(.PACKAGE.$(L).library)"
			R += $(**:B:S:N=[-+]l$(L)|$(CC.PREFIX.ARCHIVE)$(L)$(CC.LIB.TYPE:?*($(CC.LIB.TYPE:/ /|/G))??)$(CC.SUFFIX.ARCHIVE):@??$(P)$(L)?)
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
 * the canonicalized last package name is returned
 */

.PACKAGE.GLOBAL. =
.PACKAGE.LOCAL. =

.PACKAGE.INIT. : .FUNCTION .PROBE.INIT
	local T1 T3 T4 T5 T6 T7
	local B D G H I L N P Q S T V W X Z IP LP LPL LPV
	for T3 $(%)
		I := $(PACKAGE_$(T3)_INCLUDE)
		IP := $(.PACKAGE.$(T3).include)
		if IP == "/*"
			if ! I
				I := $(IP)
			end
			IP =
		end
		if ! IP
			IP = include
		end
		L := $(PACKAGE_$(T3)_LIB)
		LP := $(.PACKAGE.$(T3).lib)
		if LP == "/*"
			if ! L
				L := $(LP)
			end
			LP =
		end
		if ! LP
			LP = lib
		end
		if ( !I || !L )
			if ( !I && !L )
				T1 = $(INSTALLROOT)/$(IP)/$(T3)
				if "$(T1:P=X)"
					for P lib ""
						T1 = $(INSTALLROOT)/$(LP)/$(P)$(T3)$(CC.SUFFIX.SHARED)
						if "$(T1:P=X)"
							eval
							PACKAGE_$(T3) = $(INSTALLROOT)
							end
							break
						end
					end
				end
			end
			if ! ( T1 = "$(PACKAGE_$(T3))" )
				if ! .PACKAGE.GLOBAL.
					.PACKAGE.GLOBAL. := $(PATH:/:/ /G:D) $(OPTDIRS:/:/ /G)
					.PACKAGE.GLOBAL. := $(.PACKAGE.GLOBAL.:T=F:U)
				end
				T4 := $(PACKAGE_PATH:/:/ /G) $(.PACKAGE.DIRS.) $(.PACKAGE.GLOBAL.) $(.PACKAGE.GLOBAL.:/:/ /G:C%$%/$(T3)%)
				if T3 == "*[!0-9]+([0-9])"
					T4 += $(.PACKAGE.GLOBAL.:/:/ /G:C%$%/$(T3:C,[0-9]*$,,)%)
				end
				V =
				if T5 = "$(PACKAGE_$(T3)_VERSION)"
					N := $(T3)-$(T5) $(T3)$(T5) $(T3)R$(T5) $(T3)r$(T5)
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
				elif T3 == "*[0-9][rR]+([0-9])"
					N := $(T3)
					T3 := $(N:/[rR][0-9]*$//)
				else
					N :=
				end
				if B = "$(T3:N=[!0-9]*+([0-9.]):/[0-9]*$//)"
					B := @($(T3)|$(B))
				else
					B := $(T3)
				end
				N += $(T3)
				if T3 == "*[!0-9]+([0-9])"
					N += $(T3:C,[0-9]*$,,)
				end
				LPL := $(CC.STDLIB:B) $(LP)
				LPL := $(LPL:U)
				T4 := $(T4:N!=//*:T=F:U)
				T7 := $(T4:X=$(IP)/$(T3):N!=//*:T=F:D:D)
				T1 =
				for S $(CC.SUFFIX.SHARED) $(CC.SUFFIX.ARCHIVE)
					for D $(N) /
						T5 := $(T7) $(T4:N=*/$(D):D) $(T4)
						T5 := $(T5:U)
						for P lib ""
							if ! V
								if ! .PACKAGE.LOCAL.
									.PACKAGE.LOCAL. := $(PKGDIRS:T=F:P=A:U)
								end
								for X $(.PACKAGE.LOCAL.)
									X := $(X)/$(P)$(T3)$(S)
									if ( T1 = "$(X:P=X:O=1:D:D)" )
										break 4
									end
								end
							end
							for LPV $(LPL)
								X := $(T5:C%$%/$(D)/$(LPV)/$(P)$(T3)$(S)%:C%^//%/%)
								if ( T1 = "$(X:N!=//*:P=X:O=1:D:D)" )
									break 4
								end
								if T6 = "$(X:D:N!=//*:P=X:O=1)"
									if Z = "$(T6:L>=?(lib)$(B)*$(S)*([0-9.]))"
										if V
											if W = "$(Z:N=*$(V)*:O=1)"
												T1 := $(T6:D)
												break 4
											end
										end
										T1 := $(T6:D)
										break 4
									end
								end
							end
						end
					end
				end
				if ! "$(T1)"
					if ! ( T1 = "$(T4:C%$%/$(IP)/$(T3)%:N!=//*:P=X:O=1:D:D)" )
						T1 := $(T4:N=*/$(T3):C%$%/$(IP)%:N!=//*:P=X:O=1:D)
					end
					if K > 0 && ! "$(T1)"
						continue
					end
				end
				if T1 == "/"
					T1 = /usr
				end
				if K > 0
					K$(K) := 1
				end
				eval
				PACKAGE_$(T3) = $(T1:P=C)
				end
			end
			if T1
				.PACKAGE.DIRS. += $(T1)
				if !I
					I := $(T1)/$(IP)
					H := $(I)/$(T3)/.
					if "$(H:P=X)"
						I := $(I)/$(T3)
					end
					eval
					PACKAGE_$(T3)_INCLUDE = $(I)
					end
				end
				if !L
					L := $(T1)/$(LP)
					eval
					PACKAGE_$(T3)_LIB = $(L)
					end
				end
				if ! .INITIALIZED.
					eval
					_PACKAGE_$(T3) == 1
					end
				end
			end
		end
	end
	return $(T3)

/*
 * evaluate a logical expression of package ids
 * used by :VARIANT:
 */

PACKAGES : .SPECIAL .FUNCTION
	if ! "$(%)"
		return 1
	end
	: $(.PACKAGE.INIT. $(%:/[^a-zA-Z0-9_.]/ /G))
	return $(%:/\([A-Za-z0-9_.][A-Za-z0-9_.]*\)/"$$(PACKAGE_\1_LIB)"!=""/G:@/"  *"/" \&\& "/G:E)

":PACKAGE:" : .MAKE .OPERATOR
	local I T N P V version insert=0 install=1 library=-l
	if "$(<)"
		/* a separate include handles package definitions for now */
		eval
		$$(<:V) :package: $$(>:V)
			$(@:V)
		end
		return
	end
	if ! "$(ancestor)"
		/* please convert me to long op names */
		let ancestor = $(".":P=L=*:C,[^/],,G:O!:H=:O=1) - 2
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
				if N == "dynamic"
					library := -l
				elif N == "static"
					library := +l
					if V
						library := -l
					else
						library :=
					end
				elif N == "debug|insert|install|profile|threads|version"
					$(N) := $(V)
				end
			end
		elif T != "*:(command|force)"
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
				if N = "$(I:T=F)"
					include + "$(I)"
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
			.PACKAGE.$(P).library := $(library)
			if "$(V)"
				PACKAGE_$(P)_VERSION := $(V)
			end
			I := $(insert)
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
				if N == "dynamic"
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
				end
			end
			if ! "$(.PACKAGE.$(P).library)"
				-l$(P) : .VIRTUAL
			else
				eval
				_PACKAGE_$(P) == 1
				end
				if install && ! .PACKAGE.install
					if "$(INCLUDEDIR:V)" == "\$\(INSTALLROOT\)/include"
						.PACKAGE.install = 1
						INCLUDEDIR := $(INCLUDEDIR:V)/$(P)
					end
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
		end
	end

/*
 * :PACKAGE_INIT: foo.c bar.c ...
 *
 *	rhs built and copied to $(BINDIR) if not already there
 *	rhs appear in $(BINDIR) for .LIST.PACKAGE.BINARY
 *	for source that compiles either standalone or with richer libraries
 */

":PACKAGE_INIT:" : .MAKE .OPERATOR
	local I B
	.LIST.PACKAGE.BINARY : .LIST.PACKAGE.INIT
	.LIST.PACKAGE.INIT : .MAKE
		local I
		for I $(*)
			print ;;;$(I:T=F:P=A);$(BINDIR)/$(I:B)
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
		cc-$$(<) : .ALWAYS .LOCAL .FORCE .RECURSE.SEMAPHORE
			set -
			if	test "0" != "$$(PACKAGES $(>))"
			then	if	test ! -d $$(<)
				then	mkdir $$(<)
				fi
				$$(-silent:Y%%echo $$(.RWD.:C%$%/%)$$(<): >&2%)
				cd $$(<)
				$$(MAKE) --file=$$(MAKEFILE) --keepgoing $$(-) --errorid=$$(<) VARIANT=$$(<:/cc-//) VARIANTID=$$(<:/cc-//:N=[a-zA-Z]*:?-??)$$(<:/cc-//) .ATTRIBUTE.$(IFFEGENDIR)/%:.ACCEPT MAKEPATH=..:$$(MAKEPATH) $$(=:N!=MAKEPATH=*) $$(.RECURSE.ARGS.:N!=.CC-*) $(@:V:@Q)
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
 */

":cc:" : .MAKE .OPERATOR .PROBE.INIT
	if "$(CC)" != "$(cc)"
		.CLOBBER. += null.mo null.ms
		:: $(>)
		$(<) :: $(>:G=%.c:B:S=$(CC.SUFFIX.OBJECT))
		$(>:G=%.c:B:S=$(CC.SUFFIX.OBJECT)) : .CLEAR .JOINT .DO.cc
	end

.DO.cc : .USE .ALWAYS .LOCAL .FORCE
	$(-exec:?silent ??)$(MAKE) --file=/dev/null --file=$(MAKEFILE) $(-) --errorid=cc $(<) $(=) CC=$(cc:@Q) CCFLAGS=$(CCFLAGS:VP:@Q)

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
			.UNION. : $(T) $(?$(X:B:S):T=F:P=S=$(.RECURSE.OFFSET.):T!=G)
		end
	else
		.UNION. : $(.FILES.:T=F:T!=G)
		$(*.UNION.:N!=[-+]l*) : -ARCHIVE -COMMAND -OBJECT
		.UNION. : $(...:T!=XS:T=F:A=.REGULAR:P=S=$(.RECURSE.OFFSET.):T!=G)
		.UNION. : $(...:T=XSFA:T=F:A=.REGULAR:P=S=$(.RECURSE.OFFSET.):T!=G)
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
			if CC.DLL
				CC.PIC := $(CC.DLL)
			else
				CC.DLL := $(CC.PIC)
			end
			if ! CC.DLLBIG
				CC.DLLBIG := $(CC.DLL)
			end
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
	if CC.SUFFIX.DYNAMIC
		.ATTRIBUTE.%$(CC.SUFFIX.SHARED) : .ARCHIVE .SCAN.IGNORE
		.SOURCE.%$(CC.SUFFIX.SHARED) : .FORCE $$(*.SOURCE.%.ARCHIVE)
	end
	if ! CC.SUFFIX.OBJECT
		CC.SUFFIX.OBJECT = .o
	end
	.ATTRIBUTE.%$(CC.SUFFIX.OBJECT) : .OBJECT
	CC.SHARED += $(CC.SHARED.REGISTRY)

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
				CCFLAGS := $(CCFLAGS:N!=-g|-O*|-p*) -g
			end
			T1 := $(TI) = $($(TI):N=*=*)
			if ! T5
				T2 := $(T2:F=%(upper)S)
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
		CCFLAGS := $(CCFLAGS:V:N!=$(CC.OPTIMIZE)|$$\(CC.OPTIMIZE\)) $(CC.DEBUG)
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
			$(T4) : $($(T3:F=%(upper)S)DIR)
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
		local B D H I L N Q S T V W X Z K=0 K0=0 IP LP
		LDLIBRARIES += $$(.PACKAGE.LIBRARIES. $(T1))
		T2 =
		for T3 $(T1)
			if T3 == "{"
				if ! "$(-mam:N=static*)"
					let K = K + 1
					local K$(K)=0
				end
				continue
			elif T3 == "}"
				if ! "$(-mam:N=static*)"
					let K = K - 1
				end
				continue
			elif $(K$(K)) > 0
				continue
			end
			T3 := $(.PACKAGE.INIT. $(T3))
			I := $(PACKAGE_$(T3)_INCLUDE)
			L := $(PACKAGE_$(T3)_LIB)
			T2 += $(T3)
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
					print -um setv PACKAGE_$(T3) $(T4:N=P:?${INSTALLROOT}?$(H)?)
					print -um setv PACKAGE_$(T3)_INCLUDE ${PACKAGE_$(T3)}/$(T)
					print -um setv PACKAGE_$(T3)_LIB ${PACKAGE_$(T3)}/$(L:B:S)
					T4 += I L
				end
			end
			if I
				if ! "$(CC.STDINCLUDE:N=$(I))"
					.SOURCE.h : $(I)
					if "$(I:B)" == "$(T3)" && ! "$(CC.STDINCLUDE:N=$(I:D))"
						.SOURCE.h : $(I:D)
					end
				elif "$(-mam:N=static*)"
					.SOURCE.h : $(I)
					CC.STDINCLUDE := $(CC.STDINCLUDE:N!=$(I))
					if "$(I:B)" == "$(T3)"
						.SOURCE.h : $(I:D)
						CC.STDINCLUDE := $(CC.STDINCLUDE:N!=$(I:D))
					end
				end
				if "$(-mam:N=static*)"
					.MAMPACKAGE. += PACKAGE_$(T3)_INCLUDE
					if ! "$(T4:N=I)"
						print -um setv PACKAGE_$(T3)_INCLUDE $(T4:N=P:?${INSTALLROOT}/$(I:B:S)?$(I)?)
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
					.MAMPACKAGE. += PACKAGE_$(T3)_LIB
					if ! "$(T4:N=L)"
						print -um setv PACKAGE_$(T3)_LIB $(T4:N=P:?${INSTALLROOT}/$(L:B:S)?$(L)?)
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
	if !nativepp
		if CC.CC && ! ( T4 = "$(CC.PROBEPP)" )
			T4 := $(cctype:@P=P=C,pp)
		end
		if ! "$(CC.DIALECT:N=LIBPP)" || CPP != "$\(MAKEPP\)"
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
	end
	if "$(CCFLAGS:N=-I-)" || nativepp && ! "$(CC.DIALECT:N=-I-)"
		T3 += $$(*:A=.SCAN.c:@?$$$(*.SOURCE.%.LCL.INCLUDE:I=$$$$(!$$$$(*):A=.LCL.INCLUDE|.STD.INCLUDE:P=D):$(.CC.NOSTDINCLUDE.):/^/-I/)??) $$(&:T=D)
		if nativepp == "[1-9]*"
			error 1 local include files may be ignored by the native C preprocessor
		end
	else
		if ! prefixinclude
			T3 += -D:noprefix
		end
		T3 += $$(*:A=.SCAN.c:@?$$$(*.SOURCE.%.LCL.INCLUDE:I=$$$$(!$$$$(*):A=.LCL.INCLUDE:P=D):/^/-I/) -I- $$$(*.SOURCE.%.STD.INCLUDE:I=$$$$(!$$$$(*):A=.STD.INCLUDE:P=D):$(.CC.NOSTDINCLUDE.):/^/-I/)??) $$(&:T=D)
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
	if "$(CC.REPOSITORY)"
		T3 += $(T5:V) $$(CCFLAGS:N=-[D]*) $$(.PTR.PREREQS:A=.SCAN.c:@?$$$(*.SOURCE.%.LCL.INCLUDE:I=$$$$(.PTR.PREREQS:A=.LCL.INCLUDE:P=D):/^/-I/) $(CC.DIALECT:N=-I-) $$$(*.SOURCE.%.LCL.INCLUDE:I=$$$$(.PTR.PREREQS:A=.LCL.INCLUDE|.STD.INCLUDE:P=D):$(.CC.NOSTDINCLUDE.):/^/-I/)??) $$(&.PTR.CHECK:T=D)
		.APPEND.%.COMMAND : .PTR.CHECK .PTR.UPDATE
		T3 := $$(@:V:N=*$\(CC*(LD)\)*:@?$(T3:V)??)
	end
	if "$(CC.ARFLAGS)"
		.ARCHIVE.o : .CLEAR .USE .ARPREVIOUS (CC) (AR)
			$(.ARPREVIOUS.$(<:B:S):@?$(IGNORE) $$(AR) d $$(<) $$(.ARPREVIOUS.$$(<:B:S))$$("\n")??)$(CC) $(CC.ARFLAGS) -o $(<) $(*)
		.SHARED.o : .CLEAR .USE (CC)
			$(CC) $(CC.SHARED) -o $(<) $(*$(**):N!=*$(CC.SUFFIX.ARCHIVE))
		.ATTRIBUTE.%.a : -ARCHIVE
	end
	IFFEFLAGS += -c '$$(CC) $$(CCFLAGS) $$(LDFLAGS)' $$(-mam:N=static*:??-S '$$(CC.STATIC)')
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
		if LDRUNPATH
			CCLDFLAGS &= $(.CC.LD.RUNPATH.)
		end
		LDFLAGS &= $$(*.SOURCE.%.ARCHIVE:I=$$$(*:N=-l*:P=D):$(.CC.NOSTDLIB.):P=A:/^/-L/) $(T3:V)
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
				T2 := $(".$(T1:F=%(upper)S)":A=.IMMEDIATE|.TARGET:A!=.ATTRIBUTE)
				if T2
					T1 := $(T2)
				end
			end
			$(T3) : $(T1:V)
		end
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
		return $(%:/.//:F=%(lower)S)
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

.CC- .VARIANTS : .ONOBJECT $$(*.SOURCE:L<=cc-*:$$(variants:@?N=$$(variants)??)) $$(.VARIANTS.:$$(variants:@?N=$$(variants)??))

.CC-INSTALL : .ONOBJECT .ALL $$(*.INSTALL:N=*-*$$(CC.SUFFIX.ARCHIVE))

.CC.LD.RUNPATH. : .FUNCTION
	if "$(CC.DIALECT:N=DYNAMIC)" && ( "$(CCLDFLAGS:N=$(CC.DYNAMIC))" || ! "$(CCLDFLAGS:N=$(CC.STATIC))" ) && CC.LD.RUNPATH && LDRUNPATH
		return $(CC.LD.RUNPATH)$(LDRUNPATH):/$(CC.HOSTTYPE)/lib
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
		$(RM) -Rf $(<)
		$(MKDIR) $(<)
		cd $(<)
		$(AR) x $(*:P=A)
	.MAKE : $(F)
	.DONE : .DONE.$(F)
	.DONE.$(F) : .FORCE
		$(RM) -Rf $(<:/.DONE.//)
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

.COMMON.SAVE : .NULL .VIRTUAL .IGNORE .FOREGROUND

.CLEAN : .CLEARARGS .ONOBJECT
	$(IGNORE) $(RM) $(RMFLAGS) $(.BUILT.:A=.REGULAR:A!=.ARCHIVE|.COMMAND:N!=*.(m[klos]|db$(CC.SUFFIX.SHARED:?|$(CC.SUFFIX.SHARED:/\.//).*??))$(cleanignore:?|($(cleanignore))??))

.CLOBBER : .CLEARARGS .ONOBJECT .NOSTATEFILE
	$(IGNORE) $(RM) $(RMFLAGS) -R $(.CLOBBER.:T=F:P=L) $(.BUILT.:A=.REGULAR)

.CLOBBER.INSTALL : .ONOBJECT
	$(IGNORE) $(RM) $(RMFLAGS) -R $(.INSTALL.LIST.:T=F:P=L:A=.REGULAR)

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

.LIST.MANIFEST : .ONOBJECT .COMMON.SAVE .MAKE
	print $(.MANIFEST.:/ /$("\n")/G)

.LIST.PACKAGE.LOCAL : .ONOBJECT .COMMON.SAVE .MAKE
	print $(.MANIFEST.:P=A:/^/;;;/:/ /$("\n")/G)

.LIST.PACKAGE.BINARY : .ONOBJECT .MAKE
	.UNION : .CLEAR $(.INSTALL.LIST.:N=$(INSTALLROOT)/*:T=F:P=A)
	if package.strip
		local I
		for I $(*.UNION:$(PACKAGE_OPTIMIZE:N=space:Y%:N=$(INSTALLROOT)/(bin|fun|lib)/*:N!=*$(CC.SUFFIX.ARCHIVE)|$(INSTALLROOT)/lib/lib?(/*)%%))
			if "$(I:T=Y)" == "*/?(x-)(dll|exe)"
				print ^^filter $(STRIP) $(STRIPFLAGS) $(I)^$(I)
			else
				print ^^^$(I)
			end
		end
	else
		print $(*.UNION:/^/;;;/:/ /$("\n")/G)
	end

.LIST.PACKAGE.SOURCE : .ONOBJECT .COMMON.SAVE .MAKE
	local I
	PROTOEDIT = P=A
	for I $(.MANIFEST.:P=A)
		if I == "*.[chly]"
			print ^^$(PROTO) $(PROTOFLAGS) -dp $(I)^$(I)
		elif I == "*.sh"
			print ^^$(PROTO) $(PROTOFLAGS) -dp -c'#' $(I)^$(I)
		elif I == "*.sml"
			print ^^$(PROTO) $(PROTOFLAGS) -dp -c'(*)' $(I)^$(I)
		else
			print ^^^$(I)
		end
	end

.LIST.SOURCE.TGZ : .ONOBJECT .COMMON.SAVE .MAKE
	local F P
	PROTOEDIT = P=A
	for F $(.MANIFEST.)
		P := $(F:T=F)
		if F == "*.[chly]"
			print ^^$(PROTO) $(PROTOFLAGS) -dp $(P)^$(F)
		elif F == "*.sh"
			print ^^$(PROTO) $(PROTOFLAGS) -dp -c'#' $(P)^$(F)
		elif F == "*.sml"
			print ^^$(PROTO) $(PROTOFLAGS) -dp -c'(*)' $(P)^$(F)
		else
			print ^^^$(P)^$(F)
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
	$(".RECURSE.OFFSET.:=":R)$(*.VIEW:O=2:@?$$(.MANIFEST.:P=L:N!=[-/]*:C@.*@{ $$(DIFF) $$(DIFFFLAGS) $$(*.VIEW:O=2)/& & || true; } >> $(official_out:D=$$(*.VIEW:O=2):B:S); $$(MV) & $$(*.VIEW:O=2)/&;@)?: no lower view?)

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
		CC.DIALECT =
		CC.DLL = ${mam_cc_DLL}
		CC.DLLBIG = ${mam_cc_DLLBIG}
		CC.DYNAMIC =
		CC.HOSTTYPE = $(_hosttype_)
		CC.SHARED =
		CC.SHELLMAGIC = ${mam_cc_SHELLMAGIC}
		CC.STATIC =
		_hosttype_ = ${_hosttype_=`package`}
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
	.MAM.CCFLAGS := $(CCFLAGS:VP)
	print -um setv CCFLAGS $(CCFLAGS:VP:@Y&"$(CCFLAGS:VP)"&&)
	CCFLAGS = ${CCFLAGS}
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
	HOSTCC = ${HOSTCC}
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
	print -um setv MAKE nmake
	MAKE := $(-never:?${MAKE}?$$$(<:A=.ALWAYS:@Y%$(MAKE)%${MAKE}%)?)
	print -um setv MAKEFLAGS
	MAKEFLAGS = ${MAKEFLAGS}
	print -um setv PR pr
	PR = ${PR}
	print -um setv PRFLAGS
	PRFLAGS = ${PRFLAGS}
	print -um setv SHELL /bin/sh
	SHELL = ${SHELL}
	print -um setv SILENT
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
		$(MV) $(<) $(<).old 2>/dev/null || true; test '' = '$(*)' || { $(CP) $(*) $(<) $(.DO.INSTALL.OPT.) ;}
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
