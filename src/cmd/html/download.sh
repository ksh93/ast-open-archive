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
: download item hosttype ...

command=download
case $(getopts '[-][123:xyz]' opt --xyz 2>/dev/null; echo 0$opt) in
0123)	ARGV0="-a $command"
	USAGE=$'
[-?
@(#)download (AT&T Labs Research) 2000-12-07
]
'$USAGE_LICENSE$'
[+NAME?download - generate web site download instructions for an mm document]
[+DESCRIPTION?\bdownload\b is a \bksh\b(1) script that generates web site
	download instructions for an \bmm\b(1) document. \aitem\a names the
	item to be downloaded and \atype\a ... names the supported architecture
	type(s) as reported by \bpackage\b(1).]
[+?\bdownload\b is most often used in the \b.sh\b directive of \bmm2html\b(1)
	or \btroff2html\b(1): \b.sh download warp sgi.mips3 linux.i386\b.]

item [ type ... ]

[+SEE ALSO?\bmm2html\b(1), \bpackage\b(1), \btroff2html\b(1)]
'
	id=${USAGE##????}
	id=${id%']'*}
	;;
*)	ARGV0=""
	USAGE='i:[file] [ file ... ]'
	id='mm2html (AT&T Labs Research) 1999-04-15'
	;;
esac

while	getopts $ARGV0 "$USAGE" OPT
do	case $OPT in
	i)	. $OPT || exit 1
		;;
	*)	echo "Usage: $command [ options ] item [ type ... ]" >&2
		exit 2
		;;
	esac
done
shift $OPTIND-1
case $# in
0|1)	exit ;;
esac

item=$1
shift
print -r -- ".H 1 Download"
print -r -- "Check the"
print -r -- ".xx link=\"../faq.html	FAQ\""
print -r -- "for information on binary installations."
print -r -- "Source and binary packages for"
print -r -- ".B ${item}"
print -r -- "are available at the"
print -r -- ".xx link=\"/sw/download/	package\""
print -r -- "download site; evaluation binaries containing only"
print -r -- ".B ${item}"
print -r -- "are also available for these system architectures:"
print -r -- ".BL"
for type
do	print -r -- ".LI"
	print -r -- ".xx link=\"./${item}-${type}.tgz	${type}\""
done
print -r -- ".LE"
print -r -- ".PP"
print -r -- "NOTE: Click the right mouse button to make sure the file is"
print -r -- "dowloaded and not displayed (go figure)."
