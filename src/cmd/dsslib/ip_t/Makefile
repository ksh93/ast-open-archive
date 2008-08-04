:PACKAGE: ast

LICENSE = since=2000,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

_BLD_pt == 1

ip_t plugin=dss static :LIBRARY: ip_t.c lib.c \
	ire.c itl.c \
	ptaddresses.c ptcopy.c ptcover.c ptdifference.c ptequal.c \
	ptintersect.c ptinvert.c ptmatch.c ptopen.c ptprint.c ptranges.c \
	ptsize.c ptstats.c ptsubset.c ptunion.c ptdump.c ptrebit.c \
	fv.c \
	ptvaddresses.c ptvcopy.c ptvcover.c ptvdifference.c ptvdump.c \
	ptvequal.c ptvintersect.c ptvinvert.c ptvmatch.c ptvmin.c ptvmax.c \
	ptvopen.c ptvprint.c ptvranges.c ptvrebit.c ptvsize.c ptvstats.c \
	ptvsubset.c ptvunion.c \
	-ldss

$(INCLUDEDIR) :INSTALLPROTO: bgp.h pt.h ptv.h fv.h

:INSTALLDIR: bgpsize

bgpsize :: bgpsize.c

PAXFILTER = ;*.[ch];$(PROTO) $(PROTOFLAGS) -c "" -p

ptvsa.tgz : $$("ptvsa.manifest":T=F:T=I:/[[:space:]][[:space:]]*/ /G)
	$(PAX) $(PAXFLAGS) -wvf $(<) -x $(<:/.*\.//) -s ',.*/,,' -A $(PAXFILTER:@Q) $(*)

:: RELEASE README-ptvsa ptvsa.manifest ptvsa.omk
