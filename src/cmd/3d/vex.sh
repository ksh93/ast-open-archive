########################################################################
#                                                                      #
#               This software is part of the ast package               #
#                  Copyright (c) 1989-2005 AT&T Corp.                  #
#                      and is licensed under the                       #
#                  Common Public License, Version 1.0                  #
#                            by AT&T Corp.                             #
#                                                                      #
#                A copy of the License is available at                 #
#            http://www.opensource.org/licenses/cpl1.0.txt             #
#         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         #
#                                                                      #
#              Information and Software Systems Research               #
#                            AT&T Research                             #
#                           Florham Park NJ                            #
#                                                                      #
#                 Glenn Fowler <gsf@research.att.com>                  #
#                  David Korn <dgk@research.att.com>                   #
#                   Eduardo Krell <ekrell@adexus.cl>                   #
#                                                                      #
########################################################################
: vex skip expand command ...
# vpath expand args matching pattern
# @(#)vex (AT&T Bell Laboratories) 04/01/93
case $# in
[012])	print -u2 "Usage: $0 skip expand command ..."; exit 1 ;;
esac
skip=$1
shift
expand=$1
shift
command=$1
shift
integer argc=0 noexpand=0
for arg
do	if	((noexpand))
	then	noexpand=0
	else	case $arg in
		-[$skip])
			noexpand=1
			;;
		-*)	;;
		$expand)
			x=$(vpath "$arg" 2>/dev/null)
			case $x in
			?*)	arg=${x#$PWD/} ;;
			esac
			;;
		esac
	fi
	argv[argc]=$arg
	((argc+=1))
done
"$command" "${argv[@]}"
