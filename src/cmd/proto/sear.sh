####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1990-2002 AT&T Corp.                #
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
: self extracting archive generator for ratz

COMMAND=sear
cc="ncc -D_DLL -O -Y-Os"
cmd="dir"
ico=
opt=
out=install.exe
src=

case `(getopts '[-][123:xyz]' opt --xyz; echo 0$opt) 2>/dev/null` in
0123)	ARGV0="-a $COMMAND"
	USAGE=$'
[-?
@(#)$Id: sear (AT&T Labs Research) 2001-12-12 $
]
'$USAGE_LICENSE$'
[+NAME?sear - generate a win32 ratz self extracting archive]
[+DESCRIPTION?\bsea\b generates a win32 self extracting archive
	that can be used to install win32 files and applications.
	Each \afile\a operand is a \btgz\b (\btar\b(1) \bgzip\b(1))
	archive that is appended to the self extracting archive.
	If no \afile\a operands are specified then the standard
	input is read.]
[+?When the self extracting archive is executed it creates a temporary
	directory and unpacks each \afile\a. If \b--command\b=\acommand\a
	was specified when the archive was generated then \acommand\a
	is executed by \bCMD.EXE\b. The temporary directory is then
	removed.]
[c:cc?The C compiler command and flags are set to \acc\a.]:[cc:='$cc$']
[i:icon?The resource icon is set to
	\aicon\a.]:[icon:=$INSTALLROOT/lib/sear/sear.ico]
[m:meter?Set the \bratz\b(1) \b--meter\b option when the archive is
	extracted.]
[o:output?The self extracting file name is \afile\a.]:[file:='$out$']
[r:ratz?The \bratz\b(1) source file is set to
	\apath\a.]:[path:=$INSTALLROOT/lib/sear/ratz.c]
[x:command?The \bCMD.EXE\b command string to run after the top level files
	have been read into the temporary directory.]:[command:='$cmd$']
[v:verbose?Set the \bratz\b(1) \b--verbose\b option when the archive is
	extracted.]

[ file ] ...

[+EXIT STATUS?]{
	[+0?Success.]
	[+>0?An error occurred.]
}
[+SEE ALSO?\bratz\b(1), \bgunzip\b(1), \bpax\b(1), \bCMD.EXE\b(m$)]
'
	;;
*)	ARGV0=""
	USAGE="c:[command]i:[icon]o:[output]r:[ratz-src] [ file ] ..."
	;;
esac

usage()
{
	OPTIND=0
	getopts $ARGV0 "$USAGE" OPT '-?'
	exit 2
}

while	getopts $ARGV0 "$USAGE" OPT
do	case $OPT in
	c)	cc=$OPTARG
		;;
	i)	ico=$OPTARG
		;;
	m)	opt=${opt}m
		;;
	o)	out=$OPTARG
		;;
	r)	src=$OPTARG
		;;
	x)	cmd=$OPTARG
		;;
	v)	opt=${opt}v
		;;
	*)	usage
		;;
	esac
done
shift $((OPTIND-1))

case $src in
'')	f=ratz.c
	for i in ${PATH//:/ }
	do	if	test -f ${i%/*}/lib/$COMMAND/$f
		then	src=${i%/*}/lib/$COMMAND/$f
			break
		fi
	done
	case $src in
	'')	echo "$COMMAND: $f: cannot locate ratz source" >&2
		exit 1
		;;
	esac
	;;
*)	if	test ! -f $src 
	then	echo "$COMMAND: $src: cannot read ratz source" >&2
		exit 1
	fi
	;;
esac
case $ico in
'')	f=sear.ico
	for i in ${PATH//:/ }
	do	if	test -f ${i%/*}/lib/$COMMAND/$f
		then	ico=${i%/*}/lib/$COMMAND/$f
			break
		fi
	done
	case $ico in
	'')	echo "$COMMAND: $f: cannot locate icon source" >&2
		exit 1
		;;
	esac
	;;
*)	if	test ! -f $ico 
	then	echo "$COMMAND: $ico: cannot read icon" >&2
		exit 1
	fi
	;;
esac
case $opt in
?*)	cc="$cc -D_SEAR_OPTS=\"-$opt\"" ;;
esac

tmp=/tmp/sear$$
obj=${src##*/}
obj=${obj%.*}.o
trap 'rm -f $obj $tmp.*' 0 1 2 3
res=$tmp.res
typeset -H host_ico=$ico host_rc=$tmp.rc host_res=$tmp.res
print -r "sear ICON \"${host_ico//\\/\\\\}\"" > $tmp.rc
if	! rc -x -r -fo"$host_res" "$host_rc"
then	exit 1
fi
export nativepp=-1
if	! $cc -D_SEAR_SEEK=0 -D_SEAR_EXEC="\"$cmd\"" -o "$out" "$src" "$res"
then	exit 1
fi
size=$(wc -c < $out)
if	! $cc -D_SEAR_SEEK=$(($size)) -D_SEAR_EXEC="\"$cmd\"" -o "$out" "$src" "$res"
then	exit 1
fi
pax -x tgz -wv "$@" >> "$out"
