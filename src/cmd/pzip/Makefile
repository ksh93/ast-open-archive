:PACKAGE: pz ast

LICENSE = since=1998,author=gsf

:ALL:

:INSTALLDIR: pcmp r2c

pzip :: pzip.c -lbz

pcmp :: pcmp.c

pop :: pop.c

pin :: pin.c -lbz -ltsp LICENSE=since=1998,author=gsf+alb

-ltsp : .DONTCARE

r2c :: r2c.c

rectify :: rectify.c

funzip :: funzip.c

:TEST: pzip

:: NOTES RELEASE PROMO.mm compare mkprt tabulate

:WWW: pzip.mm pin pop pzip

:MSGKEY: pzip
	pzip -lline --keys 2>&1 || true
