####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1999-2001 AT&T Corp.                #
#        and it may only be used by you under license from         #
#                       AT&T Corp. ("AT&T")                        #
#         A copy of the Source Code Agreement is available         #
#                at the AT&T Internet web site URL                 #
#                                                                  #
#       http://www.research.att.com/sw/license/ast-open.html       #
#                                                                  #
#        If you have copied this software without agreeing         #
#        to the terms of the license you are infringing on         #
#           the license and copyright and are violating            #
#               AT&T's intellectual property rights.               #
#                                                                  #
#                 This software was created by the                 #
#                 Network Services Research Center                 #
#                        AT&T Labs Research                        #
#                         Florham Park NJ                          #
#                                                                  #
#               Glenn Fowler <gsf@research.att.com>                #
####################################################################
# mamprobe - generate MAM cc probe info
# this script written to make it through all sh variants
# Glenn Fowler <gsf@research.att.com>

case $-:$BASH_VERSION in
*x*:[0123456789]*)	: bash set -x is broken :; set +ex ;;
esac

command=mamprobe

# check the options

case `(getopts '[-][123:xyz]' opt --xyz; echo 0$opt) 2>/dev/null` in
0123)	USAGE=$'
[-?
@(#)$Id: mamprobe (AT&T Labs Research) 2001-10-31 $
]
[+NAME?mamprobe - generate MAM cc probe info]
[+DESCRIPTION?\bmamprobe\b generates MAM (make abstract machine) \bcc\b(1)
	probe information for use by \bmamake\b(1). The probed compiler is
	\acc-path\a and the information is placed in \ainfo-path\a.
	\ainfo-path\a is usually \b$INSTALLROOT/lib/probe/C/mam/\b\ahash\a,
	where \ahash\a is a hash of \acc-path\a.]
[+?\bmamprobe\b and \bmamake\b are used in the bootstrap phase of
	\bpackage\b(1) installation before \bnmake\b(1) is built. The
	probed variables are:]{
		[+mam_cc_DLL?compile for a small dll]
		[+mam_cc_DLLBIG?compile for a large dll]
		[+mam_cc_L?\b-L\b\adir\a supported]
		[+mam_cc_SHELLMAGIC?a magic line to be placed at the top
			of installed shell scripts]
}

info-path cc-path

[+SEE ALSO?\bpackage\b(1), \bmamake\b(1), \bnmake\b(1), \bprobe\b(1)]
'
	while	getopts -a "$command" "$USAGE" OPT
	do	:
	done
	shift `expr $OPTIND - 1`
	;;
*)	while	:
	do	case $# in
		0)	break ;;
		esac
		case $1 in
		--)	shift
			break
			;;
		-*)	echo $command: $1: unknown option >&2
			;;
		*)	break
			;;
		esac
		echo "Usage: $command info-path cc-path" >&2
		exit 2
	done
	;;
esac

# check the args

case $# in
0|1)	echo "Usage: $command info-path cc-path" >&2; exit 2 ;;
esac
case $1 in
/*)	;;
*)	echo "$command: $1: info-path must be absolute" >&2; exit 1 ;;
esac
info=$1
shift
cc=$*
src=.c
obj=.o
exe=.exe

# create the info dir if necessary

i=X$info
ifs=$IFS
IFS=/
set $i
IFS=$ifs
while	:
do	i=$1
	shift
	case $i in
	X)	break ;;
	esac
done
case $info in
//*)	path=/ ;;
*)	path= ;;
esac
while	:
do	case $# in
	0|1)	break ;;
	esac
	comp=$1
	shift
	case $comp in
	'')	continue ;;
	esac
	path=$path/$comp
	if	test ! -d $path
	then	mkdir $path || exit
	fi
done

# generate info in a tmp file and rename when finished

tmp=/tmp/mam$$
trap "cd /; rm -rf $tmp" 0 1 2 3 15
mkdir $tmp || exit
cd $tmp || exit
exec < /dev/null > info 2>/dev/null

echo "note generated by $0 for $cc"

# nmake _hosttype_

_hosttype_=`package || $SHELL -c package` # unixware.i386 ksh needs the alt!
echo "setv _hosttype_ $_hosttype_"

# mam_cc_DLL is the cc dll compilation option

case $_hosttype_ in
win32.*|cygwin.*|os2.*)
	echo "setv mam_cc_DLL -D_BLD_DLL"
	echo "setv mam_cc_DLLBIG -D_BLD_DLL"
	;;
*)	dll=
	echo '#include <stdio.h>
extern int i;
int i = 1;
extern int f(){return(i);} int main(){FILE* fp=stdin;return(f());}' > dll$src
	if	$cc -c dll$src 2>e
	then	e=`wc -l e`
		s=`wc dll$obj`
		for opt in -Kpic -KPIC -fpic -pic +z '-G 0' -D_DLL_BLD -Wc,exportall
		do	rm -f dll$obj dll$exe
			$cc $opt -c dll$src 2>e && test -f dll$obj || continue
			$cc $opt -o dll$exe dll$obj && test -f dll$exe || {
				rm -f dll$exe
				$cc -o dll$exe dll$obj && test -f dll$exe && continue
			}
			case $opt in
			*-D[abcdefghijklmnopqrstuvwxyz]*=*)
				dll=$opt
				break
				;;
			-Wc,exportall)
				# get specific since sgi gets this far too
				rm -f dll$exe dll.x
				$cc -Wl,dll -o dll$exe dll$obj || continue
				test -f dll$exe || continue
				test -f dll.x || continue
				dll="$opt -D_SHARE_EXT_VARS"
				so=.x
				sd=.dll
				shared=-Wl,dll
				probe_sd=
				probe_shared=
				lddynamic=-Bdynamic
				ldstatic=-Bstatic
				break
				;;
			esac
			case `wc -l e` in
			$e)	;;
			*)	continue ;;
			esac
			case $opt in
			???*)	ns=`wc dll$obj`
				case $ns in
				$s)	continue ;;
				esac
				m=`echo " $opt" | sed -e 's/^ [-+]//g' -e 's/./-& /g' -e 's/[-+] //g'`
				rm -f dll$obj
				if	$cc $m -c dll$src 2>e && test -f dll$obj
				then	case `wc dll$obj` in
					$ns)	continue ;;
					esac
				fi
				dll=$opt
				break
				;;
			*)	case `wc dll$obj` in
				$s)	;;
				*)	dll=$opt
					break
					;;
				esac
				;;
			esac
		done
		case $dll in
		?*)	echo "setv mam_cc_DLL $dll -D_BLD_DLL"
			DLL=$dll
			set x x -Kpic -KPIC -fpic -fPIC -pic -PIC +z +Z
			while	:
			do	shift
				shift
				case $# in
				0|1)	break ;;
				esac
				case $dll in
				$1)	rm -f dll$obj dll$exe
					$cc $2 -c dll$src 2>e && test -f dll$obj || break
					$cc $2 -o dll$exe dll$obj && test -f dll$exe || {
						rm -f dll$exe
						$cc -o dll$exe dll$obj && test -f dll$exe && break
					}
					DLL=$2
					break
					;;
				esac
			done
			echo "setv mam_cc_DLLBIG $DLL -D_BLD_DLL"
			;;
		esac
	fi
	;;
esac

# mam_cc_L defined if cc -Ldir works

case $_hosttype_ in
win32.*|cygwin.*|os2.*)
	echo "setv mam_cc_L 1"
	;;
*)	echo 'main(){return 0;}' > main$src
	if	$cc -c main$src >/dev/null
	then	if	$cc -L. main$obj -lc >/dev/null
		then	case $dll in
			'')	;;
			*)	echo "setv mam_cc_L 1" ;;
			esac
		fi
	fi
	;;
esac

# mam_cc_SHELLMAGIC defined if installed shell scripts need magic

echo 'exec 4<&0
echo ok' > ok
chmod +x ok
case `(eval ./ok | /bin/sh) 2>/dev/null` in
ok)	;;
*)	echo '#!/bin/env sh
exec 4<&0
echo ok' > ok
	chmod +x ok
	case `(eval ./ok | /bin/sh) 2>/dev/null` in
	ok)	echo 'setv mam_cc_SHELLMAGIC #!/bin/env sh'
		;;
	*)	for i in /emx/bin/bash.exe /emx/bin/sh.exe
		do	if	test -x $i
			then	echo 'setv mam_cc_SHELLMAGIC #!'$i
				break
			fi
		done
		;;
	esac
	;;
esac

# all done

cp info $info
