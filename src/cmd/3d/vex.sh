####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1989-2000 AT&T Corp.                #
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
#                David Korn <dgk@research.att.com>                 #
#                 Eduardo Krell <ekrell@adexus.cl>                 #
#                                                                  #
####################################################################
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
