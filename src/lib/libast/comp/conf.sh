########################################################################
#                                                                      #
#               This software is part of the ast package               #
#                  Copyright (c) 1985-2004 AT&T Corp.                  #
#                      and is licensed under the                       #
#          Common Public License, Version 1.0 (the "License")          #
#                        by AT&T Corp. ("AT&T")                        #
#      Any use, downloading, reproduction or distribution of this      #
#      software constitutes acceptance of the License.  A copy of      #
#                     the License is available at                      #
#                                                                      #
#         http://www.research.att.com/sw/license/cpl-1.0.html          #
#         (with md5 checksum 8a5e0081c856944e76c69a1cf29c2e8b)         #
#                                                                      #
#              Information and Software Systems Research               #
#                            AT&T Research                             #
#                           Florham Park NJ                            #
#                                                                      #
#                 Glenn Fowler <gsf@research.att.com>                  #
#                  David Korn <dgk@research.att.com>                   #
#                   Phong Vo <kpv@research.att.com>                    #
#                                                                      #
########################################################################
: generate conf info
#
# @(#)conf.sh (AT&T Research) 2004-08-11
#
# this script generates these files from the table file in the first arg
# the remaining args are the C compiler name and flags
#
#	conflib.h	common generator definitions
#	conflim.h	limits.h generator code
#	confmap.c	internal index to external op map data
#	confmap.h	internal index to external op map definitions
#	confstr.c	confstr() implementation
#	conftab.c	readonly string table data
#	conftab.h	readonly string table definitions
#	confuni.h	unistd.h generator code
#	pathconf.c	pathconf() and fpathconf() implementation
#	sysconf.c	sysconf() implementation
#
# you may think it should be simpler
# but you shall be confused anyway
#

case $-:$BASH_VERSION in
*x*:[0123456789]*)	: bash set -x is broken :; set +ex ;;
esac

command=conf

shell=`eval 'x=123&&integer n=\${#x}\${x#1?}&&((n==330/(10)))&&echo ksh' 2>/dev/null`

append=0
debug=
extra=0
index=0
keep_call='*'
keep_name='*'
trace=
verbose=0
while	:
do	case $1 in
	-a)	append=1 ;;
	-c*)	keep_call=${1#-?} ;;
	-d*)	debug=$1 ;;
	-l)	extra=1 ;;
	-n*)	keep_name=${1#-?} ;;
	-t*)	trace=${1#-?} ;;
	-v)	verbose=1 ;;
	-*)	echo "Usage: $command [-a] [-ccall-pattern] [-dN] [-l] [-nname_pattern] [-t[s]] [-v] conf.tab" >&2; exit 2 ;;
	*)	break ;;
	esac
	shift
done
generated="/* : : generated by $command from $1 : : */"
ifs=$IFS
nl='
'
sp=' '
ob='{'
cb='}'
sym=[ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_]*
tmp=conf.tmp
case $verbose:$debug in
1:?*)	echo "$command: debug=$debug keep_call=$keep_call keep_name=$keep_name" >&2 ;;
esac

case $# in
0)	case $extra in
	0)	echo "$command: table argument expected" >&2
		exit 1
		;;
	esac
	tab=/dev/null
	;;
*)	tab=$1
	shift
	if	test ! -f $tab
	then	echo "$command: $tab: cannot read" >&2
		exit 1
	fi
	;;
esac
case $# in
0)	cc=cc ;;
*)	cc=$* ;;
esac

rm -f $tmp.*
case $debug in
'')	trap "rm -f $tmp.*" 0 1 2 ;;
esac

# set up the names and keys

keys=
lastindex=0

case $append$extra in
00)	case $verbose in
	1)	echo "$command: reading $tab" >&2 ;;
	esac
	exec < $tab
	while	:
	do	IFS=""
		read line
		eof=$?
		IFS=$ifs
		case $eof in
		0)	;;
		*)	break ;;
		esac
		case $line in
		""|\#*)	;;
		"	"*)
			set x $line
			shift
			echo "$*" >> $tmp.h
			case $name in
			?*)	local=LOCAL
				case $section in
				[01])	;;
				*)	local=${local}${section} ;;
				esac
				eval CONF_values_${key}='${sp}_${local}_${name}$'CONF_values_${key}
				name=
				;;
			esac
			;;
		*)	set x $line
			shift; name=$1
			shift; index=$1
			shift; standard=$1
			shift; call=$1
			shift; section=$1
			shift; flags=$1
			define=
			values=
			script=
			headers=
			while	:
			do	shift
				case $# in
				0)	break ;;
				esac
				case $1 in
				"{")	case $# in
					1)	IFS=""
						while	read line
						do	case $line in
							"}")	break ;;
							esac
							script=$script$nl$line
						done
						IFS=$ifs
						eval script_$name='$'script
						break
						;;
					*)	shift
						eval script='$'script_$1
						case $2 in
						"}")	shift ;;
						esac
						;;
					esac
					;;
				*.h)	headers=$headers$nl#include$sp'<'$1'>'
					;;
				*)	values=$values$sp$1
					;;
				esac
			done
			case $call in
			CS|SI)	key=CS ;;
			*)	key=$call ;;
			esac
			case $name in
			*VERSION*)key=${key}_${standard}${section} ;;
			esac
			key=${key}_${name}
			keys="$keys$nl$key"
			eval CONF_name_${key}='$'name
			eval CONF_index_${key}='$'index
			eval CONF_standard_${key}='$'standard
			eval CONF_call_${key}='$'call
			eval CONF_section_${key}='$'section
			eval CONF_flags_${key}='$'flags
			eval CONF_define_${key}='$'define
			eval CONF_values_${key}='$'values
			eval CONF_script_${key}='$'script
			eval CONF_headers_${key}='$'headers
			eval CONF_keys_${name}=\"'$'CONF_keys_${name} '$'key\"
			if	test $index -gt $lastindex
			then	lastindex=$index
			fi
			;;
		esac
	done
	;;
esac
case $debug in
-d1)	for key in $keys
	do	eval name=\"'$'CONF_name_$key\"
		case $name in
		?*)	eval index=\"'$'CONF_index_$key\"
			eval standard=\"'$'CONF_standard_$key\"
			eval call=\"'$'CONF_call_$key\"
			eval section=\"'$'CONF_section_$key\"
			eval flags=\"'$'CONF_flags_$key\"
			eval define=\"'$'CONF_define_$key\"
			eval values=\"'$'CONF_values_$key\"
			eval script=\"'$'CONF_script_$key\"
			eval headers=\"'$'CONF_headers_$key\"
			printf "%29s %35s %3d %8s %2s %1d %5s %s$nl" "$name" "$key" "$index" "$standard" "$call" "$section" "$flags" "$define${values:+$sp=$values}${headers:+$sp$headers$nl}${script:+$sp$ob$script$nl$cb}"
			;;
		esac
	done
	exit
	;;
esac

systeminfo='
#if !defined(SYS_NMLEN)
#define SYS_NMLEN	9
#endif
#include <sys/systeminfo.h>'
echo "$systeminfo" > $tmp.c
$cc -E $tmp.c >/dev/null 2>&1 || systeminfo=

# check for local additions

case $verbose in
1)	echo "$command: check local confstr(),pathconf(),sysconf(),sysinfo() keys" >&2 ;;
esac
{
	echo "#include <unistd.h>$systeminfo
int i = 0;" > $tmp.c
	$cc -E $tmp.c
} |
sed \
	-e '/^#[^0123456789]*1[ 	]*".*".*/!d' \
	-e 's/^#[^0123456789]*1[ 	]*"\(.*\)".*/\1/' |
sort -u > $tmp.f
sed \
	-e '/^[ 	]*#[ 	]*define[ 	][ 	]*[ABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789]*[CPS][CIS]_/!d' \
	-e 's,^[ 	]*#[ 	]*define[ 	]*,,' \
	-e '/^[^ 	]*[ 	][ 	]*[0123456789]/!d' \
	-e 's,[ 	].*,,' \
	-e '/^[S_]/!d' \
	`cat $tmp.f` 2>/dev/null |
sort -u > $tmp.v
case $debug in
-d2)	exit ;;
esac

HOST=`package | sed -e 's,[0123456789.].*,,' | tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ`
case $HOST in
'')	HOST=SYSTEM ;;
esac

exec < $tmp.v

while	read line
do	case $line in
	*_[CS][SI]_*_STR|SI_*_STR)	continue ;;
	esac
	flags=F
	section=
	define=$line
	IFS=_
	set $line
	IFS=$ifs
	case $1 in
	'')	case $# in
		0)	continue ;;
		esac
		shift
		;;
	esac
	case $1 in
	CS|PC|SC|SI)
		call=$1
		shift
		standard=$1
		;;
	*)	flags=${flags}R
		standard=$1
		while	:
		do	case $# in
			0)	continue 2 ;;
			esac
			shift
			case $1 in
			CS|PC|SC|SI)
				call=$1
				shift
				break
				;;
			O|o|OLD|old)
				continue 2
				;;
			esac
			standard=${standard}_$1
		done
		;;
	esac
	case $1 in
	SET)	continue ;;
	esac
	case $standard in
	_*)	standard=`echo $standard | sed 's,^_*,,'` ;;
	esac
	case $standard in
	XBS5)	;;
	[0123456789]*)
		section=$standard
		standard=POSIX
		;;
	*[0123456789])
		eval `echo $standard | sed 's,\(.*\)\([0123456789]*\),standard=\1 section=\2,'`
		;;
	esac
	case $flags in
	*R*)	case $call in
		SI)	;;
		*)	flags=${flags}U ;;
		esac
		;;
	*)	case $standard in
		POSIX|SVID|XBS5|XOPEN|XPG|AES|AST)
			case $call in
			SI)	;;
			*)	flags=${flags}U ;;
			esac
			shift
			;;
		C)	shift
			;;
		*)	standard=
			;;
		esac
		;;
	esac
	case $standard in
	'')	standard=$HOST
		case $call in
		SI)	;;
		*)	flags=${flags}U ;;
		esac
		case $call in
		CS|PC|SC)
			case $define in
			_${call}_*)
				standard=POSIX
				;;
			esac
			;;
		esac
		;;
	esac
	case $section in
	'')	section=1 ;;
	esac
	name=
	while	:
	do	case $# in
		0)	break ;;
		esac
		case $name in
		'')	name=$1 ;;
		*)	name=${name}_$1 ;;
		esac
		shift
	done
	case $name in
	'')	;;
	CONFORMANCE|FS_3D|HOSTTYPE|LIBPATH|LIBPREFIX|LIBSUFFIX|PATH_ATTRIBUTES|PATH_RESOLVE|UNIVERSE)
		;;
	*)	lastindex=`expr $lastindex + 1`
		index=$lastindex
		values=
		script=
		headers=
		case $call in
		CS|SI)	key=CS ;;
		*)	key=$call ;;
		esac
		case $name in
		*VERSION*)key=${key}_${standard}${section} ;;
		esac
		key=${key}_${name}
		eval x='$'CONF_name_$key
		case $x in
		'')	case $call in
			SI)	flags=O$flags ;;
			esac
			old=QQ
			case $name in
			*VERSION*)old=${old}_${standard}${section} ;;
			esac
			old=${old}_${name}
			eval x='$'CONF_name_$old
			case $x in
			?*)	eval CONF_name_$old=
				eval index='$'CONF_index_$old
				eval flags='$'flags'$'CONF_flags_$old
				eval values='$'CONF_values_$old
				eval script='$'CONF_script_$old
				eval headers='$'CONF_headers_$old
				;;
			esac
			keys="$keys$nl$key"
			eval CONF_name_${key}='$'name
			eval CONF_index_${key}='$'index
			eval CONF_standard_${key}='$'standard
			eval CONF_call_${key}='$'call
			eval CONF_section_${key}='$'section
			eval CONF_flags_${key}=D'$'flags
			eval CONF_define_${key}='$'define
			eval CONF_values_${key}='$'values
			eval CONF_script_${key}='$'script
			eval CONF_headers_${key}='$'headers
			;;
		*)	eval x='$'CONF_define_$key
			case $x in
			?*)	case $call in
				CS)	eval x='$'CONF_call_$key
					case $x in
					SI)	;;
					*)	define= ;;
					esac
					;;
				*)	define=
					;;
				esac
				;;
			esac
			case $define in
			?*)	eval CONF_define_${key}='$'define
				eval CONF_call_${key}='$'call
				eval x='$'CONF_call_${key}
				case $x in
				QQ)	;;
				*)	case $flags in
					*R*)	flags=R ;;
					*)	flags= ;;
					esac
					;;
				esac
				case $call in
				SI)	flags=O$flags ;;
				esac
				eval CONF_flags_${key}=D'$'flags'$'CONF_flags_${key}
				;;
			esac
			eval x='$'CONF_index_$key
			if	test $index -lt $x
			then	eval CONF_index_${key}='$'index
			fi
			old=QQ
			case $name in
			*VERSION*)old=${old}_${standard}${section} ;;
			esac
			old=${old}_${name}
			eval CONF_name_$old=
		esac
		;;
	esac
done

# sort keys by name

keys=`for key in $keys
do	eval echo '$'CONF_name_$key '$'key
done | sort -u | sed 's,.* ,,'`
case $debug in
-d3)	for key in $keys
	do	eval name=\"'$'CONF_name_$key\"
		case $name in
		?*)	eval index=\"'$'CONF_index_$key\"
			eval standard=\"'$'CONF_standard_$key\"
			eval call=\"'$'CONF_call_$key\"
			eval section=\"'$'CONF_section_$key\"
			eval flags=\"'$'CONF_flags_$key\"
			eval define=\"'$'CONF_define_$key\"
			eval values=\"'$'CONF_values_$key\"
			eval script=\"'$'CONF_script_$key\"
			eval headers=\"'$'CONF_headers_$key\"
			printf "%29s %35s %3d %8s %2s %1d %5s %s$nl" "$name" "$key" "$index" "$standard" "$call" "$section" "$flags" "$define${values:+$sp=$values}${headers:+$sp$headers$nl}${script:+$sp$ob$script$nl$cb}"
			;;
		esac
	done
	exit
	;;
esac

cat > $tmp.6 <<!
	/*
	 * some implementations (could it beee aix) think empty
	 * definitions constitute symbolic constants
	 */

	{
	long	num;
	char*	str;
	int	hit;
!

# mark the dups CONF_PREFIXED

prev_key=
prev_name=
for key in $keys
do	eval name=\"'$'CONF_name_$key\"
	case $name in
	'')	continue
		;;
	$prev_name)
		eval CONF_flags_${prev_key}=P'$'CONF_flags_${prev_key}
		eval CONF_flags_${key}=P'$'CONF_flags_${key}
		;;
	esac
	prev_name=$name
	prev_key=$key
done

# walk through the table

case $shell in
ksh)	integer len limit_max name_max ;;
esac
limit_max=1
name_max=1
standards=
export tmp name index standard call cc

for key in $keys
do	eval name=\"'$'CONF_name_$key\"
	case $name in
	'')		continue ;;
	$keep_name)	;;
	*)		continue ;;
	esac
	eval call=\"'$'CONF_call_$key\"
	case $call in
	$keep_call)	;;
	*)		continue ;;
	esac
	eval index=\"'$'CONF_index_$key\"
	eval standard=\"'$'CONF_standard_$key\"
	eval section=\"'$'CONF_section_$key\"
	eval flags=\"'$'CONF_flags_$key\"
	eval define=\"'$'CONF_define_$key\"
	eval values=\"'$'CONF_values_$key\"
	eval script=\"'$'CONF_script_$key\"
	eval headers=\"'$'CONF_headers_$key\"
	conf_name=$name
	conf_index=$index
	case $call in
	QQ)	call=XX
		for c in SC PC CS
		do	cat > $tmp.c <<!
#include "FEATURE/standards"
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>$systeminfo$headers
#include <stdio.h>
main()
{
	return _${c}_${name} == 0;
}
!
			if	$cc -o $tmp.exe $tmp.c >/dev/null 2>&1
			then	call=$c
				case $standard in
				C)	standard=POSIX ;;
				esac
				flags=FU
				break
			fi
		done
		;;
	esac
	case " $standards " in
	*" $standard "*)
		;;
	*)	standards="$standards $standard"
		;;
	esac
	conf_standard=CONF_${standard}
	case $call in
	CS)	conf_call=CONF_confstr
		;;
	PC)	conf_call=CONF_pathconf
		;;
	SC)	conf_call=CONF_sysconf
		;;
	SI)	conf_call=CONF_sysinfo
		;;
	XX)	conf_call=CONF_nop
		;;
	*)	echo "$command: $name: $call: invalid call" >&2
		exit 1
		;;
	esac
	conf_section=$section
	conf_flags=0
	case $flags in
	*[ABCEGHIJKQTVWYZabcdefghijklmnopqrstuvwxyz_123456789]*)
		echo "$command: $name: $flags: invalid flag(s)" >&2
		exit 1
		;;
	esac
	case $flags in
	*F*)	conf_flags="${conf_flags}|CONF_FEATURE" ;;
	esac
	case $flags in
	*L*)	conf_flags="${conf_flags}|CONF_LIMIT" ;;
	esac
	case $flags in
	*M*)	conf_flags="${conf_flags}|CONF_MINMAX" ;;
	esac
	case $flags in
	*N*)	conf_flags="${conf_flags}|CONF_NOSECTION" ;;
	esac
	case $flags in
	*P*)	conf_flags="${conf_flags}|CONF_PREFIXED" ;;
	esac
	case $flags in
	*S*)	conf_flags="${conf_flags}|CONF_STANDARD" ;;
	esac
	case $flags in
	*U*)	conf_flags="${conf_flags}|CONF_UNDERSCORE" ;;
	esac
	case $shell in
	ksh)	conf_flags=${conf_flags#0?} ;;
	esac
	case $verbose in
	1)	case $standard in
		????)	sep=" " ;;
		???)	sep="  " ;;
		??)	sep="   " ;;
		?)	sep="    " ;;
		*)	sep="" ;;
		esac
		echo "$command: test: $sep$standard $call $name" >&2
		;;
	esac
	case $script in
	?*)	echo "$script" > $tmp.z
		chmod +x $tmp.z
		values="$values `./$tmp.z 2>/dev/null`"
		;;
	esac
	case $call in
	CS|SI)	conf_flags="${conf_flags}|CONF_STRING" ;;
	esac
	case $flags in
	*L*)	conf_value=$conf_name ;;
	*)	conf_value=0 ;;
	esac
	case $section in
	[01])	;;
	*)	case $flags in
		*N*)	;;
		*)	name=${section}_${name} ;;
		esac
		standard=${standard}${section}
		;;
	esac
	case $call in
	XX)	conf_op=0
		;;
	*)	case $flags in
		*D*)	conf_op=${define} ;;
		*O*)	conf_op=${call}_${name} ;;
		*R*)	conf_op=_${standard}_${call}_${name} ;;
		*S*)	conf_op=_${call}_${standard}_${name} ;;
		*U*)	conf_op=_${call}_${name} ;;
		*)	conf_op=${call}_${name} ;;
		esac
		eval V='$'_dUp_$conf_op
		case $V in
		'')	eval _dUp_$conf_op=1 ;;
		*)	continue ;;
		esac
		cat > $tmp.c <<!
#include "FEATURE/standards"
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>$systeminfo$headers
#include <stdio.h>
main()
{
#ifdef TEST_enum
#if ${conf_op}
	(
#endif
#undef ${conf_op}
	return ${conf_op} == 0;
#endif
#ifdef TEST_notmacro
#ifdef ${conf_op}
	(
#endif
	return 0;
#endif
#ifdef TEST_macro
#ifdef ${conf_op}
	return ${conf_op} == 1;
#else
	return 0;
#endif
#endif
}
!
		if	$cc -DTEST_enum -o $tmp.exe $tmp.c >/dev/null 2>&1
		then	echo "#define _ENUM_${conf_op}	1" >> $tmp.e 2>/dev/null
			if	$cc -DTEST_notmacro -o $tmp.exe $tmp.c >/dev/null 2>&1
			then	echo "#define ${conf_op}	${conf_op}" >> $tmp.e 2>/dev/null
			fi
		elif	$cc -DTEST_macro -o $tmp.exe $tmp.c >/dev/null 2>&1
		then	:
		else	# not a number -- undefined or empty (who thunk that?)
			continue
		fi
		echo ${index} ${conf_op} >> $tmp.m
		case $call:$flags in
		SI:*O*)	;;
		*)	cat >> $tmp.6 <<!
	printf("#undef	${conf_op}\n");
	printf("#define ${conf_op}	(-${index})\n");
!
			;;
		esac
		;;
	esac
	case $standard:$flags in
	C:*)	;;
	*:*L*)	{
		echo "	hit = 0;"
		case $call in
		PC)	cat <<!
#if _lib_pathconf && defined(${conf_op})
	if ((num = pathconf("/", ${conf_op})) != -1)
		hit = 1;
	else
#endif
!
			;;
		SC)	cat <<!
#if _lib_sysconf && defined(${conf_op})
	if ((num = sysconf(${conf_op})) != -1)
		hit = 1;
	else
#endif
!
			;;
		esac
		echo "	{"
		endif=
		default=
		for i in $conf_name $values
		do	case $i in
			_${standard}_${conf_name})
				;;
			$sym)	case $i in
				_LOCAL_*)	cat <<!
#if	defined(${i})
!
					;;
				*)	cat <<!
#if	defined(${i})
!
					;;
				esac
				cat <<!
	if (!hit && ${i} > 0)
	{
		hit = 1;
		num = ${i};
	}
#else
!
				endif="#endif
$endif"
				case $conf_op:$flags:$i in
				0*|*X*:*)
					;;
				*:*:$conf_name)
					echo "#ifndef	${conf_op}"
					endif="#endif
$endif"
					;;
				esac
				;;
			*)	case $default in
				"")	default=${i} ;;
				esac
				;;
			esac
		done
		case $default in
		?*)	cat <<!
	if (!hit)
	{
		hit = 1;
		num = ${default};
	}
!
			;;
		esac
		echo "${endif}	}"
		cat <<!
	if (hit)
	{
		printf("#undef	${conf_name}\n");
		printf("#define ${conf_name}		%ld\n", num);
	}
	else
		num = -1;
	lim[${limit_max}] = num;
#ifndef ${conf_name}
#define ${conf_name}	(lim[${limit_max}])
#endif
!
		} >> $tmp.l
		case $shell in
		ksh)	((limit_max=limit_max+1)) ;;
		*)	limit_max=`expr $limit_max + 1` ;;
		esac
		;;
	esac
	case $call:$standard:$flags in
	*:C:*)	;;
	[PSX][CX]:*:*[FM]*)
		case $flags in
		*M*)	header=l ;;
		*)	header=6 ;;
		esac
		minmax=
		for i in $values
		do	case $i in
			$sym)	;;
			*)	case $minmax in
				''|'"'*'"')	minmax=${i} ;;
				'"'*)		minmax="${minmax} ${i}" ;;
				*)		minmax=${i} ;;
				esac
				;;
			esac
		done
		macro=_${standard}_${conf_name}
		conf_value=${macro}
		values="$values ${macro}"
		case $minmax in
		"")	case $call in
			CS|SI)	default=0 ;;
			*)	default=1 ;;
			esac
			;;
		*)	default=$minmax ;;
		esac
		{
		case $call in
		CS|SI)	type="char*" var=str fmt='\"%s\"' ;;
		*)	type=long var=num fmt='%ld' ;;
		esac
		cat <<!
#if defined(${macro})
	{
		static ${type}	x[] = { ${default}, ${macro} };
		if ((sizeof(x)/sizeof(x[0])) == 1)
		{
			printf("#undef	${macro}\n");
			${var} = x[0];
		}
		else
			${var} = x[1];
	}
/* this comment works around a shell bug that loses a here doc newline */
!
		case $minmax in
		?*)	cat <<!
#else
	${var} = ${minmax};
#endif
!
			;;
		esac
		cat <<!
	printf("#undef	${macro}\n");
	printf("#define ${macro}	${fmt}\n", ${var});
!
		case $minmax in
		"")	cat <<!
#endif
!
			;;
		esac
		} >> $tmp.$header
		;;
	esac
	case $call in
	CS|SI)	conf_value=0 something= ;;
	*)	something=-0 ;;
	esac
	echo $conf_name $conf_section $conf_standard $conf_value $conf_flags $conf_call $conf_op >> $tmp.G
	case $shell in
	ksh)	len=${#conf_name}
		if	((len>=name_max))
		then	((name_max=len+1))
		fi
		;;
	*)	len=`echo ${conf_name} | wc -c`
		if	expr \( $len - 1 \) \>= ${name_max} >/dev/null
		then	name_max=$len
		fi
		;;
	esac
	case $conf_op in
	0)	;;
	*)	{
		echo "#if	${conf_op}+0
case ${conf_op}:"
		endif="#endif"
		minmax=
		for i in $name $values
		do	case $i in
			$sym)	case $something in
				'')	cat > $tmp.c <<!
#include "FEATURE/standards"
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>$systeminfo$headers
">>>" $i "<<<"
!
					i=`$cc -E $tmp.c 2>/dev/null | sed -e '/">>>".*"<<<"/!d' -e 's/.*">>>"[ 	]*\([^ 	]*\)[ 	]*"<<<".*/\1/'`
					case $i in
					$i)		;;
					'"'*'"')	echo "		return($i);" ;;
					*'"'*)		;;
					*)		echo "		return(\"$i\");" ;;
					esac
					;;
				*)	echo "#ifdef	$i
		return($i${something});
#else"
					endif="$endif
#endif"
					;;
				esac
				;;
			*)	case $flags in
				*M*)	minmax=$i ;;
				esac
				;;
			esac
		done
		case $minmax in
		?*)	echo "		return($minmax${something});" ;;
		*)	echo "		break;" ;;
		esac
		echo "$endif"
		} >> $tmp.$call
		;;
	esac
done
case $debug in
-d4)	exit ;;
esac

# internal to external map

base=confmap
case $verbose in
1)	echo "$command: generate ${base}.h internal to external map header" >&2 ;;
esac
{
cat <<!
#pragma prototyped
#define ${base}		_conf_map
${generated}
extern const short	${base}[];
!
} | proto > $tmp.0
case $debug in
-d5)	echo $command: $tmp.0 ${base}.h ;;
*)	cmp -s $tmp.0 ${base}.h 2>/dev/null || mv $tmp.0 ${base}.h ;;
esac

case $verbose in
1)	echo "$command: generate ${base}.c internal to external map" >&2 ;;
esac
sort -n $tmp.m | {
case $shell in
ksh)	integer next ;;
esac
next=0
while	read index macro
do	case $shell in
	ksh)	while	:
		do	((next=next+1))
			((next>=$index)) && break
			echo "	-1,"
		done
		;;
	*)	while	:
		do	next=`expr $next + 1`
			expr $next \>= $index > /dev/null && break
			echo "	-1,"
		done
		;;
	esac
	cat <<!
#if	($macro+0) || _ENUM_$macro
	$macro,
#else
	-1,
#endif
!
done
echo $next >&3
} > $tmp.c 3> $tmp.x
map_max=`cat $tmp.x`
{
cat <<!
#pragma prototyped
#include "FEATURE/limits.lcl"
#include "FEATURE/unistd.lcl"
#include "${base}.h"

${generated}
!
if	test -s $tmp.e
then	cat <<!

/*
 * enum used on an extensible namespace -- bad idea
 */

!
	cat $tmp.e
fi
cat <<!

/*
 * internal to external conf index map
 */

const short ${base}[] =
{
	$map_max,
!
cat $tmp.c
echo "};"
} | proto > $tmp.1
case $debug in
-d5)	echo $command: $tmp.1 ${base}.c ;;
*)	cmp -s $tmp.1 ${base}.c 2>/dev/null || mv $tmp.1 ${base}.c ;;
esac

# conf string table

base=conftab
case $verbose in
1)	echo "$command: generate ${base}.h string table header" >&2 ;;
esac
case $shell in
ksh)	((name_max=name_max+3)); ((name_max=name_max/4*4)) ;; # bsd /bin/sh !
*)	name_max=`expr \( $name_max + 3 \) / 4 \* 4` ;;
esac
{
cat <<!
#pragma prototyped

#ifndef _CONFTAB_H
#define _CONFTAB_H
$systeminfo

${generated}

#define conf		_conf_data
#define conf_elements	_conf_ndata

#define prefix		_conf_prefix
#define prefix_elements	_conf_nprefix

#define CONF_nop	0
#define	CONF_confstr	1
#define CONF_pathconf	2
#define CONF_sysconf	3
#define CONF_sysinfo	4

!
index=0
for standard in $standards
do	echo "#define CONF_${standard}	${index}"
	case $shell in
	ksh)	((index=index+1)) ;;
	*)	index=`expr ${index} + 1` ;;
	esac
done
echo "#define CONF_call	${index}"
cat <<!

#define CONF_DEFINED	(1<<0)
#define CONF_FEATURE	(1<<1)
#define CONF_LIMIT	(1<<2)
#define CONF_MINMAX	(1<<3)
#define CONF_NOSECTION	(1<<4)
#define CONF_PREFIXED	(1<<5)
#define CONF_STANDARD	(1<<6)
#define CONF_STRING	(1<<7)
#define CONF_UNDERSCORE	(1<<8)
#define CONF_USER	(1<<9)

typedef struct
{
	const char	name[${name_max}];
	long		value;
	short		flags;
	short		standard;
	short		section;
	short		call;
	short		op;
} Conf_t;

typedef struct
{
	const char	name[8];
	short		length;
	short		standard;
	short		call;
} Prefix_t;

extern const Conf_t	conf[];
extern int		conf_elements;

extern const Prefix_t	prefix[];
extern int		prefix_elements;

#endif
!
} | proto > $tmp.2
case $debug in
-d5)	echo $command: $tmp.2 ${base}.h ;;
*)	cmp -s $tmp.2 ${base}.h 2>/dev/null || mv $tmp.2 ${base}.h ;;
esac

case $verbose in
1)	echo "$command: generate ${base}.c string table" >&2 ;;
esac
{
cat <<!
#pragma prototyped
#include <ast.h>
#include "${base}.h"

${generated}

/*
 * prefix strings -- the first few are indexed by Conf_t.standard
 */

const Prefix_t prefix[] =
{
!
for standard in $standards
do	case $shell in
	ksh)	len=${#standard} ;;
	*)	len=`echo ${standard} | wc -c`; len=`expr $len - 1` ;;
	esac
	echo "	\"${standard}\",	${len},	CONF_${standard},	-1,"
done
cat <<!
	"XX",		2,	CONF_POSIX,	CONF_nop,
	"CS",		2,	CONF_POSIX,	CONF_confstr,
	"PC",		2,	CONF_POSIX,	CONF_pathconf,
	"SC",		2,	CONF_POSIX,	CONF_sysconf,
	"SI",		2,	CONF_SVID,	CONF_sysinfo,
};

int	prefix_elements = elementsof(prefix);

/*
 * conf strings sorted in ascending order
 */

const Conf_t conf[] =
{
!
if	test -f $tmp.G
then	sort -u < $tmp.G |
	while	read name section standard value flags call op
	do	case $op in
		$sym)	echo "#if	$op+0" ;;
		esac
		case $value in
		$sym)	echo "\"$name\",
#ifdef	$value
	$value, $flags|CONF_DEFINED,
#else
	0, $flags,
#endif
	$standard, $section, $call, $op,"
			;;
		*)	echo \"$name\", $value, $flags, $standard, $section, $call, $op,
			;;
		esac
		case $op in
		$sym)	echo "#endif" ;;
		esac
	done
else	echo '""'
fi
cat <<!
};

int	conf_elements = elementsof(conf);
!
} | proto > $tmp.3
case $debug in
-d5)	echo $command: $tmp.3 ${base}.c ;;
*)	cmp -s $tmp.3 ${base}.c 2>/dev/null || mv $tmp.3 ${base}.c ;;
esac

# conf generator definitions

base=conflib
case $verbose in
1)	echo "$command: generate ${base}.h generator header" >&2 ;;
esac
{
cat <<!
#pragma prototyped
#define _ast_sysconf(x)	0
#undef	_sysconf
#define _sysconf(x)	0

${generated}

!
cat $tmp.h
cat <<!

#undef	_ast_sysconf
#undef	_sysconf
!
} | proto > $tmp.4
case $debug in
-d5)	echo $command: $tmp.4 ${base}.h ;;
*)	cmp -s $tmp.4 ${base}.h 2>/dev/null || mv $tmp.4 ${base}.h ;;
esac

# limits.h generation code

base=conflim
case $verbose in
1)	echo "$command: generate ${base}.c <limits.h> generator" >&2 ;;
esac
{
cat <<!
	/*
	 * some implementations (could it beee aix) think empty
	 * definitions constitute symbolic constants
	 */

	{
	long	num;
	char*	str;
	int	hit;
	long	lim[${limit_max}+1];

!
test -f $tmp.l && cat $tmp.l
echo "	}"
} > $tmp.5
case $debug in
-d5)	echo $command: $tmp.5 ${base}.h ;;
*)	cmp -s $tmp.5 ${base}.h 2>/dev/null || mv $tmp.5 ${base}.h ;;
esac

# unistd.h generation code

base=confuni
case $verbose in
1)	echo "$command: generate ${base}.c <unistd.h> generator" >&2 ;;
esac
echo "	}" >> $tmp.6
case $debug in
-d5)	echo $command: $tmp.6 ${base}.h ;;
*)	cmp -s $tmp.6 ${base}.h 2>/dev/null || mv $tmp.6 ${base}.h ;;
esac

# confstr implementation

base=confstr pfx=CS
case $verbose in
1)	echo "$command: generate ${base}.c ${base}() implementation" >&2 ;;
esac
{
cat <<!
#pragma prototyped
#include <ast.h>
#include <error.h>

#ifndef ${base}

NoN(${base})

#else

${generated}

#include "confmap.h"
#include "conflib.h"

static char*
local_${base}(int op)
{
	switch (op)
	{
!
test -f $tmp.${pfx} && cat $tmp.${pfx}
cat <<!
	default:
		break;
	}
	return(0);
}

size_t
${base}(int op, char* buf, size_t siz)
{
	char*	s;
	int	n;
#if _lib_${base}
#undef	${base}
	if (((n = op) >= 0 || -op <= confmap[0] && (n = confmap[-op]) >= 0) && (n = ${base}(n, buf, siz)) > 0)
		return(n);
#endif
	if (s = local_${base}(op))
	{
		if ((n = strlen(s) + 1) >= siz)
		{
			if (siz == 0)
				return(n + 1);
			buf[n = siz - 1] = 0;
		}
		memcpy(buf, s, n);
		return(n);
	}
	errno = EINVAL;
	return(0);
}

#endif
!
} | proto > $tmp.7
case $debug in
-d5)	echo $command: $tmp.7 ${base}.c ;;
*)	cmp -s $tmp.7 ${base}.c 2>/dev/null || mv $tmp.7 ${base}.c ;;
esac

# pathconf implementation

base=pathconf pfx=PC
case $verbose in
1)	echo "$command: generate ${base}.c ${base}() implementation" >&2 ;;
esac
{
cat <<!
#pragma prototyped
#include <ast.h>
#include <error.h>
#include <ls.h>

#ifndef ${base}

NoN(${base})

#else

${generated}

#include "confmap.h"
#include "conflib.h"

static long
statconf(struct stat* st, int op)
{
	switch (op)
	{
!
test -f $tmp.${pfx} && cat $tmp.${pfx}
cat <<!
	default:
		break;
	}
	errno = EINVAL;
	return(-1);
}

long
f${base}(int fd, int op)
{
	int		n;
	struct stat	st;
#if _lib_f${base}
#undef	f${base}
	if ((n = op) >= 0 || -op <= confmap[0] && (n = confmap[-op]) >= 0)
	{
		long	val;
		int	olderrno;
		int	syserrno;

		olderrno = errno;
		errno = 0;
		val = f${base}(fd, n);
		syserrno = errno;
		errno = olderrno;
		if (val != -1L || syserrno == 0)
			return(val);
	}
#endif
	return((n = fstat(fd, &st)) ? n : statconf(&st, op));
}

long
${base}(const char* path, int op)
{
	int		n;
	struct stat	st;
#if _lib_${base}
#undef	${base}
	if ((n = op) >= 0 || -op <= confmap[0] && (n = confmap[-op]) >= 0)
	{
		long	val;
		int	olderrno;
		int	syserrno;

		olderrno = errno;
		errno = 0;
		val = ${base}(path, n);
		syserrno = errno;
		errno = olderrno;
		if (val != -1L || syserrno == 0)
			return(val);
	}
#endif
	return((n = stat(path, &st)) ? n : statconf(&st, op));
}

#endif
!
} | proto > $tmp.8
case $debug in
-d5)	echo $command: $tmp.8 ${base}.c ;;
*)	cmp -s $tmp.8 ${base}.c 2>/dev/null || mv $tmp.8 ${base}.c ;;
esac

# sysconf implementation

base=sysconf pfx=SC
case $verbose in
1)	echo "$command: generate ${base}.c ${base}() implementation" >&2 ;;
esac
{
cat <<!
#pragma prototyped
#include <ast.h>
#include <error.h>

#ifndef ${base}

NoN(${base})

#else

${generated}

#include "confmap.h"
#include "conflib.h"

long
${base}(int op)
{
	int	n;
#if _lib_${base}
#undef	${base}
	if ((n = op) >= 0 || -op <= confmap[0] && (n = confmap[-op]) >= 0)
	{
		long	val;
		int	olderrno;
		int	syserrno;

		olderrno = errno;
		errno = 0;
		val = ${base}(n);
		syserrno = errno;
		errno = olderrno;
		if (val != -1L || syserrno == 0)
			return(val);
	}
#endif
	switch (op)
	{
!
test -f $tmp.${pfx} && cat $tmp.${pfx}
cat <<!
	default:
		break;
	}
	errno = EINVAL;
	return(-1);
}

#endif
!
} | proto > $tmp.9
case $debug in
-d5)	echo $command: $tmp.9 ${base}.c ;;
*)	cmp -s $tmp.9 ${base}.c 2>/dev/null || mv $tmp.9 ${base}.c ;;
esac
