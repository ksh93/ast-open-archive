/*
 * NOTE: this provides ast enhanced versions of INIT executables
 */

:PACKAGE: ast

LICENSE = since=1990,author=gsf

.SOURCE.c : ../INIT ../../lib/libpp

proto :: RELEASE PROMO.mm proto.1 proto.c

mamake :: mamake.c LICENSE=since=1999,author=gsf

ratz :: ratz.c

release :: release.c LICENSE=since=1999,author=gsf

sear :: sear.sh LICENSE=since=2001,author=dgk+gsf

$(LIBDIR)/sear :INSTALLDIR: ratz.c sear.ico

:TEST: proto
