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
:
# convert MAM dependency info to dot input
integer level=0
list[0]=all
ignore=""
[[ $1 == -i* ]] && {
	ignore=${1#-i}
	shift
	[[ $ignore == "" ]] && {
		[[ $# == 0 ]] && {
			print -u2 "Usage: mamdot [-i ignore-pattern] < mamfile"
			exit 1
		}
		ignore=$1
		shift
	}
}
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
