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
: convert MAM dependency info to dot input

COMMAND=mamdot
case `(getopts '[-][123:xyz]' opt --xyz; echo 0$opt) 2>/dev/null` in
0123)	ARGV0="-a $COMMAND"
	USAGE=$'
[-?
@(#)$Id: mamdot (AT&T Labs Research) 1997-02-16 $
]
'$USAGE_LICENSE$'
[+NAME?mamdot - make abstract machine to dot dependency graph conversion filter]
[+DESCRIPTION?\bmamdot\b reads MAM (Make Abstract Machine) target and
	prerequisite file descriptions from the standard input and writes a
	\bdot\b(1) description of the dependency graph on the standard output.
	Mamfiles are generated by the \b--mam\b option of \bnmake\b(1) and
	\bgmake\b(1).]
[i:ignore?Dependency names matching the shell \apattern\a are
	ignored.]:[pattern]
[+SEE ALSO?\bdot\b(1), \bgmake\b(1), \bnmake\b(1)]
'
	;;
*)	ARGV0=""
	USAGE="i:[pattern]"
	;;
esac

usage()
{
	OPTIND=0
	getopts $ARGV0 "$USAGE" OPT '-?'
	exit 2
}

ignore=
while	getopts $ARGV0 "$USAGE" OPT
do	case $OPT in
	i)	ignore=$OPT
		;;
	*)	usage
		;;
	esac
done

integer level=0
list[0]=all
print "digraph mam {"
print "rankdir = LR"
print "node [ shape = box ]"
while read -r label op arg junk
do	[[ $arg == */* || $op != @(make|prev|done) ]] || {
		[[ $op == make ]] && print "\"$label::$arg\" [ label = \"$arg\" ]"
		arg=$label::$arg
	}
	case $op in
	make)	[[ $arg == $ignore ]] || list[level]=${list[level]}$'\n'\"$arg\"
		level=level+1
		list[level]=
		;;
	prev)	[[ $arg == $ignore ]] || list[level]=${list[level]}$'\n'\"$arg\"
		;;
	done)	[[ ${list[level]} ]] && [[ $arg != $ignore ]] &&
			print "\"$arg\" -> {${list[level]} }"
		level=level-1
		;;
	esac
done
print "}"
