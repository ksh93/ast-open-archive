####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1987-2001 AT&T Corp.                #
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
:
# @(#)tar.sh (AT&T Labs Research) 1989-04-06
#
# tar -> pax interface script
#

command=tar
usage="
Usage: $command c[vwfbB[[0-9][hlm]]] [pathname ...]
       $command r[vwfbB[[0-9][hlm]]] [files ...]
       $command t[vwfbB[[0-9][hlm]]
       $command u[vwfbB[[0-9][hlm]]] [pathname ...]
       $command x[vwfblmpB[[0-9][hlm]]] [pathname ...]"

case $1 in
*[tx]*)		mode="-r" ;;
*[cru]*)	mode="-w" ;;
*)		print -u2 "$command: one of crtux must be specified$usage"; exit 1 ;;
esac
options="-P"
exec=eval
file="-o tape=0"
list=""
r_ok="1"
w_ok="1"
arg=$1
lastopt=""
shift
for opt in `echo '' $arg | sed -e 's/^ -//' -e 's/./& /g'`
do	case $opt in
	[0-9])	file="-o tape=$opt" ;;
	[hlm])	case $lastopt in
		[0-9])	file="${file}$opt" ;;
		*)	case $opt in
			h)	options="$options -L" ;;
			l)	;;
			m)	r_ok="" options="$options -$opt" ;;
			esac
			;;
		esac
		;;
	[v])	options="$options -$opt" ;;
	b)	case $# in
		0)	print -u2 "$command: blocking factor argument expected$usage"; exit 1 ;;
		esac
		options="$options -b ${1}b"
		shift
		;;
	c)	r_ok="" ;;
	f)	case $# in
		0)	print -u2 "$command: file name argument expected$usage"; exit 1 ;;
		esac
		case $1 in
		-)	file="" ;;
		*)	file="-f '$1'" ;;
		esac
		shift
		;;
	p)	options="$options -pe" ;;
	r)	r_ok="" options="$options -a" ;;
	t)	w_ok="" list="1" ;;
	u)	r_ok="" options="$options -u" ;;
	w)	options="$options -o yes" ;;
	x)	w_ok="" ;;
	B)	options="$options -b 10k" ;;
	D)	case $exec in
		eval)	exec=print ;;
		*)	exec="eval args" ;;
		esac
		;;
	*)	print -u2 "$command: invalid option -$opt$usage"; exit 1 ;;
	esac
	lastopt=$opt
done
case $mode in
-r)	case $r_ok in
	"")	print -u2 "$command: options inconsistent with archive read"; exit 1 ;;
	esac
	;;
-w)	case $w_ok in
	"")	print -u2 "$command: options inconsistent with archive write"; exit 1 ;;
	esac
	case $# in
	0)	set - "." ;;
	esac
	options="$options -x ustar"
	;;
esac
case $list in
"1")	mode="" ;;
esac
case $exec in
eval)	$exec pax $mode $options $file '"$@"' ;;
*)	$exec pax $mode $options $file "$@" ;;
esac
