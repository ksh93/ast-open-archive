####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1987-2002 AT&T Corp.                #
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
#            Information and Software Systems Research             #
#                        AT&T Labs Research                        #
#                         Florham Park NJ                          #
#                                                                  #
#               Glenn Fowler <gsf@research.att.com>                #
####################################################################
: tw + pax for backup
#
# @(#)bax (AT&T Bell Laboratories) 05/09/95
#
# ../lib/bax/init on $PATH to customize
#

command=bax
customize=../lib/${command}/init

#
# no stinking environment
#

root= arch_name= arch_keep= big_size= big_magic_keep=
name_skip= path_skip= reg_magic_skip= src_name= src_magic_skip=

ifs=$IFS
IFS=:
path=$PATH
IFS=$ifs
for dir in $path
do	if	test -f $dir/$customize
	then	. $dir/$customize
		break
	fi
done

#
# default dirs
#

root=${root:-'/'}

#
# keep these architecture specific files: */${arch_name}/*/${arch_keep}
#

arch_name=${arch_name:-'arch'}
arch_keep=${arch_keep:-'bin|fun|lib'}

#
# files bigger than ${big_size} must have magic that matches ${big_magic_keep}
#

big_size=${big_size:-5000000}
big_magic_keep=${big_magic_keep:-'mail'}

#
# ${name_skip} are base names to be skipped
# ${path_skip} are path prefixes to be skipped
#

name_skip=${name_skip:-'?(*.)(old|save|tmp)?(.*)|.*-cache'}
path_skip=${path_skip:-'/dev|/usr/local/ast/ship'}

#
# regular files with magic that matches ${reg_magic_skip} are skipped
#

reg_magic_skip=${reg_magic_skip:-'core|cql * index|history|TeX dvi'}

#
# files under */${src_name}/* are skipped if magic matches ${src_magic_skip}
#

src_name=${src_name:-'src'}
src_magic_skip=${src_magic_skip:-'executable|object|(archive|shared) library'}

case $# in
[01])	echo "Usage: $command delta base [dir ...]" >&2
	exit 2
	;;
esac
delta=$1
shift
base=$2
shift
case $# in
0)	set $root ;;
esac
dirs=
while	:
do	case $# in
	0)	break ;;
	esac
	dirs="$dirs -d $1"
	shift
done

tw $dirs -mH -e "

begin:	int	arch_bit = 1;
	int	src_bit = 2;
	char*	m;

sort:	name;

select:	if (name == '${name_skip}')
		status = SKIP;
	else if (type == REG)
	{
		m = magic;
		if (size > ${big_size} && m != '*@(${big_magic_keep})*' ||
		    m == '*@(${reg_magic_skip})*' ||
		    (parent.local & src_bit) && m == '*@(${src_magic_skip})*')
			status = SKIP;
	}
	else if (type == DIR)
	{
		local = (parent.local & src_bit);
		if (dev != parent.dev || path == '${path_skip}' || (parent.parent.local & arch_bit) && name != '${arch_keep}')
			status = SKIP;
		else if (name == '${arch_name}')
			local |= arch_bit;
		else if (name == '${src_name}')
			local |= src_bit;
	}
	return 1;
" | pax -o "write ordered physical verbose file=$delta base=$base"
