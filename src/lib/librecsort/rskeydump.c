/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1996-2003 AT&T Corp.                *
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
*                 Phong Vo <kpv@research.att.com>                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * rskey dump
 */

#include "rskeyhdr.h"

static void
#if __STD_C
dump(register Rskey_t* kp, Sfio_t* sp, register Field_t* fp, const char* type)
#else
dump(kp, sp, fp, type)
register Rskey_t*	kp;
Sfio_t*			sp;
register Field_t*	fp;
char*			type;
#endif
{
	sfprintf(sp, "%s[%d]\n", type, fp->index);
	sfprintf(sp, "\tbegin field = %d\n", fp->begin.field);
	sfprintf(sp, "\t begin char = %d\n", fp->begin.index);
	sfprintf(sp, "\t  end field = %d\n", fp->end.field);
	sfprintf(sp, "\t   end char = %d\n", fp->end.index);
	sfprintf(sp, "\t      ccode = %d\n", fp->code);
	sfprintf(sp, "\t      coder = %c\n", fp->flag ? fp->flag : '?');
	sfprintf(sp, "\t       keep = %s\n", fp->keep == kp->state->all ? "all" : fp->keep == kp->state->print ? "print" : fp->keep == kp->state->dict ? "dict" : fp->keep ? "UNKNOWN" : "all");
	sfprintf(sp, "\t      trans = %s\n", fp->trans == kp->state->ident ? "ident" : fp->trans == kp->state->fold ? "fold" : fp->trans ? "UNKNOWN" : "ident");
	sfprintf(sp, "\t      bflag = %d\n", fp->bflag);
	sfprintf(sp, "\t      eflag = %d\n", fp->eflag);
	sfprintf(sp, "\t      rflag = %d\n", fp->rflag);
	sfprintf(sp, "\t      style = %s\n", fp->standard ? "standard" : "obsolete");
}

/*
 * dump the sort options and fields
 */

void
#if __STD_C
rskeydump(register Rskey_t* kp, register Sfio_t* sp)
#else
rskeydump(kp, sp)
register Rskey_t*	kp;
register Sfio_t*	sp;
#endif
{
	register Field_t*	fp;

	sfprintf(sp, "state\n");
	sfprintf(sp, "\tmethod=%s\n", kp->meth->name);
	sfprintf(sp, "\tinsize=%ld outsize=%ld\n", kp->insize, kp->outsize);
	sfprintf(sp, "\talignsize=%ld procsize=%ld recsize=%ld\n", kp->alignsize, kp->procsize, kp->recsize);
	sfprintf(sp, "\tmerge=%d reverse=%d stable=%d uniq=%d verbose=%d\n", kp->merge, !!(kp->type & RS_REVERSE), !(kp->type & RS_DATA), !!(kp->type & RS_UNIQ), kp->verbose);
	sfprintf(sp, "\ttab='%c' keys=%s maxfield=%d", kp->tab ? kp->tab : ' ', kp->coded ? "coded" : "", kp->field.maxfield);
	if (kp->fixed)
		sfprintf(sp, " fixed=%d", kp->fixed);
	sfprintf(sp, "\n");
	sfprintf(sp, "\trecsort data%s %d key%s %d\n", (kp->disc->type & RS_DSAMELEN) ? " DSAMELEN" : "", kp->disc->data, (kp->disc->type & RS_KSAMELEN) ? " KSAMELEN" : "", kp->disc->keylen);
	for (fp = &kp->field.global; fp; fp = fp->next)
		dump(kp, sp, fp, "field");
	for (fp = kp->accumulate.head; fp; fp = fp->next)
		dump(kp, sp, fp, "accumulate");
}
