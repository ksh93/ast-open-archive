################################################################
#                                                              #
#           This software is part of the ast package           #
#              Copyright (c) 1984-2000 AT&T Corp.              #
#      and it may only be used by you under license from       #
#                     AT&T Corp. ("AT&T")                      #
#       A copy of the Source Code Agreement is available       #
#              at the AT&T Internet web site URL               #
#                                                              #
#     http://www.research.att.com/sw/license/ast-open.html     #
#                                                              #
#     If you received this software without first entering     #
#       into a license with AT&T, you have an infringing       #
#           copy and cannot use it without violating           #
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
: ppcc [ options ] pp-command ... cc-command ...
#
# preprocess with pp-command then run cc-command
#
# @(#)ppcc (AT&T Research) 02/14/97
#
command=ppcc
clobber=
doti=
exec=1
linki=
multiopt=o
old=
post=
ppfile=
ppld=
ppstdout=
src1="*.[cCsS]"
src2="*.[cC][xX][xX]"
src3="*.[cC][pP][pP]"
src4="*.[aA][sS][mM]"
tmp=/tmp/pC$$
verbose=
usage="Usage: $command [ -nv ] [ -o dialect-option ] pp-command ... cc-command ..."
while	:
do	case $# in
	0)	break ;;
	esac
	case $1 in
	-o*)	# these are CC.DIALECT values from $(probe C make ...)
		case $1 in
		-o)	case $# in
			1)	echo "$usage" >&2
				exit 2
				;;
			esac
			shift
			opt=$1
			;;
		*)	case $RANDOM in
			$RANDOM)opt=`echo x$1 | sed s/...//` ;;
			*)	opt=${1#??} ;;
			esac
			;;
		esac
		case $opt in
		NO*|no*)value=
			case $RANDOM in
			$RANDOM)opt=`echo $opt | sed s/..//` ;;
			*)	opt=${1#??} ;;
			esac
			;;
		*)	case $opt in
			*=*)	case $RANDOM in
				$RANDOM)eval `echo $opt | sed 's/\\([^=]*\\)/opt=\\1 value/'` ;;
				*)	value=${opt#*=}
					opt=${opt%%=*}
					;;
				esac
				;;
			*)	value=1
				;;
			esac
			;;
		esac
		case $opt in
		DOTI|doti)
			doti=$value
			;;
		LINKI|linki)
			linki=$value
			;;
		MULTI|multi)
			case $value in
			"")	multiopt=
				;;
			*)	multiopt=$multiopt$value
				;;
			esac
			;;
		PPLD|ppld)
			ppld=$value
			;;
		exec)	exec=$value
			;;
		verbose)verbose=$value
			;;
		*)	# ignore the rest for fore/back compatibility
			;;
		esac
		;;
	-*)	case $1 in
		-*[!iklnvO]*)
			echo "$usage" >&2
			exit 2
			;;
		esac
		case $1 in
		*i*)	doti=1 ;;
		esac
		case $1 in
		*k*)	doti=1 linki=1 ;;
		esac
		case $1 in
		*l*)	ppld=1 ;;
		esac
		case $1 in
		*n*)	exec= ;;
		esac
		case $1 in
		*v*)	verbose=1 ;;
		esac
		case $1 in
		*O*)	old=1 ;;
		esac
		;;
	*)	break
		;;
	esac
	shift
done
case $# in
0)	echo "$usage" >&2
	exit 2
	;;
esac
trap '	status=$?
	trap "" 0
	case $clobber in
	?*)	case $verbose in
		1)	echo + rm -rf $clobber ;;
		esac
		case $exec in
		1)	rm -rf $clobber ;;
		esac
		;;
	esac
	case $post in
	?*)	case $verbose in
		1)	echo + $post ;;
		esac
		case $exec in
		1)	eval "$post" ;;
		esac
		;;
	esac
	exit $status
' 0 1 2 3 15
case $old:$RANDOM in
*:$RANDOM|1:*)
	# the hard way -- it has not kept up with the easy way below
	nfile=
	set '' "$@" '' "$1"
	while	:
	do	shift
		shift
		case $1 in
		[-+]*)	;;
		*)	break ;;
		esac
		set '' "$@" "$1"
	done
	while	:
	do	arg=$1
		shift
		case $arg in
		'')	break
			;;
		-[$multiopt])
			case $1 in
			?*)	set '' "$@" "$arg"
				shift
				arg=$1
				shift
				;;
			esac
			;;
		-E)	ppstdout=1
			doti=1
			trap 0 1 2 3 15
			;;
		-P)	ppfile=1
			doti=1
			trap 0 1 2 3 15
			;;
		[-+]*)	;;
		$src1|$src2|$src3|$src4)
			nfile=_$nfile
			eval input$nfile='$arg'
			case $ppstdout in
			"")	case $doti in
				"")	file=`echo $arg | sed -e "s,.*/,," -e "s,^,$tmp/,"`
					case $clobber in
					"")	clobber=$tmp
						case $verbose in
						1)	print -r -- + mkdir $tmp ;;
						esac
						case $exec in
						1)	mkdir $tmp || exit ;;
						esac
						;;
					esac
					;;
				*)	file=`echo $arg | sed -e "s,.*/,," -e "s,[.][^.]*,.i,"`
					clobber="$clobber $file"
					;;
				esac
				eval output$nfile='$file'
				;;
			esac
			;;
		esac
		set '' "$@" "$arg"
		shift
	done
	# sub-shell to split pp and cc arg lists
	(
	set '' "$@" '' "$1"
	while	:
	do	shift
		shift
		case $1 in
		[-+]*)	;;
		*)	shift
			break
			;;
		esac
		set '' "$@" "$1"
	done
	case $doti$ppstdout in
	"")	set '' "$@" -D-L
		shift
		;;
	esac
	shift
	while	:
	do	arg=$1
		shift
		case $arg in
		'')	break
			;;
		-[$multiopt])
			case $1 in
			?*)	shift ;;
			esac
			continue
			;;
		-[DIU]*|-[a-zA-Z][a-zA-Z0-9_]*)
			;;
		[-+]*)	continue
			;;
		$src1|$src2|$src3|$src4)
			break
			;;
		esac
		set '' "$@" "$arg"
		shift
	done
	case $nfile in
	?*)	while	:
		do	arg=$1
			shift
			case $arg in
			'')	break ;;
			esac
		done
		n=
		while	:
		do	case $n in
			$nfile)	break ;;
			esac
			n=_$n
			eval input='$input'$n
			case $ppstdout in
			"")	eval output='$output'$n
				case $verbose in
				1)	echo + "$@" "$input" "$output" ;;
				esac
				case $exec in
				1)	"$@" "$input" "$output" || exit ;; 
				esac
				;;
			*)	case $verbose in
				1)	echo + "$@" "$input" ;;
				esac
				case $exec in
				1)	"$@" "$input" || exit ;; 
				esac
				;;
			esac
		done
		;;
	esac
	) || exit
	case $ppfile$ppstdout in
	?*)	exit ;;
	esac
	while	:
	do	shift
		case $1 in
		[-+]*)	;;
		*)	break ;;
		esac
	done
	set '' "$@" '' "$1"
	shift
	shift
	n=
	while	:
	do	arg=$1
		shift
		case $arg in
		'')	break
			;;
		-[$multiopt])
			case $1 in
			?*)	set '' "$@" "$arg"
				shift
				arg=$1
				shift
				;;
			esac
			;;
		-[DIU]*)case $ppld in
			"")	continue ;;
			esac
			;;
		[-+]*)	;;
		$src1|$src2|$src3|$src4)
			n=_$n
			eval arg='$output'$n
			;;
		esac
		set '' "$@" "$arg"
		shift
	done
	case $verbose in
	1)	echo + "$@" ;;
	esac
	case $exec in
	1)	"$@" ;; 
	esac
	;;
*)	integer npp=1 ncc=1 nav=1 xav=$# oav
	set -A av "$@"
	pp[0]=${av[0]}
	while	(( nav < xav ))
	do	arg=${av[nav]}
		(( nav = nav + 1 ))
		case $arg in
		[-+]*)	;;
		*)	break ;;
		esac
		pp[npp]=$arg
		(( npp = npp + 1 ))
	done
	case $doti in
	"")	pp[npp]=-D-L
		(( npp = npp + 1 ))
		;;
	esac
	cc[0]=$arg
	while	(( nav < xav ))
	do	arg=${av[nav]}
		(( nav = nav + 1 ))
		case $arg in
		-[$multiopt]|-[Qq]dir|-[Qq]option|-[Qq]path|-[Qq]produce)
			case $arg in
			-[Qq]option)
				(( oav = nav + 2 ))
				;;
			*)	(( oav = nav + 1 ))
				;;
			esac
			if	(( oav > xav ))
			then	(( oav = xav ))
			fi
			while	(( nav < oav ))
			do	cc[ncc]=$arg
				(( ncc = ncc + 1 ))
				arg=${av[nav]}
				(( nav = nav + 1 ))
			done
			;;
		-[DIU]*|-[a-zA-Z][a-zA-Z0-9_]*)
			pp[npp]=$arg
			(( npp = npp + 1 ))
			case $ppld in
			"")	case $arg in
				-[DIU]*) continue ;;
				esac
				;;
			esac
			case $arg in
			-D-*)	continue ;;
			esac
			;;
		-E)	ppstdout=1
			doti=1
			trap 0 1 2 3 15
			continue
			;;
		-P)	ppfile=1
			doti=1
			trap 0 1 2 3 15
			continue
			;;
		-#*|-dryrun|--dryrun|-verbose|--verbose)
			verbose=1
			;;
		[-+]*)	;;
		$src1|$src2|$src3|$src4)
			case $ppstdout in
			"")	out=${arg##*/}
				case $doti in
				1)	out=${out%.*}.i
					clobber="$clobber $out"
					if	test -L $out 2>/dev/null
					then	case $verbose in
						1)	echo + rm -f $out ;;
						esac
						case $exec in
						1)	rm -f $out ;;
						esac
					fi
					case $linki:$ppfile in
					1:)	post="$post ln -s $arg $out;" ;;
					esac
					;;
				*)	case $clobber in
					"")	clobber=$tmp
						case $verbose in
						1)	print -r -- + mkdir $tmp ;;
						esac
						case $exec in
						1)	mkdir $tmp || exit ;;
						esac
						;;
					esac
					out=$tmp/$out
					;;
				esac
				pp[npp+1]=$out
				;;
			esac
			pp[npp]=$arg
			arg=$out
			case $verbose in
			1)	print -r -- + "${pp[@]}" ;;
			esac
			case $exec in
			1)	"${pp[@]}" || exit ;;
			esac
			;;
		esac
		cc[ncc]=$arg
		(( ncc = ncc + 1 ))
	done
	case $ppfile$ppstdout in
	"")	case $verbose in
		1)	print -r -- + "${cc[@]}" ;;
		esac
		case $exec in
		1)	"${cc[@]}" ;;
		esac
		;;
	esac
esac
