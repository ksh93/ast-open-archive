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
