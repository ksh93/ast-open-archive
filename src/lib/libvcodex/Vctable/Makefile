#	Makefile for the table compression methods
#	Written by Binh Dao Vo and Kiem-Phong Vo

INCDIR=	../../../../include
LIBDIR=	../../../../lib

CXINCS=
CCINCS= -I. -I.. -I$(INCDIR) $(CXINCS)
CXFLAGS=
CCMODE= -O
CCFLAGS= $(CCMODE) $(CXFLAGS) $(CCINCS)
CC=cc

VCTSRCS=	vctranspose.c vcplan.c vctable.c
VCTOBJS=	vctranspose.o vcplan.o vctable.o

.c.o :
	$(CC) -c $(CCFLAGS) $*.c

install:	$(VCTOBJS)
clean :
	-(rm $(VCTOBJS); exit 0) >/dev/null 2>&1
