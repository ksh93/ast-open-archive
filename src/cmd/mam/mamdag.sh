####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1989-2002 AT&T Corp.                #
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
#            Information and Software Systems Research             #
#                        AT&T Labs Research                        #
#                         Florham Park NJ                          #
#                                                                  #
#               Glenn Fowler <gsf@research.att.com>                #
####################################################################
:

# convert MAM dependency info to dag input
#
# @(#)mamdag (AT&T Labs Research) 1998-04-01

integer level=0 line=0
list[0]=all
print .GR 7.50 10.0
print draw nodes as Box ';'
while	read op arg arg2 argx
do	line=line+1
	case $op in
	[0-9]*)	op=$arg
		arg=$arg2
		arg2=$arg3
		arg3=$argx
		argx=
		;;
	esac
	case $op in
	make)	case " ${list[level]} " in
		*" \"$arg\" "*)	;;
		*)		list[level]="${list[level]} \"$arg\"" ;;
		esac
		level=level+1
		list[level]=\"$arg\"
		;;
	prev)	case " ${list[level]} " in
		*" \"$arg\" "*)	;;
		*)		list[level]="${list[level]} \"$arg\"" ;;
		esac
		;;
	done)	case ${list[level]} in
		*' '*)	print ${list[level]} ';'
		esac
		if	(( level <= 0 ))
		then	print -u2 "$0: line $line: $op $arg: no matching make op"
		else	level=level-1
		fi
		;;
	esac
done
print .GE
