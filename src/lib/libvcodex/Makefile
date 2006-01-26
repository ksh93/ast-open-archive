#########################################################################
#	Libvcodex.a makefile						#
#									#
#	Written by Kiem-Phong Vo					#
#########################################################################

# The below parameters should be edited as appropriate per installation.
# VCSFIO=1 if Sfio is used or 0 if Stdio is desired.
#	In the latter case, -lsfio should be taken off "libs".
# VCTABLE=1 if table transforms are included or 0 if excluded.
# 	In the latter case,  vctable should be taken off "targets".
VCSFIO=0
libs=-lcdt #-lsfio
VCTABLE=1
targets=vcdelta vchuff vcmisc vctable

INCDIR=	../../../include
LIBDIR=	../../../lib
BINDIR=	../../../bin
MANDIR= ../../../man

CC=cc
CXINCS=
CCINCS=	-I. -I$(INCDIR) $(CXINCS)
CXFLAGS= -DVCSFIO=$(VCSFIO) -DVCTABLE=$(VCTABLE)
CCMODE= -O
CCFLAGS= $(CCMODE) $(CXFLAGS) $(CCINCS)

#### Common Vcodex functions ############################################
VCSRCS=	vcopen.c vcclose.c vcio.c vcdisc.c vcextract.c \
	vccontext.c vclog.c vcsfxsort.c vcperiod.c vclzparse.c \
	vcsetbuf.c vcbcktsort.c vcgetmeth.c vcsfio.c
VCOBJS=	vcopen.o vcclose.o vcio.o vcdisc.o vcextract.o \
	vccontext.o vclog.o vcsfxsort.o vcperiod.o vclzparse.o \
	vcsetbuf.o vcbcktsort.o vcgetmeth.o vcsfio.o
VCHDRS=	ast_common.h vcodex.h

#### Delta compression ##################################################
VCDOBJS=Vcdelta/vcdelta.o Vcdelta/vcdbmove.o \
	Vcdelta/vcdcache.o Vcdelta/vcdtable.o Vcdelta/vcdtblinit.o \
	Vcwindow/vcwopen.o Vcwindow/vcwclose.o Vcwindow/vcwfile.o \
	Vcwindow/vcwgetmeth.o Vcwindow/vcwngram.o Vcwindow/vcwmirror.o \
	Vcwindow/vcwvote.o Vcwindow/vcwdecode.o Vcwindow/vcwprefix.o

#### Huffman coding methods #############################################
VCHOBJS=Vchuff/vchsize.o Vchuff/vchbits.o \
	Vchuff/vchtrie.o Vchuff/vchcode.o \
	Vchuff/vchuffman.o Vchuff/vchuffpart.o Vchuff/vchuffgroup.o

#### Miscellaneous methods ##############################################
VCMOBJS=Vcmisc/vcmap.o Vcmisc/vcrle.o Vcmisc/vcmtf.o Vcmisc/vcbwt.o

#### Table transform methods ############################################
VCTOBJS=Vctable/vctranspose.o Vctable/vcplan.o Vctable/vctable.o \
	VcATT/vcama.o VcATT/vcss7.o

OBJS=	$(VCOBJS) $(VCDOBJS) $(VCHOBJS) $(VCMOBJS)
HDRS=	$(VCHDRS)

.c.o :
	$(CC) -c $(CCFLAGS) $*.c

all:	install

libvcodex.a : ast_common.h FEATURE/vcodex $(VCOBJS) $(targets)
	-(if test "$(VCTABLE)" = 1; then echo $(VCTOBJS) >tblobjs; else echo >tblobjs; fi)
	ar cr libvcodex.a $(OBJS) `cat tblobjs`;
	-(ranlib libvcodex.a; rm tblobjs; exit 0) >/dev/null 2>&1

ast_common.h: features/common
	$(BINDIR)/iffe - set cc $(CC) $(CCMODE) $(CXFLAGS) : \
		run features/common >ast_common.h
FEATURE/vcodex: features/vcodex
	$(BINDIR)/iffe set cc $(CC) $(CCFLAGS) : run features/vcodex

vcdelta: vcwindow
	cd Vcdelta; \
	$(MAKE) SHELL="$(SHELL)" CC="$(CC)" CCMODE="$(CCMODE)" \
		CXFLAGS="$(CXFLAGS)" CXINCS="$(CXINCS)"
vcwindow:
	cd Vcwindow; \
	$(MAKE) SHELL="$(SHELL)" CC="$(CC)" CCMODE="$(CCMODE)" \
		CXFLAGS="$(CXFLAGS)" CXINCS="$(CXINCS)"

vchuff:
	cd Vchuff; \
	$(MAKE) SHELL="$(SHELL)" CC="$(CC)" CCMODE="$(CCMODE)" \
		CXFLAGS="$(CXFLAGS)" CXINCS="$(CXINCS)"

vcmisc:
	cd Vcmisc; \
	$(MAKE) SHELL="$(SHELL)" CC="$(CC)" CCMODE="$(CCMODE)" \
		CXFLAGS="$(CXFLAGS)" CXINCS="$(CXINCS)"

vctable: vcatt
	cd Vctable; \
	$(MAKE) SHELL="$(SHELL)" CC="$(CC)" CCMODE="$(CCMODE)" \
		CXFLAGS="$(CXFLAGS)" CXINCS="$(CXINCS)"; 
vcatt:
	cd VcATT; \
	$(MAKE) SHELL="$(SHELL)" CC="$(CC)" CCMODE="$(CCMODE)" \
		CXFLAGS="$(CXFLAGS)" CXINCS="$(CXINCS)";

vczip:
	cd Vczip; \
	$(MAKE) SHELL="$(SHELL)" CC="$(CC)" CCMODE="$(CCMODE)" \
		CXFLAGS="$(CXFLAGS)" CXINCS="$(CXINCS)" libs="$(libs)" \
		vczip; \
	cd ..; \
	(rm $(BINDIR)/vczip $(BINDIR)/vcunzip; exit 0) >/dev/null 2>&1; \
	cp Vczip/vczip $(BINDIR); ln $(BINDIR)/vczip $(BINDIR)/vcunzip;

install_me:	install
install:	install_lib vczip
install_lib:	libvcodex.a
	cp vcodex.3 $(MANDIR)/man3; \
	cp $(HDRS) $(INCDIR); \
	cp libvcodex.a $(LIBDIR); \
	(ranlib $(LIBDIR)/libvcodex.a; exit 0) >/dev/null 2>&1; \

clean :
	-(rm -rf $(OBJS) ast_common.h FEATURE libvcodex.a; exit 0) >/dev/null 2>&1
	-(rm Vczip/vczip Vczip/vcunzip; exit 0) >/dev/null 2>&1
