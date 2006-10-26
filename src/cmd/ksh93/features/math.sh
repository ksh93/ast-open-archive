########################################################################
#                                                                      #
#               This software is part of the ast package               #
#           Copyright (c) 1982-2006 AT&T Knowledge Ventures            #
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
#                  David Korn <dgk@research.att.com>                   #
#                                                                      #
########################################################################
: generate the ksh math builtin table
: include math.tab
: include FEATURE/isoc

# @(#)math.sh (AT&T Research) 2006-10-26

command=$0
iffeflags="-n -v"
iffehdrs="math.h ieeefp.h"
ifferefs=""
iffelibs="-lm"
table=/dev/null

eval $1
shift
while	:
do	case $# in
	0)	break ;;
	esac
	case $1 in
	*.tab)	table=$1 ;;
	*/isoc)	ifferefs=`sed -e '1,/-D/d' -e '/#define/!d' -e 's/#define.\([^        ]*\).*/-D\1/' $1` ;;
	esac
	shift
done

names=
tests=

: read the table

exec < $table
while	read type args name aka comment
do	case $type in
	[fi])	names="$names $name"
		tests="$tests,$name,${name}l"
		eval TYPE_$name=$type ARGS_$name=$args AKA_$name=$aka
		;;
	esac
done

: check the math library

eval `iffe $iffeflags -c "$cc" - lib $tests $iffehdrs $ifferefs $iffelibs 2>&$stderr`
tests=
for name in $names
do	eval x='$'_lib_${name}l y='$'_lib_${name}
	case $x:$y in
	1:*)	tests="$tests,${name}l" ;;
	*:1)	tests="$tests,${name}" ;;
	esac
done
eval `iffe $iffeflags -c "$cc" - dat $tests $iffehdrs $ifferefs $iffelibs 2>&$stderr`
tests=
for name in $names
do	eval x='$'_dat_${name}l y='$'_dat_${name}
	case $x:$y in
	1:*)	tests="$tests,${name}l" ;;
	*:1)	tests="$tests,${name}" ;;
	esac
done
eval `iffe $iffeflags -c "$cc" - npt $tests $iffehdrs $ifferefs 2>&$stderr`
tests=
for name in $names
do	eval x='$'_lib_${name}l y='$'_lib_${name}
	case $x in
	'')	tests="$tests,${name}l" ;;
	esac
	case $y in
	'')	tests="$tests,${name}" ;;
	esac
done
eval `iffe $iffeflags -c "$cc" - mac $tests $iffehdrs $ifferefs 2>&$stderr`

cat <<!
#pragma prototyped

/* : : generated by $command from $table : : */

typedef Sfdouble_t (*Math_f)(Sfdouble_t,...);

!
case $_hdr_ieeefp in
1)	echo "#include <ieeefp.h>"
	echo
	;;
esac

: generate the intercept functions and table entries

nl='
'
ht='	'
tab=
ldouble=
for name in $names
do	eval x='$'_lib_${name}l y='$'_lib_${name}
	case $x:$y in
	1:1)	ldouble=1
		break
		;;
	esac
done
for name in $names
do	eval x='$'_lib_${name}l y='$'_lib_${name} r='$'TYPE_${name} a='$'ARGS_${name} aka='$'AKA_${name}
	case $x:$y in
	1:*)	f=${name}l
		t=Sfdouble_t
		local=
		;;
	*:1)	f=${name}
		t=double
		local=$ldouble
		;;
	*)	continue
		;;
	esac
	eval n='$'_npt_$f m='$'_mac_$f d='$'_dat_$f
	case $r in
	i)	L=int r=int R=1 ;;
	*)	L=Sfdouble_t r=$t R=0 ;;
	esac
	case $d:$m:$n in
	1:*:*|*:1:*)
		;;
	*:*:1)	code="extern $r $f("
		sep=
		for p in 1 2 3 4 5 6 7
		do	code="$code${sep}$t"
			case $a in
			$p)	break ;;
			esac
			sep=","
		done
		code="$code);"
		echo "$code"
		;;
	esac
	case $local:$m:$d in
	1:*:*|*:1:*|*:*:)
		args=
		code="static $L local_$f("
		sep=
		for p in 1 2 3 4 5 6 7 8 9
		do	args="$args${sep}a$p"
			code="$code${sep}Sfdouble_t a$p"
			case $a in
			$p)	break ;;
			esac
			sep=","
		done
		code="$code){return $f($args);}"
		echo "$code"
		f=local_$f
		;;
	esac
	for x in $name $aka
	do	tab="$tab$nl$ht\"\\0${R}${a}${x}\",$ht(Math_f)$f,"
	done
done
tab="$tab$nl$ht\"\",$ht$ht(Math_f)0"

cat <<!

/*
 * first byte is two-digit octal number.  Last digit is number of args
 * first digit is 0 if return value is double, 1 for integer
 */
const struct mathtab shtab_math[] =
{$tab
};
!
