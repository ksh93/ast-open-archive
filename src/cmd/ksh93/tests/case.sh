####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1982-2001 AT&T Corp.                #
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
#                David Korn <dgk@research.att.com>                 #
####################################################################
function err_exit
{
	print -u2 -n "\t"
	print -u2 -r $Command: "$@"
	let Errors+=1
}

Command=$0
integer Errors=0

bar=foo2
bam=foo[3]
for i in foo1 foo2 foo3 foo4 foo5 foo6
do	foo=0
	case $i in
	foo1)	foo=1;;
	$bar)	foo=2;;
	$bam)	foo=3;;
	foo[4])	foo=4;;
	${bar%?}5)
		foo=5;;
	"${bar%?}6")
		foo=6;;
	esac
	if	[[ $i != foo$foo ]]
	then	err_exit "$i not matching correct pattern"
	fi
done
f="[ksh92]"
case $f in
\[*\])  ;;
*)      err_exit "$f does not match \[*\]";;
esac
exit $((Errors))
