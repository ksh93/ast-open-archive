:PACKAGE: ast

LICENSE = since=1998,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

pz 1.0 :LIBRARY: RELEASE pzip.3 pzip.h pzlib.h \
	pzopen.c pzhead.c pzpart.c \
	pzdeflate.c pzinflate.c pzlib.c \
	pzread.c pzwrite.c pzsync.c pzsplit.c \
	pzfixed.c pzconvert.c sfdcpzip.c \
	-lz -ldll

pzline 1.0 :LIBRARY: pzline.c -lpz

$(INCLUDEDIR) :INSTALLPROTO: pzip.h

if CC.HOSTTYPE == "win32*" || "$(-mam)" == "static*"
pzread.o : .CC.NOOPTIMIZE
end
