:PACKAGE: ast

VERSION			== "2.7"
RELEASE			== "1994-10-01"
LICENSE			=  gnu,package="GNU diff",since=1988

DIFF_PROGRAM		== "diff"
DEFAULT_EDITOR_PROGRAM	== "ed"
PR_PROGRAM		== "pr"

diff::	diff.c analyze.c dir.c io.c util.c context.c ed.c ifdef.c \
	cmpbuf.c normal.c side.c \
	LICENSE+='author="Paul Eggert,Richard Stallman,Mike Haertel,David Hayes,Len Tower"'

diff3::	diff3.c \
	LICENSE+='author="Randy Smith"'

sdiff::	sdiff.c \
	LICENSE+='since=1992,author="Thomas Lord"'

:: COPYING ChangeLog NEWS INSTALL README diagmeet.note \
	diff.info diff.info-1 diff.info-2 diff.info-3 diff.info-4 \
	diff.texi texinfo.tex \
	Makefile.in config.hin

:MSGFUN: fatal message \
	perror_fatal perror_with_exit perror_with_name pfatal_with_name
