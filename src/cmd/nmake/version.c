/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1984-2003 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*    If you have copied or used this software without agreeing     *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * make version and initialization script
 *
 * NOTE: include this file with binary distributions
 */

#ifndef STAMP
#define STAMP		"\n@(#)$Id: make (AT&T Research) BOOTLEG $\0\n"
#endif

#ifndef IDNAME
#define IDNAME		"make"
#endif

static char		id[] = STAMP;

char*			idname = IDNAME;

char*			version = id;

char*			initstatic =
"\
MAKEARGS = Nmakeargs:nmakeargs:Makeargs:makeargs\n\
MAKEFILES = Nmakefile:nmakefile:Makefile:makefile\n\
MAKEPP = $(MAKERULESPATH:/:/ /G:D:X=cpp:P=X:O=1)\n\
MAKEPPFLAGS = -I- $(PPFLAGS) -D:'map \"/#<(comment|rules)>/\"' -D-P\n\
MAKERULES = makerules\n\
MAKERULESPATH = $(LOCALRULESPATH):$(MAKELOCALPATH):$(PATH:/:/ /G:D:B=lib/make:@/ /:/G):$(MAKELIB):$(INSTALLROOT|HOME)/lib/make:/usr/local/lib/make:/usr/lib/make\n\
OLDMAKE = /bin/make\n\
PPFLAGS = $(*.SOURCE.mk:/^/-I/) $(-:N=-[DU]*)\n\
- : .MAKE .NULL .VIRTUAL .FORCE .REPEAT .FOREGROUND .IGNORE .MULTIPLE\n\
";


char*			initdynamic =
"\
.SOURCE.mk : . $(-file:/:/ /:D) $(-:N=-I[!-]*:/-I//) $(MAKERULESPATH:/:/ /G:T=F)\n\
VROOT := $(\".\":T=F:P=L*)\n\
if ( VOFFSET = \"$(VROOT:O=2)\" ) && ! \"$(VPATH)\"\n\
	MAKEPATH := $(VPATH:/:.*//:T=F)\n\
	VROOT := $(VROOT:O=1)\n\
	while \"$(VROOT)\" != \"|.|$(MAKEPATH)\" && \"$(VROOT:B:S)\" == \"$(VOFFSET:B:S)\"\n\
		VROOT := $(VROOT:D)\n\
		VOFFSET := $(VOFFSET:D)\n\
	end\n\
	VOFFSET := $(VROOT:P=R)\n\
	VROOT := $(PWD:P=R=$(VROOT))\n\
else\n\
	VOFFSET := $(VPATH:/:/ /G:O=1:P=R)\n\
	VROOT := $(PWD:P=R=$(VPATH:/:/ /G:O=1))\n\
end\n\
MAKEPATH := $(*.VIEW:N!=.:@/ /:/G)\n\
";
