################################################################
#                                                              #
#           This software is part of the ast package           #
#              Copyright (c) 1990-2000 AT&T Corp.              #
#      and it may only be used by you under license from       #
#                     AT&T Corp. ("AT&T")                      #
#       A copy of the Source Code Agreement is available       #
#              at the AT&T Internet web site URL               #
#                                                              #
#     http://www.research.att.com/sw/license/ast-open.html     #
#                                                              #
#     If you received this software without first entering     #
#       into a license with AT&T, you have an infringing       #
#           copy and cannot use it without violating           #
#             AT&T's intellectual property rights.             #
#                                                              #
#               This software was created by the               #
#               Network Services Research Center               #
#                      AT&T Labs Research                      #
#                       Florham Park NJ                        #
#                                                              #
#             Glenn Fowler <gsf@research.att.com>              #
#              David Korn <dgk@research.att.com>               #
#               Eduardo Krell <ekrell@adexus.cl>               #
#                                                              #
################################################################
# NOTE: retain this comment
dir=$_ # NOTE: this line must appear before any commands
#
# 3d [options] [cmd ...]
#
# @(#)3d (AT&T Research) 1998-02-14
#
# start up a 3d shell -- shared library implementation
# vpath initialized from VPATH and path args
#
# options:
#
#	-c		passed to $SHELL
#	-d debug	set 3D debug trace level
#	-l		3D lite
#	-m mount	3D mount initialization
#	-n		show but don't execute
#	-o file		write debug trace to file
#	-s shell	set alternate ksh path
#	-t trace	set system call trace pid
#	-v version	select alternate 3D version
#	-x		passed to $SHELL
#
case $RANDOM in
$RANDOM)exec ksh 3d "$@" ;;
esac
test . -ef "$PWD" || PWD=`pwd`
case $dir in
*/3d)	;;
*)	case $0 in
	*/*)	dir=$0
		;;
	*)	dir=`whence 3d 2>/dev/null`
		dir=`whence -p $dir 2>/dev/null`
		;;
	esac
esac
case $dir in
/*)	dir=${dir%/*} ;;
*)	dir=${dir#./}; dir=$PWD/${dir%/*} ;;
esac
abi=
dir=${dir%/*}
case $dir in
*/arch/sgi.mips[0-9]*)
	if	test -d /lib32 -o -d /lib64
	then	d=${dir%-*}
		d=${d%?}
		for a in 2: 3:N32 4:64
		do	t=${a#*:}
			a=${a%:*}
			if	test -d $d$a
			then	abi="$abi $d$a:$t"
			fi
		done
	fi
	;;
esac
version=
mount="- -"
full="/dev/fdp/local/nam/user /#fs/nam/name/unique /dev/fdp /#nam /dev/tcp /#nam /dev/udp /#nam /dev/fd /#fd"
options=
output=
show=
shell=$SHELL
trace=
SHELL=
case $VPATH in
*:*)	ifs=$IFS
	IFS=:
	o=
	for a in $VPATH
	do	case $a in
		""|-)	;;
		*)	case $o in
			""|-)	;;
			*)	mount="$mount $o $a" ;;
			esac
			;;
		esac
		o=$a
	done
	IFS=$ifs
	;;
esac
while	:
do	case $# in
	0)	break ;;
	esac
	case $1 in
	--)	shift
		break
		;;
	[-+]?*)	x=${1#?}
		while	:
		do	case $x in
			"")	break ;;
			esac
			a=${x#?}
			o=${x%$a}
			case $o in
			l)	full=
				;;
			n)	show="print -u2 --"
				;;
			[dmostv])
				case $a in
				"")	case $# in
					0)	x='?'; continue ;;
					esac
					shift
					a=$1
					;;
				esac
				case $o in
				d)	trace="$trace/debug=$a" ;;
				m)	mount="$mount $a" ;;
				o)	output=$a ;;
				s)	SHELL=$a ;;
				t)	trace="$trace/trace=$a" ;;
				v)	case $a in
					/*)	version=$a ;;
					*)	version=.$a ;;
					esac
					;;
				esac
				break
				;;
			c)	break 2
				;;
			x)	options="$options -$o"
				;;
			?)	print -u2 "Usage: 3d [-clnx] [-d debug] [-m mount] [-o file] [-s shell] [-t trace] [-v version] [cmd ...]"
				exit 2
				;;
			esac
			x=$a
		done
		;;
	*)	break
		;;
	esac
	shift
done
case $full in
?*)	mount="$full $mount" ;;
esac
mount="- /#option/3d$trace $mount"
case $output in
?*)	mount="$output /#fs/option $mount" ;;
esac
default=$version
case $version in
/*)	;;
*)	for x in $dir/lib/lib3d.s[lo]$version
	do	case $x in
		*/lib3d.s[lo]*[a-zA-Z]*)
			;;
		*)	version=$x
			break
			;;
		esac
	done
	;;
esac
exp=
if	test -f "$version"
then	if	test -d $dir/3d
	then	dir=$dir/3d
	fi
	case :$FPATH: in
	*:$dir/fun:*)	;;
	*)		export FPATH=$dir/fun${FPATH:+:$FPATH} ;;
	esac
	case :$PATH: in
	*:$dir/bin:*)	;;
	*)		export PATH=$dir/bin:$PATH ;;
	esac
	exp="$exp LD_PRELOAD=$version"
	case $abi:-:$version in
	?*:-:$dir/*)
		v=${version#$dir/}
		for a in $abi
		do	d=${a%:*}
			a=${a#*:}
			exp="$exp _RLD${a}_LIST=$d/$v:DEFAULT"
		done
		;;
	*)	v=${version%.*}
		s=${version##*.}
		if	test -f ${v}-n32.$s
		then	exp="$exp _RLD64_LIST=$version:DEFAULT _RLDN32_LIST=$version-n32.$s:DEFAULT _RLD_LIST=${v}-o32.$s:DEFAULT"
		elif	test -f ${v}-64.$s -o -f ${v}-o32.$s
		then	exp="$exp _RLD64_LIST=$version-64.$s:DEFAULT _RLDN32_LIST=$version:DEFAULT _RLD_LIST=${v}-o32.$s:DEFAULT"
		else	exp="$exp _RLD_LIST=$version:DEFAULT"
		fi
		if	test -f ${dir}/lib/libdir64.$s
		then	exp="$exp:${dir}/lib/libdir64.$s"
		fi
		;;
	esac
	export $exp
	case $mount in
	??*)	export __=${mount#' '} ;;
	*)	unset __ ;;
	esac
	unset VPATH
else	print -u2 "$0: $version: no shared library"
	case $default in
	"*")	;;
	*)	exit 1 ;;
	esac
	version=
	options=
fi
case $SHELL in
"")	t=`whence ksh 2>/dev/null`
	SHELL=`PATH=$dir/bin:$PATH whence -p $t 2>/dev/null`
	;;
esac
SHELL=${SHELL:-$shell}
SHELL=${SHELL:-/bin/sh}
export SHELL
case `vpath /#option/3d 2>/dev/null` in
*/2d)	$show vpath $mount ;;
esac
case $1:$#:$options in
-*)	;;
*:0:*|*:*:?*)
	shell="$SHELL $options"
	;;
*)	t=`whence $1 2>/dev/null`
	t=`whence -p $t 2>/dev/null`
	case $t in
	""|"''"|'""')	shell="$SHELL $options" ;;
	*)		shell= ;;
	esac
	;;
esac
case "$shell : $1" in
"$SHELL : -c")
	shift
	$show eval "$@"
	;;
*)	case $show in
	"")	exec $shell "$@" ;;
	*)	case $version in
		"")	$show __="'$__'" exec $shell "$@" ;;
		*)	$show __="'$__'" $exp exec $shell "$@" ;;
		esac
		;;
	esac
	;;
esac
