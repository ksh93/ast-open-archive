/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1998-2000 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped

/*
 * Glenn Fowler
 * AT&T Labs Research
 */

static const char usage[] =
"[-1l?\n@(#)pzip fixed line verification (AT&T Labs Research) 1999-08-26\n]"
USAGE_LICENSE
"[+LIBRARY?-lpzline - pzip fixed line verification discipline library]"
"[+DESCRIPTION?\b-lpzline\b is a \bpzip\b(1) verification discipline"
"	library for newline terminated fixed length record data. This"
"	discipline checks records as they are compressed. Short records"
"	are dropped and long records are truncated from the front.]"
"[l:log?Dropped or truncated record data is written to \afile\a.s in the form:"
"	\aoffset\a \asize\a \adata\a\\n. If \adata\a is ASCII then the log"
"	file will also be ASCII. If \b--log\b is not specified or"
"	\b--verbose\b is specified then a message with the invalid record"
"	offset and size is written to the standard error.]:[file]"
;

#include <pzip.h>
#include <error.h>

typedef struct
{
	char*		logfile;
	Sfio_t*		log;
} State_t;

#define LOG(p,s,f,b,n,m,c,d)	(((d)->errorf||(s)->log)?logmsg(p,s,f,b,n,m,c,d):0)

/*
 * log invalid data
 */

static int
logmsg(Pz_t* pz, State_t* state, Sfio_t* sp, unsigned char* data, ssize_t n, const char* msg, Sfulong_t* count, Pzdisc_t* disc)
{
	Sfoff_t		off;

	if (disc->errorf || state->log)
	{
		(*count)++;
		off = sftell(sp);
		if ((!state->log || (pz->flags & PZ_VERBOSE)) && disc->errorf)
			(*disc->errorf)(pz, disc, 2, "%s: %s at %I*d (%I*d/%I*d) -- %s", pz->path, msg, sizeof(off), off, sizeof(n), n, sizeof(pz->part->row), pz->part->row, count == &pz->count.dropped ? "dropped" : "truncated");
		if (state->log && (data || (data = (unsigned char*)sfgetr(sp, -1, 0))))
		{
			sfprintf(state->log, "%I*u %I*d ", sizeof(off), off, sizeof(n), n);
			sfwrite(state->log, data, n);
			sfputc(state->log, '\n');
		}
	}
	return 0;
}

/*
 * read and verify the next record
 */

static ssize_t
lineread(Pz_t* pz, Sfio_t* sp, void* data, Pzdisc_t* disc)
{
	register State_t*	state = (State_t*)disc->local;
	register size_t		row = pz->part->row;
	register unsigned char*	s;
	register ssize_t	n;

	for (;;)
	{
		s = (unsigned char*)sfgetr(sp, '\n', 0);
		n = sfvalue(sp);
		if (!s)
		{
			if (n)
			{
				LOG(pz, state, sp, s, n, "last record incomplete", &pz->count.dropped, disc);
				return -1;
			}
			return 0;
		}
		if (n < row)
			LOG(pz, state, sp, s, n, "short record", &pz->count.dropped, disc);
		else
		{
			if (n > row)
			{
				n -= row;
				LOG(pz, state, sp, s, n, "long record", &pz->count.truncated, disc);
				s += n;
			}
			break;
		}
	}
	memcpy(data, s, row);
	return row;
}

/*
 * handle pzip events
 */

static int
lineevent(Pz_t* pz, int op, void* data, size_t size, Pzdisc_t* disc)
{
	register State_t*	state = (State_t*)disc->local;
	int			r;

	switch (op)
	{
	case PZ_CLOSE:
		if (state->log)
		{
			r = sfclose(state->log);
			state->log = 0;
			if (r)
			{
				if (disc->errorf)
					(*disc->errorf)(pz, disc, ERROR_SYSTEM|2, "%s: write error", state->logfile);
				return -1;
			}
		}
		break;
	case PZ_OPTION:
		switch (optstr(NiL, usage))
		{
		case 'l':
			if (!(state->logfile = vmstrdup(pz->vm, opt_info.arg)))
			{
				if (disc->errorf)
					(*disc->errorf)(pz, disc, ERROR_SYSTEM|2, "%s: out of space", data);
				return -1;
			}
			if (!(state->log = sfopen(NiL, state->logfile, "w")))
			{
				if (disc->errorf)
					(*disc->errorf)(pz, disc, ERROR_SYSTEM|2, "%s: cannot create", state->logfile);
				return -1;
			}
			return 1;
		}
		break;
	}
	return 0;
}

/*
 * install the pzip verification discipline
 */

#ifdef __EXPORT__
#define extern	__EXPORT__
#endif

extern const char*
pz_init(Pz_t* pz, Pzdisc_t* disc)
{
	register State_t*	state;

	if (pz->flags & PZ_WRITE)
	{
		if (!(state = vmnewof(pz->vm, 0, State_t, 1, 0)))
			return 0;
		disc->local = (void*)state;
		disc->readf = lineread;
		disc->eventf = lineevent;
	}
	return usage;
}
