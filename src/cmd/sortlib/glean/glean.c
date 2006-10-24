/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*              Copyright (c) 2006 AT&T Knowledge Ventures              *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                      by AT&T Knowledge Ventures                      *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * sort uniq summary discipline
 */

static const char usage[] =
"[-1lp0?\n@(#)$Id: sortglean (AT&T Research) 2006-06-29 $\n]"
USAGE_LICENSE
"[+NAME?sortglean - glean minima and/or maxima from record stream]"
"[+DESCRIPTION?The \bsortglean\b \bsort\b(1) discipline gleans minima "
    "and maxima from a record stream. Each record is categorized by the main "
    "sort key. If there is no main sort key then all records are in one "
    "category. If the \b--min\b key sorts less than the current category "
    "minimum or if the \b--max\b key sorts greater than the category maximum "
    "then the category minimum or maximum is updated and the record is "
    "written to the standard output.]"
"[+?Note that \b,\b is the plugin option separator. Literal \b,\b must "
    "either be quoted or escaped \aafter\a normal shell expansion.]"
"[c:count?Precede each output record with its category ordinal.]"
"[f:flush?Flush each line written to the standard output.]"
"[m:min?Mimima \bsort\b(1) key specification.]:[sort-key]"
"[M:max?Maxima \bsort\b(1) key specification.]:[sort-key]"
"[+EXAMPLES]"
    "{"
        "[+sort -t, -k1,1 -k2,2n -lglean,flush,min='\"3,3n\"',min='\"4,4\"' old.dat -?Sorts "
            "on the \b,\b separated fields 1 (string) and 2 (numeric) and "
            "lists the current minimal records per field 3 (numeric) and "
            "field 4 (string), for each value of the catenation of fields 1 and 2.]"
    "}"
"[+SEE ALSO?\bsort\b(1)]"
;

#include <ast.h>
#include <debug.h>
#include <dt.h>
#include <error.h>
#include <recsort.h>
#include <vmalloc.h>

typedef struct Data_s
{
	void*		data;
	size_t		size;
	size_t		len;
} Data_t;

typedef struct Category_s
{
	Dtlink_t	link;
	Sfulong_t	count;
	Data_t		key;
	Data_t		min;
	Data_t		max;
} Category_t;

typedef struct State_s
{
	Rsdisc_t	rsdisc;
	Dtdisc_t	dtdisc;
	Rskeydisc_t	kydisc;
	Dt_t*		categories;
	Rskey_t*	min;
	Rskey_t*	max;
	Data_t		key;
	Vmalloc_t*	vm;
	int		count;
	Sfulong_t	total;
} State_t;

static int
save(Vmalloc_t* vm, register Data_t* p, void* data, size_t len)
{
	if (p->size < len)
	{
		p->size = roundof(len, 256);
		if (!(p->data = vmnewof(vm, p->data, char, p->size, 0)))
		{
			error(ERROR_SYSTEM|2, "out of space [save]");
			return -1;
		}
	}
	p->len = len;
	if (data)
		memcpy(p->data, data, len);
	return 0;
}

static int
gleancmp(Dt_t* dt, void* a, void* b, Dtdisc_t* disc)
{
	Category_t*	x = (Category_t*)a;
	Category_t*	y = (Category_t*)b;

	message((-4, "gleancmp a:%d:%-*.*s: b:%d:%-*.*s:", x->key.len, x->key.len, x->key.len, x->key.data, y->key.len, y->key.len, y->key.len, y->key.data));
	if (x->key.len < y->key.len)
		return -1;
	if (x->key.len > y->key.len)
		return 1;
	return memcmp(x->key.data, y->key.data, x->key.len);
}

static int
glean(Rs_t* rs, int op, Void_t* data, Void_t* arg, Rsdisc_t* disc)
{
	State_t*		state = (State_t*)disc;
	register Rsobj_t*	r;
	register Category_t*	p;
	Category_t		x;
	ssize_t			k;
	int			n;
	Data_t			t;

	switch (op)
	{
	case RS_POP:
		vmclose(state->vm);
		return 0;
	case RS_READ:
		r = (Rsobj_t*)data;
		x.key.data = r->key;
		x.key.len = r->keylen;
		if (!(p = (Category_t*)dtsearch(state->categories, &x)))
		{
			if (!(p = vmnewof(state->vm, 0, Category_t, 1, r->keylen)))
			{
				error(ERROR_SYSTEM|2, "out of space [category]");
				return -1;
			}
			p->key.len = r->keylen;
			p->key.data = (void*)(p + 1);
			memcpy(p->key.data, r->key, r->keylen);
			dtinsert(state->categories, p);
		}
		state->total++;
		p->count++;
		message((-2, "glean record p=%p %I*u/%I*u key='%-*.*s' r:%d:%-*.*s: '%-*.*s'", p, sizeof(p->count), p->count, sizeof(state->total), state->total, r->keylen, r->keylen, r->key, x.key.len, x.key.len, x.key.len, x.key.data, r->datalen, r->datalen, r->data));
		n = 0;
		if (state->min)
		{
			if (state->min->disc->defkeyf)
			{
				if ((k = state->min->disc->keylen) <= 0)
					k = 4;
				k *= r->datalen;
				if (save(state->vm, &state->key, 0, k))
					return -1;
				if ((k = (*state->min->disc->defkeyf)(rs, r->data, r->datalen, state->key.data, state->key.size, state->min->disc)) < 0)
					return -1;
				t.len = state->key.len = k;
				t.data = state->key.data;
			}
			else
			{
				t.data = r->data + state->min->disc->key;
				if ((k = state->min->disc->keylen) <= 0)
					k += r->datalen - state->min->disc->key;
				t.len = k;
			}
			message((-1, "glean min a:%d:%-*.*s:", t.len, t.len, t.len, t.data));
			if (!p->min.data)
				n = -1;
			else
			{
				message((-1, "glean min b:%d:%-*.*s:", p->min.len, p->min.len, p->min.len, p->min.data));
				if ((k = t.len) < p->min.len)
					k = p->min.len;
				if (!(n = memcmp(t.data, p->min.data, k)))
					n = t.len - p->min.len;
			}
			if (n >= 0)
				n = 0;
			else if (state->min->disc->defkeyf)
			{
				t = state->key;
				state->key = p->min;
				p->min = t;
			}
			else if (save(state->vm, &p->min, t.data, t.len))
				return -1;
		}
		if (state->max && (!n || !p->max.data))
		{
			if (state->max->disc->defkeyf)
			{
				if ((k = state->max->disc->keylen) <= 0)
					k = 4;
				k *= r->datalen;
				if (save(state->vm, &state->key, 0, k))
					return -1;
				if ((k = (*state->max->disc->defkeyf)(rs, r->data, r->datalen, state->key.data, state->key.size, state->max->disc)) < 0)
					return -1;
				t.len = state->key.len = k;
				t.data = state->key.data;
			}
			else
			{
				t.data = r->data + state->max->disc->key;
				if ((k = state->max->disc->keylen) <= 0)
					k += r->datalen - state->max->disc->key;
				t.len = k;
			}
			message((-1, "glean max a:%d:%-*.*s:", t.len, t.len, t.len, t.data));
			if (!p->max.data)
				n = 1;
			else
			{
				message((-1, "glean max b:%d:%-*.*s:", p->max.len, p->max.len, p->max.len, p->max.data));
				if ((k = t.len) < p->max.len)
					k = p->max.len;
				if (!(n = memcmp(t.data, p->max.data, k)))
					n = t.len - p->max.len;
			}
			if (n <= 0)
				n = 0;
			else if (state->max->disc->defkeyf)
			{
				t = state->key;
				state->key = p->max;
				p->max = t;
			}
			else if (save(state->vm, &p->max, t.data, t.len))
				return -1;
		}
		if (n)
		{
			if (state->count)
				sfprintf(sfstdout, "%I*u/%I*u ", sizeof(p->count), p->count, sizeof(state->total), state->total);
			sfwrite(sfstdout, r->data, r->datalen);
		}
		return RS_DELETE;
	}
	return -1;
}

Rsdisc_t*
rs_disc(Rskey_t* key, const char* options)
{
	register State_t*	state;
	Vmalloc_t*		vm;

	if (!(vm = vmopen(Vmdcheap, Vmbest, 0)) || !(state = vmnewof(vm, 0, State_t, 1, 0)))
		error(ERROR_SYSTEM|3, "out of space [state]");
	state->vm = vm;
	key->type &= ~RS_DATA;
	state->dtdisc.link = offsetof(Category_t, link);
	state->dtdisc.comparf = gleancmp;
	state->kydisc.version = RSKEY_VERSION;
	state->kydisc.errorf = errorf;
	state->rsdisc.eventf = glean;
	state->rsdisc.events = RS_READ|RS_POP;
	if (!(state->categories = dtnew(vm, &state->dtdisc, Dttree)))
		error(ERROR_SYSTEM|3, "out of space [dictionary]");
	if (options)
	{
		for (;;)
		{
			switch (optstr(options, usage))
			{
			case 0:
				break;
			case 'c':
				state->count = opt_info.num;
				continue;
			case 'f':
				sfset(sfstdout, SF_LINE, 1);
				continue;
			case 'm':
				if (!state->min && !(state->min = rskeyopen(&state->kydisc)))
					error(ERROR_SYSTEM|3, "out of space [min]");
				state->min->tab = key->tab;
				state->min->type = key->type;
				if (rskey(state->min, opt_info.arg, 0))
					goto drop;
				continue;
			case 'M':
				if (!state->max && !(state->max = rskeyopen(&state->kydisc)))
					error(ERROR_SYSTEM|3, "out of space [max]");
				state->max->tab = key->tab;
				state->max->type = key->type;
				if (rskey(state->max, opt_info.arg, 0))
					goto drop;
				continue;
			case '?':
				error(ERROR_USAGE|4, "%s", opt_info.arg);
				goto drop;
			case ':':
				error(2, "%s", opt_info.arg);
				goto drop;
			}
			break;
		}
		if (state->min && rskeyinit(state->min) || state->max && rskeyinit(state->max))
			goto drop;
	}
	return &state->rsdisc;
 drop:
	if (state->min)
		rskeyclose(state->min);
	if (state->max)
		rskeyclose(state->max);
	vmclose(vm);
	return 0;
}