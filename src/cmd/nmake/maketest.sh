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
: maketest op ...
# @(#)maketest (AT&T Bell Laboratories) 03/01/94

case $RANDOM in
$RANDOM)	exec ksh $0 "$@"; echo "$0: ksh required" >&2; exit 1 ;;
esac

TEST_data=TEST
TEST_junk='core ptrepository'

unset MAKE_OPTIONS

env="\
	fs3d		INSTALLROOT	MAKEPATH	VPATH		\
	PWD		TOP		FS3D		COSHELL		\
	COATTRIBUTES	NPROC		MAKEARGS	MAKEFILES	\
	MAKEPP		MAKEPPFLAGS	MAKERULES	MAKERULESPATH	\
	MAKECONVERT	MAKEIMPORT	MAKELIB		PPFLAGS		\
	VOFFSET	VROOT	MAKE_OPTIONS	PS3		PS4		\
"

function testpath # var val
{
	path=
	ifs=$IFS
	IFS=:
	eval dirs='"'$2'"'
	IFS=$ifs
	for dir in $dirs
	do	if	test ! -d $dir
		then	print -u2 $command: warning: $TEST_name: $dir: invalid $1 directory
			continue
		fi
		cd $dir
		dir=$PWD
		cd $pwd
		case $dir in
		$TEST_root)
			case $TEST_pwd in
			.)	dir='$'PWD
				;;
			*)	dir='$'PWD/$TEST_pwd
				;;
			esac
			;;
		$TEST_root/*)
			dir='$'PWD${dir#$TEST_root}
			;;
		..|../*);;
		*)	print -u2 $command: warning: $TEST_name: $dir: $1 dir must be subdir of $TEST_root
			continue
			;;
		esac
		case $path in
		"")	path=$dir
			;;
		*)	path=$path:$dir
			;;
		esac
	done
}

function listfiles
{
	typeset args dir item keep op
	while	read op args
	do	case $op in
		attr)	set $args
			case $1 in
			perm)	keep=1 ;;
			esac
			;;
		bind)	case $item in
			?*)	case $keep in
				"")	case $src in
					*"
$item/"*)
						keep=1
						;;
					esac
					;;
				esac
				case $keep in
				"")	case $item in
					*/*)	dir=${item%/*}
						case "
$gen
" in
						*"
$dir
"*)
							item=
							;;
						esac
						;;
					esac
					case $item in
					?*)	gen="$gen
$item"
						;;
					esac
					;;
				*)	src="$src
$item"
					;;
				esac
				keep=
				;;
			esac
			case $op in
			file)	keep=1 ;;
			esac
			set $args
			case $1 in
			"("*")"|/*)
				item=
				;;
			*)	case $2 in
				"")	item=$1 ;;
				*)	item=$2 ;;
				esac
				case "
$gen
$src
" in
				*"
$item
"*)
					item=
					;;
				esac
				;;
			esac
			;;
		*)	case $item in
			?*)	case $keep in
				"")	case $src in
					*"
$item/"*)
						keep=1
						;;
					esac
					;;
				esac
				case $keep in
				"")	case $item in
					*/*)	dir=${item%/*}
						case "
$gen
" in
						*"
$dir
"*)
							item=
							;;
						esac
						;;
					esac
					case $item in
					?*)	gen="$gen
$item"
						;;
					esac
					;;
				*)	src="$src
$item"
					;;
				esac
				keep=
				;;
			esac
			keep=
			item=
			;;
		esac
	done
}

function nope # message
{
	print "===== test $TEST_name $num $* ====="
	status=1
}

command=maketest

case $1 in
# run tests in subdirs
all)	shift
	case $# in
	0)	set run ;;
	esac
	if	test -x $0
	then	cmd=$0
	else	cmd="$SHELL $0"
	fi
	for dir in */$TEST_data
	do	dir=${dir%/$TEST_data}
		cd $dir
		$cmd "$@"
		cd ..
	done
	exit
	;;
# initialize this test root
ini*)	case $2 in
	?*)	dir=$2
		if	test ! -d $dir
		then	case $dir in
			*.+([0-9]))
				;;
			*)	set -- $dir.+([0-9])
				shift $#-1
				case $1 in
				*.+([0-9]))
					dir=$dir.$((${1##*.}+1))
					;;
				*)	dir=$dir.1
					;;
				esac
				;;
			esac
			print cd $dir
			mkdir $dir
		fi
		cd $dir
		;;
	esac
	test -d $TEST_data || mkdir $TEST_data
	exit
	;;
-*)	print -u2 Usage: $command "accept|all|compare|exec|gen|init|list|new|out|run|save|src|verbatim|*make*"
	exit 2
	;;
esac

TEST_root=$PWD
TEST_pwd=.
TEST_top=.
while	:
do	case $TEST_root in
	"")	print -u2 $command: not initialized: run '``'$command init"''"
		exit 1
		;;
	esac
	if	test -d $TEST_root/$TEST_data
	then	break
	fi
	case $TEST_pwd in
	.)	TEST_pwd=${TEST_root##*/}
		TEST_top=..
		;;
	*)	TEST_pwd=${TEST_root##*/}/$TEST_pwd
		TEST_top=$TEST_top/..
		;;
	esac
	TEST_root=${TEST_root%/*}
done
TEST_name=${TEST_root##*/}

op=$1
case $1 in
"")	op=nmake ;;
*make*)	;;
*)	shift ;;
esac

typeset -Z4 num i
TEST_data=$TEST_root/$TEST_data
gen=
src=

case $op in
# accept $TEST_data/new.* by renaming to $TEST_data/old.*
# gen list generated files
# src list source files
# list list gen and src files
acc*|gen*|lis*|src*)
	if	test ! -d $TEST_data -o ! -f $TEST_data/new.0001
	then	print -u2 $command: $TEST_name: no new tests
		exit 1
	fi
	cd $TEST_root
	case $op in
	acc*)	rm -f $TEST_data/old.????
		if	test -f $TEST_data/new.tst
		then	mv $TEST_data/new.tst $TEST_data/tst
		fi
		if	test ! -f $TEST_data/run
		then	print $command run '"$@"' > $TEST_data/run
		fi
		;;
	esac
	for file in $TEST_data/readme
	do	if	test -f $file
		then	case "
$src
" in
			*"
$file
"*)
				;;
			*)	src="$src
$file"
				;;
			esac
		fi
	done
	src="$src
$TEST_data/run
$TEST_data/gen
$TEST_data/src"
	if	test -f $TEST_data/tst
	then	src="$src
$TEST_data/tst"
		exec < $TEST_data/tst
		while	read num args
		do	set -- $args
			while	:
			do	case $1 in
				pwd=*)	eval $1
					break
					;;
				*=*)	shift
					continue
					;;
				*)	continue 2
					;;
				esac
			done
			case "
.
$src
" in
			*"
$pwd
"*)
				;;
			*)	src="$src
$pwd"
				;;
			esac
		done
	fi
	for new in $TEST_data/new.????
	do	old=$TEST_data/old${new#$TEST_data/new}
		exec < $new
		case $op in
		acc*)	mv $new $old ;;
		esac
		src="$src
$old"
		listfiles
	done
	case $op in
	acc*)	print "$gen" > $TEST_data/gen
		print "$src" > $TEST_data/src
		;;
	gen*)	print $gen
		;;
	lis*)	print src = $src
		print gen = $gen
		;;
	src*)	print $src
		;;
	esac
	;;
# compare old.* with new.*
com*)	if	test ! -d $TEST_data
	then	print -u2 $command: no tests to compare
		exit 1
	fi
	cd $TEST_data
	for new in new.????
	do	old=old${new#new}
		if	test ! -f "$old"
		then	print -u2 "$command: warning: $old: results not found"
		elif	cmp -s $new $old
		then	: ok
		else	print "===== test $TEST_name ${new#new.} failed ====="
			diff $new $old
		fi
	done
	;;
# prepare for new round of tests by clobbering $TEST_data/new.*
new*)	if	test -d $TEST_data
	then	cd $TEST_root
		gen="$TEST_data/num $TEST_data/new.tst"
		for file in $TEST_data/new.???? $TEST_data/old.????
		do	if	test ! -f $file
			then	continue
			fi
			exec < $file
			case $file in
			$TEST_data/new.*)	gen="$gen $file" ;;
			esac
			listfiles
		done
		for file in $TEST_data/*tst
		do	if	test ! -f $file
			then	continue
			fi
			exec < $file
			case $file in
			$TEST_data/new.*)	gen="$gen $file" ;;
			esac
			while	read num args
			do	while	:
				do	case $1 in
					pwd=*)	pwd=${1#'pwd='}
						first=1
						for file in $TEST_junk
						do	case " $gen " in
							*" $pwd/$file "*)
								;;
							*)	gen="$gen $pwd/$file"
								case $first in
								?*)	first=
									gen="$gen $pwd/*.m[los] $pwd/make[0-9][0-9][0-9]*[a-z]"
									;;
								esac
								;;
							esac
						done
						break
						;;
					*=*)	;;
					*)	break
						;;
					esac
				done
			done
		done
		first=1
		for file in $TEST_junk
		do	case " $gen " in
			*" $file "*)
				;;
			*)	gen="$gen $file"
				case $first in
				?*)	first=
					gen="$gen *.m[los] make[0-9][0-9][0-9]*[a-z]"
					;;
				esac
				;;
			esac
		done
		rm -rf $gen
	fi
	;;
# examine test output
out*)	if	test ! -d $TEST_data
	then	print -u2 $command: $TEST_name: no test output
		exit 1
	fi
	cd $TEST_root
	both=
	case $1 in
	"")	if	test -f $TEST_data/new.0001
		then	type=new
		else	type=old
		fi
		;;
	n*)	type=new
		;;
	o*)	type=old
		;;
	b*)	type=new
		both=1
		;;
	*)	print -u2 $command: $TEST_name: $op: $1: new,old,both expected
		exit 1
		;;
	esac
	for file in $TEST_data/$type.????
	do	if	test ! -f $file
		then	print -u2 $command: $TEST_name: no $type test output
			exit 1
		fi
		{
		print -- ===== test $TEST_name ${file#$TEST_data/} =====
		egrep -v '^(make|done|attr|bind|test|info|setv) ' $file
		case $both in
		?*)	file=$TEST_data/old${file#$TEST_data/new}
			print -- ===== test $TEST_name ${file#$TEST_data/} =====
			egrep -v '^(make|done|attr|bind|test|info|setv) ' $file
			;;
		esac
		} | sed -e 's/^exec //' -e 's/^\.\.\.\. //'
	done
	;;
# run all tests and compare with previous results
run)	if	test ! -f $TEST_data/tst
	then	print -u2 $command: $TEST_name: no tests to run
		exit 1
	fi
	cd $TEST_root
	if	test -f $TEST_data/gen
	then	rm -rf $(<$TEST_data/gen)
	fi
	exec < $TEST_data/tst
	case $# in
	0)	set nmake ;;
	esac
	case $1 in
	make)	shift; set nmake "$@" ;;
	esac
	set -A cmd "$@"
	status=0
	while	read num args
	do	cd $TEST_root
		eval set -- $args
		unset $env
		pwd=.
		top=.
		while	:
		do	case $1 in
			[a-z]*=*)
				eval $1
				shift
				;;
			*=*)	export $1
				shift
				;;
			*)	break
				;;
			esac
		done
		case $pwd in
		""|.)	;;
		*)	if	test ! -d $pwd
			then	nope dir $pwd not found
				continue
			fi
			cd $pwd
			vars="$vars PWD=$pwd"
			case $top in
			""|.)	;;
			*)	export INSTALLROOT=$top ;;
			esac
			;;
		esac
		case $INSTALLROOT in
		"")	export INSTALLROOT=. ;;
		esac
		op=$1
		shift
		{
		print -u2 test name $num pwd $pwd
		case $op in
		eval|exec)
			print -u2 exec "$@"
			eval "$@"
			;;
		*)	"${cmd[@]}" "$@" -o "mam=regress:/dev/stderr::$top"
			;;
		esac
		print -u2 test status $?
		case $op in
		eval|exec)
			;;
		*)	if	test -f core
			then	case $top in
				.)	;;
				*)	mv core $top ;;
				esac
			fi
			for file in *.ml
			do	if	test -f $file
				then	rm -f $file
					print -u2 test error file $file
				fi
			done
			;;
		esac
		} < /dev/null > $TEST_data/new.$num 2>&1
		if	test ! -f $TEST_data/old.$num
		then	nope old results not found
		elif	cmp -s $TEST_data/new.$num $TEST_data/old.$num
		then	: ok
		else	nope changed
			diff $TEST_data/new.$num $TEST_data/old.$num
		fi
	done
	;;
# save all related test files in stdout sharchive
sav*)	if	test ! -d $TEST_data
	then	print -u2 $command: $TEST_name: no tests to save
		exit 1
	fi
	cd $TEST_root
	title=": maketest"
	case $# in
	0)	;;
	?*)	title="$title : $*" ;;
	esac
	title="$title : ${USER:-${LOGNAME:-${HOME##*/}}}@$( (uname -n || hostname || cat /etc/whoami) 2> /dev/null ) : $(date)"
	{
	print $title
	ifs=$IFS
	dirs=". .."
	for file in $(<$TEST_data/src)
	do	if	test -d "$file"
		then	dir=$file
		elif	test -f $file
		then	case $file in
			*/*)	dir=${file%/*} ;;
			*)	dir= ;;
			esac
		else	print -u2 $command: warning: $TEST_name: $file: not found
			continue
		fi
		IFS=/
		set "" $dir
		IFS=$ifs
		path=
		while	:
		do	shift
			case $# in
			0)	break ;;
			esac
			case $path in
			"")	path=$1 ;;
			*)	path=$path/$1 ;;
			esac
			case " $dirs " in
			*" $path "*|*/.|*/..)
				;;
			*)	print "mkdir $path"
				dirs="$dirs $path"
				;;
			esac
		done
		if	test -f $file
		then	sep="@//E*O*F $file//"
			print "sed 's/^@//' > \"$file\" <<'$sep'"
			sed -e 's/^[.~@]/@&/' -e 's/^From/@&/' "$file"
			print $sep
		fi
	done
	print "exit 0"
	}
	;;
# trace all tests verbatim (no special options)
ver*)	if	test ! -f $TEST_data/tst
	then	print -u2 $command: $TEST_name: no tests to run
		exit 1
	fi
	cd $TEST_root
	if	test -f $TEST_data/gen
	then	rm -rf $(<$TEST_data/gen)
	fi
	exec < $TEST_data/tst
	case $# in
	0)	set nmake ;;
	esac
	case $1 in
	make)	shift; set nmake "$@" ;;
	esac
	set -A cmd "$@"
	status=0
	while	read num args
	do	cd $TEST_root
		eval set -- $args
		unset $env
		pwd=.
		top=.
		vars=
		while	:
		do	case $1 in
			[a-z]*=*)
				eval $1
				shift
				;;
			*=*)	export $1
				vars="$vars $1"
				shift
				;;
			*)	break
				;;
			esac
		done
		case $pwd in
		""|.)	;;
		*)	if	test ! -d $pwd
			then	nope dir $pwd not found
				continue
			fi
			cd $pwd
			vars="$vars PWD=$pwd"
			case $top in
			""|.)	;;
			*)	export INSTALLROOT=$top ;;
			esac
			;;
		esac
		case $INSTALLROOT in
		"")	export INSTALLROOT=. ;;
		esac
		op=$1
		shift
		case $op in
		eval|exec)
			print -u2 ===== exec "$@"$vars
			eval "$@"
			;;
		*)	print -u2 ===== "${cmd[@]}" "$@"$vars
			"${cmd[@]}" "$@"
			;;
		esac
	done
	;;
# generate next test
eval|exec|*mak*)
	case $1 in
	make)	shift; set -- nmake "$@" ;;
	esac
	TEST_makepath=
	case " $*" in
	*" MAKEPATH="*)
		;;
	*)	case $MAKEPATH in
		?*)	testpath MAKEPATH $MAKEPATH
			TEST_makepath=$path
			;;
		esac
		;;
	esac
	TEST_vpath=
	case " $*" in
	*" VPATH="*)
		;;
	*)	case $VPATH in
		?*)	testpath VPATH $VPATH
			TEST_vpath=$path
			;;
		esac
		;;
	esac
	dirs=$(vpath 2> /dev/null)
	case $dirs in
	?*)	skip=
		for dir in $dirs
		do	case $skip in
			?*)	skip=
				continue
				;;
			esac
			case $dir in
			$TEST_root)
				dir='$'PWD/$TEST_pwd
				;;
			$TEST_root/*)
				dir='$'PWD/${dir#$TEST_root}
				;;
			*)	skip=1
				continue
				;;
			esac
			case $TEST_vpath in
			"")	TEST_view=$dir
				;;
			*)	TEST_view="$TEST_view $dir"
				;;
			esac
		done
		;;
	esac
	if	test ! -d $TEST_data
	then	mkdir $TEST_data || exit 1
	fi
	if	test ! -f $TEST_data/num -o ! -f $TEST_data/new.tst
	then	num=1
	else	((num=$(<$TEST_data/num)+1))
	fi
	print $num > $TEST_data/num
	{
	print -u2 test name $num pwd $TEST_pwd
	case $op in
	eval|exec)
		print -u2 exec "$@"
		eval "$@"
		;;
	*)	INSTALLROOT=$TEST_top "$@" -o "mam=regress::/dev/stderr:$TEST_top"
		;;
	esac
	print -u2 test status $?
	case $op in
	eval|exec)
		;;
	*)	if	test -f core
		then	case $top in
			.)	;;
			*)	mv core $top ;;
			esac
		fi
		for file in *.ml
		do	if	test -f $file
			then	rm -f $file
				print -u2 test error file $file
			fi
		done
		;;
	esac
	} < /dev/null > $TEST_data/new.$num 2>&1
	{
	print -n "$num"
	case $TEST_pwd in
	.)	;;
	*)	print -n " pwd=$TEST_pwd top=$TEST_top" ;;
	esac
	case $COSHELL in
	coshell)
		print -n " COSHELL=coshell"
		case $COATTRIBUTES in
		?*)	print -n " COATTRIBUTES='$COATTRIBUTES'" ;;
		esac
		case $NPROC in
		?*)	print -n " NPROC=$NPROC" ;;
		esac
		;;
	esac
	if	vpath . >/dev/null 2>&1
	then	print -n " fs3d=1"
	fi
	case $op in
	eval|exec)
		;;
	*)	case $# in
		0)	;;
		*)	shift ;;
		esac
		op=make
		case $TEST_makepath in
		?*)	print -n " MAKEPATH=$TEST_makepath" ;;
		esac
		case $TEST_vpath in
		?*)	print -n " VPATH=$TEST_vpath" ;;
		esac
		;;
	esac
	print -n " $op"
	for arg
	do	case $arg in
		*"'"*)	arg=$(print -- "$arg" | sed "s/'/'\\\\''/g") ;;
		esac
		print -n " '$arg'"
	done
	print
	} >> $TEST_data/new.tst
	;;
*)	case $1 in
	"")	set "(nil)"
		;;
	esac
	print -u2 $command: $TEST_name: $1: unknown op: "accept,all,compare,exec,gen,init,list,new,out,run,save,src,verbatim,*make*" expected
	exit 1
	;;
esac
