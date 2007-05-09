########################################################################
#                                                                      #
#               This software is part of the ast package               #
#           Copyright (c) 1996-2007 AT&T Knowledge Ventures            #
#                      and is licensed under the                       #
#                  Common Public License, Version 1.0                  #
#                      by AT&T Knowledge Ventures                      #
#                                                                      #
#                A copy of the License is available at                 #
#            http://www.opensource.org/licenses/cpl1.0.txt             #
#         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         #
#                                                                      #
#              Information and Software Systems Research               #
#                            AT&T Research                             #
#                           Florham Park NJ                            #
#                                                                      #
#                 Glenn Fowler <gsf@research.att.com>                  #
#                                                                      #
########################################################################
: mm2bb - convert mm/man subset to bb code

# it keeps going and going ...
#
# \h'0*\w"URL"'HOT-TEXT\h'0'	link goto with alternate url
# \h'0*1'HOT-TEXT\h'0'		link goto
#
# .xx link="URL\tHOT-TEXT"	link goto with url request
# .xx link="HOT-TEXT"		link goto request
# .xx ref="URL\tMIME-TYPE"	head link hint

command=mm2bb
version='mm2bb (AT&T Research) 2007-04-25' # NOTE: repeated in USAGE
LC_NUMERIC=C
case $(getopts '[-][123:xyz]' opt --xyz 2>/dev/null; echo 0$opt) in
0123)	ARGV0="-a $command"
	USAGE=$'
[-?
@(#)$Id: mm2bb (AT&T Research) 2007-04-25 $
]
'$USAGE_LICENSE$'
[+NAME?mm2bb - convert mm/man subset to bb code]
[+DESCRIPTION?\bmm2bb\b is a \bsed\b(1)/\bksh\b(1) script (yes!) that
	converts input \bmm\b(1) or \bman\b(1) documents to an \bbb\b code
	document on the standard output. If \afile\a is omitted then the
	standard input is read.  \adir\a operands and directory components
	of \afile\a operands are added to the included file search list.]

[ [ dir | file ] ... ]

[+SEE ALSO?\bmm2html\b(1)]
'
	;;
*)	ARGV0=""
	USAGE=' [ file ... ]'
	;;
esac

set -o noglob

integer count row n s ndirs=0 nfiles=0
integer fd=0 head=2 line=0 lists=0 nest=0 peek=0 pp=0 so=0 soff=4
integer labels=0 reference=1 ce=0 nf=0 augment=0 tbl_ns=0 tbl_no=1 tbl_fd=1
typeset -Z2 page=01
typeset -u upper
typeset -x -l OP
typeset -x -A ds map nr outline
typeset cond dirs files fg frame label list prev text title type
typeset license html meta nl mm index authors vg references
typeset mm_AF mm_AF_cur mm_AF_old mm_AU

nl=$'\n'

html=(
	H1=(
		align=center
	)
	home=(
		href=
	)
	index=(
		left=
		top=
	)
	logo=(
		src=
	)
	width=96%
)

code=0
figure=1
file=
font=
frame=
framebody=
framelink=
framerefs=
hp=
ifs=${IFS-'
	'}
inch="     "
indent=
indent_prev=
macros=
pd="$nl"
pm=
primary=".BL|.LI|.IX"
top=
vg_ps=20

while	getopts $ARGV0 "$USAGE" OPT
do	case $OPT in
	*)	OPTIND=0
		getopts $ARGV0 "$USAGE" OPT '-?'
		exit 2
		;;
	esac
done
shift OPTIND-1
case $# in
1)	file=$1
	if	[[ ! -f $file ]]
	then	print -u2 $command: $file: cannot read
		exit 1
	fi
	x="-m $file"
	;;
*)	x=
	;;
esac

ds[Cr]='&#169;'
ds[Dt]=$(date -f "%B %d, %Y" $x)
ds[Rf]="\\u[$reference]\\d"
ds[Rg]='&#174;'
ds[CM]='&#169;'
ds[RM]='&#174;'
ds[SM]='[size=small][b]SM[/b][/size]'
ds[TM]='[size=small][b]TM[/b][/size]'

map[.Cs]=.EX
map[.Ce]=.EE
map[.Sh]=.SH
map[.Ss]=.SS
map[.Tp]=.TP

H=H$(( head + 1 ))

function warning
{
	print -u2 "$command: warning: ${file:+"$file: "}line $line:" "$@"
}

function getfiles
{
	sed	\
	-e 's%\\".*%%' \
	-e 's%\\(>=%>=%g' \
	-e 's%\\(<=%<=%g' \
	-e 's%\\'\''%'\''%g' \
	-e 's%\\`%`%g' \
	-e 's%\\+%+%g' \
	-e 's%\\0% %g' \
	-e 's%\\|% %g' \
	-e 's%\\\^% %g' \
	-e 's%\\ % %g' \
	-e 's%\\(+-%\&#177;%g' \
	-e 's%\\(-%=%g' \
	-e 's%\\(ap%~%g' \
	-e 's%\\(bu%\&#183;%g' \
	-e 's%\\(bv%|%g' \
	-e 's%\\(co%\&#169;%g' \
	-e 's%\\(dg%\&#167;%g' \
	-e 's%\\(fm%'\''%g' \
	-e 's%\\(rg%\&#174;%g' \
	-e 's%\\(sq%\&#164;%g' \
	-e 's%\\(\*\([*`'\'']\)%\1%g' \
	-e 's%\\\*\([*`'\'']\)%\1%g' \
	-e 's%\\d\([^\\]*\)\\u%\1%g' \
	-e 's%\\u\([^\\]*\)\\d%\1%g' \
	-e 's%\\v\(.\)-\([^\\]*\)\1\(.*\)\\v\1+*\2\1%\3%g' \
	-e 's%\\v\(.\)+*\([^\\]*\)\1\(.*\)\\v\1-\2\1%\3%g' \
	-e 's%\\h'\''0\*\\w"\([abcdefghijklmnopqrstuvwxyz]*:[^"]*\)"'\''\([^'\'']*\)\\h'\''0'\''%[url="\1"]\2[/url]%g' \
	-e 's%\\h'\''0\*\\w"\(/[^"]*\)"'\''\([^'\'']*\)\\h'\''0'\''%[url="\1"]\2[/url]%g' \
	-e 's%\\h'\''0\*\\w"\([^"]*\)"'\''\([^'\'']*\)\\h'\''0'\''%[url="\1"]\2[/url]%g' \
	-e 's%\\h'\''0\*1'\''\([^:/'\'']*\)\\h'\''0'\''%[url="\1"]\1[/url]%g' \
	-e 's%\\h'\''0\*1'\''\([abcdefghijklmnopqrstuvwxyz]*:[^'\'']*\)\\h'\''0'\''%[url="\1"]\1[/url]%g' \
	-e 's%\\h'\''0\*1'\''\(/[^'\'']*\)\\h'\''0'\''%[url="\1"]\1[/url]%g' \
	-e 's%\\h'\''0\*1'\''\([^'\'']*\)\\h'\''0'\''%[url="\1"]\1[/url]%g' \
	-e 's%\\h'\''0/\\w"\([^"]*\)"'\''\([^'\'']*\)\\h'\''0'\''%[url="\1"]\2[/url]%g' \
	-e 's%\\h'\''0/1'\''\([^'\'']*\)\\h'\''0'\''%[url="\1"]\1[/url]%g' \
	-e 's%\\s+\(.\)\([^\\]*\)\\s-\1%[size=big]\2[/size]%g' \
	-e 's%\\s+\(.\)\([^\\]*\)\\s0%[size=big]\2[/size]%g' \
	-e 's%\\s-\(.\)\([^\\]*\)\\s+\1%[size=small]\2[/size]%g' \
	-e 's%\\s-\(.\)\([^\\]*\)\\s0%[size=small]\2[/size]%g' \
	-e 's%\\f(\(..\)\([^\\]*\)%<\1>\2</\1>%g' \
	-e 's%\\f[PR]%\\fZ%g' \
	-e 's%\\f\(.\)\([^\\]*\)%<\1>\2</\1>%g' \
	-e 's%&lt;\([abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789][-._abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789]*@[abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.]*\)&gt;%[size=small]\&lt;[url=mailto:\1]\1[/url]\&gt;[/size]%g' \
	-e 's%\[[ABCDEFGHIJKLMNOPQRSTUVWXYZ][ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz]*[0123456789][0123456789][abcdefghijklmnopqrstuvwxyz]*]%[b]&[/b]%g' \
	-e 's%</*Z>%%g' \
	-e 's%<[146789]>%%g' \
	-e 's%</[146789]>%%g' \
	-e 's%<2>%[i]%g' \
	-e 's%</2>%[/i]%g' \
	-e 's%<3>%[b]%g' \
	-e 's%</3>%[/b]%g' \
	-e 's%<5>%[code]%g' \
	-e 's%</5>%[/code]%g' \
	-e 's%<B>%[b]%g' \
	-e 's%</B>%[/b]%g' \
	-e 's%<I>%[i]%g' \
	-e 's%</I>%[/i]%g' \
	-e 's%<L>%[code]%g' \
	-e 's%</L>%[/code]%g' \
	-e 's%<X>%[code]%g' \
	-e 's%</X>%[/code]%g' \
	-e 's%<CW>%[code]%g' \
	-e 's%</CW>%[/code]%g' \
	-e 's%<i>\([^<]*\)</i>(\([0123456789]\))%[url="../man\2/\1.html"][i]\1[/i][/url]\2%g' \
	-e 's%<b>\([^<]*\)</b>(\([0123456789]\))%[url="../man\2/\1.html"][b]\1[/b][/url]\2%g' \
	-e 's%\\s+\(.\)\(.*\)\\s-\1%[size=big]\2[/size]%g' \
	-e 's%\\s-\(.\)\(.*\)\\s+\1%[size=small]\2[/size]%g' \
	-e 's%\\e%\&#0092;%g' \
	-e '/^'\''[abcdefghijklmnopqrstuvwxyz][abcdefghijklmnopqrstuvwxyz]\>/s%.%.%' \
	-e '/^\..*".*\\/s%\\[^\*][^(]%\\&%g' \
	"$@"
}

fill=
spaced=1

function space
{
	flush
	if	(( !spaced ))
	then	spaced=1
		print
	fi
}

function flush
{
	if	[[ $fill ]]
	then	print -r -- "${fill#?}" | fmt
		fill=
		spaced=0
	fi
}

function puttext
{
	if	((nf))
	then	print -r -- "$*"
		spaced=0
	elif	[[ $1 ]]
	then	fill="$fill $*"
	else	flush
		space
	fi
}

function putop
{
	flush
	if	(( $# ))
	then	print -r -- "$*"
		spaced=0
	fi
}

function getline
{
	integer i n
	typeset data a c q v x d
	if	(( peek ))
	then	(( peek = 0 ))
		trap 'set -- "${text[@]}"' 0
		return
	fi
	while	:
	do	data=
		while	:
		do	IFS= read -r -u$fd a || {
				if	(( so > 0 ))
				then	eval exec $fd'>&-'
					if	(( ( fd = --so + soff ) == soff ))
					then	(( fd = 0 ))
					fi
					file=${so_file[so]}
					line=${so_line[so]}
					continue
				fi
				return 1
			}
			(( line++ ))
			case $a in
			*\\)	x=${a%%+(\\)}
				a=${a#"$x"}
				if	(( ! ( ${#a} & 1 ) ))
				then	data=$data$x$a
					break
				fi
				data=$data$x${a%?}
				;;
			*\\\})	data=$data${a%??}
				break
				;;
			*)	data=$data$a
				break
				;;
			esac
		done
		case $data in
		*\\[n\*]*)
			a= i=0
			while	:
			do	c=${data:i++:1}
				case $c in
				"")	break
					;;
				\\)	x=${data:i:1}
					case $x in
					[n\*])	(( i++ ))
						c=${data:i++:1}
						case $c in
						\()	c=${data:i:2}
							(( i += 2 ))
							;;
						esac
						case $x in
						n)	a=$a${nr[$c]} ;;
						*)	a=$a${ds[$c]} ;;
						esac
						continue
						;;
					\\)	a=$a$c
						while	:
						do	c=${data:i++:1}
							case $c in
							\\)	;;
							*)	break ;;
							esac
							a=$a$c
						done
						;;
					esac
					;;
				esac
				a=$a$c
			done
			data=$a
			;;
		esac
		case $data in
		.?*)	case $data in
			*[\"]*)
				unset v
				a= i=0 n=0 q=
				while	:
				do	c=${data:i++:1}
					case $c in
					"")	break ;;
					esac
					case $c in
					$q)	q=
						case $c in
						\>)	;;
						*)	continue
							;;
						esac
						;;
					[\"\<])	case $q in
						"")	case $c in
							\<)	q=\>
								;;
							*)	q=$c
								continue
								;;
							esac
							;;
						esac
						;;
					\ |\	)
						case $q in
						"")	case $a in
							?*)	v[n++]=$a
								a=
								;;
							esac
							continue
							;;
						esac
						;;
					esac
					a=$a$c
				done
				case $a in
				?*)	v[n++]=$a ;;
				esac
				set -A text -- "${v[@]}"
				;;
			*)	set -A text -- $data
				;;
			esac
			case ${text[0]} in
			.el|.ie|.if)
				set -- "${text[@]}"
				shift
				x=$1
				shift
				case ${text[0]} in
				.e*)	if	(( nest <= 0 ))
					then	warning "unmatched ${text[0]}"
						n=0
					else	n=$(( ! ${cond[nest--]} ))
					fi
					;;
				.i*)	case $x in
					!*)	x=${x#?}
						n=1
						;;
					*)	n=0
						;;
					esac
					case $x in
					t|\'@(*)\'\1\'|+([\-+0123456789])=\1)
						(( n = ! n ))
						;;
					+([\-+0123456789])=+([\-+0123456789]))
						;;
					[0123456789]*[0123456789])
						(( n = $x ))
						;;
					esac
					case ${text[0]} in
					.ie)	cond[++nest]=$n ;;
					esac
					;;
				esac
				if	(( ! n ))
				then	case $@ in
					\\\{*)	while	read -r -u$fd data
						do	(( line++ ))
							case $data in
							*\\\})	break ;;
							esac
						done
						;;
					esac
					continue
				fi
				set -A text -- "$@"
				case ${text[0]} in
				\\\{*)	text[0]=${text[0]#??} ;;
				esac
				;;
			.so)	x=${text[1]}
				for d in "${dirs[@]}"
				do	if	[[ -f "$d$x" ]]
					then	(( fd = so + soff ))
						tmp=/tmp/m2h$$
						getfiles "$d$x" > $tmp
						eval exec $fd'< $tmp'
						rm $tmp
						so_file[so]=$file
						file=$d$x
						so_line[so]=$line
						(( line = 0 ))
						(( so++ ))
						continue 2
					fi
				done
				warning "$x: $op cannot read"
				continue
				;;
			.xx)	data=
				set -- "${text[@]}"
				shift
				while	:
				do	case $# in
					0)	break ;;
					esac
					nam=${1%%=*}
					case $nam in
					no?*)	nam=${nam#no} val=0 ;;
					*)	val=${1#*=} ;;
					esac
					shift
					case $nam in
					label|link*|ref)
						case $val in
						*'	'*)
							url=${val%%'	'*}
							txt=${val#*'	'}
							;;
						*'\\t'*)
							url=${val%%'\\t'*}
							txt=${val#*'\\t'}
							;;
						*)	url=$val
							txt=$val
							;;
						esac
						case $url in
						*[:/.]*)	pfx= ;;
						*)		pfx='#' ;;
						esac
						case $url in
						*'${html.'*'}'*)
							eval url=\"$url\"
							;;
						esac
						case $nam in
						label)	if	(( labels >= 0 ))
							then	nam=name
								label[labels++]=$txt
								puttext "[url=\"$url\"]$txt[/url]"
							fi
							;;
						link*)	tar=
							case $nam in
							link)	case $frame$top$vg in
								?*)	case $url in
									*([abcdefghijklmnopqrstuvwxyz]):*|/*)
										tar=" target=_top"
										;;
									esac
									;;
								esac
								;;
							esac
							nam=href
							data="${data}[url=\"$pfx$url\"]$txt[/url]"
							;;
						esac
						;;
					esac
				done
				case $data in
				'')	continue ;;
				esac
				set -A text -- "$data"
				break
				;;
			esac
			case ${text[0]} in
			@($primary))
				: primary macros take precedence
				;;
			.[BILMRUX]?([BILMRUX])|.F|.FR|.MW|.RF)
				case $macros:${text[0]} in
				mm:.RF)	break ;;
				esac
				typeset font1 font2 op
				set -- "${text[@]}"
				op=$1
				shift
				case $op in
				.[BIL]R)case $#:$2 in
					2':('[0123456789]')'*([,.?!:;]))
						x=${2#'('*')'}
						y=${2%$x}
						n=$y
						case $op in
						.B*)	font1=b ;;
						.L*)	font1=code ;;
						*)	font1=i ;;
						esac
						case $macros in
						man)	set -A text -- "[url=\"../man$n/$1.html\"][$font1]$1[/$font1][/url]$y$x" ;;
						*)	set -A text -- "[url=\"${html.man:=../man}/man$n/$1.html\"][$font1]$1[/$font1][/url]$y$x" ;;
						esac
						break
						;;
					esac
					;;
				.F)	op=.L ;;
				.FR)	op=.LR ;;
				.MW)	op=.L ;;
				.RF)	op=.RL ;;
				esac
				case $#:$macros:$op in
				0:*)	;;
				*:man:.?)
					set -- "$*"
					;;
				esac
				case $# in
				0)	getline
					set -- "$*"
					;;
				esac
				set -- "$indent""$@"
				case $font in
				"")	data=
					;;
				?*)	data="[/$font]"
					font=
					;;
				esac
				font1=${op#.}
				case $font1 in
				?)	font2=R
					;;
				*)	font2=${font1#?}
					font1=${font1%?}
					;;
				esac
				case $font1 in
				B)	font1=b ;;
				I)	font1=i ;;
				[LMX])	font1=code ;;
				R)	font1= ;;
				esac
				case $font2 in
				B)	font2=b ;;
				I)	font2=i ;;
				[LMX])	font2=code ;;
				R)	font2= ;;
				esac
				font=$font2
				while	:
				do	case $# in
					0)	break ;;
					esac
					case $font in
					$font2)	font=$font1 ;;
					*)	font=$font2 ;;
					esac
					case $1 in
					"")	;;
					*"[size"*)
						case $font in
						"")	data="$data$1" ;;
						*)	data="$data[$font]$1[/$font]" ;;
						esac
						;;
					*)	case "$1 $2" in
						*"[size"*)
							case $font in
							"")	data="$data$1 $2" ;;
							*)	data="$data[$font]$1 $2[/$font]" ;;
							esac
							shift
							;;
						*)	case $font in
							"")	data="$data$1" ;;
							*)	data="$data[$font]$1[/$font]" ;;
							esac
							;;
						esac
						;;
					esac
					shift
				done
				font=
				set -A text -- $data
				;;
			.PD)	case $1 in
				0)	pd= ;;
				*)	pd=$nl ;;
				esac
				continue
				;;
			.PF|.PH)continue
				;;
			.SB)	set -- "${text[@]}"
				shift
				case $# in
				0)	getline ;;
				esac
				set -A text -- "[size=small][b]""$@""[/b][/size]"
				;;
			.SG)	continue
				;;
			.SM)	set -- "${text[@]}"
				shift
				case $# in
				0)	getline ;;
				esac
				set -A text -- "[size=small]""$@""[/size]"
				;;
			*)	x=${map[${text[0]}]}
				case $x in
				?*)	text[0]=$x ;;
				esac
				;;
			esac
			;;
		*)	set -A text -- "$data"
			;;
		esac
		break
	done
	trap 'set -- "${text[@]}"' 0
}

function heading
{
	typeset op=$1 i o options beg end
	integer count

	shift
	case $op in
	.H)	case $# in
		0)	count=1 ;;
		*)	count=$1; shift ;;
		esac
		options=
		;;
	.H*|.AS)count=1
		;;
	*)	count=2
		;;
	esac
	case $* in
	"")	putop
		;;
	*)	if	(( count == 1 ))
		then	beg="$beg[color=red][size=big]"
			end="[/size][/color]$end"
		fi
		space
		putop "$beg $* $end"
		space
		;;
	esac
}

function tbl_attributes
{
	typeset d f i u x
	typeset -F0 w
	case $1 in
	[aAcC]*)a="$a align=center" ;;
	[lL]*)	a="$a align=left" ;;
	[nN]*)	a="$a align=right char=." ;;
	[rR]*)	a="$a align=right" ;;
	esac
	case $i in
	*[wW]\(+([0-9.])*\)*)
		x=${i##*[wW]\(}
		x=${x%%\)*}
		u=${x##+([0-9.])}
		x=${x%$u}
		case $u in
		c)	(( w=x*75/2.54 )) ;;
		i)	(( w=x*75 )) ;;
		m)	(( w=x*75*12/72 )) ;;
		n)	(( w=x*75*12/72/2 )) ;;
		p)	(( w=x*75/72 )) ;;
		P)	(( w=x*75/6 )) ;;
		*)	(( w=x*75*12/72/2 )) ;;
		esac
		a="$a width=$w"
		;;
	esac
	case $i in
	*[bB]*)		b="$b[b]" e="[/b]$e" ;;
	esac
	case X$i in
	*[!0-9.][iI]*)	b="$b[i]" e="[/i]$e" ;;
	esac
}

dirs[++ndirs]=""
for i
do	if [[ -d $i ]]
	then	dirs[++ndirs]=$i/
	else	files[++nfiles]=$i
		if [[ $i == */* ]]
		then	dirs[++ndirs]=${i%/*}/
		fi
	fi
done
document="${files[@]}"

getfiles "${files[@]}" |
while	:
do	getline || {
		[[ $title != '' ]] && break
		set -- .TL
	}
	case $1 in
	.)	: comment
		;;
	.*)	op=$1
		shift
		case $op in
		.AC)	: ignore $op
			;;
		.AE)	: ignore $op
			;;
		.AF)	case $mm_AF_cur in
			"")	mm_AF_cur="[i]$*" ;;
			*)	mm_AF_cur="${mm_AF_cur}${nl}$*" ;;
			esac
			;;
		.AL|.[IR]S|.VL)
			case $macros:$op in
			mm:.RS)	Rf="\\u[$reference]\\d"
				references="$references$nl[list][$reference][/list]"
				while	getline
				do	case $1 in
					.RF)	break ;;
					esac
					references="$references$nl$*"
				done
				(( reference++ ))
				continue
				;;
			esac
			type[++lists]=.AL
			list[lists]=DL
			case $op in
			.AL)	case $1 in
				'')	type[++lists]=.al
					list[lists]=OL
					;;
				[0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ])
					type[++lists]=.al
					list[lists]=OL
					;;
				esac
				;;
			.[IR]S)	;;
			.VL)	case $1 in
				?*)	type[++lists]=.al
					list[lists]=DL
					;;
				esac
				;;
			esac
			;;
		.AS|.H|.HU|.SH|.SS|.ce|.CE)
			if ((nf))
			then	nf=0
				putop "[/code]"
			fi
			if ((ce))
			then	ce=0
			fi
			case $hp in
			?*)	indent=${indent#$hp}
				hp=
				;;
			esac
			if	(( lists > pp ))
			then	case ${type[@]:0:lists} in
				*.[Aa][Ll]*|*.[IiTt][Pp]*)
					while	:
					do	case ${type[lists]} in
						.[Aa][Ll]|.[IiTt][Pp])
							case ${type[lists]} in
							.AL|.IP|.TP)break ;;
							esac
							;;
						*)	break
							;;
						esac
					done
					;;
				esac
			fi
			(( pp = lists ))
			end=
			case ${mm.title} in
			?*)	beg="$beg[size=big][color=blue]"
				end="[/color][/size]$end"
				space
				putop "$beg ${mm.title} $end"
				space
				mm.title=
				case ${mm.author} in
				?*)	IFS=:
					set -- ${mm.author#:} : "$@"
					IFS=$ifs
					while	:
					do	case $# in
						0)	break ;;
						esac
						x=$1
						shift
						case $x in
						:)	break ;;
						esac
						putop "$x"
					done
					;;
				esac
				case $mm_AU in
				?*)	putop "${mm_AU#?}"
					case $mm_AF_cur in
					?*)	mm_AF="${mm_AF_cur}[/i]" ;;
					esac
					case $mm_AF in
					?*)	putop "$mm_AF" ;;
					esac
					;;
				esac
				;;
			esac
			case $op in
			.AS)	heading $op Abstract
				;;
			.ce)	case $# in
				0)	count=1 ;;
				*)	count=$1 ;;
				esac
				while	(( count-- > 0 )) && read -r data
				do	putop "$data"
				done
				;;
			.CE)	;;
			.S[HS])	macros=man
				while	(( lists > 0 ))
				do	case ${type[lists--]} in
					.S[HS])	break ;;
					esac
				done
				type[++lists]=$op
				list[lists]=DL
				case $op in
				.SS)	type[++lists]=.XX
					list[lists]=DL
					;;
				esac
				case $op in
				.SH)	heading .H 2 "$@" ;;
				*)	heading .H 3 "$@" ;;
				esac
				type[++lists]=.XX
				list[lists]=DL
				;;
			*)	heading $op "$@"
				;;
			esac
			;;
		.AT)	: ignore $op
			;;
		.AU)	case $mm_AF_cur in
			?*)	case $mm_AF in
				?*)	case $mm_AU in
					?*)	mm_AU="${mm_AU}${mm_AF}" ;;
					esac
					;;
				esac
				mm_AF="${mm_AF_cur}[/i]"
				mm_AF_cur=""
				;;
			esac
			mm_AU="${mm_AU}$nl$1"
			;;
		.BL)
			i=
			for ((n = 1; n <= lists; n++))
			do	i=$i${list[n]}
			done
			case $i in
			*UL*UL*)i=disc ;;
			*UL*)	i=circle ;;
			*)	i=square ;;
			esac
			type[++lists]=.AL
			list[lists]=UL
			;;
		.BP)	unset parm
			while	[[ $1 == *=* ]]
			do	eval parm="( ${1%%=*}='${1#*=}' )"
				shift
			done
			unset oparm
			oparm=$parm
			i=$1
			if	[[ $i == *.@(gif|png) ]]
			then	for i
				do	f=
					for d in "${dirs[@]}"
					do	if [[ -f "$d$i" ]]
						then	f=$d$i
							break
						fi
					done
					if [[ ! $f ]]
					then	print -u2 "$command: warning: $i: data file not found"
					fi
					if	[[ ! ${oparm.alt} ]]
					then	u=${i##*/}
						u=${i%.*}
						parm=( alt=$u )
					fi
					if	[[ ! ${oparm.title} ]]
					then	u=${i##*/}
						u=${i%.*}
						if	[[ ${parm.category} ]]
						then	u="${parm.category} $u"
						elif	[[ ${oparm.category} ]]
						then	u="${oparm.category} $u"
						fi
						parm=( title=$u )
					fi
					putop "[img=\"$i\"]"
				done
			else	i=${i%.*}.gif
				case $frame in
				?*)	[[ -f $frame-$i ]] && i=$frame-$i ;;
				esac
				f=
				for d in "${dirs[@]}"
				do	if	[[ -f "$d$1" ]]
					then	f=$d$1
						break
					fi
				done
				if [[ ! $f ]]
				then	print -u2 "$command: $1: data file not found"
				elif [[ $f -nt $i ]]
				then	ps2gif $f $i
				fi
				putop "[img=\"$i\"]"
			fi
			;;
		.CT)	: ignore $op
			;;
		.DE|.fi)
			if ((nf))
			then	nf=0
				putop "[/code]"
			fi
			;;
		.DF|.DS|.nf)
			if ((!nf))
			then	nf=1
				putop "[code]"
			fi
			;;
		.DT)	case $macros in
			man)	;;
			*)	putop "${ds[Dt]}" ;;
			esac
			;;
		.EE)if ((nf))
			then	nf=0
				putop "[/code]"
			fi
			if [[ $fg ]]
			then	putop "[color=blue]$fg[/color]"
			fi
			indent=${indent#$inch}
			;;
		.EX)if ((!nf))
			then	nf=1
				putop "[code]"
			fi
			indent=$inch$indent
			case $# in
			2)	fg="Figure $1: $2" ;;
			*)	fg= ;;
			esac
			;;
		.FE)	putop " ] "
			;;
		.FG)	putop "[color=blue]Figure $figure: $*[/color]"
			(( figure++ ))
			;;
		.FS)	putop " [ "
			;;
		.HP|.LP|.P|.PP)
			case $hp in
			?*)	indent=${indent#$hp}
				hp=
				;;
			esac
			if	(( lists > pp ))
			then	case ${type[@]:0:lists} in
				*.[Aa][Ll]*|*.[IiTt][Pp]*)
					while	:
					do	case ${type[lists]} in
						.[Aa][Ll]|.[IiTt][Pp])
							case ${type[lists]} in
							.AL|.IP|.TP)break ;;
							esac
							;;
						*)	break
							;;
						esac
					done
					;;
				esac
			fi
			(( pp = lists ))
			putop
			;;
		.HY)	: ignore $op
			;;
		.IP|.LI|.TF|.TP)
			case $macros:$op in
			mm:.TP)	continue ;;
			esac
			case $op in
			.IP|.LP|.TF|.TP)OP=$op
				case ${type[lists]} in
				$op|$OP);;
				*)	type[++lists]=$op
					list[lists]=DL
					putop
					;;
				esac
				case $op in
				.IP|.LP|.TF)
					set -- "[code]$*[/code]"
					;;
				.TP)	getline
					;;
				esac
				;;
			esac
			case ${list[lists]} in
			"")	warning "$op: no current list"
				;;
			DL)	case $# in
				0)	getline ;;
				esac
				putop "[list][b]$*[/b][/list]"
				;;
			*)	putop "[list][b]$*[/b][list]"
				;;
			esac
			;;
		.IX)	: ignore $op
			;;
		.LE|.[IR]E)
			case ${type[@]} in
			*.[Aa][Ll]*)
				space
				while	(( lists > 0 ))
				do	case ${type[lists--]} in
					.AL)	break ;;
					esac
				done
				;;
			*)	warning "$op: no current list type"
				;;
			esac
			case $op:$pd in
			.[IR]E:?*)	putop ;;
			esac
			;;
		.LX)	: ignore $op
			;;
		.MT)	macros=mm
			;;
		.ND|.Dt)ds[Dt]=$*
			;;
		.NL)	type[++lists]=.AL
			list[lists]=OL
			;;
		.OK)	mm.keywords="$*"
			;;
		.OP)	: .OP opt arg arg-append arg-prepend
			x="$4[b]&#45;$1[/b][i]$2[/i]"
			case $3 in
			'[]')	x="[ $x ]" ;;
			?*)	x="$x$3" ;;
			esac
			putop "$x"
			;;
		.PM)	case ${html.company} in
			'')	pm= ;;
			*)	pm="${html.company//\&/&amp\;} " ;;
			esac
			case $1 in
			'')	pm= ;;
			C|NDA)	pm="${pm}CONFIDENTIAL" ;;
			RG)	pm="${pm}PROPRIETARY (REGISTERED)" ;;
			RS)	pm="${pm}PROPRIETARY (RESTRICTED)" ;;
			*)	pm="${pm}PROPRIETARY" ;;
			esac
			case $pm in
			?*)	pm="[color=blue]${pm}[/color]" ;;
			esac
			;;
		.PU)	: ignore $op
			;;
		.SA)	: ignore $op
			;;
		.SU)	: ignore $op
			;;
		.TH|.TL): .TL junk junk
			: .TH item section foot_center foot_left head_center
			title=1
			case $macros:$op in
			:.TH)	macros=man ;;
			:.TL)	macros=mm ;;
			esac
			case $op in
			.TH)	case $3 in
				?*)	dc[++dcs]=$3 ;;
				esac
				case $4 in
				?*)	dl[++dls]=$4 ;;
				esac
				case $5 in
				'')	case $2 in
					1*)	sec="USER COMMANDS " ;;
					2*)	sec="SYSTEM CALLS" ;;
					3C)	sec="COMPATIBILITY FUNCTIONS" ;;
					3F)	sec="FORTRAN LIBRARY ROUTINES" ;;
					3K)	sec="KERNEL VM LIBRARY FUNCTIONS" ;;
					3L)	sec="LIGHTWEIGHT PROCESSES LIBRARY" ;;
					3M)	sec="MATHEMATICAL LIBRARY" ;;
					3N)	sec="NETWORK FUNCTIONS" ;;
					3R)	sec="RPC SERVICES LIBRARY" ;;
					3S)	sec="STANDARD I/O FUNCTIONS" ;;
					3V)	sec="SYSTEM V LIBRARY" ;;
					3X)	sec="MISCELLANEOUS LIBRARY FUNCTIONS" ;;
					3*)	sec="C LIBRARY FUNCTIONS" ;;
					4*)	sec="DEVICES AND NETWORK INTERFACES" ;;
					4F)	sec="PROTOCOL FAMILIES" ;;
					4P)	sec="PROTOCOLS" ;;
					4*)	sec="DEVICES AND NETWORK INTERFACES" ;;
					5*)	sec="FILE FORMATS" ;;
					6*)	sec="GAMES AND DEMOS" ;;
					7*)	sec="PUBLIC FILES AND TABLES" ;;
					8*)	sec="MAINTENANCE COMMANDS" ;;
					L*)	sec="LOCAL COMMANDS" ;;
					*)	sec="SECTION $2" ;;
					esac
					;;
				*)	sec=$5
					;;
				esac
				mm.title="$sec -- $1($2)"
				;;
			.TL)	getline || break
				case ${html.title} in
				'')	mm.title=$* ;;
				esac
				;;
			esac
			;;
		.TM)	: ignore $op
			;;
		.TX)	: ignore $op
			;;
		.UC)	: ignore $op
			;;
		.VE)	: ignore $op
			;;
		.VS)	: ignore $op
			;;
		.al)	: ignore $op
			;;
		.bd)	: ignore $op
			;;
		.br)	putop
			;;
		.de|.am.ig)
			end=..
			case $#:$op in
			0:*)	;;
			*:.ig)	end=$1 ;;
			esac
			: ignore $op to $end
			while	getline
			do	case $1 in
				$end)	break ;;
				esac
			done
			;;
		.ds)	op=$1
			shift
			ds[$op]=$*
			;;
		.f[tBILPR])
			case $op in
			.f[BILPR])
				set -- ${op#.f}
				;;
			esac
			case $1 in
			5|TT)	font=code
				puttext "[$font]"
				;;
			B)	font=b
				puttext "[$font]"
				;;
			I)	font=i
				puttext "[$font]"
				;;
			*)	case $font in
				?*)	puttext "[/$font]"
					font=
					;;
				esac
				;;
			esac
			;;
		.fp)	: ignore $op
			;;
		.hc)	: ignore $op
			;;
		.hw)	: ignore $op
			;;
		.hy)	: ignore $op
			;;
		.in)	indent_prev=$indent
			case $1 in
			"")	i=$indent_prev; indent_prev=$indent; indent=$i ;;
			-*)	indent=${indent#$inch} ;;
			+*)	indent=$inch$indent ;;
			*)	indent=$inch ;;
			esac
			;;
		.lf)	: ignore $op
			;;
		.ll)	: ignore $op
			;;
		.ne)	: ignore $op
			;;
		.nh)	: ignore $op
			;;
		.nr)	op=$1
			shift
			nr[$op]=$*
			;;
		.ns)	: ignore $op
			;;
		.ps)	case $1 in
			-*)	putop "[size=small]" ;;
			+*)	putop "[size=big]" ;;
			esac
			;;
		.sh)	case $HTMLPATH in
			'')	;;
			*)	(( fd = so + soff ))
				file=/tmp/m2h$$
				path=$PATH
				eval PATH=$HTMLPATH "$*" > $file
				PATH=$path
				eval exec $fd'< $file'
				rm $file
				so_file[so]=$file
				file=$1
				so_line[so]=$line
				(( line = 0 ))
				(( so++ ))
				;;
			esac
			;;
		.sn)	for d in "${dirs[@]}"
			do	if	[[ -f "$d$1" ]]
				then	cat "$d$1"
					continue 2
				fi
			done
			warning "$1: $op cannot read"
			;;
		.sp|.SP)space
			;;
		.ta)	: ignore $op
			;;
		.ti)	: ignore $op
			;;
		.ul)	: ignore $op
			;;
		*)	warning "$op: unknown op"
			;;
		esac
		;;
	""|[\ \	]*)
		case $macros in
		man)	leading=1
			prefix="[code]"
			blank=
			while	:
			do	case $1 in
				*([\ \	]))
					case $leading in
					1)	leading= ;;
					*)	blank=$'\n' ;;
					esac
					;;
				[\ \	]*)
					 puttext "$prefix$blank$inch$indent$*"
					blank=
					leading=
					prefix=
					;;
				*)	(( peek = 1 ))
					break
					;;
				esac
				getline || break
			done
			case $prefix in
			"")	putop "[/code]" ;;
			esac
			;;
		*)	puttext "$indent$*"
			;;
		esac
		;;
	*)	puttext "$indent$*"
		;;
	esac
done
flush
case $references in
?*)	heading .H 1 References
	puttext "$references"
	flush
	;;
esac
exit $code
