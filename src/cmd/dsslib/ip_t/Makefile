:PACKAGE: ast

LICENSE = since=2000,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

_BLD_pt == 1

ip_t plugin=dss static :LIBRARY: ip_t.c lib.c \
		ire.c itl.c \
		ptaddresses.c ptcopy.c ptcover.c ptdifference.c ptequal.c \
		ptintersect.c ptinvert.c ptmatch.c ptopen.c ptprint.c \
		ptranges.c ptsize.c ptstats.c ptsubset.c ptunion.c ptdump.c \
		ptrebit.c \
		-ldss

$(INCLUDEDIR) :INSTALLPROTO: bgp.h pt.h

:: RELEASE
