########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1982-2013 AT&T Intellectual Property          #
#                      and is licensed under the                       #
#                 Eclipse Public License, Version 1.0                  #
#                    by AT&T Intellectual Property                     #
#                                                                      #
#                A copy of the License is available at                 #
#          http://www.eclipse.org/org/documents/epl-v10.html           #
#         (with md5 checksum b35adb5213ca9657e911e9befb180842)         #
#                                                                      #
#              Information and Software Systems Research               #
#                            AT&T Research                             #
#                           Florham Park NJ                            #
#                                                                      #
#                  David Korn <dgk@research.att.com>                   #
#                                                                      #
########################################################################
#
# tests for \u[...] and \w[...] input and output
#

function err_exit
{
	print -u2 -n "\t"
	print -u2 -r ${Command}[$1]: "${@:2}"
	(( Errors++ ))
}
alias err_exit='err_exit $LINENO'

Command=${0##*/}
integer Errors=0

locales="en_US.UTF-8 en_US.ISO-8859-15 zh_CN.GB18030"
supported="C.UTF-8"

for lc_all in $locales
do	if	{
			PATH=/bin:/usr/bin:$PATH locale -a | grep -w ${lc_all%.*} &&
			LC_ALL=$lc_all PATH=/bin:/usr/bin:$PATH iconv -f ${lc_all#*.} -t UTF-8 </dev/null
		} >/dev/null 2>&1
	then	supported+=" $lc_all"
	else	: "LC_ALL=$lc_all not supported" :
	fi
done

exp0=$'0000000 24 27 e2 82 ac 27 0a'
exp2=$'\'\\u[20ac]\''
exp1='$'$exp2

for lc_all in $supported
do

got=$(LC_OPTIONS=nounicode $SHELL -c 'export LC_ALL='${lc_all}'; printf "%q\n" "$(printf "\u[20ac]")"' | iconv -f ${lc_all#*.} -t UTF-8 | od -tx1 | head -1)
[[ $got == "$exp0" ]] || err_exit "${lc_all} nounicode FAILED -- locale probably not supported -- expected '$exp0', got '$got'"

got=$(LC_OPTIONS=unicode $SHELL -c 'export LC_ALL='${lc_all}'; printf "%(nounicode)q\n" "$(printf "\u[20ac]")"' | iconv -f ${lc_all#*.} -t UTF-8 | od -tx1 | head -1)
[[ $got == "$exp0" ]] || err_exit "${lc_all} (nounicode) FAILED -- locale probably not supported -- expected '$exp0', got '$got'"

got=$(LC_OPTIONS=unicode $SHELL -c 'export LC_ALL='${lc_all}'; printf "%q\n" "$(printf "\u[20ac]")"')
[[ $got == "$exp1" || $got == "$exp2" ]] || err_exit "${lc_all} unicode FAILED -- expected $exp1, got $got"

got=$(LC_OPTIONS=nounicode $SHELL -c 'export LC_ALL='${lc_all}'; printf "%(unicode)q\n" "$(printf "\u[20ac]")"')
[[ $got == "$exp1" || $got == "$exp2" ]] || err_exit "${lc_all} (unicode) FAILED -- expected $exp1, got $got"

done

exit $((Errors<125?Errors:125))