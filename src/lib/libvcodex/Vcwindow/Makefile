#	Makefile for the windowing matching methods
#	Written by Kiem-Phong Vo

INCDIR=	../../../../include
LIBDIR=	../../../../lib

CXINCS=
CCINCS= -I. -I.. -I$(INCDIR) $(CXINCS)
CXFLAGS=
CCMODE= -O
CCFLAGS= $(CCMODE) $(CXFLAGS) $(CCINCS)
CC=cc

VCWHDRS=vcwhdr.h
VCWSRCS=vcwopen.c vcwclose.c vcwfile.c vcwgetmeth.c vcwngram.c \
	vcwmirror.c vcwvote.c vcwdecode.c vcwprefix.c
VCWOBJS=vcwopen.o vcwclose.o vcwfile.o vcwgetmeth.o vcwngram.o \
	vcwmirror.o vcwvote.o vcwdecode.o vcwprefix.o

.c.o :
	$(CC) -c $(CCFLAGS) $*.c

install_me:	$(VCWOBJS)
install:	install_me

# 3d update
update:
	cp Makefile $(VCWHDRS) $(VCWSRCS) ...

clean :
	-(rm $(VCWOBJS); exit 0) >/dev/null 2>&1
