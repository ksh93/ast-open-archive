:PACKAGE: ast

LICENSE = since=1989,author=gsf

$(LIBDIR)/probe :INSTALLDIR: mode=u+s,+x probe

probe :: RELEASE probe.c +last

$(BINDIR)/probe :INSTALL:
	test -f $(<) || $(LN) -s $(BINDIR:P=R=$(LIBDIR))/probe/probe $(<) || $(LN) $(LIBDIR)/probe/probe $(<) || :
