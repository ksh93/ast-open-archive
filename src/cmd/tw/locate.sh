####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1989-2004 AT&T Corp.                #
#        and it may only be used by you under license from         #
#                       AT&T Corp. ("AT&T")                        #
#         A copy of the Source Code Agreement is available         #
#                at the AT&T Internet web site URL                 #
#                                                                  #
#       http://www.research.att.com/sw/license/ast-open.html       #
#                                                                  #
#    If you have copied or used this software without agreeing     #
#        to the terms of the license you are infringing on         #
#           the license and copyright and are violating            #
#               AT&T's intellectual property rights.               #
#                                                                  #
#            Information and Software Systems Research             #
#                        AT&T Labs Research                        #
#                         Florham Park NJ                          #
#                                                                  #
#               Glenn Fowler <gsf@research.att.com>                #
#                                                                  #
####################################################################
: gnu locate wrapper for tw

COMMAND=locate
case `(getopts '[-][123:xyz]' opt --xyz; echo 0$opt) 2>/dev/null` in
0123)	ARGV0="-a $COMMAND"
	USAGE=$'
[-?
@(#)$Id: locate (AT&T Labs Research) 1999-01-23 $
]
'$USAGE_LICENSE$'
[+NAME?locate - locate files in pathname database]
[+DESCRIPTION?\blocate\b matches file patterns in a pathname database
	that is updated daily by \bupdatedb\b(1).]
[d:database?File database path.]:[path]
[i:ignorecase?Ignore case in all pattern match expressions.]
[n:show?Show underlying \btw\b(1) command but do not execute.]

pattern ...

[+OPERANDS]{
	[+pattern?One or more file patterns.
		/ is matched by pattern metacharacters.]
}
[+ENVIRONMENT]{
	[+FINDCODES?Path name of locate database.]
	[+LOCATE_PATH?Alternate path name of locate database.]
}
[+FILES]{
	[+lib/find/codes?Default locate database.]
}
[+SEE ALSO?\bupdatedb\b(1), \btw\b(1)]
'
	;;
*)	ARGV0=""
	USAGE="nd:[path] pattern ..."
	;;
esac

usage()
{
	OPTIND=0
	getopts $ARGV0 "$USAGE" OPT '-?'
	exit 2
}

db=
opts=
show=
while	getopts $ARGV0 "$USAGE" OPT
do	case $OPT in
	d)	db="$db:$OPTARG"
		;;
	i)	opts="$opts -I"
		;;
	n)	show="print --"
		;;
	*)	usage
		;;
	esac
done
case $db in
'')	db=${LOCATE_PATH} ;;
esac
case $db in
?*)	db="-F${db#:}" ;;
esac
shift $OPTIND-1
case $# in
0)	usage
	;;
1)	;;
*)	pat=$1
	while	:
	do	shift
		case $# in
		0)	break ;;
		esac
		pat="$pat|$1"
	done
	set -- "$pat"
	;;
esac
$show tw -d / $opts $db -f "$@"
