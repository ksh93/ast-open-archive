:PACKAGE: ast

LICENSE = since=1996,author=kpv+gsf

VERSION = 1.0

recsort $(VERSION) :LIBRARY: RELEASE recsort.3 recsort.h rshdr.h \
		rsclear.c rsclose.c rsdisc.c rslist.c \
		rsmerge.c rsmethod.c rsopen.c rsprocess.c rswrite.c \
		rs-radix.c rs-rasp.c rs-splay.c rs-verify.c \
		rskeyopen.c rskey.c rskeymeth.c rskeydump.c

$(INCLUDEDIR) :INSTALLPROTO: recsort.h

"win32*" :NOOPTIMIZE: rskey.c
