################################################################
#                                                              #
#           This software is part of the ast package           #
#              Copyright (c) 1989-2000 AT&T Corp.              #
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
#              David Korn <dgk@research.att.com>               #
#               Eduardo Krell <ekrell@adexus.cl>               #
#                                                              #
################################################################
# NOTE: retain this comment
dir=$_ # NOTE: this line must appear before any commands

command=3d

case $RANDOM in
$RANDOM)exec ksh 3d "$@" ;;
esac

case $(getopts '[-][123:xyz]' opt --xyz 2>/dev/null; echo 0$opt) in
0123)	ARGV0="-a $command"
	USAGE=$'
[-?
@(#)3d (AT&T Labs Research) 2000-02-14
]
'$USAGE_LICENSE$'
[+NAME?3d - execute a command with nDFS filesystem services enabled]
[+DESCRIPTION?\b3d\b executes a dynamically linked \acommand\a
	with \an\a\bDFS\b filesystem services enabled. It intercepts pathname
	system calls and applies the \an\a\bDFS\b operations specified
	by the \bVPATH\b environment variable, the command line mounts,
	and the \bvpath\b command. If \acommand\a is omitted then an
	interactive \bsh\b(1) is started. All processes executed by \acommand\a
	also have \an\a\bDFS\b enabled.]
[+?The main \b3d\b service is \aviewpathing\a. A \aviewpath\a mounts one
	directory tree on top of another. Files in the top level
	(directory tree) obscure files in the lower level. Lower level files
	have copy-on-write semantics to the top level. i.e., files in the lower
	level are readonly; if a lower level file is modified it is first
	copied to the top level before modification. Viewpath mounts
	directories can be chained. All levels but the top in a chain are
	readonly. The traditional \bVPATH=\b\ad1:d2:d3 viewpathing is
	specified by the two \b3d\b mounts "\ad1 d2\a" and "\ad2 d3\a".]
[+?The following service mounts are initialized by default:]{
	[+/dev/fd?\b/dev/fd/\b\an\a refers to the open file descriptor \an\a
		in the current process.]
	[+/dev/fdp?\b/dev/fdp/local/\aservice\a opens a 2-way stream to
		\aservice\a on the local host. The stream supports file
		descriptor exchange.]
	[+/dev/tcp?\b/dev/tcp/\b\ahost\a\b/\b\aservice\a opens a 2-way
		connection to \aservice\a on \ahost\a. \binetd\b(8) services
		are named either by \binet.\b\aservice\a or by port number.]
	[+/dev/udp?\b/dev/udb/\b\ahost\a\b/\b\aservice\a opens a datagram
		connection to \aservice\a on \ahost\a. \binetd\b(8) services
		are named either by \binet.\b\aservice\a or by port number.]
}
[c:command?Ignored; allows \b3d\b to be invoked like \bsh\b(1).]
[d:debug?Set the debug trace level to \alevel\a. Higher levels produce more
	output.]#[level]
[l:lite?Disable the default service mounts.]
[m:mount?Mount the path \atop\a onto the path \abottom\a. If \abottom\a is a
	directory then the directory \atop\a is viewpathed on top of \abottom\a.
	Otherwise \abottom\a must match a \b3d\b \bfs\b path of the form
	\b/#\b\aspecial\a.]:["top bottom"]
[n!:exec?Execute \acommand\a. \b--noexec\b shows how \acommand\a would be
	invoked but does not execute.]
[o:output?\b--trace\b and \b--debug\b output is written to \afile\a instead
	of the standard error.]:[file]
[s:shell?Sets the \bsh\b(1) executable path to \afile\a.]:[file:=ksh]
[t:trace?If \aid\a is \b1\b then intercepted system calls for all child
	processes will be traced. If \aid\a is greater than \b1\b then
	intercepted system calls for the current process and all
	children will be traced.]#[id]
[v:version?Use \b3d\b version \amajor.minor\a instead of the latest
	version.]:[major.minor]
[x:?Passed to the \b3d\b \bsh\b(1).]
[+FS PATHS?\b3d\b \bfs\b paths of the form \b/#\b\aspecial\a[\b/\b...]]
	control the semantics of \b3d\b and allow services to be attached to
	the file namespace. Unsupported \bfs\b paths are silently ignored. The
	supported \bfs\b paths are:]{
	[+/#fd?]
	[+/#fs/\aname\a[/\aoption\a/...]]?]
	[+/#map?]
	[+/#null?]
	[+/#option?]{
		[+2d?]
		[+3d?]
		[+boundary?]
		[+call=\avalue\a?]
		[+count=\avalue\a?]
		[+debug=\alevel\a?]
		[+dump=\b[\avalue\a]]?]{
			[+call=\avalue\a?]
			[+file=\avalue\a?]
			[+fs=\avalue\a?]
			[+map=\avalue\a?]
			[+mount=\avalue\a?]
			[+safe=\avalue\a?]
			[+state=\avalue\a?]
			[+view=\avalue\a?]
		}
		[+file=\avalue\a?]
		[+fork?]
		[+init?]
		[+license=\apath\a?]
		[+mount=\avalue\a?]
		[+table=\avalue\a?]
		[+test=\abitmask\a?]
		[+timeout=\avalue\a?]
		[+timestamp?]
		[+trace=\avalue\a?]
		[+version=1?]
	}
	[+/#pwd?]
	[+/#safe?]
	[+/#view?]
}
[+DETAILS?\an\a\bDFS\b semantics are applied by intercepting system calls with
	a dll that is preloaded at process startup before \amain\a() is called.
	The interception mechanism may involve the environment on some systems;
	in those cases commands like \benv\b(1) that clear the enviroment
	before execution may defeat the \b3d\b intercepts. The intercepted
	system calls are listed by \bvpath - /#option/dump=call\b. Note that
	implementation system call names may differ from those listed in
	section 2 of the manual; \bstat\b(2) is especially vulnerbale to
	mangling.]
}
[+?\b3d\b ignores calls not present in a particular host system. In addition,
	\b3d\b only works on dynamically linked executables that have neither
	set-uid nor set-gid permissions.]

[ command [ arg ... ] ]

[+ENVIRONMENT?\b3d\b is implemented by four components: the \b3d\b script,
	located on \b$PATH\b; the \b3d\b dll (shared library), located either
	on \b$PATH\b or in one of the \b../lib\b* directories on \b$PATH\b,
	depending on local compilation system conventions; and the \b2d\b
	\bsh\b(1) alias and \b_2d\b \bsh\b(1) function that allow commands
	to be run with \an\a\bDFS\b semantics disabled. Systems like
	\bsgi.mips3\b that support multiple a.out formats may have multiple
	versions of the \b3d\b dll. In all cases the \b3d\b script handles
	the dll search.]
[+EXAMPLES?]{
	[+$ 3d?]
	[+$ 3d $INSTALLROOT $PACKAGEROOT?]
	[+$ VPATH=$INSTALLROOT:$PACKAGEROOT 3d?]
	[+$ 3d ls -l?]
	[+$ 2d ls -l?]
}
[+SEE ALSO?\benv\b(1), \bie\b(1), \bsh\b(1), \btrace\b(1), \bwarp\b(1),
	\bstat\b(2)]
'
	;;
*)	ARGV0=""
	USAGE='clnxd:m:o:s:t:v:[ command [ arg ... ] ]'
	;;
esac

test . -ef "$PWD" || PWD=`pwd`
case $dir in
*/3d)	;;
*)	case $0 in
	*/*)	dir=$0
		;;
	*)	dir=`whence 3d 2>/dev/null`
		dir=`whence -p $dir 2>/dev/null`
		;;
	esac
esac
case $dir in
/*)	dir=${dir%/*} ;;
*)	dir=${dir#./}; dir=$PWD/${dir%/*} ;;
esac
abi=
dir=${dir%/*}
case $dir in
*/arch/sgi.mips[0-9]*)
	if	test -d /lib32 -o -d /lib64
	then	d=${dir%-*}
		d=${d%?}
		for a in 2: 3:N32 4:64
		do	t=${a#*:}
			a=${a%:*}
			if	test -d $d$a
			then	abi="$abi $d$a:$t"
			fi
		done
	fi
	;;
esac
version=
mount="- -"
full="/dev/fdp/local/nam/user /#fs/nam/name/unique /dev/fdp /#nam /dev/tcp /#nam /dev/udp /#nam /dev/fd /#fd"
options=
output=
show=
shell=$SHELL
trace=
SHELL=
case $VPATH in
*:*)	ifs=$IFS
	IFS=:
	o=
	for a in $VPATH
	do	case $a in
		""|-)	;;
		*)	case $o in
			""|-)	;;
			*)	mount="$mount $o $a" ;;
			esac
			;;
		esac
		o=$a
	done
	IFS=$ifs
	;;
esac

: grab the options

while	getopts $ARGV0 "$USAGE" OPT
do	case $OPT in
	c)	break ;;
	d)	trace="$trace/debug=$OPTARG" ;;
	l)	full= ;;
	m)	mount="$mount $OPTARG" ;;
	n)	show="print -u2 --" ;;
	o)	output=$OPTARG ;;
	s)	SHELL=$OPTARG ;;
	t)	trace="$trace/trace=$OPTARG" ;;
	v)	case $OPTARG in
		/*)	version=$OPTARG ;;
		*)	version=.$OPTARG ;;
		esac
		;;
	x)	options="$options -$OPT" ;;
	*)	exit 2 ;;
	esac
done
shift $OPTIND-1

case $full in
?*)	mount="$full $mount" ;;
esac
mount="- /#option/3d$trace $mount"
case $output in
?*)	mount="$output /#fs/option $mount" ;;
esac
default=$version
case $version in
/*)	;;
*)	for x in $dir/lib/lib3d.s[lo]$version
	do	case $x in
		*/lib3d.s[lo]*[a-zA-Z]*)
			;;
		*)	version=$x
			break
			;;
		esac
	done
	;;
esac
exp=
if	test -f "$version"
then	if	test -d $dir/3d
	then	dir=$dir/3d
	fi
	case :$FPATH: in
	*:$dir/fun:*)	;;
	*)		export FPATH=$dir/fun${FPATH:+:$FPATH} ;;
	esac
	case :$PATH: in
	*:$dir/bin:*)	;;
	*)		export PATH=$dir/bin:$PATH ;;
	esac
	exp="$exp LD_PRELOAD=$version"
	case $abi:-:$version in
	?*:-:$dir/*)
		v=${version#$dir/}
		for a in $abi
		do	d=${a%:*}
			a=${a#*:}
			exp="$exp _RLD${a}_LIST=$d/$v:DEFAULT"
		done
		;;
	*)	v=${version%.*}
		s=${version##*.}
		if	test -f ${v}-n32.$s
		then	exp="$exp _RLD64_LIST=$version:DEFAULT _RLDN32_LIST=$version-n32.$s:DEFAULT _RLD_LIST=${v}-o32.$s:DEFAULT"
		elif	test -f ${v}-64.$s -o -f ${v}-o32.$s
		then	exp="$exp _RLD64_LIST=$version-64.$s:DEFAULT _RLDN32_LIST=$version:DEFAULT _RLD_LIST=${v}-o32.$s:DEFAULT"
		else	exp="$exp _RLD_LIST=$version:DEFAULT"
		fi
		if	test -f ${dir}/lib/libdir64.$s
		then	exp="$exp:${dir}/lib/libdir64.$s"
		fi
		;;
	esac
	export $exp
	case $mount in
	??*)	export __=${mount#' '} ;;
	*)	unset __ ;;
	esac
	unset VPATH
else	print -u2 "$command: $version: no shared library"
	case $default in
	"*")	;;
	*)	exit 1 ;;
	esac
	version=
	options=
fi
case $SHELL in
"")	t=`whence ksh 2>/dev/null`
	SHELL=`PATH=$dir/bin:$PATH whence -p $t 2>/dev/null`
	;;
esac
SHELL=${SHELL:-$shell}
SHELL=${SHELL:-/bin/sh}
export SHELL
case `vpath /#option/3d 2>/dev/null` in
*/2d)	$show vpath $mount ;;
esac
case $1:$#:$options in
-*)	;;
*:0:*|*:*:?*)
	shell="$SHELL $options"
	;;
*)	t=`whence $1 2>/dev/null`
	t=`whence -p $t 2>/dev/null`
	case $t in
	""|"''"|'""')	shell="$SHELL $options" ;;
	*)		shell= ;;
	esac
	;;
esac
case "$shell : $1" in
"$SHELL : -c")
	shift
	$show eval "$@"
	;;
*)	case $show in
	"")	exec $shell "$@" ;;
	*)	case $version in
		"")	$show __="'$__'" exec $shell "$@" ;;
		*)	$show __="'$__'" $exp exec $shell "$@" ;;
		esac
		;;
	esac
	;;
esac
