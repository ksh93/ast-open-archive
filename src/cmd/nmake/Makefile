:PACKAGE: ast # + cpp 

LICENSE = since=1984,author=gsf

include "Version.mk"

ID = make

BINDINDEX ==
DEBUG == 1

RULES = Makerules.mk $(ALTRULES)
RULESFILES = Scanrules.mk S.mk SYSTEM.mk TABLE.mk \
		debug.mk mkservice.mk yacc.mk \
		pkg-tcl.mk pkg-X11.mk rules-win32.mk
RULESDIR = $(LIBDIR)/$(ID)

IDNAME == "$(ID)"
PATHCHECK == "$(ID)"
STAMP == "\n@(#)$(ID) ($(OWNER)) $(VERSION:?$$(VERSION) ??)$(RELEASE:?$$(RELEASE) ??)$(DATE)\0\n"

clobber = !(nmake)

nmake :: FEATURES HISTORY README RELEASE REFERENCES \
	PROMO.mm make-intro.0 nmake.1 nmake.8 Version.mk \
	$(RULES) $(RULES:B:S=.1:N=*rules.1:F=%(lower)S) $(RULESFILES) \
	make.h expand.h options.h \
	archive.c bind.c command.c dump.c expand.c main.c make.c \
	mam.c metarule.c misc.c object.c option.c parse.c read.c \
	rule.c scan.c state.c trap.c variable.c version.c -lcoshell

if CC.HOSTTYPE == "win32*" || "$(-mam)" == "static*"
expand.o object.o : .CC.NOOPTIMIZE
end

:READONLY: version.c

$(RULESDIR) :INSTALLDIR: Makeinstall.mk $(RULES) $(RULESFILES) ppcc

ppcc :: ppcc.sh

:INSTALLDIR: convert maketest

convert :: convert.c

maketest :: maketest.sh

C $(ID) :PROBE: make.probe probe.ini

probe.ini : (CC.HOSTTYPE) probe.win32
	t=$(CC.HOSTTYPE)
	ifs=$IFS
	IFS=.
	set x $t
	IFS=$ifs
	t=$2
	set x $(*)
	while	:
	do	shift
		case $# in
		0)	break ;;
		esac
		case $1 in
		*probe.$t)
			break
		esac
	done
	case $1 in
	?*)	cmp -s $1 $(<) || cp $1 $(<) ;;
	*)	: > $(<)
	esac

Makeinstall.mk : $$(-mam:N=static*:??$$(cctype:@P=P=C,pp)?)
	{
		echo '/* : : generated from $(cctype) assertions : : */'
		set -
		f=$(*|"`$(LIBDIR)/probe/probe -kg C pp $(cctype:Q)`")
		a="`sed -n -e 's/^#define #\(.*\)(\(.*\))/_\1_/p' $f | sort -u` _feature_"
		for n in $a
		do	eval $n=
		done
		eval `$(SED) -n -e 's/^#define #\(.*\)(\(.*\))/_\1_="$_\1_ \2"/p' $f`
		: some systems botch the symlink and sticky dir combo
		rm -rf /tmp/mkl$$
		mkdir /tmp/mkl$$
		text='hard link'
		echo "$text" > /tmp/mkl$$/nl
		( ln /tmp/mkl$$/nl /tmp/mkl$$/hl ) 2>/dev/null &&
		case `cat /tmp/mkl$$/hl` in
		"$text")	_feature_="$_feature_ ln" ;;
		esac
		ln -s /tmp/mkl$$/hl /tmp/mkl$$/sl 2>/dev/null && rm /tmp/mkl$$/sl && _feature_="$_feature_ ln-s"
		rm -rf /tmp/mkl$$
		set -x
		for n in $a
		do	eval echo \$n = \$$n
		done
	} > $(<)

local T

for T $(RULES:B)
	$(T:F=%(lower)S:D=$$(RULESDIR):B:S=.mo) :INSTALL: $(T:B:S=.mo)
end

.MAKEINIT : .TARGETS

.TARGETS : .MAKE
	$(RULESDIR)/makerules.mo :
