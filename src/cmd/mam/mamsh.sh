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
#                          AT&T Research                           #
#                         Florham Park NJ                          #
#                                                                  #
#               Glenn Fowler <gsf@research.att.com>                #
#                                                                  #
####################################################################
: mam to sh script filter

COMMAND=mamsh
case `(getopts '[-][123:xyz]' opt --xyz; echo 0$opt) 2>/dev/null` in
0123)	ARGV0="-a $COMMAND"
	USAGE=$'
[-?
@(#)$Id: mamsh (AT&T Labs Research) 1994-01-01 $
]
'$USAGE_LICENSE$'
[+NAME?mamsh - make abstract machine to shell script conversion filter]
[+DESCRIPTION?\bmamsh\b converts \aMAM\a (Make Abstract Machine)
	instructions on the standard input to a \bsh\b(1) script
	that updates all targets on the standard output. The script
	summarily builds each target; use \bmamake\b(1), \bnmake\b(1)
	or \bgmake\b(1) for target out-of-date testing.]
[+SEE ALSO?\bmamake\b(1), \bnmake\b(1)]
'
	;;
*)	ARGV0=""
	USAGE=""
	;;
esac

usage()
{
	OPTIND=0
	getopts $ARGV0 "$USAGE" OPT '-?'
	exit 2
}

while	getopts $ARGV0 "$USAGE" OPT
do	case $OPT in
	*)	usage ;;
	esac
done

echo ": : generated from makefile abstraction : :"
echo 'eval ${1+"$@"}'
sed -n -e 's/^[0-9][0-9 	]*//' -e 's/^setv \([^ ]*\) \(.*\)/\1=${\1-\2}/p' -e 's/^exec [^ ]* //p' ${1+"$@"}
