:PACKAGE: ast

LICENSE = since=1998,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

VERSION = 1.0

DEBUG == 1

warp $(VERSION) :LIBRARY: warp.c +ldll

warp :: warp.sh

:: README RELEASE PROMO.mm

:TEST: warp

:INSTALLDIR: date-www

../std/date.o : .DO.NOTHING

date-www :: ../std/date.o +last

:WWW: warp.mm warp date-www testwarp-www
