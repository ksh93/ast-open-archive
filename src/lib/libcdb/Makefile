:PACKAGE: ast

LICENSE = since=1997,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.SUFFIX.DYNAMIC:@?$(CC.DLL)??)

cdb 2.2 :LIBRARY: NOTES RELEASE cdb.3 cdb.h cdbmethod.h cdblib.h \
	cdbopen.c cdbread.c cdbwrite.c cdbseek.c cdbmove.c \
	cdbmap.c cdbcache.c cdbdrop.c cdbempty.c cdblib.c \
	cdbtypes.c cdbmeth.c cdbdump.c cdberror.c cdbimage.c \
	cdbcdb.c cdbflat.c \
	cdbgzip.c cdbpzip.c cdbvdelta.c \
	-lexpr -lpz -lz -ldll -lvdelta -lm

$(INCLUDEDIR) :INSTALLPROTO: cdb.h cdbmethod.h
