:PACKAGE: ast cpp:force

if CC.HOSTTYPE == "linux.alpha"
	/* workaround for "BUG IN DYNAMIC LINKER ld.so" */
	:PACKAGE: :static
	CCLDFLAGS += $(CC.STATIC)
end

LICENSE = since=1984,author=gsf

include "Version.mk"

ID = make

BINDINDEX ==
DEBUG == 1

RULES = Makerules.mk $(ALTRULES)
RULESFILES = Scanrules.mk S.mk SYSTEM.mk TABLE.mk \
		debug.mk mkservice.mk yacc.mk \
		rules-win32.mk \
		pkg-tcl.mk pkg-X11.mk pkg-freetype.mk
RULESDIR = $(LIBDIR)/$(ID)

IDNAME == "$(ID)"
PATHCHECK == "$(ID)"
STAMP == "\n@(#)\$Id: $(ID) ($(OWNER)) $(VERSION:?$$(VERSION) ??)$(RELEASE:?$$(RELEASE) ??)$(DATE) \$\0\n"

clobber = !(nmake)

nmake :: make-intro.0 nmake.1 nmake.8 Version.mk \
	$(RULES) $(RULES:B:S=.1:N=*rules.1:F=%(lower)s) $(RULESFILES) \
	make.h expand.h options.h \
	archive.c bind.c command.c dump.c expand.c main.c make.c \
	mam.c metarule.c misc.c object.c option.c parse.c read.c \
	rule.c scan.c state.c trap.c variable.c version.c \
	-lcoshell -lardir

"win32*" :NOOPTIMIZE: expand.c object.c parse.c

$(RULESDIR) :INSTALLDIR: Makeinstall.mk $(RULES) $(RULESFILES) ppcc

:: nmake.mm PROMO.mm FEATURES HISTORY README RELEASE REFERENCES

ppcc :: ppcc.sh

:: SCCS.mk

:INSTALLDIR: convert

convert :: convert.c

.SOURCE : tests

:TEST: -- assert assign attribute automatic baserules common diagnostics \
	edit immediate implicit internal interrupt metarule option panic \
	prereq quoting scan shell state statement syntax view

Makeinstall.mk : $$(-mam:N=static*:??$$(cctype:@P=P=C,pp)?)
	{
		echo '/* : : generated from $(cctype) assertions : : */'
		set -
		f=$(*|"`$(LIBDIR)/probe/probe -kg C pp $(cctype:Q)`")
		a="`$(SED) -n -e 's/^#define #\(.*\)(\(.*\))/_\1_/p' $f | sort -u` _feature_ STDCAT STDCHMOD STDCMP STDCP STDED STDEDFLAGS STDLN STDMV STDRM"
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
		( /bin/mkdir -p /tmp/mkl$$/mk/dir ) 2>/dev/null &&
		test ! -d ./-p &&
		_feature_="$_feature_ mkdir-p"
		rm -rf ./-p /tmp/mkl$$ || true
		if	( ed ) < /dev/null > /dev/null 2>&1
		then	STDED=ed
		else	STDED=ex
		fi
		STDEDFLAGS=-
		set STDCAT cat STDCHMOD chmod STDCMP cmp STDCP cp STDLN ln STDMV mv STDRM rm
		while	:
		do	case $# in
			0|1)	break ;;
			esac
			p=$2
			for d in /bin /usr/bin /usr/sbin
			do	if	test -x $d/$p
				then	p=$d/$p
					break
				fi
			done
			eval $1=\$p
			shift
			shift
		done
		if	test -x /bin/cat.exe
		then	if	/bin//cat /bin/cat >/dev/null 2>&1
			then	: ok
			else	: .exe challenged
				for n in STDCAT STDCHMOD STDCMP STDCP STDLN STDMV STDRM
				do	eval $n=\"execrate \$$n\"
				done
			fi
		fi
		set -x
		for n in $a
		do	eval echo \$n = \$$n
		done
	} > $(<)

local T

for T $(RULES:B)
	$(T:F=%(lower)s:D=$$(RULESDIR):B:S=.mo) :INSTALL: $(T:B:S=.mo)
end

/*
 * this works around a master source race condition
 */

.MAKEINIT : .TARGETS

.TARGETS : .MAKE
	$(RULESDIR)/makerules.mo :
