################################################################
#                                                              #
#           This software is part of the ast package           #
#                Copyright (c) 2000 AT&T Corp.                 #
#      and it may only be used by you under license from       #
#                     AT&T Corp. ("AT&T")                      #
#       A copy of the Source Code Agreement is available       #
#              at the AT&T Internet web site URL               #
#                                                              #
#     http://www.research.att.com/sw/license/ast-open.html     #
#                                                              #
#      If you have copied this software without agreeing       #
#      to the terms of the license you are infringing on       #
#         the license and copyright and are violating          #
#             AT&T's intellectual property rights.             #
#                                                              #
#               This software was created by the               #
#               Network Services Research Center               #
#                      AT&T Labs Research                      #
#                       Florham Park NJ                        #
#                                                              #
#             Glenn Fowler <gsf@research.att.com>              #
#                                                              #
################################################################
: message catalog administration

command=msgadmin

case `getopts '[-][123:xyz]' opt --xyz 2>/dev/null; echo 0$opt` in
0123)	ARGV0="-a $command"
	USAGE=$'
[-?
@(#)msgadmin (AT&T Labs Research) 2000-04-26
]
'$USAGE_LICENSE$'
[+NAME?'$command$' - message catalog file administration]
[+DESCRIPTION?\b'$command$'\b administers message catalog files. If no \afile\a
	operands are specified then all message files in the local
	\b$INSTALLROOT\b source tree are operated on. Exaclty one of
	\b--generate\b, \b--remove\b, \b--translate\b, or \b--verify\b
	must be specified.]
[D:debug?Passed to \btranslate\b(1).]
[c:cache?Passed to \btranslate\b(1).]
[d:dialect?Operate on the dialects in the \b,\b separated \adialect\a list.
	\b-\b means all dialects supported by \btranslate\b(1).]:[dialect:=-]
[g:generate?Generate and install \bgencat\b(1) binary message catalogs.]
[n:show?Show commands but do not execute.]
[o:omit?Omit \btranslate\b(1) methods matching the \bksh\b(1)
	\apattern\a.]:[pattern]
[r:remove?Remove all translated message files and work directories.]
[s:share?Generate and install \bmsggen\b(1) machine independent binary
	message catalogs.]
[t:translate?Translate using \btranslate\b(1).]
[v:verify?Verify that translated message files satisfy \bgencat\b(1) syntax.]

[ file ... ]

[+SEE ALSO?\bgencat\b(1), \bksh\b(1), \bmsggen\b(1), \btranslate\b(1)]
'
	;;
*)	ARGV0=""
	USAGE="Dcd:gno:rstv [ file ... ]"
	;;
esac

usage()
{
	OPTIND=0
	getopts $ARGV0 "$USAGE" OPT '-?'
	exit 2
}

messages()
{
	if	[[ $PACKAGEROOT && -d $PACKAGEROOT ]]
	then	MSGROOT=$PACKAGEROOT
	else	MSGROOT=$HOME
	fi
	set -- $MSGROOT/arch/*/src/cmd/INIT/INIT.msg
	[[ -f $1 ]] || { print -u2 $"$command: INIT.msg: not found"; exit 1; }
	MSGROOT=${1%/src/cmd/INIT/INIT.msg}
	for i in $MSGROOT/src/@(cmd|lib)/*/*.msg
	do	grep -q '^1 ' $i && print -r $i
	done
}

integer n
typeset cache dialect=- exec force omit op show verbose
typeset dir=$INSTALLROOT gen=gencat

while	getopts $ARGV0 "$USAGE" OPT
do	case $OPT in
	D)	debug=-D ;;
	c)	cache=-c ;;
	d)	dialect=$OPTARG ;;
	f)	force=1 ;;
	g)	op=generate ;;
	n)	exec=print show=-n ;;
	o)	omit="-o $OPTARG" ;;
	r)	op=remove ;;
	s)	gen=msggen dir=$dir/share ;;
	t)	op=translate ;;
	v)	op=verify ;;
	*)	usage ;;
	esac
done
shift $OPTIND-1

[[ $INSTALLROOT ]] || { print -u2 $"$command: INSTALLROOT not defined"; exit 1; }

case $op in

generate)
	dir=$dir/lib/locale
	[[ -d $dir ]] || { print -u2 $"$command: $dir: not found"; exit 1; }
	(( ! $# )) && set -- C $(ls *-*.msg | sed 's,.*-\(.*\)\.msg,\1,' | sort -u)
	owd=$PWD
	for locale
	do	case $locale in
		C)	set -- $(messages) ;;
		*)	set -- *-$locale.msg ;;
		esac
		if	[[ ! -f $1 ]]
		then	print -u2 "$command: $locale: no message files"
		else	nwd=$dir/$locale/LC_MESSAGES
			[[ -d $nwd ]] || $exec mkdir -p $nwd || exit
			[[ -d $nwd ]] && { cd $nwd || exit; }
			for file
			do	case $file in
				/*)	name=${file##*/}
					name=${name%*.msg}
					;;
				*)	name=${file%-$locale.msg}
					file=$owd/$file
					;;
				esac
				if	[[ $force || ! $name -nt $file ]]
				then	print -u2 $locale $name:
					$exec rm -f $name $name.*
					$exec $gen $name $file
				fi
			done
			cd $owd
		fi
	done
	;;

remove)	(( !$# )) && set -- *.msg translate.tmp
	$exec rm -rf "$@"
	;;

translate)
	(( !$# )) && set -- $(messages)
	translate -lmv $cache $debug $omit $show $dialect "$@"
	;;

verify)	(( ! $# )) && set -- *.msg
	for file
	do	n=0
		while	read -r num txt
		do	if	[[ $num == +([0-9]) ]]
			then	((n++))
				if	[[ $n != $num ]]
				then	if	(( n == $num-1 ))
					then	print -u2 "$file: [$n] missing"
					else	print -u2 "$file: [$n-$(($num-1))] missing"
					fi
					n=$num
				fi
			fi
		done < $file
	done
	;;

*)	usage
	;;

esac
