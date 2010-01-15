:PACKAGE: ast

LICENSE = since=2000,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

_BLD_pt == 1

ip_t plugin=dss static :LIBRARY: iv.3 ptv.3 \
	ip_t.c lib.c \
	ire.c itl.c \
	ptaddresses.c ptcopy.c ptcover.c ptdifference.c ptequal.c \
	ptintersect.c ptinvert.c ptmatch.c ptopen.c ptprint.c ptranges.c \
	ptsize.c ptstats.c ptsubset.c ptunion.c ptdump.c ptrebit.c \
	fv.c \
	ivopen.c ivclose.c ivmeth.c iv-flat.c iv-nested.c \
	ptvaddresses.c ptvcopy.c ptvcover.c ptvdifference.c ptvdump.c \
	ptvequal.c ptvintersect.c ptvinvert.c ptvmatch.c ptvmin.c ptvmax.c \
	ptvopen.c ptvprint.c ptvranges.c ptvrebit.c ptvsize.c ptvstats.c \
	ptvsubset.c ptvunion.c \
	-ldss

$(INCLUDEDIR) :INSTALLPROTO: bgp.h fv.h iv.h ivmethods.h pt.h ptv.h

iv :TEST: testiv iv-prefix.dat iv-addr.dat testiv.out
	set +x
	$(*:O<4) > testiv.tmp
	cmp -s testiv.tmp $(*:O=4) || echo iv test failed

:INSTALLDIR: bgpsize testiv testptv

bgpsize :: bgpsize.c

testiv :: testiv.c -ldssip_t

testptv :: testptv.c -ldssip_t

PAXFILTER = ;*.[ch];$(PROTO) $(PROTOFLAGS) -c "" -p

ivsa.tgz ptvsa.tgz : $$(<:B:S=.manifest:T=F:T=I:/[[:space:]][[:space:]]*/ /G)
	$(PAX) $(PAXFLAGS) -wf $(<) -x $(<:/.*\.//) -s ',.*/,,' -A $(PAXFILTER:@Q) $(*)

:: RELEASE \
	README-ivsa ivsa.manifest ivsa.omk \
	README-ptvsa ptvsa.manifest ptvsa.omk
