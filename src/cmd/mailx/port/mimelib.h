/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*Copyright (c) 1978-2004 The Regents of the University of Californi*
*                                                                  *
*          Permission is hereby granted, free of charge,           *
*       to any person obtaining a copy of THIS SOFTWARE FILE       *
*            (the "Software"), to deal in the Software             *
*              without restriction, including without              *
*           limitation the rights to use, copy, modify,            *
*                merge, publish, distribute, and/or                *
*            sell copies of the Software, and to permit            *
*            persons to whom the Software is furnished             *
*          to do so, subject to the following disclaimer:          *
*                                                                  *
*THIS SOFTWARE IS PROVIDED BY The Regents of the University of Cali*
*         ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,         *
*            INCLUDING, BUT NOT LIMITED TO, THE IMPLIED            *
*            WARRANTIES OF MERCHANTABILITY AND FITNESS             *
*             FOR A PARTICULAR PURPOSE ARE DISCLAIMED.             *
*IN NO EVENT SHALL The Regents of the University of California and *
*         BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,          *
*           SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES           *
*           (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT            *
*          OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,           *
*           DATA, OR PROFITS; OR BUSINESS INTERRUPTION)            *
*          HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,          *
*          WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
*           (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING            *
*           IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,            *
*        EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.        *
*                                                                  *
*            Information and Software Systems Research             *
*The Regents of the University of California and AT&T Labs Research*
*                         Florham Park NJ                          *
*                                                                  *
*                        Kurt Shoens (UCB)                         *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * mime/mailcap internal interface
 */

#ifndef _MIMELIB_H
#define _MIMELIB_H	1

#include <ast.h>
#include <cdt.h>
#include <magic.h>
#include <sfstr.h>
#include <tok.h>

struct Mime_s;

typedef void (*Free_f)(struct Mime_s*);

#define _MIME_PRIVATE_ \
	Dtdisc_t	dict;		/* cdt discipline		*/ \
	Magicdisc_t	magicd;		/* magic discipline		*/ \
	Mimedisc_t*	disc;		/* mime discipline		*/ \
	Dt_t*		cap;		/* capability tree		*/ \
	Sfio_t*		buf;		/* string buffer		*/ \
	Magic_t*	magic;		/* mimetype() magic handle	*/ \
	Free_f		freef;		/* avoid magic lib if possible	*/ \

#include <mime.h>
#include <ctype.h>

#endif
