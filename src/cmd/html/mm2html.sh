####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1996-2001 AT&T Corp.                #
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
: mm2html - convert mm/man subset to html

# it keeps going and going ...
#
# \h'0*\w"URL"'HOT-TEXT\h'0'	link goto with alternate url
# \h'0*1'HOT-TEXT\h'0'		link goto
# \h'0/\w"LABEL"'TEXT\h'0'	local link label with alternate text
# \h'0/1'LABEL\h'0'		local link label
#
# .xx meta.NAME="CONTENT"	<meta name="NAME" content="CONTENT">
# .xx label="LABEL"		local link label request
# .xx link="URL\tHOT-TEXT"	link goto with url request
# .xx link="HOT-TEXT"		link goto request
# .xx ref="URL\tMIME-TYPE"	head link hint
# .xx begin=internal		begin internal text
# .xx end=internal		end internal text
#
# .sn file			like .so but text copied to output

command=mm2html
version='mm2html (AT&T Labs Research) 2001-01-01'
case $(getopts '[-][123:xyz]' opt --xyz 2>/dev/null; echo 0$opt) in
0123)	ARGV0="-a $command"
	USAGE=$'
[-?
@(#)'$version$'
]
'$USAGE_LICENSE$'
[+NAME?mm2html - convert mm/man subset to html]
[+DESCRIPTION?\bmm2html\b is a \bsed\b(1)/\bksh\b(1) script (yes!) that
	converts input \bmm\b(1) or \bman\b(1) documents to an \bhtml\b
	document on the standard output. If \afile\a is omitted then the
	standard input is read. \btroff2html\b(1) is similar but does a full
	\btroff\b(1) parse.]
[f:frame?Generate framed HTML files in:]:[name]{
	[+\aname\a\b.html?The main body.]
	[+\aname\a\b-index.html?The frame index (if \b--index\b specified).]
	[+\aname\a\b-temp.html?Temporary frame goto labels.]
}
[g:global-index?Generate a standalone \bindex.html\b for framed HTML.]
[h:html?Read html options from \afile\a. Unknown options
	are silently ignored. See the \b.xx\b request below for a description
	of the options. The file pathname may be followed by URL style
	\aname=value\a pairs that are evaluated as if they came from
	\afile.\a]:[file[??name=value;...]]]
[l:license?Read license identification options from \afile\a. Unknown options
	are silently ignored. See the \b.xx\b request below for a description
	of the options. The file pathname may be followed by URL style
	\aname=value\a pairs that are evaluated as if they came from
	\afile\a.]:[file[??name=value;...]]]
[o:option?Sets a space or \b,\b separated list of \b--license\b options. Option
	values with embedded spaces must be quoted.]:[[no]]name=value]
[x:index?Generate a standalone \aname\a\b-index.html\b for framed HTML where
	\aname\a is specified by \b--frame\b.]

[ file ... ]

[+EXTENSIONS?\b.xx\b \aname\a[=\avalue\a]] is a special \bmm2html\b
	request that handles program tracing, \bhtml\b extensions and \atroff\a
	macro package magic. Supported operations are:]{
	[+author=text?Specifies the contact information for the document
		HEAD section.]
	[+background=URL?Specifies the document background URL.]
	[+logo=URL?Specifies the logo/banner image URL that is centered
		at the top of the document.]
	[+mailto=address?Sets the email \aaddress\a to send comments and
		suggestions.]
	[+meta.name?Emits the \bhtml\b tag \b<META name=\b\aname\a
		\bcontent=\b\acontent\a\b>\b.]
	[+package=text?\atext\a is prepended to the \bhtml\b document title.]
	[+title=text?Sets the document title.]
}
[+?Local URL links are generated for all top level headings. These can be
	referenced by embedding the benign (albeit convoluted) \atroff\a
	construct \\h'\'$'0*\\w\"label\"'\'$'text\\h'\'$'0'\'$', where
	\alabel\a is the local link label and \atext\a is the hot link text.
	If \alabel\a and \atext\a are the same then use
	\\h'\'$'0*1'\'$'text\\h'\'$'0'\'$'.]
[+?\bman\b(1) links are generated for bold or italic identifiers that are
	immediately followed by a parenthesized number.]
[+SEE ALSO?\btroff2html\b(1), \bhtml2rtf\b(1)]
'
	;;
*)	ARGV0=""
	USAGE='i:[file] [ file ... ]'
	;;
esac

set -o noglob

integer count fd=0 head=2 line=0 lists=0 nest=0 peek=0 pp=0 so=0 soff=4 row n s
integer labels=0 reference=1
typeset -Z2 page=01
typeset -u upper
typeset -x -l OP
typeset -x -A ds map nr outline
typeset cond frame label list prev text trailer type
typeset license html meta nl mm index authors vg header references ss
typeset mm_AF mm_AF_cur mm_AF_old mm_AU

nl=$'\n'

license=(
	author=
)
html=(
	BODY=(
		bgcolor=#ffffff
	)
	TABLE=(
		align=left
		bgcolor=#ffd0d0
		bordercolor=#ffffff
		border=0
	)
	H1=(
		align=center
	)
	FRAMESET=(
		rows=80,*
		border=0
		frameborder=0
		framespacing=0
	)
	home=(
		href=
	)
	ident=1
	index=(
		left=
		top=
	)
	logo=(
		src=
	)
	magic=(
		plain='<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN" "http://www.w3.org/TR/REC-html40/loose.dtd">'
		frame='<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Frameset//EN" "http://www.w3.org/TR/REC-html40/frameset.dtd">'
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
ifs=$IFS
inch="     "
indent=
indent_prev=
macros=
pd='<P>'
pm=
primary=".BL|.LI|.IX"
ss="verdana,arial,helvetica,geneva,sans-serif"
vg_ps=20

function options
{
	typeset i o q v
	IFS="${IFS},"
	set $OPTARG
	IFS=$ifs
	for i
	do	case $q in
		'')	o=${i%%=*}
			v=${i#*=}
			case $v in
			\'*)	q=\'; v=${v#?}; continue ;;
			\"*)	q=\"; v=${v#?}; continue ;;
			esac
			;;
		*)	v="$v $i"
			case $i in
			*$q)	v=${v%?} ;;
			*)	continue ;;
			esac
			;;
		esac
		case $o in
		no*)	unset ${o#no}
			;;
		*)	case $v in
			"")	v=1 ;;
			esac
			case $o in
			*.*|[A-Z]*)
				eval $o="'$v'"
				;;
			*)	eval license.$o="'$v'"
				;;
			esac
			;;
		esac
	done
}

while	getopts $ARGV0 "$USAGE" OPT
do	case $OPT in
	f)	frame=$OPTARG
		;;
	g)	index=global
		;;
	h)	case $OPTARG in
		*\?*)	. ${OPTARG%%\?*} || exit 1
			eval "html=( ${OPTARG#*\?} )"
			;;
		*)	. $OPTARG || exit 1
			;;
		esac
		;;
	l)	case $OPTARG in
		*\?*)	. ${OPTARG%%\?*} || exit 1
			eval "license=( ${OPTARG#*\?} )"
			;;
		*)	. $OPTARG || exit 1
			;;
		esac
		;;
	o)	options "$OPTARG"
		;;
	x)	index=local
		;;
	*)	echo "Usage: $command [-x] [-f name] [-h file[?name=value;...]]
	       [-l file[?name=value;...]] [-o [no]name=value] [ file ... ]" >&2
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

HTMLPATH=
for i in $HOME ${PATH//:/ }
do	d=${i%/bin}/lib/html
	if	[[ -d $d ]]
	then	if	[[ -w $d || -x $i/$command ]]
		then	HTMLPATH=$HTMLPATH:$d
			if	[[ -x $i/$command ]]
			then	break
			fi
		fi
	fi
done
HTMLPATH=${HTMLPATH#:}

ds[Cr]='&#169;'
ds[Dt]=$(date -f "%B %d, %Y" $x)
ds[Rf]="\\u[$reference]\\d"
ds[Rg]='&#174;'
ds[CM]='&#169;'
ds[RM]='&#174;'
ds[SM]='<FONT SIZE=-6><B><SUP>SM</SUP></B></FONT>'
ds[TM]='<FONT SIZE=-6><B><SUP>TM</SUP></B></FONT>'

map[.Cs]=.EX
map[.Ce]=.EE
map[.Ss]=.SS
map[.Tp]=.TP

H=H$(( head + 1 ))

function warning
{
	print -u2 "$command: warning: ${file:+"$file: "}line $line:" "$@"
}

function getline
{
	integer i n
	typeset data a c q v x
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
					[\"\<]) case $q in
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
					t|\'@(*)\'\1\'|+([\-+0-9])=\1)
						(( n = ! n ))
						;;
					+([\-+0-9])=+([\-+0-9]))
						;;
					[0-9]*[0-9])
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
					2':('[0-9]')'*([,.?!:;]))
						x=${2#'('*')'}
						y=${2%$x}
						n=$y
						case $op in
						.B*)	font1=STRONG ;;
						.L*)	font1=TT ;;
						*)	font1=EM ;;
						esac
						case $macros in
						man)	set -A text -- "<NOBR><A href=\"../man$n/$1.html\"><$font1>$1</$font1></A>$y$x</NOBR>" ;;
						*)	set -A text -- "<NOBR><A href=\"${html.man:=../man}/man$n/$1.html\"><$font1>$1</$font1></A>$y$x</NOBR>" ;;
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
				?*)	data="</$font>"
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
				B)	font1=STRONG ;;
				I)	font1=EM ;;
				[LMX])	font1=TT ;;
				R)	font1= ;;
				esac
				case $font2 in
				B)	font2=STRONG ;;
				I)	font2=EM ;;
				[LMX])	font2=TT ;;
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
					*"<FONT SIZE"*)
						case $font in
						"")	data="$data$1" ;;
						*)	data="$data<$font>$1</$font>" ;;
						esac
						;;
					*)	case "$1 $2" in
						*"<FONT SIZE"*)
							case $font in
							"")	data="$data$1 $2" ;;
							*)	data="$data<$font>$1 $2</$font>" ;;
							esac
							shift
							;;
						*)	case $font in
							"")	data="$data$1" ;;
							*)	data="$data<$font>$1</$font>" ;;
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
				*)	pd='<P>' ;;
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
				set -A text -- "<FONT SIZE=-2><B>""$@""</B></FONT>"
				;;
			.SM)	set -- "${text[@]}"
				shift
				case $# in
				0)	getline ;;
				esac
				set -A text -- "<FONT SIZE=-2>""$@""</FONT>"
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

function ident
{
	case $frame in
	'')	print -r -- "${html.magic.plain}" ;;
	*)	print -r -- "${html.magic.frame}" ;;
	esac
	print -r -- "<HTML>"
	print -r -- "<HEAD>"
	print -r -- "<META name=\"generator\" content=\"$version\">${meta}"
}

function title
{
	ident
	case $header in
	?*)	print -r -- "$header" ;;
	esac
	print -r -- "<TITLE>" $* "</TITLE>"
	case ${license.author} in
	?*)	print -r -- "<META name=\"author\" content=\"${license.author}\">" ;;
	esac
	case $vg in
	?*)	print -r -- "<SCRIPT language='javascript' src='../lib/slide.js'></SCRIPT>"
		case $pages in
		?*)	print -r -- "<SCRIPT language='javascript'>last_slide=$pages</SCRIPT>" ;;
		esac
		print -r -- "<STYLE>
BODY { font-family:times; font-size:$((vg_ps))pt; }
H1 { font-family:times; font-size:$((vg_ps+4))pt; }
H2 { font-family:times; font-size:$((vg_ps+2))pt; }
H3 { font-family:times; font-size:$((vg_ps))pt; }
TH { font-family:${ss}; font-size:$((vg_ps-1))pt; }
TD { font-family:${ss}; font-size:$((vg_ps-1))pt; }
#notes {
	position:relative;
	text-align:center;
	visibility:hidden;
	background:#bbbbbb;
}
#tutorial {
	position:relative;
	text-align:center;
	visibility:hidden;
	background:papayawhip;
}
</STYLE>
</HEAD>"
		return
		;;
	esac
	print -r -- "</HEAD>"
	case ${html.heading} in
	?*)	case ${html.heading} in
		?*)	html.toolbar=
			hit=
			if	[[ -f ${html.heading} ]]
			then	hit=${html.heading}
			elif	[[ -f $HOME/${html.heading} ]]
			then	hit=$HOME/${html.heading}
			else	ifs=$IFS
				IFS=:
				set "" $HOME $PATH
				IFS=$ifs
				for i
				do	if	[[ -f ${i%/bin}/lib/${html.heading} ]]
					then	hit=${i%/bin}/lib/${html.heading}
						break
					fi
				done
			fi
			case $hit in
			"")	print -u2 "$command: ${html.heading}: cannot read"
				code=1
				;;
			*)	eval "cat <<!
$(cat $hit)
!"
				;;
			esac
			;;
		esac
		;;
	*)	print -r -- "<BODY" ${html.BODY/'('@(*)')'/\1} ">"
		case ${html.width} in
		?*)	case ${html.width} in
			*%)	x="align=center " ;;
			*)	x="" ;;
			esac
			print -r -- "<TABLE border=0 ${x}width=${html.width}><TR><TD valign=top align=left>"
			trailer="$trailer$nl</TD></TR></TABLE>"
			;;
		esac
		case $frame in
		'')	logo=${html.logo/'('@(*)')'/\1}
			case ${html.ident}:${logo} in
			*:*"src=''"*|*:)
				;;
			1:?*)	case ${html.home.href} in
				?*)	html.home.href=${html.home.href%/*.html}/
					print -r -- "<A" ${html.home/'('@(*)')'/\1} "><IMG" ${logo} "></A>"
					;;
				*)	print -r -- "<IMG" ${logo} ">"
					;;
				esac
				;;
			esac
			;;
		esac
		print -r -- "<!--INDEX--><!--/INDEX-->"
		;;
	esac
	case $pm in
	?*)	print -r "$pm"
		trailer="$pm$nl$trailer"
		;;
	esac
	trailer="${trailer}${nl}</BODY>"
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
		options=" align=center"
		;;
	*)	count=2
		options=" align=center"
		;;
	esac
	case "$op $count" in
	".H"*" 1")print -n -r -- "<P><HR>" ;;
	esac
	case $* in
	"")	print -r -- "<P>"
		;;
	*)	eval o='$'{html.H$count}
		for i in $o
		do	case $i in
			align=center)
				beg="$beg<CENTER>"
				end="</CENTER>$end"
				;;
			color=*|face=*)
				beg="$beg<FONT $i>"
				end="</FONT>$end"
				;;
			esac
		done
		(( count += head ))
		print -nr -- "$beg<H$count$options>"
		case $count in
		[0123])	print -nr -- "<A name=\"$*\">$*</A>"
			label[labels++]=$*
			;;
		*)	print -nr "$*"
			;;
		esac
		print -r -- "</H$count>$end"
		;;
	esac
}

if	[[ $frame != '' ]]
then	framebody=$frame.html
	exec > $framebody || exit
fi
document="$@"

sed	\
	-e 's%\\".*%%' \
	-e 's%\\(>=%>=%g' \
	-e 's%\\(<=%<=%g' \
	-e 's%&%\&amp;%g' \
	-e 's%<%\&lt;%g' \
	-e 's%>%\&gt;%g' \
	-e 's%\[%\&#0091;%g' \
	-e 's%\]%\&#0093;%g' \
	-e 's%\\&amp;%<NULL>%g' \
	-e 's%\\'\''%'\''%g' \
	-e 's%\\`%`%g' \
	-e 's%\\-%\&#45;%g' \
	-e 's%\\+%+%g' \
	-e 's%\\0%\&nbsp;%g' \
	-e 's%\\|%\&nbsp;%g' \
	-e 's%\\\^%\&nbsp;%g' \
	-e 's%\\ %\&nbsp;%g' \
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
	-e 's%\\d\([^\\]*\)\\u%<SUB>\1</SUB>%g' \
	-e 's%\\u\([^\\]*\)\\d%<SUP>\1</SUP>%g' \
	-e 's%\\v\(.\)-\([^\\]*\)\1\(.*\)\\v\1+*\2\1%<SUB>\3</SUB>%g' \
	-e 's%\\v\(.\)+*\([^\\]*\)\1\(.*\)\\v\1-\2\1%<SUP>\3</SUP>%g' \
	-e 's%\\h'\''0\*\\w"\([a-z]*:[^"]*\)"'\''\([^'\'']*\)\\h'\''0'\''%<A href="\1" target=_top>\2</A>%g' \
	-e 's%\\h'\''0\*\\w"\(/[^"]*\)"'\''\([^'\'']*\)\\h'\''0'\''%<A href="\1" target=_top>\2</A>%g' \
	-e 's%\\h'\''0\*\\w"\([^"]*\)"'\''\([^'\'']*\)\\h'\''0'\''%<A href="\1">\2</A>%g' \
	-e 's%\\h'\''0\*1'\''\([^:/'\'']*\)\\h'\''0'\''%<A href="#\1">\1</A>%g' \
	-e 's%\\h'\''0\*1'\''\([a-z]*:[^'\'']*\)\\h'\''0'\''%<A href="\1" target=_top>\1</A>%g' \
	-e 's%\\h'\''0\*1'\''\(/[^'\'']*\)\\h'\''0'\''%<A href="\1" target=_top>\1</A>%g' \
	-e 's%\\h'\''0\*1'\''\([^'\'']*\)\\h'\''0'\''%<A href="\1" target=_parent>\1</A>%g' \
	-e 's%\\h'\''0/\\w"\([^"]*\)"'\''\([^'\'']*\)\\h'\''0'\''%<A name="\1">\2</A>%g' \
	-e 's%\\h'\''0/1'\''\([^'\'']*\)\\h'\''0'\''%<A name="\1">\1</A>%g' \
	-e 's%\\s+\(.\)\([^\\]*\)\\s-\1%<FONT SIZE=+\1>\2</FONT>%g' \
	-e 's%\\s+\(.\)\([^\\]*\)\\s0%<FONT SIZE=+\1>\2</FONT>%g' \
	-e 's%\\s-\(.\)\([^\\]*\)\\s+\1%<FONT SIZE=-\1>\2</FONT>%g' \
	-e 's%\\s-\(.\)\([^\\]*\)\\s0%<FONT SIZE=-\1>\2</FONT>%g' \
	-e 's%\\f(\(..\)\([^\\]*\)%<\1>\2</\1>%g' \
	-e 's%\\f[PR]%\\fZ%g' \
	-e 's%\\f\(.\)\([^\\]*\)%<\1>\2</\1>%g' \
	-e 's%&lt;\([a-zA-Z0-9_.]*@[a-zA-Z0-9_.]*\)&gt;%<SMALL>\&lt;<A href=mailto:\1>\1</A>\&gt;</SMALL>%g' \
	-e 's%\[[A-Z][A-Za-z]*[0-9][0-9][a-z]*]%<CITE>&</CITE>%g' \
	-e 's%</*Z>%%g' \
	-e 's%<[146-9]>%%g' \
	-e 's%</[146-9]>%%g' \
	-e 's%<2>%<EM>%g' \
	-e 's%</2>%</EM>%g' \
	-e 's%<3>%<STRONG>%g' \
	-e 's%</3>%</STRONG>%g' \
	-e 's%<5>%<TT>%g' \
	-e 's%</5>%</TT>%g' \
	-e 's%<B>%<STRONG>%g' \
	-e 's%</B>%</STRONG>%g' \
	-e 's%<I>%<EM>%g' \
	-e 's%</I>%</EM>%g' \
	-e 's%<L>%<TT>%g' \
	-e 's%</L>%</TT>%g' \
	-e 's%<X>%<TT>%g' \
	-e 's%</X>%</TT>%g' \
	-e 's%<CW>%<TT>%g' \
	-e 's%</CW>%</TT>%g' \
	-e 's%<EM>\([^<]*\)</EM>(\([0-9]\))%<NOBR><A href="../man\2/\1.html"><EM>\1</EM></A>\2</NOBR>%g' \
	-e 's%<STRONG>\([^<]*\)</STRONG>(\([0-9]\))%<NOBR><A href="../man\2/\1.html"><STRONG>\1</STRONG></A>\2</NOBR>%g' \
	-e 's%<TT>\([^<]*\)</TT>(\([0-9]\))%<NOBR><A href="../man\2/\1.html"><TT>\1</TT></A>\2</NOBR>%g' \
	-e 's%\\s+\(.\)\(.*\)\\s-\1%<FONT SIZE=+\1>\2</FONT>%g' \
	-e 's%\\s-\(.\)\(.*\)\\s+\1%<FONT SIZE=-\1>\2</FONT>%g' \
	-e 's%\\c%<JOIN>%g' \
	-e 's%\\e%<NULL>\\<NULL>%g' \
	-e '/^\..*".*\\/s%\\%\\\\%g' \
"$@" | while	:
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
			"")	mm_AF_cur="<P><I>$*" ;;
			*)	mm_AF_cur="${mm_AF_cur}<BR>$*" ;;
			esac
			;;
		.AL|.[IR]S|.VL)
			case $macros:$op in
			mm:.RS)	Rf="\\u[$reference]\\d"
				references="$references$nl<DT>[$reference]<DD>"
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
			print -r -- "<DL COMPACT>"
			case $op in
			.AL)	case $1 in
				[0-9a-zA-Z])
					type[++lists]=.al
					list[lists]=OL
					print -r -- "<OL type=\"$1\">"
					;;
				esac
				;;
			.[IR]S)	print -r -- "<DT><DD>"
				;;
			esac
			;;
		.AS|.H|.HU|.SH|.SS|.ce)
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
							print -rn -- "</${list[lists--]}>"
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
			print -r -- "<P>"
			end=
			case ${mm.title} in
			?*)	print -r -- "<HR>"
				case ${mm_AU}${mm.author}${mm.keywords} in
				?*)	print -r -- "<CENTER>" ;;
				esac
				for i in ${html.H1}
				do	case $i in
					align=center)
						beg="$beg<CENTER>"
						end="</CENTER>$end"
						;;
					color=*|face=*)
						beg="$beg<FONT $i>"
						end="</FONT>$end"
						;;
					esac
				done
				print -r -- "<$H>$beg${mm.title}$end</$H>"
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
						print -r -- "$x<BR>"
					done
					;;
				esac
				case $mm_AU in
				?*)	print -r -- "$mm_AU"
					case $mm_AF_cur in
					?*)	mm_AF="${mm_AF_cur}</I>" ;;
					esac
					case $mm_AF in
					?*)	print -r -- "$mm_AF" ;;
					esac
					;;
				esac
				case ${mm_AU}${mm.author}${mm.keywords} in
				?*)	print -r -- "</CENTER>" ;;
				esac
				;;
			esac
			case $op in
			.AS)	print -r -- "<P>"
				heading $op Abstract
				;;
			.ce)	case $# in
				0)	count=1 ;;
				*)	count=$1 ;;
				esac
				print -r -- "<CENTER>"
				while	(( count-- > 0 )) && read -r data
				do	print -r -- "$data<BR>"
				done
				print -r -- "</PRE></CENTER>"
				;;
			*)	case $op in
				.SH|.SS)macros=man
					while	(( lists > 0 ))
					do	print -r -- "</${list[lists]}>"
						case ${type[lists--]} in
						.SH|$op)	break ;;
						esac
					done
					type[++lists]=$op
					list[lists]=DL
					print -r -- "<DL COMPACT>"
					beg="<DT>"
					end="<DD>"
					case $op in
					.SH)	set 2 "$@"
						;;
					.SS)	set 3 "$@"
						type[++lists]=.XX
						list[lists]=DL
						print -r -- "<DL>"
						;;
					esac
					op=.H
					;;
				*)	beg=
					end=
					;;
				esac
				heading $op "$@"
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
				mm_AF="${mm_AF_cur}</I>"
				mm_AF_cur=""
				;;
			esac
			mm_AU="${mm_AU}<BR>$1"
			;;
		.BL|.bL|.sL)
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
			print -r -- "<UL type=$i>"
			;;
		.BP)	i=${1%.*}.gif
			case $frame in
			?*)	[[ -f $frame-$i ]] && i=$frame-$i ;;
			esac
			print -r -- "<CENTER><IMG src=\"$i\"></CENTER>"
			if [[ ! -f $1 ]]
			then	print -u2 "$command: $1: data file not found"
			elif [[ $1 -nt $i ]]
			then	ps2gif $1 $i
			fi
			;;
		.DE|.dE|.fi)
			print -r -- "</PRE>"
			;;
		.DF|.DS|.dS|.nf)
			print -r -- "<PRE>"
			;;
		.DT)	case $macros in
			man)	;;
			*)	print -r -- "${ds[Dt]}" ;;
			esac
			;;
		.EE|.eE)print -r -- "</PRE>"
			indent=${indent#$inch}
			;;
		.EX|.eX)print -r -- "<PRE>"
			indent=$inch$indent
			;;
		.FE)	print -r -- '&nbsp;]&nbsp'
			;;
		.FG)	print -r -- "<H4 align=center>Figure $figure: $*</H4>"
			(( figure++ ))
			;;
		.FS)	print -r -- '&nbsp;[&nbsp'
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
							print -rn -- "</${list[lists--]}>"
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
			print -r -- "<P>"
			;;
		.HY)	: ignore $op
			;;
		.IP|.LI|.TF|.TP|.bI|.sI)
			case $op in
			.IP|.LP|.TF|.TP)OP=$op
				case ${type[lists]} in
				$op|$OP);;
				*)	type[++lists]=$op
					list[lists]=DL
					print -r -- "$pd<DL COMPACT>"
					case $1 in
					[0-9]*)	type[++lists]=.tp
						list[lists]=DL
						print -r -- "<DL COMPACT>"
						;;
					esac
					;;
				esac
				case $op in
				.IP|.LP|.TF)
					set -- "<TT>$*</TT>"
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
				print -r -- "<DT>$*<DD>"
				;;
			*)	case $op in
				.bI|.sI)print -r -- "<P>" ;;
				esac
				print -r -- "<LI>$*"
				;;
			esac
			;;
		.IX)	: ignore $op
			;;
		.LE|.[IR]E|.bE|.sE)
			case ${type[@]} in
			*.[Aa][Ll]*)
				while	(( lists > 0 ))
				do	print -r -- "</${list[lists]}>"
					case ${type[lists--]} in
					.AL)	break ;;
					esac
				done
				;;
			*)	warning "$op: no current list type"
				;;
			esac
			case $op:$pd in
			.[IR]E:?*)	print -r -- "$pd" ;;
			esac
			;;
		.LX)	: ignore $op
			;;
		.MT)	macros=mm
			;;
		.ND)	ds[Dt]=$*
			;;
		.NL)	type[++lists]=.AL
			list[lists]=OL
			print -r -- "<OL>"
			;;
		.OK)	mm.keywords="$*"
			;;
		.OP)	: .OP opt arg arg-append arg-prepend
			x="$4<STRONG>&#45;$1</STRONG><I>$2</I>"
			case $3 in
			'[]')	x="[ $x ]" ;;
			?*)	x="$x$3" ;;
			esac
			print -r -- "$x"
			;;
		.PM|.pM)case ${html.company} in
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
			?*)	
				case $op in
				.pM)	pm="<TABLE align=center cellpadding=2 border=4 bgcolor=lightgrey><TR><TD><FONT font face=\"${ss}\"><B>${pm}</B></FONT></TD></TR></TABLE>" ;;
				*)	pm="<HR><CENTER><$H>${pm}</$H></CENTER>" ;;
				esac
				;;
			esac
			;;
		.PU)	: ignore $op
			;;
		.SA)	: ignore $op
			;;
		.SU)	: ignore $op
			;;
		.TH|.TL): .TL junk junk
			: .TH item section
			case $macros:$op in
			:.TH)	macros=man ;;
			:.TL)	macros=mm ;;
			esac
			case ${html.title} in
			?*)	title=${html.title}
				;;
			*)	case $title in
				'')	title="$document $macros document" ;;
				esac
				title="$package $title"
				;;
			esac
			title $title
			case $op in
			.TH)	case $2 in
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
				print -r -- "<H3><TABLE width=100%><TR><TH align=left>$1($2)<TH align=center><A href=\"\" TITLE-\"Command Index\">$sec</A><TH align=right>$1($2)</TR></TABLE></H3>"
				print -r -- "<HR>"
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
		.TS)	ifs=$IFS
			(( row=0 ))
			unset opts
			eval "opts=${html.TABLE}"
			tab=$'\t'
			while	getline
			do	set -- ${@//[-\|_=]/ }
				case $# in
				0)	continue ;;
				esac
				(( row++ ))
				tbl[row]=$*
				case $1 in
				.TE)	continue 2 ;;
				esac
				case $* in
				*";")	(( row=0 ))
					for i in ${@//[,\;]/ }
					do	case $i in
						center)	opts.align=center
							;;
						expand)	opts.align=center opts.width="90%"
							;;
						*box)	opts.border=1
							;;
						linesize'('*')')
							opts.border=${i//*'('@(*)')'*/\1}
							;;
						tab'('*')'*)
							tab=${i//*'('@(*)')'*/\1}
							;;
						tab'(')	case $* in
							*'tab(,'*)	tab=',' ;;
							*'tab(;'*)	tab=';' ;;
							*'tab( '*)	tab=' ' ;;
							*)		tab=$'\t' ;;
							esac
							;;
						esac
					done
					;;
				*".")	break
					;;
				esac
			done
			case ${opts.border} in
			0)	opts.cellpadding=0
				opts.cellspacing=0
				unset opts.bgcolor
				;;
			*)	case ${opts.cellpadding} in
				'')	opts.cellpadding=2 ;;
				esac
				case ${opts.cellspacing} in
				'')	opts.cellspacing=2 ;;
				esac
				;;
			esac
			print -r -- "<TABLE" ${opts/'('@(*)')'/\1} ">"
			for ((n = 1; n < row; n++))
			do	getline || break
				case $1 in
				[_=]*)	((n--)); continue ;;
				esac
				print -rn -- "<TR>"
				IFS=$tab
				set -- $*
				IFS=$ifs
				set -A Q ${tbl[n]}
				(( q=0 ))
				(( s=1 ))
				while	:
				do	a= b= e=
					i=${Q[q++]}
					case $i in
					'')	break ;;
					*s*)	(( s++ )); continue ;;
					*c*)	a="$a align=center" ;;
					*l*)	a="$a align=left" ;;
					*r*)	a="$a align=right" ;;
					esac
					case $i in
					*b*)	b="<B>" e="</B>" ;;
					*i*)	b="<I>" e="</I>" ;;
					esac
					while	[[ ${Q[q]} == *s* ]]
					do	(( s++ ))
						(( q++ ))
					done
					if	(( s > 1 ))
					then	a="$a colspan=$s"
						(( s=1 ))
					fi
					print -rn -- "<TH$a>$b$1$e</TH>"
					case $# in
					0|1)	break ;;
					esac
					shift
				done
				print -r -- "</TR>"
			done
			prev=
			attr=${tbl[row]}
			while	getline
			do	case $1 in
				[_=]*)	continue ;;
				.TE)	break ;;
				esac
				IFS=$tab
				set -- $*
				IFS=$ifs
				case $* in
				*"\\")	prev=$prev$'\n'${*/"\\"/}
					;;
				*)	print -rn -- "<TR>"
					IFS=$'\t'
					set -- $prev$'\n'$*
					IFS=$ifs
					for i in $attr
					do	beg=
						end=
						pos=left
						case $i in
						*c*)	pos=center ;;
						*l*)	pos=left ;;
						*[nr]*)	pos=right ;;
						esac
						case $i in
						*b*)	beg="$beg<STRONG>" end="</STRONG>$end" ;;
						esac
						case $i in
						*i*)	beg="$beg<EM>" end="</EM>$end" ;;
						esac
						print -rn -- "<TD align=$pos>$beg$1$end</TD>"
						case $# in
						0|1)	break ;;
						esac
						shift
					done
					print -r -- "</TR>"
					prev=
					;;
				esac
			done
			print -r -- "</TABLE>"
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
		.br)	print -r -- "<BR>"
			;;
		.de)	case $op in
			.de)	end=..
				;;
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
			5|TT)	font=TT
				print -n -r -- "<$font>"
				;;
			[BI])	font=$1
				print -n -r -- "<$font>"
				;;
			*)	case $font in
				?*)	print -n -r -- "</$font>"
					font=
					;;
				esac
				;;
			esac
			;;
		.in)	indent_prev=$indent
			case $1 in
			"")	i=$indent_prev; indent_prev=$indent; indent=$i ;;
			-*)	indent=${indent#$inch} ;;
			+*)	indent=$inch$indent ;;
			*)	indent=$inch ;;
			esac
			;;
		.ll)	: ignore $op
			;;
		.ne)	: ignore $op
			;;
		.nr)	op=$1
			shift
			nr[$op]=$*
			;;
		.ps|.pS)case $1 in
			[-+][0-9])
				print -r -- "<FONT SIZE=$1>"
				;;
			esac
			;;
		.sh)	case $HTMLPATH in
			'')	;;
			*)	(( fd = so + soff ))
				file=/tmp/m2h$$
				eval PATH=$HTMLPATH "$*" > $file
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
		.sn)	if	[[ ! -f $1 ]]
			then	warning "$1: $op cannot read"
			else	cat "$1"
			fi
			;;
		.so)	if	[[ ! -f $1 ]]
			then	warning "$1: $op cannot read"
			else	(( fd = so + soff ))
				eval exec $fd'< $1'
				so_file[so]=$file
				file=$1
				so_line[so]=$line
				(( line = 0 ))
				(( so++ ))
			fi
			;;
		.sp|.SP)case $1 in
			[0-9]*)	count=$1
				while	(( count >= 0 ))
				do	(( count-- ))
					print -r -- "<BR>"
				done
				;;
			*)	print -r -- "<P>"
				;;
			esac
			;;
		.ta)	: ignore $op
			;;
		.ti)	: ignore $op
			;;
		.xx)	while	:
			do	case $# in
				0)	break ;;
				esac
				nam=${1%%=*}
				val=${1#*=}
				shift
				case $nam in
				begin|end)
					set -- $val
					case $nam in
					begin)	upper=$1 ;;
					end)	upper=/$1 ;;
					esac
					shift
					case $# in
					0)	print -r -- "<!--${upper}-->" ;;
					*)	print -r -- "<!--${upper} $@-->" ;;
					esac
					;;
				label|link|ref)
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
					label)	nam=name
						label[labels++]=$txt
						print -r -- "<A $nam=\"$url\">$txt</A>"
						;;
					link)	nam=href
						tar=
						case $frame in
						?*)	case $url in
							*([a-z]):*|/*)	tar=" target=_top" ;;
							esac
							;;
						esac
						if	[[ $frame != '' && $title == '' ]]
						then	rm $framebody
							framelink=$pfx$url
						else	print -r -- "<A $nam=\"$pfx$url\"$tar>$txt</A>"
						fi
						;;
					ref)	case $txt in
						$url)	x="<LINK href=\"$url\">" ;;
						*)	x="<LINK href=\"$url\" type=\"$txt\">" ;;
						esac
						case $framelink in
						'')	meta="$meta$nl$x" ;;
						*)	framerefs="$framerefs$nl$x" ;;
						esac
						;;
					esac
					;;
				meta.*)	meta="$meta$nl<META name=\"${nam#*.}\" content=\"$val\">"
					;;
				logo)	eval html.$nam.src='$'val
					;;
				logo*|title|[A-Z]*)
					eval html.$nam='$'val
					;;
				*)	eval license.$nam='$'val
					;;
				esac
			done
			;;
		.vG)	vg=$1
			set +o noglob
			rm -f [0-9][0-9].html index.html outline.html
			set -o noglob
			exec > $page.html
			outline[$page]=$2
			header='<BASEFONT face="geneva,arial,helvetica" size=5>'
			title "${2//\<*([!>])\>/}"
			print "<BODY bgcolor='#ffffff'>"
			print "${pm}<CENTER>"
			print "<BR><H1><FONT color=red>$2</FONT></H1><BR>"
			shift 2
			for name
			do	[[ $name == "-" ]] && name=""
				print -- "<BR>$name"
			done
			print "</CENTER>"
			print "<P>"
			;;
		.vH)	print -r -- "<CENTER>
<BR>
<H1><FONT color=red> $1 </FONT></H1>
<BR>
</CENTER>"
			;;
		.vP)	while	(( lists > 0 ))
			do	print -r -- "</${list[lists--]}>"
			done
			print -r -- "${trailer}
</HTML>"
			((page++))
			exec > $page.html
			outline[$page]=$1
			trailer=
			title "${1//\<*([!>])\>/}"
			print "<BODY bgcolor='#ffffff'>"
			print "${pm}<CENTER>"
			print "<BR><H1><FONT color=red>$1</FONT></H1>"
			print "</CENTER>"
			print "<P>"
			;;
		.nS)	print -r -- "<DIV id='notes'>"
			;;
		.tS)	print -r -- "<DIV id='tutorial'>"
			;;
		.nE|.tE)print -r -- "</DIV>"
			;;
		*)	warning "$op: unknown op"
			;;
		esac
		;;
	""|[\ \	]*)
		case $macros in
		man)	leading=1
			prefix="<TT><PRE>"
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
					print -r -- "$prefix$blank$inch$indent$*"
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
			"")	print -r -- "</PRE></TT>" ;;
			esac
			;;
		*)	print -r -- "$indent$*"
			;;
		esac
		;;
	*)	print -r -- "$indent$*"
		;;
	esac
done
while	(( lists > 0 ))
do	print -r -- "</${list[lists--]}>"
done
case $references in
?*)	heading .H 1 References
	print "<P>"
	print "<DL compact>"
	print -r -- "$references"
	print "</DL>"
	;;
esac
print -r -- "<HR>"
case ${html.ident} in
1)	case ${license.author} in
	?*)	IFS=',+'
		set -- ${license.author}
		IFS=$ifs
		n=0
		h=0
		for a
		do	((n++))
			v=${contributor[$a]}
			case $v in
			?*)	a=$v ;;
			esac
			authors[n]=$a
			IFS='<>'
			set -- $a
			IFS=$ifs
			case $2 in
			?*)	case $h in
				0)	h=1
					print -r "<P>Send comments and suggestions to "
					;;
				*)	print -r ", "
					;;
				esac
				print -rn "<A href=\"mailto:$2?subject=$title\">"
				set -- $1
				print -rn $*
				print -rn "</A>"
				;;
			esac
		done
		case $h in
		1)	print -r "." ;;
		esac
		;;
	*)	case ${html.MAILTO} in
		?*)	print -r "<P>Send comments and suggestions to <A href=\"mailto:${html.MAILTO}?subject=$title\">${html.MAILTO}</A>." ;;
		esac
		;;
	esac
	sp="<P>"
	for i in "${authors[@]}" "${license.organization}" "${license.corporation} ${license.company}" "${license.address}" "${license.location}" "${license.phone}"
	do	case $i in
		''|' ')	;;
		*)	print -r -- "$sp${i//\&/&amp\;}"; sp="<BR>" ;;
		esac
	done
	;;
esac
print -r -- "<P>"
print -r -- "${ds[Dt]}"
case ${html.footing} in
?*)	html.toolbar=
	hit=
	if	[[ -f ${html.footing} ]]
	then	hit=${html.footing}
	elif	[[ -f $HOME/${html.footing} ]]
	then	hit=$HOME/${html.footing}
	else	ifs=$IFS
		IFS=:
		set "" $HOME $PATH
		IFS=$ifs
		for i
		do	if	[[ -f ${i%/bin}/lib/${html.footing} ]]
			then	hit=${i%/bin}/lib/${html.footing}
				break
			fi
		done
	fi
	case $hit in
	"")	print -u2 "$command: ${html.footing}: cannot read"
		code=1
		;;
	*)	eval "cat <<!
$(cat $hit)
!"
		;;
	esac
	;;
esac
case ${html.toolbar} in
?*)	hit=
	if	[[ -f ${html.toolbar} ]]
	then	hit=${html.toolbar}
	elif	[[ -f $HOME/${html.toolbar} ]]
	then	hit=$HOME/${html.toolbar}
	else	ifs=$IFS
		IFS=:
		set "" $HOME $PATH
		IFS=$ifs
		for i
		do	if	[[ -f ${i%/bin}/lib/${html.toolbar} ]]
			then	hit=${i%/bin}/lib/${html.toolbar}
				break
			fi
		done
	fi
	case $hit in
	"")	print -u2 "$command: ${html.toolbar}: cannot read"
		code=1
		;;
	*)	eval "cat <<!
$(cat $hit)
!"
		;;
	esac
	;;
esac
trailer="$trailer$nl</HTML>"
print -r -- "$trailer"
case $frame in
?*)	if	[[ $framelink == '' && ${html.labels} != '' ]] && (( labels > 1 ))
	then
		exec > $frame-temp.html || exit
		print -r -- "<B><FONT font face=\"${ss}\">"
		print -r -- "<TABLE align=center cellpadding=2 border=4 bgcolor=lightgrey><TR>"
		for ((n = 0; n < labels; n++))
		do	print -r -- "<TD><A href=\"#${label[n]}\">${label[n]}</A></TD>"
		done
		print -r -- "</TR></TABLE>"
		print -r -- "</FONT></B>"
		exec > /dev/null || exit
		ed $framebody <<!
/<!--INDEX-->/r $frame-temp.html
w
q
!
		rm $frame-temp.html
	fi
	case $index in
	?*)	case $index in
		local)	exec > $frame-index.html || exit ;;
		global)	exec > index.html || exit ;;
		esac
		unset html.FRAMESET.rows html.FRAMESET.cols
		ident
		case $framelink in
		'')	framelink=$frame.html ;;
		esac
		print -r -- "<TITLE>$title</TITLE>$framerefs
</HEAD>
<FRAMESET" ${html.FRAMESET/'('@(*)')'/\1} ${html.index.top.FRAMESET/'('@(*)')'/\1} ">
	<FRAME" marginwidth=0 marginheight=0 ${html.FRAME/'('@(*)')'/\1} ${html.index.top.FRAME/'('@(*)')'/\1} "scrolling=no>
	<FRAME" marginwidth=0 marginheight=0 ${html.FRAME/'('@(*)')'/\1} src="'$framelink'" "scrolling=auto>
</FRAMESET>
</HTML>"
		;;
	esac
	;;
*)	if	[[ ${html.labels} != '' ]] && (( labels > 1 ))
	then
		print -r -- "<!--LABELS-->"
		print -r -- "<B><FONT font face=\"${ss}\">"
		print -r -- "<TABLE bordercolor=white cellpadding=0 cellspacing=0 width=90% border=0 align=center><TR>"
		for ((n = 0; n < labels; n++))
		do	if (( labels > 8 && (n & 7) == 7 ))
			then	print -r -- "</TR><TR>"
			fi
			print -r -- "<TD align=left><A href=\"#${label[n]}\">${label[n]}</A></TD>"
		done
		print -r -- "</TR></TABLE>"
		print -r -- "</FONT></B>"
		print -r -- "<!--/LABELS-->"
	fi
	;;
esac
case $vg in
?*)	frame=$vg
	trailer=
	pages=${page#0}
	exec > index.html
	title "${outline[01]//\<*([!>])\>/}"
	print -r -- "<FRAMESET marginheight=0 marginwidth=0 cols='135,*,66' border=0 noresize onload='goto_slide(1)'>
  <FRAME src=../lib/prev.html name='prev' noresize scrolling=no marginwidth=0 marginheight=0>
  <FRAME src=../lib/start.html name='slide' noresize scrolling=auto marginwidth=0 marginheight=0>
  <FRAME src=../lib/next.html name='next' noresize scrolling=no marginwidth=0 marginheight=0>
</FRAMESET>
</HTML>"
	exec > outline.html
	title "${title} outline"
	print -r -- "<BODY  bgcolor='#ffffff'>
${pm}<CENTER>
<BR><H1><FONT color=red>outline</FONT></H1>
<P>
<TABLE cellspacing=1 cellpadding=1 border=1 class=box>"
	for ((page=1; page <= pages; page++))
	do	print "<TR><TD><A href='javascript:goto_slide(${page#0})'><CENTER>${outline[$page]}</CENTER></A></TD></TR>"
	done
	print "</TABLE>
</CENTER>
</BODY>
</HTML>"
	;;
esac
exit $code
