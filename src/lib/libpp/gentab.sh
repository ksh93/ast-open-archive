####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1986-2002 AT&T Corp.                #
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
# Glenn Fowler
# AT&T Bell Laboratories
#
# @(#)gentab (gsf@research.att.com) 07/17/94
#
# C table generator
#
#	%flags [ prefix=<prefix> ] [ index=<index> ] [ init=<init> ]
#
#	%keyword <name> [ prefix=<prefix> ] [ index=<index> ] [ init=<init> ]
#
#	%sequence [ prefix=<prefix> ] [ index=<index> ] [ init=<init> ]
#

case `(typeset -u s=a n=0; ((n=n+1)); print $s$n) 2>/dev/null` in
A1)	shell=ksh
	typeset -u ID
	typeset -i counter err_line
	;;
*)	shell=bsh
	;;
esac
command=$0
counter=0
define=1
err_line=0
type=""
index=""
table=1
while	:
do	case $1 in
	-d)	table=0 ;;
	-t)	define=0 ;;
	*)	break ;;
	esac
	shift
done
case $1 in
"")	err_file=""
	;;
*)	exec <$1
	err_file="\"$1\", "
	;;
esac
while	read line
do	case $shell in
	ksh)	((err_line=err_line+1)) ;;
	*)	err_line=`expr $err_line + 1` ;;
	esac
	set '' $line
	shift
	case $1 in
	[#]*)	echo "/*"
		while	:
		do	case $1 in
			[#]*)	shift
				echo " * $*"
				read line
				set '' $line
				shift
				;;
			*)	break
				;;
			esac
		done
		echo " */"
		echo
		;;
	esac
	eval set '""' $line
	shift
	case $1 in
	"")	;;
	%flags|%keywords|%sequence)
		case $type in
		%flags|%sequence)
			if	test $define = 1
			then	echo
			fi
			;;
		%keywords)
			if	test $table = 1
			then	echo "	0,	0"
				echo "};"
				echo
			elif	test $define = 1
			then	echo
			fi
			;;
		esac
		case $index in
		?*)	eval $index=$counter ;;
		esac
		type=$1
		shift
		name=""
		prefix=""
		index=""
		init=""
		case $type in
		%keywords)
			case $1 in
			"")	echo "$command: ${err_file}line $err_line: $type table name omitted" >&2
				exit 1
				;;
			esac
			name=$1
			shift
			if	test $table = 1
			then	echo "$name"'[] ='
				echo "{"
			fi
			;;
		esac
		eval "$@"
		case $init in
		"")	case $type in
			%flags|%sequence)
				init=0
				;;
			*)	init=1
				;;
			esac
			;;
		esac
		case $index in
		"")	counter=$init
			;;
		*)	eval value=\$$index
			case $value in
			"")		counter=$init ;;
			[0123456789]*)	counter=$value ;;
			esac
			;;
		esac
		;;
	%*)	echo "$command: ${err_file}line $err_line: $1: unknown keyword" >&2
		exit 1
		;;
	*)	while	:
		do	case $1 in
			"")	break
				;;
			*)	case $shell in
				ksh)	ID=${1#[!abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_]} ;;
				*)	ID=`echo $1 | tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ | sed 's/^[^ABCDEFGHIJKLMNOPQRSTUVWXYZ_]//'` ;;
				esac
				case $type in
				%flags)	if	test $define = 1
					then	case $counter in
						32) echo "$command: ${err_file}line $err_line: warning: $1: too many flag bits" >&2 ;;
						1[56789]|[23][0123456789]) long=L ;;
						*) long= ;;
						esac
						echo "#define $prefix$ID	(1$long<<$counter)"
					fi
					;;
				%keywords)
					if	test $define = 1
					then	echo "#define $prefix$ID	$counter"
					fi
					if	test $table = 1
					then	echo "	\"$1\",	$prefix$ID,"
					fi
					;;
				%sequence)
					if	test $define = 1
					then	echo "#define $prefix$ID	$counter"
					fi
					;;
				esac
				case $shell in
				ksh)	((counter=counter+1)) ;;
				*)	counter=`expr $counter + 1` ;;
				esac
				shift
				;;
			esac
		done
		;;
	esac
done
case $type in
%keywords)
	if	test $table = 1
	then	echo "	0,	0"
		echo "};"
	fi
	;;
esac
exit 0
