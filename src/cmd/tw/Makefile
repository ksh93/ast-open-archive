:PACKAGE: ast

LICENSE = since=1989,author=gsf

tw :: RELEASE PROMO.mm tw.1 tw.c expr.c ftwlocal.c cmdarg.c -lexpr

find :: find.c ftwlocal.c cmdarg.o LICENSE=since=1989,author=gsf+dgk

xargs :: xargs.c cmdarg.o

locate :: locate.sh

updatedb :: updatedb.sh

:TEST: find tw xargs

:: dag.tw dot.tw du.tw ls.tw
