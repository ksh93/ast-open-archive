########################################################################
#                                                                      #
#               This software is part of the ast package               #
#                  Copyright (c) 1996-2004 AT&T Corp.                  #
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
#                                                                      #
########################################################################
: download item hosttype ...

command=download
case $(getopts '[-][123:xyz]' opt --xyz 2>/dev/null; echo 0$opt) in
0123)	ARGV0="-a $command"
	USAGE=$'
[-?
@(#)$Id: download (AT&T Labs Research) 2000-12-07 $
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
