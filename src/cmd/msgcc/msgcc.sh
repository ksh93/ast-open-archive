####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 2000-2001 AT&T Corp.                #
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
####################################################################
: C language message catalog compiler

__command__=msgcc
case `(getopts '[-][123:xyz]' opt --xyz; echo 0$opt) 2>/dev/null` in
0123)	ARGV0="-a $__command__"
	USAGE=$'
[-?
@(#)msgcc (AT&T Labs Research) 2000-04-20
]
'$USAGE_LICENSE$'
[+NAME?msgcc - C language message catalog compiler]
[+DESCRIPTION?\bmsgcc\b is a C language message catalog compiler. It accepts
	\bcc\b(1) style options and arguments. A \bmsgcpp\b(1) \b.mso\b file
	is generated for each input \b.c\b file. If the \b-c\b option is not
	specified then a \bgencat\b(1) format \b.msg\b file is generated from
	the input \b.mso\b and \b.msg\b files. If \b-c\b is not specified then
	a \b.msg\b suffix is appended to the \b-o\b \afile\a if it doesn\'t
	already have a suffix. The default output is \ba.out.msg\b if \b-c\b
	and \b-o\b are not specified.]
[+?If \b-M-new\b is not specified then messages are merged with those in the
	pre-existing \b-o\b file.]
[M?Set a \bmsgcc\b specific \aoption\a. \aoption\a may be:]:[-option]{
	[+delete?Messages in the \b-o\b file that are not in new
		\b.msg\b file arguments are deleted.]
	[+mkmsgs?The \b-o\b file is assumed to be in \bmkmsgs\b(1) format.]
	[+new?Create a new \b-o\b file.]
	[+set=\anumber\a?Set the message set number to \anumber\a. The default
		is \b1\b.]
}

file ...

[+SEE ALSO?\bcc\b(1), \bcpp\b(1), \bgencat\b(1), \bmsggen\b(1),
	\bmsgcpp\b(1), \bmsgcvt\b(1)]
'
	;;
*)	ARGV0=""
	USAGE="M:[-option] [ cc-options ] file ..."
	;;
esac

usage()
{
	OPTIND=0
	getopts $ARGV0 "$USAGE" OPT '-?'
	exit 2
}

keys()
{
	$1 --??keys -- 2>&1 | grep '^".*"$'
}

typeset -A __index__
typeset __compile__ __delete__ __mkmsgs__ __new__ __preprocess__
integer __i__=0 __args__=0 __code__=0 __files__=0 __max__=0 __num__=0 __skip__=0
integer __set__=1 __sources__=0 __cmds__=0
__out__=a.out.msg
__OUT__=

case " $* " in
*" --"*|*" -?"*)
	while	getopts $ARGV0 "$USAGE" OPT
	do	case $OPT in
		*)	break ;;
		esac
	done
	;;
esac
while	:
do	case $# in
	0)	break ;;
	esac
	__arg__=$1
	case $__arg__ in
	-c)	__compile__=1
		;;
	-[DIU]*)__argv__[__args__]=$__arg__
		(( __args__++ ))
		;;
	-E)	__preprocess__=1
		;;
	-M-delete)
		__delete__=1
		;;
	-M-mkmsgs)
		__mkmsgs__=1
		;;
	-M-new)	__new__=1
		;;
	-M-set=*)
		__set__=${__arg__#*=}
		;;
	-o)	case $# in
		1)	print -u2 $"$__command__: output argument expected"
			exit 1
			;;
		esac
		shift
		__out__=${1%.*}.msg
		__OUT__=$1
		;;
	[-+]*|*.[aAlLsS]*)
		;;
	*.[cCiI]*|*.[oO]*)
		case $__arg__ in
		*.[oO]*);;
		*)	__srcv__[__files__]=$__arg__
			(( __sources__++ ))
			;;
		esac
		__arg__=${__arg__##*/}
		__arg__=${__arg__%.*}.mso
		__objv__[__files__]=$__arg__
		(( __files__++ ))
		;;
	*.ms[go])
		__objv__[__files__]=$__arg__
		(( __files__++ ))
		;;
	*)	__cmdv__[__cmds__]=$__arg__
		(( __cmds__++ ))
		;;
	esac
	shift
done
__cmdv__[__cmds__]=${__out__%.msg}
(( __cmds__++ ))

# generate the .mso files

if	[[ $__OUT__ && $__compile__ ]]
then	__objv__[0]=$__OUT__
fi

for (( __i__=0; __i__<=__files__; __i__++ ))
do	if	[[ ${__srcv__[__i__]} ]]
	then	if	(( __sources__ > 1 ))
		then	print "${__srcv__[__i__]}:"
		fi
		if	[[ $__preprocess__ ]]
		then	msgcpp "${__argv__[@]}" "${__srcv__[__i__]}"
		else	msgcpp "${__argv__[@]}" "${__srcv__[__i__]}" > "${__objv__[__i__]}"
		fi
	fi
done

# combine the .mso and .msg files

if	[[ ! $__compile__ && ! $__preprocess__ ]]
then	if	[[ ! $__new__ && -r $__out__ ]]
	then	__tmp__=$__out__.tmp
		trap '__code__=$?; rm -f ${__tmp__}*; exit $__code__' 0 1 2
		while	read -r __line__
		do	if	(( $__skip__ ))
			then	if	[[ $__line__ == '%}'* ]]
				then	__skip__=0
				fi
				continue
			fi
			if	[[ $__mkmsgs__ && $__line__ == '%{'* ]]
			then	__skip__=1
				continue
			fi
			if	[[ $__mkmsgs__ ]]
			then	if	[[ $__line__ == '%#'*';;'* ]]
				then	__line__=${__line__#'%#'}
					__num__=${__line__%';;'*}
					read -r __line__
				elif	[[ $__line__ == %* ]]
				then	continue
				else	print -u2 $"$__command__: unrecognized line=$__line__"
					__code__=1
				fi
			else	if	[[ $__line__ == '$'* ]]
				then	continue
				else	__num__=${__line__%%' '*}
					__line__=${__line__#*'"'}
					__line__=${__line__%'"'}
				fi
			fi
			(( __index__["$__line__"]=__num__ ))
			__text__[$__num__]=$__line__
			if	(( __max__ < __num__ ))
			then	(( __max__=__num__ ))
			fi
		done < $__out__
	else	__tmp__=$__out__
	fi
	if	(( __code__ ))
	then	exit $__code__
	fi
	exec 1>$__tmp__ 9>&1
	print -r -- '$'" ${__out__%.msg} message catalog"
	print -r -- '$translation'" $__command__ $(date -f %Y-%m-%d)"
	print -r -- '$set'" $__set__"
	print -r -- '$quote "'
	sort -u "${__objv__[@]}" | {
		while	read -r __line__
		do	__op__=${__line__%% *}
			__line__=${__line__#* }
			case $__op__ in
			cmd)	__a1__=${__line__%% *}
				case $__a1__ in
				dot_cmd)	__a1__=. ;;
				esac
				keys $__a1__
				;;
			def)	__a1__=${__line__%% *}
				__a2__=${__line__#* }
				eval $__a1__='$'__a2__
				;;
			str)	print -r -- "$__line__"
				;;
			var)	__a1__=${__line__%% *}
				__a2__=${__line__#* }
				case $__a1__ in
				[0-9]*)	eval __v__='$'$__a2__
					__v__='"'${__v__:__a1__+1}
					;;
				*)	eval __v__='$'$__a1__
					;;
				esac
				if	[[ $__v__ == '"'*'"' ]]
				then	print -r -- "$__v__"
				fi
				;;
			[0-9]*)	print -r -- "$__line__"
				;;
			'$')	print -r -u9 $__op__ include $__line__
				;;
			esac
		done
		for (( __i__=0; __i__ < __cmds__; __i__++ ))
		do	keys ${__cmdv__[__i__]}
		done
	} | {
		__num__=1
		while	read -r __line__
		do	case $__line__ in
			'$'[\ \	]*)
				print -r -- "$__line__"
				continue
				;;
			'$'*|*"@(#)"*|*"<"*([a-zA-Z0-9_ .-])"@"*([a-zA-Z0-9_ .-])">"*|"http://"*)
				continue
				;;
			*[a-zA-Z][a-zA-Z]*)
				__line__=${__line__#*'"'}
				__line__=${__line__%'"'}
				if	[[ $__line__ ]]
				then	if	[[ ${__index__["$__line__"]} ]]
					then	if [[ $__delete__ ]]
						then	__num__=${__index__["$__line__"]}
							__text__[-$__num__]=1
						fi
					else	while	 [[ ${__text__[$__num__]} ]]
						do	(( __num__++ ))
						done
						if	(( __max__ < __num__ ))
						then	(( __max__=__num__ ))
						fi
						if	[[ $__delete__ ]]
						then	 __text__[-$__num__]=1
						fi
						__text__[$__num__]=$__line__
						(( __index__["$__line__"]=__num__++ ))
					fi
				fi
				;;
			esac
		done
		if	(( __max__ < __num__ ))
		then	(( __max__=__num__ ))
		fi
		for (( __num__=1; __num__ <=__max__; __num__++ ))
		do	if	[[ ${__text__[$__num__]} && ( ! $__delete__ || ${__text__[-$__num__]} ) ]]
			then	print -r -- $__num__ "\"${__text__[$__num__]}\""
			fi
		done
	}
	if [[ $__tmp__ != $__out__ ]]
	then	grep -v '^\$' $__tmp__ > ${__tmp__}n
		grep -v '^\$' $__out__ > ${__tmp__}o
		cmp -s ${__tmp__}n ${__tmp__}o ||
		mv $__tmp__ $__out__
	fi
fi
exit $__code__
