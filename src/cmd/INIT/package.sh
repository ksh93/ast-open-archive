####################################################################
#                                                                  #
#             This software is part of the ast package             #
#                Copyright (c) 1999-2001 AT&T Corp.                #
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
# package - source and binary package control
# this script written to make it through all sh variants
# Glenn Fowler <gsf@research.att.com>

case $-:$BASH_VERSION in
*x*:[0-9]*)	: bash set -x is broken :; set +ex ;;
esac

command=package
version=2001-01-01

src="cmd contrib etc lib"
use="/home /usr/common /exp /usr/local /usr/add-on /usr/addon /usr/tools /usr /opt"
lib="/usr/local/lib /usr/local/shlib"
ccs="/usr/kvm /usr/ccs/bin"
org="gnu GNU"

admin_db=admin.db
admin_env=admin.env

case `(getopts '[-][123:xyz]' opt --xyz; echo 0$opt) 2>/dev/null` in
0123)	USAGE=$'
[-?
@(#)$Id: '${command}$' (AT&T Labs Research) '${version}$' $
]'$USAGE_LICENSE$'
[+NAME?package - source and binary package control]
[+DESCRIPTION?The \bpackage\b command controls source and binary packages.
	It is a \bsh\b(1) script coded for maximal portability. All package
	files are in the \b$PACKAGEROOT\b directory tree. Binary package
	files are in the \b$INSTALLROOT\b (\b$PACKAGEROOT/arch/\b\ahosttype\a)
	tree, where \ahosttpe\a=`\bpackage\b`. All \aactions\a but \bhost\b
	and \buse\b require the current directory to be under
	\b$PACKAGEROOT\b. See \bDETAILS\b for more information.]
[+?Note that no environment variables need be set by the user; \bpackage\b
	determines the environment based on the current working directory.
	The \buse\b action starts a \bsh\b(1) with the environment initialized.
	\bCC\b, \bCCFLAGS\b, \bHOSTTYPE\b and \bSHELL\b may be set by explicit
	command argument assignments to override the defaults.]
[+?Packages are composed of components. Each component is built and installed
	by an \bast\b \bnmake\b(1) makefile. Each package is also described by
	an \bnmake\b makefile that lists its components and provides a content
	description. The package makefile and component makefiles provide all
	the information required to read, write, build and install packages.]
[+?Package recipients only need \bsh\b(1) and \bcc\b(1) to build and install
	source packages, and \bsh\b to install binary packages. \bnmake\b and
	\bksh93\b are required to write new packages. An \b$INSTALLROOT/bin/cc\b
	script may be supplied for some architectures. This script supplies
	a reasonable set of default options for compilers that accept multiple
	dialects or generate multiple object/executable formats.]
[+?The command arguments are composed of a sequence of words: zero or more
	\aqualifiers\a, one \aaction\a, and zero or more action-specific
	\aarguments\a, and zero or more \aname=value\a definitions.
	\apackage\a names a particular package. The naming scheme is a \b-\b
	separated hierarchy; the leftmost parts describe ownership, e.g.,
	\bgnu-fileutils\b, \bast-base\b. If no packages are specified then all
	packages are operated on. \boptget\b(3) documentation options are also
	supported. The default with no arguments is \bhost type\b.]
[+?The qualifiers are:]{
	[+debug|environment?Show environment and actions but do not execute.]
	[+force?Force the action to override saved state.]
	[+never?Run make -N and show other actions.]
	[+only?Only operate on the specified packages.]
	[+quiet?Do not list captured action output.]
	[+show?Run make -n and show other actions.]
	[+verbose?Provide detailed action output.]
	[+DEBUG?Trace the package script actions in detail.]
}
[+?The actions are:]{
	[+admin\b [\ball\b]] [\bdb\b \afile\a]] [\bon\b \apattern\a]] [\aaction\a ...]]?Apply \aaction\a ... to
		the hosts listed in \afile\a. If \afile\a  is omitted then
		\badmin.db\b is assumed. The caller must have
		\brcp\b(1) and \brsh\b(1) access to the hosts. Output for
		\aaction\a is saved per-host in the file
		\aaction\a\b.log/\b\ahost\a. Logs can be viewed by
		\bpackage admin\b [\bon\b \ahost\a]] \bresults\b [\aaction\a]].
		By default only local PACKAGEROOT hosts are selected from
		\afile\a; \ball\b selects all hosts. \bon\b \apattern\a selects
		only hosts matching the \b|\b separated \apattern\a. \afile\a
		contains four types of lines. Blank lines and lines beginning
		with \b#\b are ignored. Lines starting with \aid\a=\avalue\a
		are variable assignments. If a package list is not specified on
		the command line the \aaction\a applies to all packages; a
		variable assigment \bpackage\b=\"\alist\a\" applies \aaction\a
		to the packages in \alist\a for subsequent hosts in
		\afile\a. The remaining line type is a host description
		consisting of 6 tab separated fields. The first 3 are mandatory;
		the remaining 3 are updated by the \badmin\b action.
		\afile\a is saved in \afile\a\b.old\b before update.
		The fields are:]{
			[+hosttype?The host type as reported by
				\"\bpackage\b\".]
			[+[user@]]host?The host name and optionally user name
				for \brcp\b(1) and \brsh\b(1) access.]
			[+[server::]]PACKAGEROOT?The absolute remote package
				root directory and optionally the server name
				if the directory is on a different server than
				the master package root directory. If this
				directory contains an \b'$admin_env$'\b \bsh\b(1)
				script then it is sourced before \aaction\a
				is done.]
			[+date?\aYYMMDD\a of the last action.]
			[+time?Elapsed wall time for the last action.]
			[+M T W?The \badmin\b action \bmake\b, \btest\b and
				\bwrite\b action error counts.]
		}
	[+contents\b [ \apackage\a ... ]]?List description and components
		for \apackage\a on the standard output.]
	[+copyright\b [ \apackage\a ... ]]?List the general copyright notice(s)
		for \apackage\a on the standard output. Note that individual
		components in \apackage\a may contain additional or replacement
		notices.]
	[+help\b [ \aaction\a ]]?Display help text (for \aaction\a) on the
		standard output.]
	[+host\b [ \aattribute\a... ]]?List architecture/implementation
		dependent host information on the standard output. \btype\b is
		listed if no attributes are specified. Information is listed on
		a single line in \aattribute\a order. The attributes are:]{
			[+cpu?The number of cpus; 1 if the host is not a
				multiprocessor.]
			[+name?The host name.]
			[+rating?The cpu rating in pseudo mips; the value is
				useful useful only in comparisons with rating
				values of other hosts. Other than a vax rating
				(mercifully) fixed at 1, ratings can vary
				wildly but consistently from vendor mips
				ratings. \bcc\b(1) may be required to determine
				the rating.]
			[+type?The host type, usually in the form
				\avendor\a.\aarchitecture\a, with an optional
				trailing -\aversion\a. The main theme is that
				type names within a family of architectures are
				named in a similar, predictable style. OS point
				release information is avoided as much as
				possible, but vendor resistance to release
				incompatibilities has for the most part
				been futile.]
		}
	[+html\b [ \aaction\a ]]?Display html help  text (for \aaction\a) on the
		standard output.]
	[+install\b [ flat ]] [ \aarchitecture\a ... ]] \adirectory\a [ \apackage\a ... ]]?Copy
		the package binary hierarchy to \adirectory\a. If
		\aarchitecture\a is omitted then all architectures are
		installed. If \bflat\b is specified then exactly
		one \aarchitecture\a must be specified; this architecture will
		be installed in \adirectory\a without the \barch/\b\aHOSTTYPE\a
		directory prefixes. Otherwise each architecture will be
		installed in a separate \barch/\b\aHOSTTYPE\a subdirectory of
		\adirectory\a. \adirectory\a must be an existing directory.]
	[+license\b [ \apackage\a ... ]]?List the source license(s) for
		\apackage\a on the standard output. Note that individual
		components in \apackage\a may contain additional or replacement
		licenses.]
	[+list\b [ \apackage\a ... ]]?List the name, version and prerequisites
		for \apackage\a on the standard output.]
	[+make\b [ \apackage\a ]] [ \atarget\a ... ]]?Build and install. The
		default \atarget\a is \binstall\b, which makes and installs
		\apackage\a. If the standard output is a terminal then the
		output is also captured in
		\b$INSTALLROOT/lib/package/gen/make.out\b.]
	[+read\b [ \apackage\a ... | \aarchive\a ... ]]?Read the named
		package or archive(s). Must be run from the package root
		directory. Archives are searched for in \b.\b and
		\blib/package/tgz\b. Each package archive is read only once.
		The file \blib/package/tgz/\b\apackage\a[.\atype\a]]\b.tim\b tracks
		the read time. See the \bwrite\b action for archive naming
		conventions.]
	[+release\b [ [\aCC\a]]\aYY-MM-DD\a [ [\acc\a]]\ayy-mm-dd\a ]] ]]
		[ \apackage\a ]]?Display recent changes for the date range
		[\aCC\a]]\aYY-MM-DD\a (up to [\acc\a]]\ayy-mm-dd\a.), where
		\b-\b means lowest (or highest.) If no dates are specified
		then changes for the last 4 months are listed. \apackage\a may
		be a package or component name.]
	[+remove\b [ \apackage\a ]]?Remove files installed for \apackage\a.]
	[+results\b [ \bpath\b ]] [ \bold\b ]] [ \bmake\b | \btest\b ]]?List
		results and interesting messages captured by the most recent
		\bmake\b (default) or \btest\b action. \bold\b specifies the
		previous results, if any (current and previous results
		are retained.) \b$HOME/.pkgresults\b, if it exists, must
		contain an \begrep\b(1) expression of result lines to be
		ignored. \bpath\b lists the results file path name on the
		standard output.]
	[+test\b [ \apackage\a ]]?Run the regression tests for \apackage\a.
		If the standard output is a terminal then the output is also
		captured in \b$INSTALLROOT/lib/package/gen/test.out\b. In
		general a package must be made before it can be tested.
		Components tested with the \bregress\b(1) command require
		\bksh93\b.]
   	[+use\b [ \auid\a | \apackage\a | - ]] [ command ...]]?Run \acommand\a,
		or an interactive shell if \acommand\a is omitted, with the
		environment initialized for using the package (can you say
		\ashared\a \alibrary\a or \adll\a without cussing?) If either
		\auid\a or \apackage\a is specified then it is used to
		determine a \b$PACKAGEROOT\b, possibly different from the
		current directory. For example, to try out bozo`s package:
		\bpackage use bozo\b. The \buse\b action may be run from any
		directory.]
	[+verify\b [ \apackage\a ]]?Verify installed binary files against the
		checksum files in
		\b$INSTALLROOT/lib/\b\apackage\a\b/gen/*.sum\b.
		The checksum files contain mode, user and group information.
		If the checksum matches for a given file then the mode, user
		and group are changed as necessary to match the checksum entry.
		A warning is printed on the standard error for each mismatch.
		Requires the \bast\b package \bcksum\b(1) command.]
	[+write\b [\aformat\a]] \atype\a ... [ \apackage\a ...]]?Write a
		package archive for \apackage\a. All work is done in the
		\b$PACKAGEROOT/lib/package\b directory. \aformat\a-specific
		files are placed in the \aformat\a subdirectory. A
		\apackage\a[.\atype\a]]\b.tim\b file in this directory tracks
		the write time and prevents a package from being read in the
		same root it was written. If more than one file is generated
		for a particular \aformat\a then those files are placed in the
		\aformat\a/\apackage\a subdirectory. File names in the
		\aformat\a subdirectory will contain the package name,
		a \ayyyy-mm-dd\a date, and for binary packages,
		\aHOSTTYPE\a. If \apackage\a is omitted then an ordered list
		previously written packages is generated. If \bonly\b is
		specified then only named packages will be written; otherwise
		prerequisite packages are written first. Package components
		must be listed in \apackage\a\b.pkg\b. \aformat\a may be
		one of:]{
			[+exp?Generate an \bexptools\b maintainer source
				archive and \aNPD\a file, suitable for
				\bexpmake\b(1)]
			[+lcl?Generate a package archive suitable for
				restoration into the local source tree (i.e.,
				the source is not annotated for licencing.)]
			[+pkg?Generate a \bpkgmk\b(1) package suitable for
				\bpkgadd\b(1).]
			[+rpm?Generate an \brpm\b(1) package.]
			[+tgz?Generate a \bgzip\b(1) \btar\b(1) package
				archive. This is the default.]
		}
		[+?\btype\b specifies the package type. A package may be either
			\bsource\b or \bbinary\b. A source package contains all
			the source needed to build the corresponding binary
			package. One of \bsource\b or \bbinary\b must be
			specified.]
		[+?A package may be either a \bbase\b or \bdelta\b. A base
			package contains a complete copy of all components.
			A delta package contains only changes from a previous
			base package. Delta recipients must have the \bast\b
			\bpax\b(1) command (in the \bast-base\b package.) If
			neither \bbase\b nor \bdelta\b is specified, then the
			current base is overwritten if there are no deltas
			referring to the current base. Only the \btgz\b and
			\blcl\b formats support \bdelta\b. If \bbase\b is
			specified then a new base and two delta archives are
			generated: one delta to generate the new base from the
			old, and one delta to generate the old base from the
			new; the old base is then removed. If \bdelta\b is
			specified then a new delta referring to the current
			base is written.]
		[+?\apackage\a\b.pkg\b may reference other packages. By default
			a pointer to those packages is written. The recipient
			\bpackage read\b will then check that all required
			packages have been downloaded. If \bclosure\b is
			specified then the components for all package
			references are included in the generated package.
			This may be useful for \blcl\b and versioning.]
		[+?All formats but \blcl\b annotate each \bsource\b file (not
			already annotated) with a license comment as it is
			written to the package archive using \bproto\b(1).]
}
[+DETAILS?The package directory hierarchy is rooted at \b$PACKAGEROOT\b. All
	source and binaries reside under this tree. A two level viewpath is
	used to separate source and binaries. The top view is architecture
	specific, the bottom view is shared source. All building is done in
	the architecture specific view; no source view files are intentionally
	changed. This means that many different binary architectures can be
	made from a single copy of the source.]
[+?Independent \b$PACKAGEROOT\b hierarchies can be combined by appending
	\b$INSTALLROOT:$PACKAGEROOT\b pairs to \bVPATH\b. The \bVPATH\b viewing
	order is from left to right. Each \b$PACKAGEROOT\b must have a
	\b$PACKAGEROOT/lib/package\b directory.]
[+?Each package contains one or more components. Component source for the
	\afoo\a command is in \b$PACKAGEROOT/src/cmd/\b\afoo\a, and source for
	the \abar\a library is in \b$PACKAGEROOT/src/lib/lib\b\abar\a. This
	naming is for convenience only; the underlying makefiles handle
	inter-component build order. The \bINIT\b component, which contains
	generic package support files, is always made first, then the
	components named \bINIT\b*, then the component order determined by
	the closure of component makefile dependencies.]
[+?\b$PACKAGEROOT/lib/package\b contains package specific files. The package
	naming convention is \agroup\a[-\apart\a]]; e.g., \bast-base\b,
	\bgnu-fileutils\b. The *\b.pkg\b files are ast \bnmake\b(1) makefiles
	that contain the package name, package components, references to other
	packages, and a short package description. *\b.pkg\b files are used by
	\bpackage write\b to generate new source and binary packages.]
[+?\b$PACKAGEROOT/lib/package/\b\agroup\a\b.lic\b files contain license
	information that is used by the \bast\b \bproto\b(1) and \bnmake\b(1)
	commands to generate source and binary license strings. \agroup\a is
	determined by the first \b:PACKAGE:\b operator name listed in the
	component \bnmake\b makefile. \agroup\a\b.lic\b files are part of the
	licensing documentation and must not be altered; doing so violates the
	license. Each component may have its own \bLICENSE\b file that
	overrides the \agroup\a\b.lic\b file. The full text of the licenses
	are in the \b$PACKAGEROOT/lib/package/LICENSES\b and
	\b$INSTALLROOT/lib/package/LICENSES\b directories.]
[+?A few files are generated in \b$PACKAGEROOT/lib/package/gen\b and
	\b$INSTALLROOT/lib/package/gen\b. \apackage\a\b.ver\b contains one
	line consisting of \apackage version release\a \b1\b for the most
	recent instance of \apackage\a read into \b$PACKAGEROOT\b, where
	\apackage\a is the package name, \aversion\a is the \aYYYY-MM-DD\a
	base version, and \arelease\a is \b0000\b for the base release, and
	\b0\b\annn\a for the \annn\ath delta. \apackage\a\b.req\b contains
	*\b.ver\b entries for the packages required by \apackage\a, except
	that the fourth field is \b0\b instead of \b1\b. All packages except
	\bINIT\b require the \bINIT\b package. A simple sort of
	\apackage\a\b.pkg\b and *\b.ver\b determines if the required package
	have been read in. Finally, \apackage\a\b.txt\b and
	\apackage\a\a.html\b contain the README text for \apackage\a and
	all its components. Included are all changes added to the component
	\bRELEASE\b, \bCHANGES\b or \bChangeLog\b files dated since the two
	most recent base releases. Component \bRELEASE\b files contain tag
	lines of the form [\aYY\a]]\aYY-MM-DD\a [ \atext\a ]] (or \bdate\b(1)
	format dates) followed by README text, in reverse chronological
	order (newer entries at the top of the file.) \bpackage release\b
	lists this information, and \bpackage contents ...\b lists the
	descriptions and components.]
[+?\b$HOSTYPE\b names the current binary architecture and is determined by the
	output of \bpackage\b (no arguments.) The \b$HOSTTYPE\b naming scheme
	is used to separate incompatible executable and object formats. All
	architecture specific binaries are placed under \b$INSTALLROOT\b
	(\b$PACKAGEROOT/arch/$HOSTTYPE\b.) There are a few places that match
	against \b$HOSTTYPE\b when making binaries; these are limited to
	makefile compiler workarounds, e.g., if \b$HOSTTYPE\b matches
	\bhp.*\b then turn off the optimizer for these objects. All other
	architecture dependent logic is handled either by the \bast\b
	\biffe\b(1) command or by component specific configure scripts.]
[+?Each component contains an \bast\b \bnmake\b(1) makefile (either
	\bNmakefile\b or \bMakefile\b) and a \bMAM\b (make abstract machine)
	file (\bMamfile\b.) A Mamfile contains a portable makefile description
	that is used by \bmamake\b(1) to simulate \bnmake\b. Currently there is
	no support for old-make/gnu-make makefiles; if the binaries are just
	being built then \bmamake\b will suffice; if source or makefile
	modifications are anticipated then \bnmake\b (in the \bast-base\b
	package) should be used. Mamfiles are automatically generated by
	\bpackage write\b.]
[+?Most component C source is prototyped. If \b$CC\b (default value \bcc\b) is
	not a prototyping C compiler then \bpackage make\b runs \bproto\b(1) on
	portions of the \b$PACKAGEROOT/src\b tree and places the converted
	output files in the \b$PACKAGEROOT/proto/src\b tree. Converted files
	are then viewpathed over the original source. \bproto\b(1) converts an
	ANSI C subset to code that is compatible with K&R, ANSI, and C++
	dialects.]
[+?All scripts and commands under \b$PACKAGEROOT\b use \b$PATH\b relative
	pathnames (via the \bast\b \bpathpath\b(3) function); there are no
	imbedded absolute pathnames. This means that binaries generated
	under \b$PACKAGEROOT\b may be copied to a different root; users need
	only change their \b$PATH\b variable to reference the new installation
	root \bbin\b directory. \bpackage install\b installs binary packages
	in a new \b$INSTALLROOT\b.]

[ qualifier ... ] [ action ] [ arg ... ] [ n=v ... ]

[+SEE ALSO?\bautoconfig\b(1), \bcksum\b(1), \bexpmake\b(1), \bgzip\b(1),
	\bmake\b(1), \bmamake\b(1), \bnmake\b(1), \bpax\b(1), \bpkgadd\b(1),
	\bpkgmk\b(1), \bproto\b(1), \bratz\b(1), \brpm\b(1), \bsh\b(1),
	\btar\b(1), \boptget\b(3)]
'
	case $* in
	help)	set -- --man ;;
	esac
	while	getopts -a $command "$USAGE" OPT
	do	:
	done
	shift `expr $OPTIND - 1`
	;;
esac

# check the args

case $CC in
'')	CC=cc ;;
esac

action=
admin_all=0
admin_on=
exec=
force=0
hi=
html=0
ifs=$IFS
lo=
make=
makeflags='-k -K'
nl="
"
noexec=
only=0
output=
quiet=0
show=:
tab="        "
verbose=0
DEBUG=
PROTOROOT=-

while	:
do	case $# in
	0)	set host type ;;
	esac
	case $1 in
	admin|contents|copyright|host|install|license|list|make|read|release|remove|results|test|use|verify|write|TEST)
		action=$1
		shift
		break
		;;
	debug|environment)
		exec=echo make=echo show=echo
		;;
	force)	force=1
		;;
	never)	exec=echo noexec=-N
		;;
	only)	only=1
		;;
	quiet)	quiet=1
		;;
	show)	exec=echo noexec=-n
		;;
	verbose)verbose=1
		;;
	DEBUG)	DEBUG=1
		PS4='+$LINENO+ '
		set -x
		;;
	help|HELP|html|man|--[?m]*)
		case $1 in
		help)	code=0 ;;
		html)	code=0 html=1 ;;
		*)	code=2; exec 1>&2 ;;
		esac
		case $html in
		1)	bO="<HTML>
<HEAD>
<TITLE>$2 package installation instructions</TITLE>
<HEAD>
<BODY bgcolor=white link=teal vlink=dimgray>"
			eO='</BODY>
</HTML>'
			bH="<CENTER><H3><FONT face=courier color=red>"
			eH='</FONT></H3></CENTER>'
			bP='<P>'
			bL='<P><TABLE cellpadding=0 cellspacing=2>'		
			bL2='<P><TABLE border=0 cellpadding=0 cellspacing=2>'		
			eL='</TABLE><P>'
			bT='<TR><TD align=right valign=top><B>'
			bD='</B></TD><TD align=left>'	eD='</TD></TR>'
			bB='<B>'			eB='</B>'
			bI='<I>'			eI='</I>'
			bX='<PRE>'			eX='</PRE>'
			bF='<TT>'			eF='</TT>'
			Mdate='<A href=../../man/man1/date.html>date</A>(1)'
			Mgunzip='<A href=../../man/man1/gzip.html>gunzip</A>(1)'
			Mnmake='<A href=../../man/man1/nmake.html>nmake</A>(1)'
			Mpackage='<A href=../../man/man1/package.html>package</A>(1)'
			Mproto='<A href=../../man/man1/proto.html>proto</A>(1)'
			Mratz='<A href=../../man/man1/ratz.html>ratz</A>'
			Mtar='<A href=../../man/man1/tar.html>tar</A>(1)'
			;;
		*)	bO=''				eO=''
			bH=''				eH=':'
			bP=''
			bL=''				eL=''
			bL2=''
			bT='  '
			bD=' '				eD=''
			bB=''				eB=''
			bI=''				eI=''
			bX=''				eX=''
			bF='"'				eF='"'
			Mdate='date(1)'
			Mgunzip='gunzip(1)'
			Mnmake='nmake(1)'
			Mpackage='package(1)'
			Mproto='proto(1)'
			Mratz='ratz'
			Mtar='tar(1)'
			;;
		esac
		case $2 in
		binary)	echo "${bO}
${bH}Binary Package Installation Instructions${eH}
${bL}
${bT}(1)${bD}Do not install packages as ${bI}root/super-user${eI}. Although some components may
      have setuid executables, few must be owned by ${bI}root${eI}. These are best
      changed manually when the security implications are understood.${eD}
${bT}(2)${bD}Choose a package root directory and cd to it. This will be a local work
      area for all packages. See ${bB}(7)${eB} for installing packages for public use.${eD}
${bT}(3)${bD}Create the subdirectory ${bB}lib/package/tgz${eB} and download all package archives
      into that directory. The ${Mpackage} command maintains the contents of this
      directory, and deletes old archives as new ones are read in. Package
      delta archives require the most recent base, so manually removing files
      in this directory may invalidate future deltas.${eD}
${bT}(4)${bD}If the ${bB}bin/package${eB} command does not exist then manually read the ${bB}INIT${eB} binary package:${bX}
		gunzip < lib/package/tgz/INIT.${bI}YYYY-MM-DD${eI}.0000.${bI}HOSTTYPE${eI}.tgz |
			tar xvf -${eX}
      where ${bI}HOSTTYPE${eI} is compatible with your host architecture. If your system
      does not have ${Mtar} or ${Mgunzip} then download the ${Mratz} binary package,
      install it, and manually read the ${bB}INIT${eB} binary command:${bX}
		mkdir bin
		cp lib/package/tgz/ratz.${bI}YYYY-MM-DD${eI}.0000.${bI}HOSTTYPE${eI}${bB}.exe${eB} bin/ratz
		bin/ratz -lv < lib/package/tgz/INIT.YYYY-MM-DD.0000.${bI}HOSTTYPE${eI}.tgz${eX}${eD}
${bT}(5)${bD}Read all unread package archive(s):${bX}
		bin/package read${eX}
      Both binary and source packages will be read by this step.${eD}
${bT}(6)${bD}When ${bB}(5)${eB} completes run${bX}
		bin/package use${eX}
      to get an interactive ${bB}\$SHELL${eB} that sets up the environment
      for using the package binaries:${bL2}
	${bT}\$HOSTTYPE	${bD}the current host type${eD}
	${bT}\$PACKAGEROOT	${bD}the root directory for all packages (${bB}\$P${eB})${eD}
	${bT}\$INSTALLROOT	${bD}the installation root for the current host type (${bB}\$A${eB})${eD}
	${bT}\$PATH		${bD}references to the install and package bin directories${eD}
        ${bT}${bI}DLL-MAGIC${eI}	${bD}environment magic for locating package shared libraries${eD}${eL}${eD}
${bT}(7)${bD}Once you are satisfied with a package its binaries can be installed in
      a public area by:${bX}
		bin/package install ${bI}DIRECTORY PACKAGE${eI}${eX}
      and users can access the binaries by${bX}
		bin/package use ${bI}DIRECTORY${eI}${eX}
      or by exporting the environment variable definitions listed by${bX}
		bin/package environment use ${bI}DIRECTORY${eI}${eX}${eD}
${eL}${eO}"
			;;
		intro)	echo "${bO}
${bH}Package Hierarchy Details${eH}
${bP}
The package directory hierarchy is rooted at ${bB}\$PACKAGEROOT${eB}. All source and
binaries reside under this tree. A two level viewpath is used to separate
source and binaries. The top view is architecture specific, the bottom view
is shared source. All building is done in the architecture specific view;
no source view files are intentionally changed. This means that many
different binary architectures can be made from a single copy of the source.
${bP}
Each package contains one or more components. Component source for the ${bI}FOO${eI}
command is in ${bB}\$PACKAGEROOT/src/cmd/${eB}${bI}FOO${eI}, and source for the ${bI}BAR${eI} library is
in ${bB}\$PACKAGEROOT/src/lib/lib${eB}${bI}BAR${eI}. This naming is for convenience only; the
underlying makefiles handle inter-component build order. The ${bB}INIT${eB} component,
which contains generic package support files, is always made first, then the
components named ${bB}INIT${eB}*, then the order determined by the closure of component
makefile dependencies.
${bP}
${bB}\$PACKAGEROOT/lib/package${eB} contains package specific files. The package naming
convention is ${bI}GROUP${eI}[${bI}-PART${eI}]; e.g., ${bB}ast-base${eB}, ${bB}gnu-fileutils${eB}. The *${bB}.pkg${eB} files
are ${bB}ast${eB} ${Mnmake} makefiles that contain the package name, package components,
references to other packages, and a short package description. *${bB}.pkg${eB} files
are used by ${bF}package write${eF} to generate new source and binary packages.
${bP}
${bB}\$PACKAGEROOT/lib/package/${eB}${bI}GROUP${eI}${bB}.lic${eB} files contain license information that
is used by the ${bB}ast${eB} ${Mproto} and ${Mnmake} commands to generate source and
binary license strings. ${bI}GROUP${eI} is determined by the first ${bB}:PACKAGE:${eB} operator
name listed in the component ${bB}nmake${eB} makefile. ${bI}GROUP${eI}${bB}.lic${eB} files are part of the
licensing documentation and must not be altered; doing so violates the license.
Each component may have its own ${bB}LICENSE${eB} file that overrides the ${bI}GROUP${eI}${bB}.lic${eB} file.
The full text of the licenses are in the ${bB}\$PACKAGEROOT/lib/package/LICENSES${eB}
and ${bB}\$INSTALLROOT/lib/package/LICENSES${eB} directories.
${bP}
A few files are generated in ${bB}\$PACKAGEROOT/lib/package/gen${eB} and
${bB}\$INSTALLROOT/lib/package/gen${eB}. ${bI}PACKAGE${eI}${bB}.ver${eB} contains one line consisting of${bX}
	${bI}PACKAGE VERSION RELEASE${eI} 1${eX}
for the most recent instance of ${bI}PACKAGE${eI} read into ${bB}\$PACKAGEROOT${eB}, where
${bI}PACKAGE${eI} is the package name, ${bI}VERSION${eI} is the ${bI}YYYY-MM-DD${eI} base version,
and ${bI}RELEASE${eI} is 0000 for the base release, and 0${bI}NNN${eI} for the ${bI}NNN${eI}th delta.
${bI}PACKAGE${eI}${bB}.req${eB} contains *${bB}.ver${eB} entries for the packages required by
${bI}PACKAGE${eI}, except that the fourth field is 0 instead of 1. All packages
except ${bB}INIT${eB} and ${Mratz} require the ${bB}INIT${eB} package. A simple sort of ${bI}PACKAGE${eI}${bB}.pkg${eB}
and *${bB}.ver${eB} determines if the required package have been read in. Finally,
${bI}PACKAGE${eI}${bB}.txt${eB} contains the ${bB}README${eB} text for ${bI}PACKAGE${eI} and all its
components. Included are all changes added to the component ${bB}RELEASE${eB},
${bB}CHANGES${eB} or ${bB}ChangeLog${eB} files dated since the two most recent base
releases. Component ${bB}RELEASE${eB} files contain tag lines of the form
[${bI}CC${eI}]${bI}YY-MM-DD${eI} [ ${bI}TEXT${eI} ] (or ${Mdate} format dates) followed by README
text, in reverse chronological order (newer entries at the top of the
file.) ${bF}package release${eF} generates this information, and
${bF}package contents ...${eF} lists the descriptions and components.
${bP}
${bB}\$HOSTYPE${eB} names the current binary architecture and is determined by the
output of ${bF}package${eF} (no arguments.) The ${bB}\$HOSTTYPE${eB} naming scheme is used
to separate incompatible executable and object formats. All architecture
specific binaries are placed under ${bB}\$INSTALLROOT${eB} (${bB}\$PACKAGEROOT/arch/\$HOSTTYPE${eB}.)
There are a few places that match against ${bB}\$HOSTTYPE${eB} when making binaries; these
are limited to makefile compiler workarounds, e.g., if ${bB}\$HOSTTYPE${eB} matches
'hp.*' then turn off the optimizer for these objects. All other architecture
dependent logic is handled either by ${bB}\$INSTALLROOT/bin/iffe${eB} or by component
specific configure scripts.
${bP}
Each component contains an ${bB}ast${eB} ${Mnmake} makefile (either ${bB}Nmakefile${eB} or ${bB}Makefile${eB})
and a ${bI}MAM${eI} (make abstract machine) file (${bB}Mamfile${eB}.) A Mamfile contains a portable
makefile description that is used by ${bB}\$INSTALLROOT/bin/mamake${eB} to simulate
${bB}nmake${eB}. Currently there is no support for old-make/gnu-make makefiles; if
the binaries are just being built then ${bB}mamake${eB} will suffice; if source or
makefile modifications are anticipated then ${bB}nmake${eB} (from the ${bB}ast-open${eB} or
${bB}ast-base${eB} package) should be used. Mamfiles are automatically generated by
${bF}package write${eF}.
${bP}
Most component C source is prototyped. If ${bB}\$CC${eB} (default value ${bB}cc${eB}) is not a
prototyping C compiler then ${bF}package make${eF} runs ${Mproto} on portions of the
${bB}\$PACKAGEROOT/src${eB} tree and places the converted output files in the
${bB}\$PACKAGEROOT/proto/src${eB} tree. Converted files are then viewpathed over the
original source. The ${bB}ast${eB} ${Mproto} command converts an ANSI C subset to code
that is compatible with K&R, ANSI, and C++ dialects.
${bP}
All scripts and commands under ${bB}\$PACKAGEROOT${eB} use ${bB}\$PATH${eB} relative pathnames;
there are no imbedded absolute pathnames. This means that binaries generated
under ${bB}\$PACKAGEROOT${eB} may be copied to a different root; users need only change
their ${bB}\$PATH${eB} variable to reference the new instalation root bin directory.
${bF}package install${eF} installs binary packages in a new ${bB}\$INSTALLROOT${eB}.
${eO}"
			;;
		source)	echo "${bO}
${bH}Source Package Installation Instructions${eH}
${bL}
${bT}(1)${bD}Do not install packages as ${bI}root/super-user${eI}. Although some components may
      have setuid executables, few must be owned by ${bI}root${eI}. These are best
      changed manually when the security implications are understood.${eD}
${bT}(2)${bD}Choose a package root directory and cd to it. This will be a local work
      area for all packages. See ${bB}(9)${eB} for installing packages for public use.${eD}
${bT}(3)${bD}Create the subdirectory ${bB}lib/package/tgz${eB} and download all package archives
      into that directory. The ${Mpackage} command maintains the contents of this
      directory and deletes old archives as new ones are read in. Package
      delta archives require the most recent base, so manually removing files
      in this directory may invalidate future deltas.${eD}
${bT}(4)${bD}If the ${bB}bin/package${eB} command does not exist then manually read the ${bB}INIT${eB} source package:${bX}
		gunzip < lib/package/tgz/INIT.${bI}YYYY-MM-DD${eI}.0000.tgz | tar xvf -${eX}
      If your system does not have ${Mtar} or ${Mgunzip} then download the ${Mratz}
      source package, compile it, and manually read the ${bB}INIT${eB} source package:${bX}
		mkdir bin
		cp lib/package/tgz/ratz.${bI}YYYY-MM-DD${eI}.0000.c lib/package/tgz/ratz.c
		cc -o bin/ratz lib/package/tgz/ratz.c
		bin/ratz -lv < lib/package/tgz/INIT.${bI}YYYY-MM-DD${eI}.0000.tgz
${bT}(5)${bD}Read all unread package archive(s):${bX}
		bin/package read${eX}
      Both source and binary packages will be read by this step.${eD}
${bT}(6)${bD}Build and install; all generated files are placed under ${bB}arch/\$HOSTTYPE${eB}
      (${bB}\$INSTALLROOT${eB}), where ${bB}\$HOSTTYPE${eB} is defined by the output of
      ${bB}bin/package${eB} (with no arguments.) ${bI}name=value${eI} arguments are supported; ${bB}CC${eB}
      and ${bB}CCFLAGS${eB} are likely candidates. The output is written to the terminal
      and captured in ${bB}\$INSTALLROOT/lib/package/gen/make.out${eB}:${bX}
		bin/package make${eX}${eD}
${bT}(7)${bD}List make results and interesting errors from ${bB}(6)${eB}:${bX}
		bin/package results${eX}
      Run the regression tests:${bX}
		bin/package test${eX}
      List test results and errors:${bX}
		bin/package results test${eX}${eD}
${bT}(8)${bD}When ${bB}(6)${eB} or ${bB}(7)${eB} complete run:${bX}
		bin/package use${eX}
      to get an interactive ${bB}\$SHELL${eB} that sets up the environment for using
      package binaries:${bL2}
	${bT}\$HOSTTYPE	${bD}the current host type${eD}
	${bT}\$PACKAGEROOT	${bD}the root directory for all packages (\$P)${eD}
	${bT}\$INSTALLROOT	${bD}the installation root for the current host type (\$A)${eD}
	${bT}\$PATH		${bD}\$INSTALLROOT/bin:\$PACKAGEROOT/bin prepended${eD}
	${bT}\$VPATH	${bD}a multi-level view; all building done in \$INSTALLROOT${eD}
        ${bT}${bI}DLL-MAGIC${eI}	${bD}environment magic for locating package shared libraries${eD}${eL}
      From this shell you can run ${bB}nmake${eB} in any directory under
      ${bB}\$INSTALLROOT/src${eB} and rebuild just that part. If ${bB}\$INSTALLROOT/bin/nmake${eB}
      does not exist then use ${bB}\$INSTALLROOT/bin/mamake${eB} on Mamfile.
      ${bB}nmake${eB} is open source - download ${bB}ast-open${eB} or ${bB}ast-base${eB} to
      simplify future builds or to make new or delta packages.${eD}
${bT}(9)${bD}Once you are satisfied with a package its binaries can be installed in
      a public area by:${bX}
		bin/package install ${bI}DIRECTORY PACKAGE${eI}${eX}
      and users can access the binaries by${bX}
		bin/package use ${bI}DIRECTORY${eI}${eX}
      or by exporting the environment variable definitions listed by${bX}
		bin/package environment use ${bI}DIRECTORY${eI}${eX}${eD}
${eL}${eO}"
			;;
		*)	echo "Usage: $command [ qualifier ... ] [ action ] [ arg ... ] [ n=v ... ]

   The $command command controls source and binary packages. It must be run
   within the package root directory tree. See \"$command help intro\" for
   details. In the following, PACKAGE names either a package or a component
   within a package; if omitted, all packages are operated on. The default
   action is \"host type\".

   qualifier:
	debug|environment Show environment and actions; do not execute.
	force	Force the action to override saved state.
	never	Run make -N; otherwise show other actions.
	only	Only operate on the specified packages.
	quiet	Do not list captured make and test action output.
	show	Run make -n; otherwise show other actions.
	DEBUG	Trace the package script actions in detail for debugging.
   action:
	admin [ all ] [ db FILE ] [ on PATTERN ] [ action ... ]
		Apply ACTION ... to the hosts listed in FILE. If FILE is
		omitted then "admin.db" is assumed. The caller must have rcp(1)
		and rsh(1) access to the hosts. Output for the action is save
		per-host in ACTION.log/HOST. Logs can be viewed by
		\"package admin [on HOST] results [ACTION]\". By default
		only local PACKAGEROOT hosts are selected from FILE;
		\"all\" selects all hosts. \"on PATTERN\" selects only
		hosts matching the | separated PATTERN. FILE contains four
		types of lines. Blank lines and lines beginning with # are
		ignored. Lines starting with id=value are variable assignments.
		If a package list is not specified on the command line the
		action applies to all packages; a variable assigment
		package=list applies action to the packages in list for
		subsequent hosts in FILE. The remaining line type is a host
		description consisting of 6 tab separated fields. The first 3
		are mandatory; the remaining 3 are updated by the admin action:
		   hosttype
			   The host type as reported by package.
		   [user@]host
			   The host name and optionally user name for rcp(1)
			   and rsh(1) access.
		   [server:]PACKAGEROOT
			   The absolute remote package root directory and
			   optionally the server name if the directory is on
			   a different server than the master package root
			   directory. If this directory contains an $admin_env
			   sh(1) script then it is sourced before ACTION
			   is done.
		   date    YYMMDD of the last action.
		   date    Elapsed wall time of the last action.
		   M T W   The admin action make, test and write action error
			   counts.
	contents [ package ... ]
		List description and components for PACKAGE on the standard
		output.
	copyright [ package ... ]
		List the general copyright notice(s) for PACKAGE on the
		standard output. Note that individual components in PACKAGE
		may contain additional or replacement notices.
	help [ ACTION ]
		Display help text [ for ACTION ] on the standard output.
	host [ cpu name rating type ... ]
		List architecture/implementation dependent host information
		on the standard output. type is listed if no attributes are
		specified. Information is listed on a single line in attributes
		order. The attributes are:
		   cpu     The number of cpus; 1 if the host is not a
			   multiprocessor.
		   name    The host name.
		   rating  The cpu rating in pseudo mips; the value is useful
			   useful only in comparisons with rating values of
			   other hosts. Other than a vax rating fixed at 1,
			   ratings can vary wildly but consistently from
			   vendor mips ratings. cc(1) may be required to
			   determine the rating.
	           type    The host type, usually of the form
			   vendor.architecture, with an optional trailing
			   -version. The main theme is that type names within
			   a family of architectures are named in a similar,
			   predictable style. Os point release information is
			   avoided as much as possible, but vendor resistance
			   to release incompatibilities has for the most part
			   been futile.
	html [ ACTION ]
		Display html help text [ for ACTION ] on the standard output.
	install [ flat ] [ ARCHITECTURE ... ] DIR [ PACKAGE ... ]
		Copy the package binary hierarchy to DIR. If ARCHITECTURE is
		omitted then all architectures are installed. If the \"flat\"
		attribute is specified then exactly one ARCHITECTURE must be
		specified; this architecture will be installed in DIR without
		the \"arch/HOSTTYPE\" directory prefixes. Otherwise each
		architecture will be installed in a separate \"arch/HOSTTYPE\"
		subdirectory of DIR. DIR must be an existing directory.
	license [ package ... ]
		List the source license(s) for PACKAGE on the standard output.
		Note that individual components in PACKAGE may contain
		additional or replacement licenses.
	list [ PACKAGE ... ]
		List the name, version and prerequisites for PACKAGE on the
		standard output.
	make [ PACKAGE ] [ TARGET ... ]
		Build and install. The default TARGET is install, which
		makes and installs all packages. If the standard output
		is a terminal then the output is also captured in
		\$INSTALLROOT/lib/package/gen/make.out.
	read [ package ... | archive ... ]
		Read the named package archive(s). Must be run from the
		package root directory. Archives are searched for in .
		and lib/package/tgz. Each package is read only once. The
		file lib/package/tgz/package[.type].tim tracks the read time.
		See the write action for archive naming conventions.
	release [ [CC]YY-MM-DD [ [cc]yy-mm-dd ] ] [ package ]
		Display recent changes since [CC]YY-MM-DD (up to [cc]yy-mm-dd), 		where - means lowest (or highest.) If no dates are specified
		then changes for the last 4 months are listed. PACKAGE may
		be a package or component name.
	remove PACKAGE
		Remove files installed for PACKAGE.
	results [ path ] [ old ] [ make | test ]
		List results and interesting errors (if any) captured by the
		most recent make (default) or test action. old specifies the
		previous results, if any (current and previous results are
		retained.) $HOME/.pkgresults, if it exists, must contain an
		egrep(1) expression of result lines to be ignored. path lists
		the results file path name on the standard output.
	test [ PACKAGE ]
		Run the regression tests for PACKAGE. If the standard output
		is a terminal then the output is also captured in
		\$INSTALLROOT/lib/package/gen/test.out. In general a package
		must be made before it can be tested.
   	use [ uid | PACKAGE | - ] [ COMMAND ... ]
   		Run COMMAND or an interactive shell if COMMAND is omitted, with
		the environment initialized for using the package (can you say
		shared library without cussing?) If either uid or PACKAGE is
		specified then it is used to determine a \$PACKAGEROOT,
		possibly different from the current directory. For example, to
		try out bozo's package: \"package use bozo\". In this case the
		command may be run from any directory.
	verify [ PACKAGE ]
		Verify installed binary files against the checksum files in
		\$INSTALLROOT/lib/package/gen/*.sum. The checksum files contain
		mode, user and group information. If the checksum matches
		for a given file then the mode, user and group are changed
		as necessary to match the checksum entry. A warning is printed
		on the standard error for each mismatch. Requires the ast
		package cksum(1) command.
	write [closure] [exp|lcl|pkg|rpm] [base|delta] [binary|source] PACKAGE
		Write an archive for PACKAGE. The archive name is
		lib/package/SAVE/PACKAGE.RELEASE[.HOSTTYPE].VERSION.SUFFIX:
		   SAVE      [exp|lcl|pkg|rpm] or tgz by default
		   PACKAGE   package name
		   RELEASE   YYYY-MM-DD when the base archive was generated
		   HOSTTYPE  the current host type via "package host type"
			     for binary archives, omitted for source archives
		   VERSION   .0000 for base archive, YYYY-MM-DD for last delta,
			     .NNNN for delta, where NNNN is one more than the
			     most recent delta, 0001 for the first delta
		   SUFFIX    \".tgz\" for all packages except \"ratz\", which
		             either has the \".c\" source package suffix or the
			     \".exe\" binary package suffix.
		closure writes the closure of all package components, useful
		with lcl for backup and versioning. lcl omits the proto(1)
		source licensing; otherwise source files not already annotated
		are written to the archive with a license comment prepended.
		Requires the ast package nmake(1) and pax(1) commands. If a
		new base is written then two delta archives are also generated:
		one to generate the new base from the old, and one to generate
		the old base from the new; the old base is then removed.
		PACKAGE is omitted then an ordered list previously written
		packages is generated. If only is specified then only named
		packages will be written; otherwise prerequisite packages are
		written first. Package components must be listed in PACKAGE.pkg.
		Other package styles are supported, but only the lcl style
		supports deltas:
		   exp  create an exptools(1) maintainer source archive
		        and NPD file in the exp subdirectory, suitable for
			expmake(1); support files are placed in the
			exp/PACKAGE subdirectory
		   lcl	generate a package archive or delta in the lcl
			subdirectory, suitable for restoration into the
			primary source tree (no source licence annotation)
		   pkg	create a pkgmk(1) package, suitable for pkgadd(1)
		   rpm  create an rpm(1) package
   name=value:
	variable definition: typically CC=cc or CCFLAGS=-g."
			;;
		esac
		exit $code
		;;
	*=*)	set DEFAULT host type "$@"
		;;
	*)	echo "Usage: $command [ options ] [ qualifier ... ] [ action ] [ arg ... ] [ n=v ... ]" >&2
		exit 2
		;;
	esac
	shift
done

# collect command line targets and definitions

case $_PACKAGE_HOSTTYPE_ in
?*)	HOSTTYPE=$_PACKAGE_HOSTTYPE_
	KEEP_HOSTTYPE=1
	;;
*)	KEEP_HOSTTYPE=0
	;;
esac
KEEP_PACKAGEROOT=0
KEEP_SHELL=0
USER_VPATH=
args=
assign=
for i
do	case $i in
	CC=*|CCFLAGS=*)
		eval $i
		assign="$assign '$i'"
		;;
	HOSTTYPE=*)
		eval $i
		case $HOSTTYPE in
		?*)	KEEP_HOSTTYPE=1 ;;
		esac
		;;
	PACKAGEROOT=*)
		eval $i
		case $PACKAGEROOT in
		?*)	KEEP_PACKAGEROOT=1 ;;
		esac
		;;
	SHELL=*)eval $i
		case $SHELL in
		?*)	KEEP_SHELL=1 ;;
		esac
		;;
	VPATH=*)eval USER_$i
		;;
	*=*)	assign="$assign '$i'"
		;;
	*)	args="$args $i"
		;;
	esac
done

# grab action specific args

case $action in
admin)	while	:
	do	case $# in
		0)	set list
			break
			;;
		esac
		case $1 in
		all)	admin_all=1
			;;
		db)	case $# in
			1)	echo $command: $action: $1: db file argument expected >&2
				exit 1
				;;
			esac
			shift
			admin_db=$1
			;;
		on)	case $# in
			1)	echo $command: $action: $1: host pattern argument expected >&2
				exit 1
				;;
			esac
			shift
			admin_on=$1
			;;
		*)	break
			;;
		esac
		shift
	done
	admin_action=$1
	admin_args=$*
	;;
esac

# true if arg is a valid PACKAGEROOT

packageroot() # dir
{
	test -d $1/lib/$command
}

# true if no nmake or nmake too old

nonmake() # nmake
{
	_nonmake_version=`$1 -n -f - 'print $(MAKEVERSION:@/.* //:/-//G)' . 2>/dev/null || echo 19840919`
	if	test $_nonmake_version -ge 20001031
	then	case " $assign " in
		*" physical=1 "*)	;;
		*)			assign="physical=1 $assign" ;;
		esac
	fi
	test 20000101 -gt $_nonmake_version
}

# determine local host attributes

hostinfo() # attribute ...
{
	case $DEBUG in
	1)	set -x ;;
	esac
	map=
	something=
	path=$PATH
	for i in $ccs
	do	PATH=$PATH:$i
	done
	for i in $use
	do	for j in $org
		do	PATH=$PATH:$i/$j/bin
		done
		PATH=$PATH:$i/bin
	done
	case $PACKAGE_PATH in
	?*)	for i in `echo $PACKAGE_PATH | sed 's,:, ,g'`
		do	PATH=$PATH:$i/bin
		done
		;;
	esac

	# validate the args

	for info
	do	case $info in
		*/*|*[cC][cC])
			cc=$info
			;;
		cpu|name|rating|type)
			something=1
			;;
		*)	echo "$command: $action: $info: unknown attribute" >&2
			exit 1
			;;
		esac
	done
	case $something in
	"")	set "$@" type ;;
	esac
	case $DEBUG in
	'')	exec 9>&2
		exec 2>/dev/null
		;;
	esac

	# compute the info

	_hostinfo_=
	for info
	do	
	case $info in
	cpu)	cpu=`grep -ic '^processor[ 	][ 	]*:[ 	]*[0-9]' /proc/cpuinfo`
		case $cpu in
		[1-9]*)	_hostinfo_="$_hostinfo_ $cpu"
			continue
			;;
		esac
		cpu=1
		# exact match
		set							\
			hinv			'^Processor [0-9]'	\
			psrinfo			'on-line'		\

		while	:
		do	case $# in
			0)	break ;;
			esac
			i=`$1 2>/dev/null | grep -c "$2"`
			case $i in
			[1-9]*)	cpu=$i
				break
				;;
			esac
			shift 2
		done
		case $cpu in
		0|1)	set						\
			/bin/mpstat

			while	:
			do	case $# in
				0)	break ;;
				esac
				if	test -x $1
				then	case `$1 | grep -ic '^cpu '` in
					1)	cpu=`$1 | grep -ic '^ *[0-9][0-9]* '`
						break
						;;
					esac
				fi
				shift
			done
			;;
		esac
		case $cpu in
		0|1)	# token match
			set						\
			/usr/kvm/mpstat			'cpu[0-9]'	\
			/usr/etc/cpustatus		'enable'	\
			/usr/alliant/showsched		'CE'		\
			'ls /config/hw/system/cpu'	'cpu'		\
			prtconf				'cpu-unit'	\

			while	:
			do	case $# in
				0)	break ;;
				esac
				i=`$1 2>/dev/null | tr ' 	' '

' | grep -c "^$2"`
				case $i in
				[1-9]*)	cpu=$i
					break
					;;
				esac
				shift 2
			done
			;;
		esac
		case $cpu in
		0|1)	# special match
			set						\
									\
			hinv						\
			'/^[0-9][0-9]* .* Processors*$/'		\
			'/[ 	].*//'					\
									\
			/usr/bin/hostinfo				\
			'/^[0-9][0-9]* .* physically available\.*$/'	\
			'/[ 	].*//'					\

			while	:
			do	case $# in
				0)	break ;;
				esac
				i=`$1 2>/dev/null | sed -e "${2}!d" -e "s${3}"`
				case $i in
				[1-9]*)	cpu=$i
					break
					;;
				esac
				shift 3
			done
			;;
		esac
		case $cpu in
		0)	cpu=1 ;;
		esac
		_hostinfo_="$_hostinfo_ $cpu"
		;;
	name)	name=`hostname || uname -n || cat /etc/whoami || echo local`
		_hostinfo_="$_hostinfo_ $name"
		;;
	rating)	for rating in `grep -i ^bogomips /proc/cpuinfo 2>/dev/null | sed -e 's,.*:[ 	]*,,' -e 's,\(...*\)\..*,\1,' -e 's,\(\..\).*,\1,'`
		do	case $rating in
			[0-9]*)	break ;;
			esac
		done
		case $rating in
		[0-9]*)	;;
		*)	cd ${TMPDIR:-/tmp}
			tmp=hi$$
			trap 'rm -f $tmp.*' 0 1 2
			cat > $tmp.c <<!
#include <stdio.h>
#include <sys/types.h>
#if TD || TZ
#include <sys/time.h>
#else
extern time_t	time();
#endif
main()
{
	register unsigned long	i;
	register unsigned long	j;
	register unsigned long	k;
	unsigned long		l;
	unsigned long		m;
	unsigned long		t;
	int			x;
#if TD || TZ
	struct timeval		b;
	struct timeval		e;
#if TZ
	struct timezone		z;
#endif
#endif
	l = 500;
	m = 890;
	x = 0;
	for (;;)
	{
#if TD || TZ
#if TZ
		gettimeofday(&b, &z);
#else
		gettimeofday(&b);
#endif
#else
		t = (unsigned long)time((time_t*)0);
#endif
		k = 0;
		for (i = 0; i < l; i++)
			for (j = 0; j < 50000; j++)
				k += j;
#if TD || TZ
#if TZ
		gettimeofday(&e, &z);
#else
		gettimeofday(&e);
#endif
		t = (e.tv_sec - b.tv_sec) * 1000 + (e.tv_usec - b.tv_usec) / 1000;
		if (!x++ && t < 1000)
		{
			t = 10000 / t;
			l = (l * t) / 10;
			continue;
		}
#else
		t = ((unsigned long)time((time_t*)0) - t) * 1000;
		if (!x++ && t < 20000)
		{
			t = 200000l / t;
			l = (l * t) / 10;
			continue;
		}
#endif
#if PR
		printf("[ k=%lu l=%lu m=%lu t=%lu ] ", k, l, m, t);
#endif
		if (t == 0)
			t = 1;
		break;
	}
	printf("%lu\n", ((l * m) / 10) / t);
	return k == 0;
}
!
			rating=
			for o in -DTZ -DTD ''
			do	if	$CC $o -O -o $tmp.exe $tmp.c >/dev/null 2>&1 ||
					gcc $o -O -o $tmp.exe $tmp.c >/dev/null 2>&1
				then	rating=`./$tmp.exe`
					break
				fi
			done
			case $rating in
			[0-9]*)	;;
			*)	rating=1 ;;
			esac
			;;
		esac
		_hostinfo_="$_hostinfo_ $rating"
		;;
	type)	case $KEEP_HOSTTYPE in
		1)	_hostinfo_="$_hostinfo_ $HOSTTYPE"
			continue
			;;
		esac
		IFS=:
		set /:$PATH
		IFS=$ifs
		shift
		f=../lib/hostinfo/typemap
		for i
		do	case $i in
			"")	i=. ;;
			esac
			if	test -f $i/$f
			then	map="`grep -v '^#' $i/$f` $map"
			fi
		done
		h=`hostname || uname -n || cat /etc/whoami || echo local`
		a=`arch || uname -m || att uname -m || uname -s || att uname -s || echo unknown`
		case $a in 
		*[\ \	]*)	a=`echo $a | sed "s/[ 	]/-/g"` ;;
		esac
		m=`mach || machine || uname -p || att uname -p || echo unknown`
		case $m in 
		*[\ \	]*)	m=`echo $m | sed "s/[ 	]/-/g"` ;;
		esac
		x=`uname -a || att uname -a || echo unknown $host unknown unknown unknown unknown unknown`
		set "" $h $a $m $x
		expected=$1 host=$2 arch=$3 mach=$4 os=$5 sys=$6 rel=$7 ver=$8
		type=unknown
		case $host in
		*.*)	host=`echo $host | sed -e 's/\..*//'` ;;
		esac
		case $mach in
		unknown)
			mach=
			;;
		[Rr][0-3][0-9][0-9][0-9])
			mach=mips1
			;;
		[Rr][4][0-9][0-9][0-9])
			mach=mips2
			;;
		[Rr][5-9][0-9][0-9][0-9]|[Rr][1-9][0-9][0-9][0-9][0-9])
			mach=mips4
			;;
		*)	case $arch in
			34[0-9][0-9])
				os=ncr
				arch=i386
				;;
			esac
			;;
		esac
		set						\
								\
		/NextDeveloper		-d	next	-	\
		/config/hw/system/cpu	-d	tandem	mach	\

		while	:
		do	case $# in
			0)	break ;;
			esac
			if	test $2 $1
			then	os=$3
				case $4 in
				arch)	mach=$arch ;;
				mach)	arch=$mach ;;
				esac
				break
			fi
			shift 4
		done
		case $os in
		AIX*|aix*)
			type=ibm.risc
			;;
		HP-UX)	case $arch in
			9000/[78]*)
				type=hp.pa
				;;
			*)	type=hp.ux
				;;
			esac
			;;
		[Ii][Rr][Ii][Xx]*)
			set xx `hinv | sed -e '/^CPU:/!d' -e 's/CPU:[ 	]*\([^ 	]*\)[ 	]*\([^ 	]*\).*/\1 \2/' -e q | tr '[A-Z]' '[a-z]'`
			shift
			type=$1
			n=
			case $2 in
			r[0-3][0-9][0-9][0-9])
				n=1
				;;
			r[4][0-3][0-9][0-9])
				n=2
				;;
			r[4][4-9][0-9][0-9]|r[5][0-9][0-9][0-9])
				n=3
				;;
			r[6-9][0-9][0-9][0-9]|r[1-9][0-9][0-9][0-9][0-9])
				n=4
				;;
			esac
			case $rel in
			[0-4].*|5.[0-2]|5.[0-2].*)
				case $n in
				1)	;;
				*)	n=2 ;;
				esac
				;;
			5.*)	case $n in
				2)	n=3 ;;
				esac
				;;
			esac
			if	test -x $CC
			then	a=$CC
			else	IFS=:
				set /$IFS$PATH
				IFS=$ifs
				shift
				for i
				do	a=$i/$CC
					if	test -x $a
					then	break
					fi
				done
			fi
			split='
'
			a=`strings $a < /dev/null | sed -e 's/[^a-z0-9]/ /g' -e 's/[ 	][ 	]*/\'"$split"'/g' | sed -e "/^${type}[0-9]$/!d" -e "s/^${type}//" -e q`
			case $a in
			[0-9])	n=$a ;;
			esac
			case $n in
			4)	a=`$CC -${type}3 2>&1`
				case $a in
				*unknown*|*install*|*conflict*)
					;;
				*)	n=3
					;;
				esac
				;;
			esac
			case $n in
			2)	x='-n32' ;;
			3)	x='-o32' ;;
			4)	x='-o32 -n32' ;;
			esac
			for i in $x
			do	a=`$CC -${type}$n $i 2>&1`
				case $a in
				*conflict*|*mix*|*used*)
					;;
				*)	abi=$i
					break
					;;
				esac
			done
			mach=${type}$n
			type=sgi.$mach
			;;
		OSx*|SMP*|pyramid)
			type=pyr
			;;
		[Ss][Cc][Oo]*)
			type=sco
			;;
		[Ss]ol*)
			v=`echo $rel | sed -e 's/^[25]\.//' -e 's/\.[^.]*$//'`
			case $v in
			[6-9])	;;
			*)	v= ;;
			esac
			type=sol$v.sun4
			;;
		[Ss]un*)type=`echo $arch | sed -e 's/\(sun.\).*/\1/'`
			case $rel in
			[0-4]*)	;;
			*)	case $type in
				'')	case $mach in
					sparc*)	type=sun4 ;;
					*)	type=$mach ;;
					esac
					;;
				esac
				v=`echo $rel | sed -e 's/^[25]\.//' -e 's/\.[^.]*$//'`
				case $v in
				[6-9])	;;
				*)	v= ;;
				esac
				type=sol$v.$type
				;;
			esac
			;;
		[Uu][Nn][Ii][Xx]_[Ss][Vv])
			type=unixware
			;;
		$host)	type=$arch
			case $type in
			*.*|*[0-9]*86|*68*)
				;;
			*)	case $mach in
				*[0-9]*86|*68*|mips)
					type=$type.$mach
					;;
				esac
				;;
			esac
			;;
		unknown)
			case $arch in
			?*)	case $arch in
				sun*)	mach= ;;
				esac
				type=$arch
				case $mach in
				?*)	type=$type.$mach ;;
				esac
				;;
			esac
			;;
		*)	case $ver in
			FTX*|ftx*)
				case $mach in
				*[0-9][a-zA-Z]*)
					mach=`echo $mach | sed -e 's/[a-zA-Z]*$//'`
					;;
				esac
				type=stratus.$mach
				;;
			*)	type=`echo $os | sed -e 's/[0-9].*//' -e 's/[^A-Za-z_0-9.].*//'`
				case $type in
				[Uu][Ww][Ii][Nn]*|[Ww]indows_[0-9][0-9]|[Ww]indows_[Nn][Tt])
					type=win32
					arch=`echo $arch | sed -e 's/_[^_]*$//'`
					;;
				esac
				case $arch in
				'')	case $mach in
					?*)	type=$type.$mach ;;
					esac
					;;
				*)	type=$type.$arch ;;
				esac
				;;
			esac
		esac
		case $type in
		[0-9]*)	case $mach in
			?*)	type=$mach ;;
			esac
			case $type in
			*/MC)	type=ncr.$type ;;
			esac
			;;
		*.*)	;;
		*[0-9]*86|*68*)
			case $rel in
			[34].[0-9]*)
				type=att.$type
				;;
			esac
			;;
		[a-z]*[0-9])
			;;
		[a-z]*)	case $mach in
			$type)	case $ver in
				Fault*|fault*|FAULT*)
					type=ft.$type
					;;
				esac
				;;
			?*)	case $arch in
				'')	type=$type.$mach ;;
				*)	type=$type.$arch ;;
				esac
				;;
			esac
			;;
		esac
		type=`echo $type | sed -e 's%[-+/].*%%'`
		case $type in
		*.*)	lhs=`echo $type | sed -e 's/\..*//'`
			rhs=`echo $type | sed -e 's/.*\.//'`
			case $rhs in
			[x0-9]*86)		rhs=i$rhs ;;
			68*)			rhs=m$rhs ;;
			esac
			case $rhs in
			i[x2-9]86|*86pc)	rhs=i386 ;;
			esac
			case $lhs in
			[Mm][Vv][Ss])		rhs=390 ;;
			esac
			case $lhs in
			$rhs)			type=$lhs ;;
			*)			type=$lhs.$rhs ;;
			esac
			;;
		esac
		type=`echo $type | tr '[A-Z]' '[a-z]'`
		case $type in
		linux.*)x=`echo /lib/libdl.*`
			case $x in
			*libdl.so*)
				;;
			*)	type=`echo $type | sed 's/linux/&-aout/'`
				;;
			esac
			;;
		sgi.mips*)
			case $mach in
			mips2)	type=sgi.$mach
				abi=-o32
				;;
			mips3)	type=sgi.$mach
				abi=-n32
				;;
			mips[4-9])
				type=sgi.$mach
				case $abi in
				*-n32) ;;
				*) abi=-64 ;;
				esac
				;;
			*)	cd ${TMPDIR:-/tmp}
				tmp=hi$$
				trap 'rm -f $tmp.*' 0 1 2
				cat > $tmp.a.c <<!
extern int b();
main() { return b(); }
!
				cat > $tmp.b.c <<!
int b() { return 0; }
!
				abi=
				if	$CC -c $tmp.a.c
				then	for i in -n32 -o32 -64
					do	if	$CC $i -c $tmp.b.c &&
							$CC -o $tmp.exe $tmp.a.o $tmp.b.o
						then	abi=$i
							for i in 2 3 4 5 6 7 8 9
							do	case $i:$abi in
								2:-n32|2:-64|3:-64)
									continue
									;;
								esac
								if	$CC $abi -mips$i -c $tmp.b.c &&
									$CC -o $tmp.exe $tmp.a.o $tmp.b.o
								then	type=`echo $type | sed -e 's/.$//'`$i
									break
								fi
							done
							break
						fi
					done
				fi </dev/null >/dev/null 2>&1
				;;
			esac
			case $type$abi in
			sgi.mips2-o32)
				;;
			sgi.mips3)
				type=$type-o32
				;;
			sgi.mips3-n32)
				;;
			sgi.mips4)
				type=$type-o32
				;;
			sgi.mips[4-9]-64)
				;;
			*)	type=$type$abi
				;;
			esac
			;;
		esac

		# last chance mapping

		set "" "" $map
		while	:
		do	case $# in
			[012])	break ;;
			esac
			shift 2
			eval "	case \$type in
				$1)	type=\$2; break ;;
				esac"
		done
		_hostinfo_="$_hostinfo_ $type"
		;;
	esac
	done
	set '' $_hostinfo_
	shift
	_hostinfo_=$*

	# restore the global state

	PATH=$path
	case $DEBUG in
	'')	exec 2>&9
		exec 9>&-
		;;
	esac
}

# info message

note() # message ...
{
	echo $command: "$@" >&2
}

# true if arg is executable command on $PATH

executable() # command
{
	b=$1
	case $b in
	/*)	if	test -x $b
		then	_executable_=$b
			return 0
		fi
		;;
	esac
	IFS=':'
	set '' $PATH
	IFS=$ifs
	shift
	for d
	do	case $d in
		'')	d=. ;;
		esac
		if	test -x "$d/$b"
		then	_executable_=$d/$b
			return 0
		fi
	done
	return 1
}

# some actions have their own PACKAGEROOT or kick out early

case $KEEP_PACKAGEROOT in
0)	PACKAGEROOT=
	case $action in
	host)	hostinfo $args
		echo $_hostinfo_
		exit 0
		;;
	use)	case $show in
		echo)	exec=echo make=echo show=echo ;;
		esac
		set '' $args
		shift
		case $# in
		0)	;;
		*)	case $1 in
			-)	;;
			/*)	PACKAGEROOT=$1
				;;
			*)	for i in `echo $HOME | sed -e 's,/[^/]*,,'` $use
				do	if	packageroot $i/$1
					then	PACKAGEROOT=$i/$1
						break
					fi
				done
				case $PACKAGEROOT in
				'')	echo "$command: $1: package root not found" >&2
					exit 1
					;;
				esac
				;;
			esac
			shift
			;;
		esac
		run="$@"
		;;
	esac
	case $PACKAGEROOT in
	'')	PACKAGEROOT=`pwd` ;;
	esac
	;;
esac

# . must be within the PACKAGEROOT tree

i=X$PACKAGEROOT
IFS=/
set $i
IFS=$ifs
while	:
do	i=$1
	shift
	case $i in
	X)	break ;;
	esac
done
case $PACKAGEROOT in
//*)	d=/ ;;
*)	d= ;;
esac
k=0
for i
do	case $i in
	'')	continue ;;
	esac
	d=$d/$i
	case $k in
	2)	k=1
		;;
	1)	k=0
		;;
	0)	case $i in
		arch)	k=2
			;;
		*)	if	packageroot $d
			then	PACKAGEROOT=$d
			fi
			;;
		esac
		;;
	esac
done
INITROOT=$PACKAGEROOT/src/cmd/INIT
$show PACKAGEROOT=$PACKAGEROOT
$show export PACKAGEROOT
export PACKAGEROOT
case $action in
use)	packageroot $PACKAGEROOT || {
		echo "$command: $PACKAGEROOT: invalid package root directory" >&2
		exit 1
	}
	;;
*)	packageroot $PACKAGEROOT || {
		echo "$command: must be run within the package root directory tree" >&2
		exit 1
	}

	# update the basic package commands

	for i in ignore mamprobe silent
	do	test -h $PACKAGEROOT/bin/$i 2>/dev/null ||
		case `ls -t $INITROOT/$i.sh $PACKAGEROOT/bin/$i 2>/dev/null` in
		"$INITROOT/$i.sh"*)
			note update $PACKAGEROOT/bin/$i
			$exec cp $INITROOT/$i.sh $PACKAGEROOT/bin/$i || exit
			$exec chmod +x $PACKAGEROOT/bin/$i || exit
			;;
		esac
	done
	;;
esac

# initialize the package environment

case $KEEP_HOSTTYPE in
0)	hostinfo type
	HOSTTYPE=$_hostinfo_
	;;
1)	_PACKAGE_HOSTTYPE_=$HOSTTYPE
	export _PACKAGE_HOSTTYPE_
	;;
esac
$show HOSTTYPE=$HOSTTYPE
$show export HOSTTYPE
export HOSTTYPE
INSTALLROOT=$PACKAGEROOT/arch/$HOSTTYPE
case $action in
admin|install|make|read|remove|test|verify|write)
	;;
*)	if	test ! -d $INSTALLROOT
	then	INSTALLROOT=$PACKAGEROOT
	fi
	;;
esac
$show INSTALLROOT=$INSTALLROOT
$show export INSTALLROOT
export INSTALLROOT
PACKAGESRC=$PACKAGEROOT/lib/package
PACKAGEBIN=$INSTALLROOT/lib/package

# dll hackery -- why is this so complicated?

abi=
case $HOSTTYPE in
sgi.mips[0-9]*)
	if	test -d /lib32 -a -d /lib64
	then	u=`echo $INSTALLROOT | sed -e 's,-[^-/]*$,,' -e 's,.$,,'`
		for a in "n=2 v=" "n=3 v=N32" "n=4-n32 v=N32" "n=4 v=64"
		do	eval $a
			case $v in
			N32)	case $n:$HOSTTYPE in
				*-n32:*-n32)	;;
				*-n32:*)	continue ;;
				*:*-n32)	continue ;;
				esac
				;;
			esac
			if	test -d $u$n
			then	abi="$abi 'd=$u$n v=$v'"
			fi
		done
	fi
	;;
esac
case $abi in
'')	abi="'d=$INSTALLROOT v='" ;;
esac
eval "
	for a in $abi
	do	eval \$a
		eval \"
			case \\\$LD_LIBRARY\${v}_PATH: in
			\\\$d/lib:*)
				;;
			*)	x=\\\$LD_LIBRARY\${v}_PATH
				case \\\$x in
				''|:*)	;;
				*)	x=:\\\$x ;;
				esac
				LD_LIBRARY\${v}_PATH=\$d/lib\\\$x
				$show LD_LIBRARY\${v}_PATH=\\\$LD_LIBRARY\${v}_PATH
				$show export LD_LIBRARY\${v}_PATH
				export LD_LIBRARY\${v}_PATH
				;;
			esac
		\"
	done
"
case $LD_LIBRARY_PATH in
'')	;;
*)	a=0
	for d in $lib
	do	case :$LD_LIBRARY_PATH: in
		*:$d:*)	;;
		*)	if	test -d $d
			then	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$d
				a=1
			fi
			;;
		esac
	done
	case $a in
	1)	$show LD_LIBRARY_PATH=$LD_LIBRARY_PATH
		$show export LD_LIBRARY_PATH
		export LD_LIBRARY_PATH
		;;
	esac
	;;
esac
case $LIBPATH: in
$INSTALLROOT/bin:$INSTALLROOT/lib:*)
	;;
*)	case $LIBPATH in
	'')	LIBPATH=/usr/lib:/lib ;;
	esac
	LIBPATH=$INSTALLROOT/bin:$INSTALLROOT/lib:$LIBPATH
	$show LIBPATH=$LIBPATH
	$show export LIBPATH
	export LIBPATH
	;;
esac
case $SHLIB_PATH: in
$INSTALLROOT/lib:*)
	;;
*)	SHLIB_PATH=$INSTALLROOT/lib${SHLIB_PATH:+:$SHLIB_PATH}
	$show SHLIB_PATH=$SHLIB_PATH
	$show export SHLIB_PATH
	export SHLIB_PATH
	;;
esac
case $DYLD_LIBRARY_PATH: in
$INSTALLROOT/lib:*)
	;;
*)	DYLD_LIBRARY_PATH=$INSTALLROOT/lib${DYLD_LIBRARY_PATH:+:$DYLD_LIBRARY_PATH}
	$show DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH
	$show export DYLD_LIBRARY_PATH
	export DYLD_LIBRARY_PATH
	;;
esac
case $_RLD_ROOT in
$INSTALLROOT/arch*)	;;
':')	_RLD_ROOT=$INSTALLROOT/arch:/ ;;
/|*:/)	_RLD_ROOT=$INSTALLROOT/arch:$_RLD_ROOT ;;
*)	_RLD_ROOT=$INSTALLROOT/arch:$_RLD_ROOT:/ ;;
esac
$show _RLD_ROOT=$_RLD_ROOT
$show export _RLD_ROOT
export _RLD_ROOT

# now set up PATH
#
# NOTE: PACKAGEROOT==INSTALLROOT is possible for binary installations

case :$PATH: in
*:$PACKAGEROOT/bin:*)
	;;
*)	PATH=$PACKAGEROOT/bin:$PATH
	;;
esac
case :$PATH: in
*:$INSTALLROOT/bin:*)
	;;
*)	PATH=$INSTALLROOT/bin:$PATH
	;;
esac
$show PATH=$PATH
$show export PATH
export PATH

# use these if possible

KSH=$INSTALLROOT/bin/ksh
MAKE=nmake
NMAKE=$INSTALLROOT/bin/$MAKE
SUM=$INSTALLROOT/bin/sum
TEE=$INSTALLROOT/bin/tee

# grab a decent default shell

case $KEEP_SHELL in
0)	case $SHELL in
	''|/bin/*|/usr/bin/*)
		case $SHELL in
		'')	SHELL=/bin/sh ;;
		esac
		for i in ksh sh bash
		do	if	executable $i
			then	case `$_executable_ -c 'echo $KSH_VERSION'` in
				*[Pp][Dd]*)
					: pd ksh is unreliable
					;;
				*)	SHELL=$_executable_
					break
					;;
				esac
			fi
		done
		;;
	*/*ksh)	if	test -x $KSH
		then	SHELL=$KSH
		fi
		;;
	esac
	;;
esac
export SHELL
$show SHELL=$SHELL
$show export SHELL
COSHELL=$SHELL
export COSHELL
$show COSHELL=$COSHELL
$show export COSHELL

# tame the environment

case $action in
use)	;;
*)	ENV=
	ERROR_OPTIONS=
	export ENV ERROR_OPTIONS
	;;
esac

# $INSTALLROOT may be an obsolete shipment

PAX=
if	executable pax
then	case `$_executable_ -rw --?meter 2>&1` in
	*--meter*)	PAX=pax ;;
	esac
fi

# finalize the views

case $USER_VPATH in
'')	case $VPATH in
	?*)	IFS=':'
		set '' $VPATH
		shift
		IFS=$ifs
		USER_VPATH=
		for i
		do	if	packageroot $i
			then	case $USER_VPATH in
				'')	USER_VPATH=$i ;;
				?*)	USER_VPATH=$USER_VPATH:$i ;;
				esac
			fi
		done
	esac
	;;
esac
case $USER_VPATH in
?*)	IFS=':'
	set '' $USER_VPATH
	shift
	IFS=$ifs
	USER_VPATH=
	USER_VPATH_CHAIN=
	p=$PACKAGEROOT
	for i
	do	case $i in
		''|$PACKAGEROOT|$INSTALLROOT)
			;;
		?*)	USER_VPATH=$USER_VPATH:$i
			USER_VPATH_CHAIN="$USER_VPATH_CHAIN $p $i"
			p=$i
			case $PROTOROOT in
			-)	test -x $i/bin/mamake && PROTOROOT= ;;
			esac
			;;
		esac
	done
	;;
esac
VIEW_bin=$INSTALLROOT VIEW_src=$PACKAGEROOT VIEW_all="$INSTALLROOT $PACKAGEROOT"
if	(vpath) >/dev/null 2>&1 && vpath $INSTALLROOT $PACKAGEROOT $USER_VPATH_CHAIN
then	$show vpath $INSTALLROOT $PACKAGEROOT $USER_VPATH_CHAIN
else	VPATH=$INSTALLROOT:$PACKAGEROOT$USER_VPATH
	$show VPATH=$VPATH
	$show export VPATH
	export VPATH
	IFS=':'
	set '' $VPATH
	shift
	IFS=$ifs
	for i
	do	case $i in
		*/arch/*/*)
			VIEW_src="$VIEW_src $i"
			;;
		*/arch/*)
			VIEW_bin="$VIEW_bin $i"
			;;
		*)
			VIEW_src="$VIEW_src $i"
			;;
		esac
		VIEW_all="$VIEW_all $i"
	done
fi

# return 0 if arg in src|bin|all view

view() # [test] [-|type] [src|bin|all] file
{
	case $1 in
	-[dfsx])_view_T_=$1; shift ;;
	*)	_view_T_=-f ;;
	esac
	case $1 in
	-)	_view_t_= ;;
	*)	_view_t_=$1 ;;
	esac
	shift
	case $1 in
	all)	shift; _view_v_=$VIEW_all ;;
	bin)	shift; _view_v_=$VIEW_bin ;;
	src)	shift; _view_v_=$VIEW_src ;;
	*)	_view_v_=$VIEW_all ;;
	esac
	case $1 in
	/*)	if	test $_view_T_ $1
		then	_view_=$1
			return 0
		fi
		;;
	*)	for _view_d_ in $_view_v_
		do	if	test $_view_T_ $_view_d_/$1
			then	_view_=$_view_d_/$1
				return 0
			fi
		done
		;;
	esac
	_view_=
	case $_view_t_ in
	?*)	echo $command: $1: $_view_t_ not found >&2 ;;
	esac
	return 1
}

# determine the package and targets

case $action in
admin)	case $admin_action in
	results)action=$admin_action
		set '' $admin_args
		shift 2
		admin_args="admin $*"
		case $admin_on in
		'')	target=$admin_args ;;
		*)	target="on $admin_on $admin_args" ;;
		esac
		;;
	esac
	;;
*)	package=
	target=
	set '' $args
	while	:
	do	shift
		case $# in
		0)	break
			;;
		1)	if	test '' != "$1" && view - src "lib/package/$1.pkg"
			then	package=$1
				break
			fi
			;;
		esac
		target="$target $1"
	done
	;;
esac

# check that cmd args are up to date a.out's

checkaout()	# cmd ...
{
	case $PROTOROOT in
	-)	PROTOROOT=
		test -f $INITROOT/hello.c -a -f $INITROOT/p.c -a -w $PACKAGEROOT || {
			for i
			do	executable $i || {
					echo "$command: $i: command not found" >&2
					exit 1
				}
			done
			return 0
		}
		if	executable $CC
		then	_PACKAGE_cc=1
			test -f $INITROOT/hello.c -a -f $INITROOT/p.c || {
				echo "$command: $INITROOT: INIT package source not found" >&2
				exit 1
			}
			test -x $INSTALLROOT/bin/nmake || {
				# check for prototyping cc
				# NOTE: proto.c must be K&R compatible

				$CC -o hello.exe $INITROOT/hello.c && ./hello.exe >/dev/null 2>&1
				c=$?
				rm -f hello.*
				test 0 != "$c" && {
					echo "$command: $CC: not a C compiler for $HOSTTYPE" >&2
					exit 1
				}
				checkaout proto
				$CC -c $INITROOT/p.c >/dev/null 2>&1
				c=$?
				rm -f p.*
				test 0 != "$c" && {
					PROTOROOT=$PACKAGEROOT/proto
					$show PROTOROOT=$PACKAGEROOT/proto
					export PROTOROOT
					INITPROTO=$PROTOROOT/src/cmd/INIT
					note proto convert $PACKAGEROOT/src into $PROTOROOT/src
					if	test -d $PACKAGEROOT/src/cmd/nmake
					then	dirs="src/cmd/INIT src/lib/libast src/lib/libcoshell src/lib/libpp src/cmd/probe src/cmd/cpp src/cmd/nmake"
					else	dirs="src"
					fi
					(
						if	test -f $PROTOROOT/UPDATE
						then	newer="-newer $PROTOROOT/UPDATE"
						else	newer=""
						fi
						case $exec in
						'')	cd $PACKAGEROOT
							find $dirs -name '*.[CcHh]' $newer -print | proto -v -L - -C proto
							;;
						*)	$exec cd $PACKAGEROOT
							$exec "find $dirs -name '*.[CcHh]' $newer -print | proto -L - -C proto"
							;;
						esac
						$exec touch $PROTOROOT/UPDATE
					)
					if (vpath) >/dev/null 2>&1 && vpath $INSTALLROOT - && vpath $INSTALLROOT $PROTOROOT $PROTOROOT $PACKAGEROOT
					then	$show vpath $INSTALLROOT $PROTOROOT $PROTOROOT $PACKAGEROOT $USER_VPATH_CHAIN
					else	VPATH=$INSTALLROOT:$PROTOROOT:$PACKAGEROOT$USER_VPATH
						$show VPATH=$VPATH
						export VPATH
					fi
				}
			}
			for i in arch arch/$HOSTTYPE arch/$HOSTTYPE/bin
			do	test -d $PACKAGEROOT/$i || $exec mkdir $PACKAGEROOT/$i || exit
			done
		else	_PACKAGE_cc=0
		fi
		;;
	esac
	case $_PACKAGE_cc in
	'')	if	executable $CC
		then	_PACKAGE_cc=1
		else	_PACKAGE_cc=0
		fi
		;;
	esac
	for i
	do	eval j=\$_PACKAGE_AOUT_$i
		case $j in
		'')	eval _PACKAGE_AOUT_$i=1 ;;
		*)	continue ;;
		esac
		k=$_PACKAGE_cc
		if	test -f $INITROOT/$i.c
		then	k=${k}1
		else	k=${k}0
		fi
		if	test -x $INSTALLROOT/bin/$i
		then	k=${k}1
		else	k=${k}0
		fi
		: $k : compiler : source : binary :
		case $k in
		*00)	view - bin/$i && continue ;;
		esac
		case $k in
		000)	echo "$command: $i: not found: download the INIT package $HOSTTYPE binary to continue" >&2
			exit 1
			;;
		010)	echo "$command: $i: not found: set CC=C-compiler or download the INIT package $HOSTTYPE binary to continue" >&2
			exit 1
			;;
		100)	echo "$command: $i: not found: download the INIT package source or $HOSTTYPE binary to continue" >&2
			exit 1
			;;
		?01)	: accept binary
			continue
			;;
		011)	: accept binary
			continue
			;;
		esac
		case `ls -t $INITROOT/$i.c $INSTALLROOT/bin/$i 2>/dev/null` in
		"$INITROOT/$i.c"*)
			note update $INSTALLROOT/bin/$i
			if	test ! -d $INSTALLROOT/bin
			then	for j in arch arch/$HOSTTYPE arch/$HOSTTYPE/bin
				do	test -d $PACKAGEROOT/$j || $exec mkdir $PACKAGEROOT/$j || exit
				done
			fi
			if	test '' != "$PROTOROOT" -a -f $INITPROTO/$i.c
			then	$exec $CC $CCFLAGS -o $INSTALLROOT/bin/$i $INITPROTO/$i.c || exit
			else	$exec $CC $CCFLAGS -o $INSTALLROOT/bin/$i $INITROOT/$i.c || exit
			fi
			test -f $i.o && $exec rm -f $i.o
			;;
		esac
	done
}

# check that all package licenses are/were accepted
#
# NOTE: circumventing the license checks will be taken
#	as an implicit acceptance of *all* licenses

accepted=.license.accepted

checklicenses()
{
	_checklicenses_dir=lib/package
	_checklicenses_ok=1
	_checklicenses_list=`ls $PACKAGESRC/LICENSES 2>/dev/null`" -- "`ls $PACKAGEBIN/LICENSES 2>/dev/null`
	for _checklicenses_i in $_checklicenses_list
	do	case $_checklicenses_i in
		--)	_checklicenses_dir=$PACKAGEBIN; break ;; # src for now
		README)	continue ;;
		esac
		_checklicenses_gen=$_checklicenses_dir/gen
		_checklicenses_accepted=$_checklicenses_gen/$_checklicenses_i$accepted
		if	view - $_checklicenses_accepted
		then	continue
		fi
		_checklicenses_gen=$PACKAGEROOT/$_checklicenses_gen
		_checklicenses_accepted=$PACKAGEROOT/$_checklicenses_accepted
		case $exec in
		'')	echo "$command: $PACKAGESRC/LICENSES/$_checklicenses_i: license has not been accepted yet" >&2
			while	:
			do	echo
				echo "Enter y to accept, p to print, n to reject, anything else to quit:" >&2
				read _checklicenses_reply || exit
				case $_checklicenses_reply in
				[nN]*)	_checklicenses_ok=0
					continue 2
					;;
				[pP]*)	case $PAGER in
					'')	if	more /dev/null 2>/dev/null
						then	PAGER=more
						else	PAGER=cat
						fi
						;;
					esac
					$PAGER $PACKAGESRC/LICENSES/$_checklicenses_i
					;;
				[yY]*)	break
					;;
				*)	exit
					;;
				esac
			done
			test -d $_checklicenses_gen || mkdir -p $_checklicenses_gen || exit
			touch $_checklicenses_accepted || exit
			;;
		*)	echo "$command: $PACKAGESRC/LICENSES/$_checklicenses_i: license has not been accepted yet" >&2
			_checklicenses_ok=0
			;;
		esac
	done
	case $_checklicenses_ok in
	0)	echo "$command: cannot continue until all licenses have been accepted" >&2
		exit 1
		;;
	esac
}

# check package requirements against received packages

requirements() # source|binary [ package ]
{
	case $1 in
	binary)	r=$VIEW_BIN ;;
	source)	r=$VIEW_SRC ;;
	*)	r=$VIEW_ALL ;;
	esac
	shift
	case $1 in
	'')	x= ;;
	*)	x=$* ;;
	esac
	set ''
	for d in $r
	do	set "$@" $d/gen/*.ver
		case $x in
		'')	set "$@" $d/gen/*.req
			;;
		*)	for p in $x
			do	set "$@" $d/gen/$p.req
			done
			;;
		esac
	done
	shift
	e=0
	x=$*
	y=
	n=
	set ''
	for i in $x
	do	p=`echo $i | sed -e 's,.*/,,' -e 's,\....$,,'`
		if	test -f $i
		then	set "$@" $i
			y="$y $p"
		else	case $p in
			'*')	;;
			*)	n="$n $p" ;;
			esac
		fi
	done
	for i in $n
	do	case " $y " in
		*" $i "*)
			;;
		*)	echo "$command: $i: must read or write package" >&2
			e=1
			;;
		esac
	done
	case $e in
	1)	exit 1 ;;
	esac
	shift
	test 0 != "$#" && release=`sort -r "$@" | {
		q=
		e=0
		o=
		while	read p v r s
		do	q="$q
$v $r"
			case $p in
			$o)	continue ;;
			esac
			case $s in
			0)	e=1
				case $r in
				0000)	echo "$command: base package $p.$v or newer required" >&2 ;;
				*)	echo "$command: delta package $p.$v.$r or newer required" >&2 ;;
				esac
				;;
			esac
			o=$p
		done
		case $e in
		0)	echo "$q" | sort | { read v r; read v r; echo $v; } ;;
		1)	echo ERROR ;;
		esac
	}`
	case $release in
	ERROR)	case $force in
		0)	exit 1 ;;
		esac
		;;
	?*)	eval `echo $release | sed -e 's,\(.*\)-\(.*\)-\(.*\),yy=\1 mm=\2 dd=\3,'`
		# slide back 4 months
		case $mm in
		01)	mm=09 dd=1 ;;
		02)	mm=10 dd=1 ;;
		03)	mm=11 dd=1 ;;
		04)	mm=12 dd=1 ;;
		05)	mm=01 dd=0 ;;
		06)	mm=02 dd=0 ;;
		07)	mm=03 dd=0 ;;
		08)	mm=04 dd=0 ;;
		09)	mm=05 dd=0 ;;
		10)	mm=06 dd=0 ;;
		11)	mm=07 dd=0 ;;
		12)	mm=08 dd=0 ;;
		esac
		case $dd in
		1)	yy=`expr $yy - 1` ;;
		esac
		release=$yy-$mm-01
		count=1
		lo=$release
		release="-f $release -r $count"
		;;
	esac
}

# write ordered package prerequisite list to the standard output

order() # [ package ]
{
	_order_t_=lib/package/tgz
	case $action in
	binary)	_order_a_=.$HOSTTYPE ;;
	*)	_order_a_= ;;
	esac
	_order_n_=$#
	case $_order_n_ in
	0)	_order_p_=
		for _order_v_ in $VIEW_all
		do	for _order_f_ in $_order_v_/lib/package/*.pkg
			do	if	test -f $_order_f_
				then	_order_p_="$_order_p_ $_order_f_"
				fi
			done
		done
		set '' $_order_p_
		shift
	esac
	{
	if	test ratz != "$*"
	then	for _order_f_ in ratz INIT
		do	if	view -s - src $_order_t_/$_order_f_$_order_a_.tim
			then	echo $_order_f_ $_order_f_
			fi
		done
	fi
	for _order_f_
	do	while	:
		do	
			view - src $_order_f_ && break
			case $_order_f_ in
			*.pkg)	;;
			*)	_order_f_=$_order_f_.pkg; view - src $_order_f_ && break ;;
			esac
			case $_order_f_ in
			*/*)	;;
			*)	_order_f_=lib/package/$_order_f_; view - src $_order_f_ && break ;;
			esac
			echo "$command: $_order_f_: not a package" >&2
			continue 2
		done
		_order_f_=$_view_
		_order_p_=`echo $_order_f_ | sed -e 's,.*/,,' -e 's,\.pkg$,,'`
		case $_order_n_ in
		0)	view -s - src $_order_t_/$_order_p_$_order_a_.tim || continue ;;
		esac
		echo $_order_p_ $_order_p_
		case $_order_p_ in
		INIT|ratz)
			;;
		*)	echo INIT $_order_p_
			;;
		esac
		{
		while	read line
		do	case $line in
			*:PACKAGE:*)	break ;;
			esac
		done
		req=$line
		case $line in
		*\\)	while	read line
			do	req="$req "$line
				case $line in
				*\\)	;;
				*)	break ;;
				esac
			done
			;;
		esac
		req=`echo $req | sed -e 's,.*:PACKAGE:,,' -e 's,\\\\, ,g'`
		for _order_i_ in $req
		do	if	view - src lib/package/$_order_i_.pkg
			then	case $_order_u_ in
				0)	view -s - src $_order_t_/$_order_i_$_order_a_.tim || continue ;;
				esac
				echo $_order_i_ $_order_i_; echo INIT $_order_i_; echo $_order_i_ $_order_p_
			fi
		done
		} < $_order_f_
	done
	} | tsort
}

# write package component list to the standard output

components() # [ package ]
{
	case $1 in
	'')	;;
	INIT)	echo $1
		;;
	*)	view package src lib/package/$1.pkg || return 1
		p=$_view_
		{
		while	read line
		do	case $line in
			*:PACKAGE:*)	break ;;
			esac
		done
		req=$line
		case $line in
		*\\)	while	read line
			do	req="$req "$line
				case $line in
				*\\)	;;
				*)	break ;;
				esac
			done
			;;
		esac
		req=`echo $req | sed -e 's,.*:PACKAGE:,,' -e 's,\\\\, ,g'`
		r=
		for i in $req
		do	view - src lib/package/$i.pkg || r="$r $i"
		done
		echo $r
		} < $p
		;;
	esac
}

# capture command output

capture() # file command ...
{
	case $make:$noexec in
	:)	case $package in
		'')	o=$action ;;
		*)	o=$package ;;
		esac
		case $action in
		write)	d=$PACKAGESRC/gen ;;
		*)	d=$PACKAGEBIN/gen ;;
		esac
		test -d $d || $exec mkdir $d
		o=$d/$o
		case $o in
		$output)o=$o.out
			s=
			;;
		*)	output=$o
			test -f $o.out && mv $o.out $o.old
			o=$o.out
			: > $o
			note $action output captured in $o
			s="$command: $action start at `date` in $INSTALLROOT"
			case $quiet in
			0)	trap "echo \"$command: $action done  at \`date\`\" in $INSTALLROOT 2>&1 | \$TEE -a $o" 0 1 2 ;;
			*)	trap "echo \"$command: $action done  at \`date\`\" in $INSTALLROOT >> $o" 0 1 2 ;;
			esac
			;;
		esac
		case $quiet in
		0)	if	test ! -x $TEE
			then	TEE=tee
			fi
			{
				case $s in
				?*)	echo "$s"  ;;
				esac
				"$@"
			} < /dev/null 2>&1 | $TEE -a $o
			;;
		*)	{
				case $s in
				?*)	echo "$s"  ;;
				esac
				"$@"
			} < /dev/null > $o 2>&1
			;;
		esac
		;;
	*)	$make "$@"
		;;
	esac
}

package_install() # dest sum
{
	dest=$1 sum=$2
	ot=
	code=0
	sed -e '/ /!d' -e 's,[^ ]* ,,' -e 's, \(arch/[^/]*\)/, \1 ,' -e '/ arch\//!s,^[^ ]* [^ ]* [^ ]*,& .,' -e 's,/\([^ /]*\)$, \1,' $sum |
	while	read mode user group arch dir file
	do	case $only:$arch in
		1:*|?:.)t=$dest/$dir ;;
		*)	t=$dest/$arch/$dir ;;
		esac
		case $t in
		$ot)	;;
		*)	if	test ! -d $t
			then	$exec mkdir $t || exit
			fi
			ot=$t
			;;
		esac
		case $file in
		?*)	case $arch in
			.)	f=$dir/$file ;;
			*)	f=$arch/$dir/$file ;;
			esac
			if	test -f $f
			then	t=$t/$file
				case $quiet in
				0)	echo $t ;;
				esac
				$exec cp -f $f $t || code=1
				$exec chmod $mode $t || code=1
			fi
			;;
		esac
	done
	return $code
}

package_verify() # sum
{
	$exec $SUM -cp $1
}

case $action in

admin)	checklicenses
	while	test ! -f $admin_db
	do	case $admin_db in
		/*)	echo $command: $action: $admin_db: data file not found >&2
			exit 1
			;;
		esac
		view file src lib/package/admin/$admin_db || exit 1
		admin_db=$_view_
	done
	case $admin_action in
	list)	cat $admin_db
		exit
		;;
	esac
	: all work done in $PACKAGESRC/admin
	cd $PACKAGESRC/admin || exit
	packages=
	admin_log=$admin_action.log
	exec < $admin_db || exit
	test -d $admin_log || $exec mkdir $admin_log || exit
	case $admin_on in
	'')	admin_on="*" ;;
	esac
	hosts=
	pids=
	trap 'kill $pids >/dev/null 2>&1' 1 2 3 15
	while	read type host root date time make test write junk
	do	case $type in
		''|'#'*);;
		*=*)	eval "$type $host $root $date $time $make $test $write $junk"
			;;
		*)	case $host in
			*@*)	IFS=@
				set '' $host
				IFS=$ifs
				name=$3
				;;
			*)	name=$host
				;;
			esac
			case $root in
			*:*)	case $admin_all in
				0)	continue ;;
				esac
				IFS=:
				set '' $root
				IFS=$ifs
				serv=$2
				root=$3
				;;
			*)	serv=
				;;
			esac
			case $host in
			$admin_on)
				;;
			*)	continue
				;;
			esac
			hosts="$hosts $name"
			case $exec in
			'')	echo package "$admin_args" $host
				rsh $host ". ./.profile && cd $root && { test -f lib/package/admin/$admin_env && . ./lib/package/admin/$admin_env || true; } && bin/package $admin_args PACKAGEROOT=$root HOSTTYPE=$type" < /dev/null > $admin_log/$name 2>&1 &
				pids="$pids $!"
				;;
			*)	$exec rsh $host ". ./.profile && cd $root && bin/package $admin_args PACKAGEROOT=$root HOSTTYPE=$type < /dev/null > $admin_log/$name 2>&1 &"
				;;
			esac
		esac
	done
	case $exec in
	'')	# track the progress
		case $quiet in
		0)	cd $admin_log
			tail -t 4m -f $hosts
			cd ..
			;;
		esac
		# wait for the remote actions to complete
		wait
		trap - 1 2 3 15
		# update the db
		exec < $admin_db || exit
		exec 9>&1
		D=`date +%y%m%d`
		while	read line
		do	set -- $line
			case $1 in
			''|'#'*|*=*)
				;;
			*)	case " $hosts " in
				*" $2 "*)
					: ast date command assumed :
					E=`eval date -E \`egrep '[ 	](start|done)[ 	][ 	]*at[ 	]' $admin_log/$2 | sed -e 's/.*[ 	][ 	]*at[ 	][ 	]*//' -e 's/[ 	][ 	]*in[ 	].*$//' -e 's/.*/"&"/'\``
					M=$6 T=$7 W=$8
					case $admin_action in
					make)	M=`grep -c ']: \*\*\*.* code ' $admin_log/$2` ;;
					test)	T=`grep -ci 'failed' $admin_log/$2` ;;
					*)	W=`grep '^[a-z][a-z]*:' $admin_log/$2 | egrep -cv 'start at|done  at|output captured|warning:'` ;;
					esac
					case $1 in
					?|??|???|????|?????|??????|???????)
						t1='	'
						;;
					*)	t1=''
						;;
					esac
					case $2 in
					?|??|???|????|?????|??????|???????)
						t2='	'
						;;
					*)	t2=''
						;;
					esac
					case $3 in
					?|??|???|????|?????|??????|???????)
						t3='	'
						;;
					*)	t3=''
						;;
					esac
					case $E in
					?????)	E=" $E" ;;
					????)	E="  $E" ;;
					???)	E="   $E" ;;
					??)	E="    $E" ;;
					?)	E="     $E" ;;
					esac
					echo "$1$t1	$2$t2	$3$t3	$D	$E	${M:-0} ${T:-0} ${W:-0}"
					echo "$1$t1	$2$t2	$3$t3	$D	$E	${M:-0} ${T:-0} ${W:-0}" >&9
					continue
					;;
				esac
				;;
			esac
			echo "$line"
		done > $admin_db.new
		mv $admin_db $admin_db.old
		mv $admin_db.new $admin_db
		;;
	esac
	;;

contents|list)
	# all work in $PACKAGESRC

	cd $PACKAGESRC

	# generate the package list

	set '' $target $package
	shift
	argc=$#
	case $# in
	0)	set '' *.pkg
		case $2 in
		'*.pkg')
			echo $command: $action: no packages >&2
			exit 1
			;;
		esac
		set '' `echo $* | sed 's,\.pkg,,g'`
		shift
		;;
	esac
	sep="$nl    "
	echo packages in $PACKAGEROOT
	case $action in
	list)	echo
		echo "NAME${nl}VERSION${nl}RELEASE${nl}TYPE${nl}STATUS${nl}REQUIRES${nl}----${nl}-------${nl}-------${nl}----${nl}------${nl}--------" | pr -6 -a -o4 -t
		;;
	esac
	{
	omit=:
	for pkg
	do	if	test ! -f $pkg.pkg
		then	echo $command: $action: $pkg: not a package >&2
		else	if	test -f gen/$pkg.ver
			then	set '' `cat gen/$pkg.ver`
				ver=$3
				if	test -s tgz/$pkg.tim
				then	sts=local
				else	sts=
				fi
			else	ver=
				sts=unwritten
			fi
			state=0
			cmp=
			typ=
			txt=
			req=
			while	:
			do	if	read line
				then	case $state in
					0)	cmp="$cmp $line"
						case $line in
						*\\)	;;
						*)	state=1 ;;
						esac
						;;
					1)	case $line in
						'')	state=2 ;;
						*)	txt="$txt$sep$line" ;;
						esac
						;;
					esac
				else	IFS=' 	\\'
					set '' $cmp
					IFS=$ifs
					cmp=
					state=0
					while	:
					do	shift
						case $1 in
						'')	break
							;;
						':PACKAGE:')
							state=1
							continue
							;;
						INIT)	continue
							;;
						'$('*|*')')
							continue
							;;
						esac
						case $state in
						1)	if	view - lib/package/$1.pkg
							then	req="$req $1"
							elif	test "" != "$cmp"
							then	cmp="$cmp$nl$1"
							else	cmp="$1"
							fi
							;;
						esac
					done
					case $req in
					*' '*)	set $req; req=$* ;;
					*)	req= ;;
					esac
					case $txt in
					?*)	txt="$nl$txt" ;;
					esac
					case $action in
					list)	case $sts in
						'')	case `ls -t "tgz/$pkg.$ver.0000" "tgz/$pkg.tim" 2>/dev/null` in
							"tgz/$pkg.tim"*)
								sts=read
								;;
							*)	sts=unread
								;;
							esac
							;;
						esac
						echo "$pkg${nl}$ver${nl}0000${nl}$typ${nl}$sts${nl}$req"
						case $typ in
						'')	omit=$omit$pkg.$ver.0000: ;;
						esac
						;;
					*)	case $req in
						?*)	req=": $req" ;;
						esac
						echo
						echo $pkg $ver $req "$txt"
						case $cmp in
						?*)	echo "${sep}Components in this package:$nl"
							echo "$cmp" | pr -4 -o4 -t ;;
						esac
						;;
					esac
					break
				fi
			done < $pkg.pkg
		fi
	done
	case $argc:$action in
	0:list)	if	test -d tgz
		then	cd tgz
			# f:file p:package v:version r:release t:type u:update
			for f in `ls -r *[._][0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9][._]* 2>/dev/null`
			do	eval `echo "$f" | sed -e 's,\.c$,,' -e 's,\.exe$,,' -e 's,\.tgz$,,' -e 's,\([^._]*\)[._]\([0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]\)[._]\([^._]*\)[._]*\(.*\),p=\1 v=\2 r=\3 t=\4,'`
				case $t in
				'')	case $omit in
					*:$p.$v.$r:*)	continue ;;
					esac
					u=$p.tim
					;;
				*)	u=$p.$t.tim
					;;
				esac
				if	test -s "$u"
				then	s=local
				elif	test -f "$u"
				then	case `ls -t "$f" "$u" 2>/dev/null` in
					"$u"*)	s=read ;;
					*)	s=unread ;;
					esac
				else	s=unread
				fi
				echo "$p$nl$v$nl$r$nl$t$nl$s$nl"
			done
		fi
		;;
	esac
	} |
	case $action in
	list)	pr -6 -a -o4 -t | sort -u ;;
	*)	cat ;;
	esac
	case $argc in
	0)	if	test -d $PACKAGEROOT/arch
		then	echo
			echo architectures in $PACKAGEROOT
			echo
			for i in `ls $PACKAGEROOT/arch`
			do	if	test -f $PACKAGEROOT/arch/$i/lib/package/gen/host
				then	h=`cat $PACKAGEROOT/arch/$i/lib/package/gen/host`
				else	h=
				fi
				echo $i
				echo $h
				echo
				echo
			done | pr -4 -a -o4 -t
		fi
		;;
	esac
	;;

copyright)
	# all work in $PACKAGESRC

	cd $PACKAGESRC

	# generate the package list

	set '' $target $package
	shift
	argc=$#
	case $# in
	0)	set '' `echo *.lic | sed 's,\.lic,,g'`
		shift
		case $1 in
		'*')	echo $command: $action: no packages >&2
			exit 1
			;;
		esac
		;;
	esac
	checkaout proto
	for i
	do	j=$i
		while	:
		do	if	test -f $j.lic
			then	echo $j package general copyright notice
				echo
				proto -c'#' -p -s -l $j.lic -o type=verbose,author='*' /dev/null
				break
			fi
			case $j in
			*-*)	j=`echo $j | sed -e 's,-[^-]*$,,'`
				;;
			*)	echo "$command: $i: no package license" >&2
				break
				;;
			esac
		done
	done
	;;

install)checklicenses
	cd $PACKAGEROOT
	set '' $package
	shift
	case $only in
	0)	set '' `order "$@"`
		shift
		;;
	esac
	case $# in
	0)	echo "$command: at least one package name expected" >&2
		exit 1
		;;
	esac
	package=$*
	requirements - $package
	set '' $target
	shift
	case $1 in
	flat)	flat=1
		shift
		;;
	*)	flat=0
		;;
	esac
	case $# in
	0)	echo "$command: $action: target directory argument expected" >&2
		exit 1
		;;
	esac
	target=
	while	:
	do	case $# in
		1)	directory=$1
			break
			;;
		esac
		target="$target $1"
		shift
	done
	if	test ! -d $directory
	then	echo "$command: $action: $directory: target directory not found" >&2
		exit 1
	fi
	if	executable $MAKE
	then	MAKE=$_executable_
	else	echo "$command: $MAKE: not found" >&2
		exit 1
	fi
	case $target in
	'')	cd arch
		set '' *
		shift
		target=$*
		cd ..
		;;
	esac
	code=0
	for a in $target
	do	case $flat:$a in
		1:*|?:.)dest=$directory ;;
		*)	dest=$directory/arch/$a ;;
		esac
		for i in $package
		do	if	test "ratz" = "$i"
			then	: skip
			elif	test -f arch/$a/lib/package/gen/$i.sum
			then	package_install $directory arch/$a/lib/package/gen/$i.sum || code=1
			elif	test ! -d arch/$a/bin
			then	echo "$command: $a: invalid architecture" >&2
			elif	test ! -d $dest
			then	echo "$command: $dest: destination directory must exist" >&2
			elif	test "" != "$exec"
			then	(
					echo "=== $i installation manifest ==="
					cd arch/$a
					(
					cd lib/package
					INSTALLROOT=$PACKAGEROOT/arch/$a
					VPATH=$INSTALLROOT:$PACKAGEROOT:$VPATH
					export INSTALLROOT VPATH
					$MAKE -s $makeflags -f $i.pkg $qualifier list.install $assign
					) | sort -u
				)
			else	(
					cd arch/$a
					(
					cd lib/package
					INSTALLROOT=$PACKAGEROOT/arch/$a
					VPATH=$INSTALLROOT:$PACKAGEROOT:$VPATH
					export INSTALLROOT VPATH
					eval capture \$MAKE -s \$makeflags -f \$i.pkg \$qualifier list.install $assign
					) | sort -u | pax -drw $dest
				)
			fi
		done
	done
	exit $code
	;;

make)	checklicenses
	cd $PACKAGEROOT
	case $package in
	'')	lic="lib/package/*.lic"
		;;
	*) 	for i in $package
		do	lic=lib/package/$i.lic
			case $i in
			*-*)	lic="$lic lib/package/"`echo $i | sed 's,-.*,,'`".lic" ;;
			esac
		done
		;;
	esac
	requirements source $package
	package=`components $package`

	# verify the top view

	if	test ! -d $INSTALLROOT/src
	then	note initialize the $INSTALLROOT view
	fi
	for i in arch arch/$HOSTTYPE
	do	test -d $PACKAGEROOT/$i || $exec mkdir $PACKAGEROOT/$i || exit
	done
	for i in bin fun include lib lib/package lib/package/gen src man man/man1 man/man3 man/man8
	do	test -d $INSTALLROOT/$i || $exec mkdir $INSTALLROOT/$i || exit
	done
	for d in $src
	do	i=src/$d
		if	test -d $PACKAGEROOT/$i
		then	test -d $INSTALLROOT/$i || $exec mkdir $INSTALLROOT/$i || exit
		fi
		for i in src/$d/*
		do	for j in Mamfile Nmakefile nmakefile Makefile makefile
			do	if	test -f $i/$j
				then	test -d $INSTALLROOT/$i || $exec mkdir $INSTALLROOT/$i || exit
					break
				fi
			done
		done
	done
	for i in $lic
	do	test -f $i || continue
		cmp -s $i $INSTALLROOT/$i || $exec cp $PACKAGEROOT/$i $INSTALLROOT/$i
	done
	if	test ! -f $INSTALLROOT/bin/.fpath
	then	case $exec in
		'')	echo ../fun > $INSTALLROOT/bin/.fpath ;;
		*)	$exec "echo ../fun > $INSTALLROOT/bin/.fpath" ;;
		esac
	fi
	checkaout mamake ratz release

	# all work under $INSTALLROOT/src

	$make cd $INSTALLROOT/src

	# record the build host name

	case $noexec in
	'')	hostinfo name
		echo "$_hostinfo_" | sed 's,\..*,,' > $PACKAGEBIN/gen/host
		;;
	esac

	# make in parallel if possible

	case $NPROC in
	'')	hostinfo cpu
		case $_hostinfo_ in
		0|1)	;;
		*)	NPROC=$_hostinfo_
			$show NPROC=$NPROC
			$show export NPROC
			export NPROC
			;;
		esac
		;;
	esac

	# generate nmake first if possible

	if	test ! -x $NMAKE -a -d $PACKAGEROOT/src/cmd/nmake
	then	if	nonmake $MAKE
		then	note make $NMAKE with mamake
			eval capture mamake \$makeflags \$noexec install nmake $assign
			case $make$noexec in
			'')	if	test ! -x $NMAKE
				then	echo "$command: $action: errors making $NMAKE" >&2
					exit 1
				fi
				;;
			*)	make=echo
				;;
			esac
			note accept generated files for $NMAKE
			eval capture \$NMAKE \$makeflags \$noexec -t recurse install nmake $assign
			note make the remaining targets with $NMAKE
		else	eval capture $MAKE \$makeflags \$noexec install nmake $assign
			case $make$noexec in
			'')	if	test ! -x $NMAKE
				then	echo "$command: $action: errors making $NMAKE" >&2
					exit 1
				fi
				;;
			*)	make=echo
				;;
			esac
		fi
	fi

	# generate ksh next if possible

	if	test "$KEEP_SHELL" != 1 -a ! -x $KSH -a -d $PACKAGEROOT/src/cmd/ksh93
	then	if	nonmake $MAKE
		then	m=mamake
		else	m=nmake
		fi
		eval capture $m \$makeflags \$noexec install ksh93 $assign
		case $make$noexec in
		'')	if	test ! -x $KSH
			then	echo "$command: $action: errors making $KSH" >&2
				exit 1
			fi
			;;
		*)	make=echo
			;;
		esac
	fi

	# run from *-ok copies since this may build nmake and ksh

	if	test -x $NMAKE
	then	cmp -s $NMAKE $NMAKE-ok 2>/dev/null ||
		$exec cp $NMAKE $NMAKE-ok
		MAKE=$NMAKE-ok
		if	test -x $TEE
		then	cmp -s $TEE $TEE-ok 2>/dev/null ||
			$exec cp $TEE $TEE-ok
			TEE=$TEE-ok
		fi
		if	test "$KEEP_SHELL" != 1 -a -x $KSH
		then	cmp -s $KSH $KSH-ok 2>/dev/null ||
			$exec cp $KSH $KSH-ok
			SHELL=$KSH-ok
			export SHELL
			COSHELL=$SHELL
			export COSHELL
		fi
	fi

	# fall back to mamake if nmake not found or too old

	if	nonmake $MAKE
	then	note make with mamake
		case $target in
		'')	target="install" ;;
		esac
		eval capture mamake \$makeflags \$noexec install $assign
	else	case $target in
		'')	target="install cc-" ;;
		esac
		eval capture \$MAKE \$makeflags \$noexec recurse \$package \$target $assign
	fi
	;;

license)# all work in $PACKAGESRC/LICENSES

	cd $PACKAGESRC/LICENSES || exit

	# generate the package list

	set '' $target $package
	shift
	argc=$#
	case $# in
	0)	set '' *
		shift
		case $1 in
		'*')	echo $command: $action: no licenses >&2
			exit 1
			;;
		esac
		;;
	esac
	for i
	do	j=$i
		while	:
		do	if	test -f $j
			then	echo $j package source license
				echo
				cat $j
				break
			fi
			case $j in
			*-*)	j=`echo $j | sed -e 's,-[^-]*$,,'`
				;;
			*)	echo "$command: $i: no package license" >&2
				break
				;;
			esac
		done
	done
	;;

read)	checklicenses
	case `pwd` in
	$PACKAGEROOT)
		;;
	*)	echo "$command: must be in package root directory" >&2
		exit 1
		;;
	esac
	code=0
	i=
	x=
	remove=
	touch=
	set '' $package $target
	case $# in
	1)	verbose=:
		set '' `ls lib/package/tgz/*[._][0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9][._]* 2>/dev/null`
		;;
	*)	verbose=
		;;
	esac
	shift
	files=
	for f
	do	if	test ! -f "$f"
		then	set '' `ls -r ${f}[._][0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9][._]* 2>/dev/null`
			if	test '' != "$2" -a -f "$2"
			then	f=$2
			else	set '' `ls -r lib/package/tgz/${f}[._][0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9][._]* 2>/dev/null`
				if	test '' != "$2" -a -f "$2"
				then	f=$2
				else	echo "$command: $f: package archive not found" >&2
					continue
				fi
			fi
		fi
		files="$files $f"
	done
	case $files in
	'')	echo "$command: lib/package/tgz: no package archives" >&2
		exit 1
		;;
	esac
	set '' `ls -r $files 2>/dev/null`
	shift
	f1= f2= f3= f4=
	for f
	do	case $f in
		ratz.*|*/ratz.*)
			f1="$f1 $f"
			;;
		INIT.*|*/INIT.*)
			f2="$f2 $f"
			;;
		INIT*|*/INIT*)
			f3="$f3 $f"
			;;
		*)	f4="$f4 $f"
			;;
		esac
	done
	for f in $f1 $f2 $f3 $f4
	do	case $f in
		*[._][0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9][._]*)
			;;
		*)	echo "$command: $f: not a package archive" >&2
			code=1
			continue
			;;
		esac
		case $f in
		*/*)	eval `echo "$f" | sed -e 's,\(.*/\)\(.*\),d=\1 a=\2,'` ;;
		*)	d= a=$f ;;
		esac
		# f:file d:dir a:base p:package v:version r:release t:type
		eval `echo "$a" | sed -e 's,\.c$,,' -e 's,\.exe$,,' -e 's,\.tgz$,,' -e 's,\([^._]*\)[._]\([0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]\)[._]\([^._]*\)[._]*\(.*\),p=\1 v=\2 r=\3 t=\4,'`
		case " $x " in
		*" $p "*)
			continue
			;;
		esac
		case $t in
		'')	w=$PACKAGESRC
			q=
			Q=
			m=
			;;
		*)	w=$PACKAGEROOT/arch/$t/lib/package
			q=".$t"
			Q="_$t"
			m="[._]$t"
			;;
		esac
		u=$d$p$q.tim
		if	test -s "$u"
		then	continue
		fi
		case $force in
		0)	case `ls -t "$f" "$u" 2>/dev/null` in
			"$u"*)	case $verbose in
				1)	note $p already read ;;
				esac
				continue
				;;
			esac
			;;
		esac
		z=
		case $r in
		0000)	# base archive
			if	test ratz = "$p"
			then	# ratz packages are not archives
				case $t in
				'')	for i in src src/cmd src/cmd/INIT
					do	test -d $PACKAGEROOT/$i || $exec mkdir $PACKAGEROOT/$i || exit
					done
					$exec cp $f $PACKAGEROOT/src/cmd/INIT/$p.c
					;;
				*)	for i in arch arch/$t arch/$t/bin
					do	test -d $PACKAGEROOT/$i || $exec mkdir $PACKAGEROOT/$i || exit
					done
					$exec cp $f $PACKAGEROOT/arch/$t/bin/$p &&
					$exec chmod +x $PACKAGEROOT/arch/$t/bin/$p
					;;
				esac
			elif	test "" != "$PAX"
			then	$exec pax -lm -ps -rvf "$f" || {
					code=1
					continue
				}
			else	if	executable gunzip && executable tar
				then	case $exec in
					'')	$exec gunzip < "$f" | tar xvf - ;;
					*)	$exec "gunzip < $f | tar xvf -" ;;
					esac || {
						code=1
						continue
					}
				else	checkaout ratz
					case $exec in
					'')	$exec ratz -lv < "$f" ;;
					*)	$exec "ratz -lv < $f" ;;
					esac || {
						code=1
						continue
					}
				fi
				if	test -f $PACKAGEBIN/gen/$p.sum
				then	while	read md5 mode usr grp file
					do	case $mode in
						0*)	case $grp in
							-)	;;
							*)	$exec chgrp $grp $file ;;
							esac
							case $usr in
							-)	;;
							*)	$exec chown $usr $file ;;
							esac
							$exec chmod $mode $file
							;;
						esac
					done < $PACKAGEBIN/gen/$p.sum
				fi
			fi
			;;
		*)	# delta archive
			test "" != "$PAX" || {
				echo "$command: $f: pax required to read delta archive" >&2
				code=1
				continue
			}
			b=${d}${p}_${v}_0000${Q}.tgz
			test -f "$b" || b=${d}${p}.${v}.0000${q}.tgz
			test -f "$b" || {
				echo "$command: $f: base archive $b required to read delta" >&2
				code=1
				continue
			}
			$exec pax -lm -ps -rvf "$f" -z "$b" || {
				code=1
				continue
			}
			case $r in
			[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9])
				note $f: generate new base $d$p.$r.0000$q.tgz
				$exec pax -rf "$f" -z "$b" -wf $d$p.$r.0000$q.tgz -x tgz || {
					code=1
					continue
				}
				case $exec in
				'')	echo $p $r 0000 1 > $w/gen/$p.ver
					;;
				*)	z=$d$p[._]$r[._]0000$q.tgz
					$exec "echo $p $r 0000 1 > $w/gen/$p.ver"
					;;
				esac
				remove="$remove $f"
				;;
			esac
			;;
		*)	echo "$command: $f: unknown archive type" >&2
			code=1
			continue
			;;
		esac

		# check for ini files

		if	test -x $w/$p.ini
		then	$exec $w/$p.ini read || {
				code=1
				continue
			}
		fi

		# add to the obsolete list

		k=
		for i in `ls $d$p[._][0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9][._]????$m* $z 2>/dev/null`
		do	case $i in
			$d$p[._][0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9][._]0000$m*)
				continue
				;;
			esac
			case $k in
			?*)	remove="$remove $k" ;;
			esac
			k=$i
		done
		x="$x $p"
		touch="$touch $u"
	done

	# drop obsolete archives

	case $remove in
	?*)	$exec rm -f $remove ;;
	esac

	# mark the updated archives

	case $touch in
	?*)	sleep 1; $exec touch $touch ;;
	esac

	# check the requirements

	case $code$exec in
	0)	requirements - $x ;;
	esac
	checklicenses
	exit $code
	;;

release)count= lo= hi=
	checkaout release
	requirements source $package
	package=`components $package`
	set '' $target
	shift
	case $# in
	0)	;;
	*)	case $1 in
		-|[0-9][0-9]-[0-9][0-9]-[0-9][0-9]|[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9])
			case $1 in
			-)	lo= release= ;;
			*)	lo=$1 release="-f $1" ;;
			esac
			shift
			case $1 in
			-|[0-9][0-9]-[0-9][0-9]-[0-9][0-9]|[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9])
				case $1 in
				-)	hi= ;;
				*)	hi=$1 release="$release -t $1" ;;
				esac
				shift
				;;
			esac
			;;
		[0-9]|[0-9][0-9]|[0-9][0-9][0-9]|[0-9][0-9][0-9][0-9]|[0-9][0-9][0-9][0-9][0-9]*)
			count=$1
			release="-r $count"
			shift
			;;
		esac
		;;
	esac
	case $# in
	0)	case $package in
		'')	package=* ;;
		esac
		;;
	*)	case $package in
		'')	package=$*
			;;
		*)	echo $command: $*: lo-date hi-date arguments expected >&2
			exit 1
			;;
		esac
		;;
	esac
	echo
	case $count:$lo:$hi in
	::)	echo "All recorded changes follow." ;;
	1::)	echo "Changes since the last release follow." ;;
	?*::)	echo "Changes since the last $count releases follow." ;;
	1:?*:)	echo "Changes since $lo or the last release follow." ;;
	*:?*:*)	echo "Changes since $lo follow." ;;
	*::?*)	echo "Changes before $hi follow." ;;
	*)	echo "Changes between $lo and $hi follow." ;;
	esac
	x=
	for r in $INSTALLROOT $PACKAGEROOT
	do	for s in $src
		do	d=$r/src/$s
			if	test -d $d
			then	cd $d
				for i in $package
				do	if	test -h $i 2>/dev/null
					then	continue
					fi
					case " $x " in
					*" $i "*)	continue ;;
					esac
					for f in RELEASE CHANGES ChangeLog
					do	if	test -f $i/$f
						then	$exec $INSTALLROOT/bin/release $release $i/$f
							x="$x $i"
							break
						fi
					done
				done
			fi
		done
	done
	;;

remove)	echo "$command: $action: not implemented yet" >&2
	exit 1
	;;

results)set '' $target
	shift
	def=make
	dir=$PACKAGEBIN/gen
	path=0
	suf=out
	on=
	while	:
	do	case $# in
		0)	break ;;
		esac
		case $1 in
		--)	shift; break ;;
		admin)	dir=$PACKAGESRC/admin ;;
		make|test|write) def=$1 ;;
		old)	suf=old ;;
		on)	case $# in
			1)	echo $command: $action: $1: host pattern argument expected >&2
				exit 1
				;;
			esac
			shift
			case $on in
			?*)	on="$on|" ;;
			esac
			on="$on$1"
			;;
		path)	path=1 ;;
		*)	break ;;
		esac
		shift
	done
	case $dir in
	*/admin)case $on in
		'')	on="*" ;;
		*)	on="@($on)" ;;
		esac
		def=$def.log/$on
		;;
	esac
	case $# in
	0)	set "$def" ;;
	esac
	m=
	t=
	for i
	do	k=0
		eval set '""' $i - $i.$suf - $dir/$i - $dir/$i.$suf -
		shift
		for j
		do	case $j in
			-)	case $k in
				1)	continue 2 ;;
				esac
				;;
			*)	if	test -f $j
				then	k=1
					case /$j in
					*/test.*)	t="$t $j" ;;
					*)		m="$m $j" ;;
					esac
				fi
				;;
			esac
		done
		echo "$command: $i action output not found" >&2
		exit 1
	done
	sep=
	case $t in
	?*)	case $path in
		0)	for j in $t
			do	echo "$sep==> $j <=="
				sep=$nl
				case $verbose in
				0)	$exec egrep '^TEST|FAIL|fail' $j
					;;
				1)	$exec cat $j
					;;
				esac
			done
			;;
		1)	echo $t
			;;
		esac
		;;
	esac
	case $m in
	?*)	case $path in
		0)	case $verbose in
			0)	if	test -f $HOME/.pkgresults
				then	i="`cat $HOME/.pkgresults`"
					case $i in
					'|'*)	;;
					*)	i="|$i" ;;
					esac
				else	i=
				fi
				for j in $m
				do	echo "$sep==> $j <=="
					sep=$nl
					$exec egrep -iv '^($||[\+\[]|cc[^-:]|kill |make: .*file system time|so|[0-9]+ error|uncrate |[0-9]+ block|ar: creat|iffe: test: |conf: (check|generate|test)|[a-zA-Z_][a-zA-Z_0-9]*=|gsf@research|ar:.*warning|cpio:|[0-9]*$|(checking|creating|touch) [/a-zA-Z_0-9])| obsolete predefined symbol | is dangerous | is not implemented| trigraph| assigned to | passed as |::__builtin|pragma.*prototyped|^creating.*\.a$|warning.*not optimized|exceeds size thresh|ld:.*preempts|is unchanged|with value >=|(-l|lib)\*|/(ast|sys)/(dir|limits|param|stropts)\.h.*redefined|usage|base registers|`\.\.\.` obsolete'"$i" $j
				done
				;;
			1)	cat $m
				;;
			esac
			;;
		1)	echo $m
			;;
		esac
	esac
	;;

test)	requirements source $package
	package=`components $package`

	# must have nmake

	if	nonmake $MAKE
	then	echo $command: $action: must have $MAKE to test >&2
		exit 1
	fi

	# all work under $INSTALLROOT/src

	$make cd $INSTALLROOT/src

	# do the tests

	eval capture \$MAKE \$makeflags \$noexec \'test : .DONTCARE .ONOBJECT\' recurse \$package \$target test cc- $assign
	;;

use)	# finalize the environment

	x=:..:$INSTALLROOT/src/cmd:$INSTALLROOT/src/lib:$INSTALLROOT
	case $CDPATH: in
	$x:*)	;;
	*)	CDPATH=$x:$CDPATH
		$show CDPATH=$CDPATH
		$show export CDPATH
		export CDPATH
		;;
	esac
	P=$PACKAGEROOT
	$show P=$P
	$show export P
	export P
	A=$INSTALLROOT
	$show A=$A
	$show export A
	export A
	case $NPROC in
	'')	hostinfo cpu
		case $_hostinfo_ in
		0|1)	;;
		*)	NPROC=$_hostinfo_
			$show NPROC=$NPROC
			$show export NPROC
			export NPROC
			;;
		esac
		;;
	esac

	# run the command

	case $run in
	'')	case $show in
		':')	$exec exec $SHELL ;;
		esac
		;;
	*)	$exec exec $SHELL -c "$run"
		;;
	esac
	;;

verify)	cd $PACKAGEROOT
	checklicenses
	requirements binary $package
	if	test ! -x $SUM
	then	echo "$command: $action: $SUM command required" >&2
		exit 1
	fi
	case $target in
	'')	cd arch
		set '' *
		shift
		target=$*
		cd ..
		;;
	esac
	code=0
	for a in $target
	do	case $package in
		'')	set '' arch/$a/lib/package/gen/*.sum
			shift
			if	test -f $1
			then	for i
				do	package_verify $i || code=1
				done
			else	echo "$command: warning: $a: no binary packages" >&2
			fi
			;;
		*)	for i in $package
			do	if	test -f arch/$a/lib/package/gen/$i.sum
				then	package_verify arch/$a/lib/package/gen/$i.sum || code=1
				else	echo "$command: warning: $a: no binary package for $i" >&2
				fi
			done
			;;
		esac
	done
	exit $code
	;;

write)	set '' $target
	shift
	action=
	qualifier=
	while	:
	do	case $1 in
		base|closure|delta|exp|lcl|pkg|rpm)
			qualifier="$qualifier $1"
			;;
		binary|source)
			action=$1
			;;
		tgz)	;;
		*)	break
			;;
		esac
		shift
	done
	case $action in
	'')	echo "$command: binary or source operand expected" >&2
		exit 1
		;;
	esac
	set '' "$@" $package
	shift
	case $only in
	0)	set '' `order "$@"`
		shift
		;;
	esac
	case $# in
	0)	echo "$command: at least one package name expected" >&2
		exit 1
		;;
	esac
	if	nonmake $MAKE
	then	echo "$command: must have $MAKE to generate archives" >&2
		exit 1
	fi

	# all work under $PACKAGEBIN

	$make cd $PACKAGEBIN

	# go for it

	for package
	do	if	view - all $package.pkg || view - all lib/package/$package.pkg
		then	eval capture \$MAKE \$makeflags \$noexec -f \$package.pkg \$qualifier \$action $assign
		else	echo "$command: $package: not a package" >&2
		fi
	done
	;;

TEST)	set '' $target $package
	shift
	case $1 in
	binary|source)
		action=$1
		shift
		;;
	esac
	order "$@"
	;;

*)	echo "$command: $action: internal error" >&2
	exit 1
	;;

esac
