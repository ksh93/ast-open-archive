/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2002-2009 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
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

static const char stats_usage[] =
"[-1?\n@(#)$Id: dss stats query (AT&T Research) 2003-05-05 $\n]"
USAGE_LICENSE
"[+LIBRARY?\findex\f]"
"[+DESCRIPTION?The stats query lists the sum, average, unbiased standard"
"	deviation, and minimum and maximum range values of the numeric"
"	\afield\a operands. If no operands are specified then all numeric"
"	fields are assumed. If \b-\b is specified then only records or"
"	groups are counted. If any of \b--average\b, \b--count\b,"
"	\b--deviation\b, \b--range\b or \b--sum\b are specified then only"
"	those values are listed.]"
"[a:average?List the average.]"
"[c:count?List the record count.]"
"[d:deviation?List the unbiased standard deviation.]"
"[g:group?Group by values in \afield\a.]:[field]"
"[l:label?Label the output with \alabel\a.]:[label]"
"[m:maxgroups?Maximum number of groupings. Values beyond \amax\a are"
"	listed as \bOVERFLOW\b.]#[max:=100]"
"[p:print?Print summary data according to \aformat\a. The format fields"
"	are:]:[format]{\ffields\f}"
"[r:range?List the minimum and maximum values.]"
"[s:sum?List the sum.]"
"\n"
"\n[ field ... ]\n"
"\n"
;

#include <dsslib.h>
#include <ast_float.h>

#define FW		15

#define STATS_AVERAGE	0x0001
#define STATS_COUNT	0x0002
#define STATS_DEVIATION	0x0004
#define STATS_FIELD	0x0008
#define STATS_MAX	0x0010
#define STATS_MIN	0x0020
#define STATS_SUM	0x0040

#define STATS_RANGE	(STATS_MAX|STATS_MIN)

struct Bucket_s; typedef struct Bucket_s Bucket_t;
struct Field_s; typedef struct Field_s Field_t;
struct Group_s; typedef struct Group_s Group_t;
struct Print_s; typedef struct Print_s Print_t;
struct State_s; typedef struct State_s State_t;
struct Total_s; typedef struct Total_s Total_t;

struct Total_s
{
	Cxnumber_t	value;
	Cxnumber_t	square;
	Cxnumber_t	min;
	Cxnumber_t	max;
	Cxunsigned_t	count;
};

struct Bucket_s
{
	Dtlink_t	link;
	Cxvalue_t*	key;
	Total_t		total[1];
};

struct Group_s
{
	Group_t*	next;
	Cxvariable_t*	variable;
	int		string;
	int		width;
};

struct Field_s
{
	Field_t*	next;
	Cxvariable_t*	variable;
};

struct Print_s
{
	Field_t*	field;
	Group_t*	group;
	Total_t*	total;
	Cxvalue_t*	key;
};

struct State_s
{
	Dtdisc_t	bucketdisc;
	Field_t*	field;
	char*		format;
	Cx_t*		print;
	Group_t*	group;
	char*		label;
	Cxvalue_t*	key;
	Dt_t*		buckets;
	Total_t*	total;
	Vmalloc_t*	vm;
	int		fields;
	int		groups;
	int		maxgroups;
	int		op;
};

static Cxvariable_t variables[] =
{
CXV("AVERAGE",  "number", STATS_AVERAGE,   "Average value.")
CXV("COUNT",    "number", STATS_COUNT,     "Number of values.")
CXV("DEVIATION","number", STATS_DEVIATION, "Unbiased standard deviation.")
CXV("FIELD",    "string", STATS_FIELD,     "Field name.")
CXV("MAX",      "number", STATS_MAX,       "Maximum value.")
CXV("MIN",      "number", STATS_MIN,       "Minimum value.")
CXV("SUM",      "number", STATS_SUM,       "Sum of values.")
};

static int
getop(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	register State_t*	state = (State_t*)((Dssrecord_t*)data)->data;
	Cxvariable_t*		variable = pc->data.variable;
	Group_t*		group;

	if (variable->data == (void*)&variables[0])
		state->op |= variable->index;
	else
	{
		group = state->group;
		for (;;)
		{
			if (!group)
			{
				if (disc->errorf)
					(*disc->errorf)(cx, disc, 2, "%s: unknown print variable", variable->name);
				return -1;
			}
			if (group->variable == variable)
				break;
			group = group->next;
		}
	}
	r->value.string.data = "";
	r->value.string.size = 0;
	return 0;
}

static int
getvalue(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	register Print_t*	print = (Print_t*)((Dssrecord_t*)data)->data;
	Cxvariable_t*		variable = pc->data.variable;
	Group_t*		group;
	Cxvalue_t*		key;
	Cxnumber_t		u;

	if (variable->data == (void*)&variables[0])
		switch (variable->index)
		{
		case STATS_AVERAGE:
			r->value.number = print->total->count ? print->total->value / (Cxinteger_t)print->total->count : 0;
			break;
		case STATS_COUNT:
			r->value.number = (Cxinteger_t)print->total->count;
			break;
		case STATS_DEVIATION:
			if (print->total->count)
			{
				u = print->total->value / (Cxinteger_t)print->total->count;
				if ((u = print->total->square + u * (u - 2 * print->total->value)) < 0)
					u = -u;
				if (print->total->count > 1)
					u /= (Cxinteger_t)(print->total->count - 1);
				r->value.number = sqrt(u);
			}
			else
				r->value.number = 0;
			break;
		case STATS_FIELD:
			if (!print->field || !(r->value.string.data = (char*)print->field->variable->name))
				r->value.string.data = "-";
			r->value.string.size = strlen(r->value.string.data);
			break;
		case STATS_MAX:
			r->value.number = print->total->max;
			break;
		case STATS_MIN:
			r->value.number = print->total->min;
			break;
		case STATS_SUM:
			r->value.number = print->total->value;
			break;
		}
	else if (key = print->key)
	{
		for (group = print->group, key = print->key; group->variable != variable; group = group->next, key++);
		r->value = *key;
	}
	else
		memset(&r->value, 0, sizeof(r->value));
	return 0;
}

static int
bucketcmp(Dt_t* dt, void* a, void* b, Dtdisc_t* disc)
{
	register State_t*	state = (State_t*)disc;
	register Cxvalue_t*	ka = (Cxvalue_t*)a;
	register Cxvalue_t*	kb = (Cxvalue_t*)b;
	register Group_t*	group;
	register int		n;

	for (group = state->group; group; group = group->next, ka++, kb++)
	{
		if (group->string)
		{
			n = ka->string.size < kb->string.size ? ka->string.size : kb->string.size;
			if (n = memcmp(ka->string.data, kb->string.data, n))
				return n;
			if (ka->string.size < kb->string.size)
				return -1;
			if (ka->string.size > kb->string.size)
				return 1;
		}
		else if (ka->number < kb->number)
			return -1;
		else if (ka->number > kb->number)
			return 1;
	}
	return 0;
}

static int
stats_beg(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	char**		argv = (char**)data;
	int		errors = error_info.errors;
	int		all;
	int		i;
	State_t*	state;
	Cxvariable_t*	variable;
	Field_t*	field;
	Field_t*	lastfield;
	Group_t*	group;
	Group_t*	lastgroup;
	Vmalloc_t*	vm;
	Dssrecord_t	record;
	Sfio_t*		tmp;

	if (!(vm = vmopen(Vmdcheap, Vmlast, 0)) || !(state = vmnewof(vm, 0, State_t, 1, 0)))
	{
		if (vm)
			vmclose(vm);
		if (disc->errorf)
			(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "out of space");
		return -1;
	}
	state->vm = vm;
	state->maxgroups = 100;
	if (!(state->print = cxopen(0, 0, disc)))
	{
		if (disc->errorf)
			(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "out of space");
		goto bad;
	}
	if (!cxscope(state->print, cx, 0, 0, disc))
	{
		cxclose(state->print);
		goto bad;
	}
	DSS(cx)->cx = state->print;
	for (i = 0; i < elementsof(variables); i++)
	{
		if (cxaddvariable(state->print, &variables[i], disc))
			goto bad;
		variables[i].data = &variables[0];
	}
	for (;;)
	{
		switch (optget(argv, stats_usage))
		{
		case 'a':
			state->op |= STATS_AVERAGE;
			continue;
		case 'c':
			state->op |= STATS_COUNT;
			continue;
		case 'd':
			state->op |= STATS_DEVIATION;
			continue;
		case 'g':
			if (!(variable = cxvariable(cx, opt_info.arg, NiL, disc)))
				goto bad;

			if (!(group = vmnewof(vm, 0, Group_t, 1, 0)))
			{
				if (disc->errorf)
					(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "out of space");
				goto bad;
			}
			group->variable = variable;
			group->string = cxisstring(variable->type) || cxisbuffer(variable->type);
			if (state->group)
				lastgroup = lastgroup->next = group;
			else
				lastgroup = state->group = group;
			state->groups++;
			continue;
		case 'l':
			if (!(state->label = vmstrdup(vm, opt_info.arg)))
			{
				if (disc->errorf)
					(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "out of space");
				goto bad;
			}
			continue;
		case 'm':
			state->maxgroups = opt_info.num;
			continue;
		case 'p':
			if (!(state->format = vmstrdup(vm, opt_info.arg)))
			{
				if (disc->errorf)
					(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "out of space");
				goto bad;
			}
			continue;
		case 'r':
			state->op |= STATS_RANGE;
			continue;
		case 's':
			state->op |= STATS_SUM;
			continue;
		case '?':
			if (disc->errorf)
			{
				(*disc->errorf)(cx, disc, ERROR_USAGE|4, "%s", opt_info.arg);
			}
			else
				goto bad;
			continue;
		case ':':
			if (disc->errorf)
				(*disc->errorf)(cx, disc, 2, "%s", opt_info.arg);
			else
				goto bad;
			continue;
		}
		break;
	}
	if (error_info.errors > errors)
		goto bad;
	argv += opt_info.index;
	if (!state->op)
		state->op = ~0;
	if (all = !*argv)
		variable = 0;
	do
	{
		if (all)
		{
			do
			{
				variable = (Cxvariable_t*)(variable ? dtnext(cx->fields, variable) : dtfirst(cx->fields));
			} while (variable && (variable->data == (void*)&variables[0] || !cxisnumber(variable->type)));
			if (!variable)
				break;
		}
		else if (streq(*argv, "-"))
			continue;
		else if (!(variable = cxvariable(cx, *argv, NiL, disc)))
			goto bad;
		else if (!cxisnumber(variable->type))
		{
			if (disc->errorf)
				(*disc->errorf)(cx, disc, 2, "%s: not a numeric field", variable->name);
			goto bad;
		}
		if (!(field = vmnewof(vm, 0, Field_t, 1, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "out of space");
			goto bad;
		}
		field->variable = variable;
		if (state->field)
			lastfield = lastfield->next = field;
		else
			lastfield = state->field = field;
		state->fields++;
	} while (all || *++argv);
	if (!state->fields)
	{
		if (all)
		{
			if (disc->errorf)
				(*disc->errorf)(cx, disc, 2, "no numeric fields");
			goto bad;
		}
		state->fields = 1;
	}
	if (!(state->total = vmnewof(vm, 0, Total_t, state->fields, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "out of space");
		goto bad;
	}
	if (state->group)
	{
		state->bucketdisc.comparf = bucketcmp;
		state->bucketdisc.link = offsetof(Bucket_t, link);
		state->bucketdisc.size = -1;
		state->bucketdisc.key = offsetof(Bucket_t, key);
		if (!(state->buckets = dtnew(vm, &state->bucketdisc, Dttree)))
		{
			if (disc->errorf)
				(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "out of space");
			goto bad;
		}
	}
	if (!(tmp = sfstropen()))
	{
		if (disc->errorf)
			(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "out of space");
		goto bad;
	}
	if (state->format)
	{
		state->print->getf = getop;
		record.data = state;
		i = dssprintf(DSS(cx), tmp, state->format, &record);
		sfclose(tmp);
		if (i < 0)
			goto bad;
		state->print->getf = getvalue;
	}
	DSS(cx)->cx = cxscope(state->print, NiL, 0, 0, disc);
	if (!state->format)
	{
		cxclose(state->print);
		state->print = 0;
	}
	expr->data = state;
	return 0;
 bad:
	vmclose(vm);
	return -1;
}

static int
stats_act(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	register State_t*	state = (State_t*)expr->data;
	register Cxvalue_t*	key;
	register Field_t*	field;
	register Group_t*	group;
	register Total_t*	total;
	Bucket_t*		bucket;
	Cxoperand_t		val;
	char*			s;
	int			range;
	int			square;

	total = state->total;
	if (state->group)
	{
		if (!state->key && !(state->key = vmnewof(state->vm, 0, Cxvalue_t, state->groups, 0)))
		{
			if (disc->errorf)
				(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		for (group = state->group, key = state->key; group; group = group->next, key++)
		{
			if (cxcast(cx, &val, group->variable, group->variable->type, data, NiL))
				return -1;
			*key = val.value;
		}
		if (bucket = (Bucket_t*)dtmatch(state->buckets, state->key))
			total = bucket->total;
		else if (dtsize(state->buckets) < state->maxgroups)
		{
			if (!(bucket = vmnewof(state->vm, 0, Bucket_t, 1, (state->fields - 1) * sizeof(Total_t))))
			{
				if (disc->errorf)
					(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "out of space");
				return -1;
			}
			for (group = state->group, key = state->key; group; group = group->next, key++)
				if (group->string)
				{
					s = key->string.data;
					if (!(key->string.data = vmnewof(state->vm, 0, char, key->string.size, 1)))
					{
						if (disc->errorf)
							(*disc->errorf)(cx, disc, ERROR_SYSTEM|2, "out of space");
						return -1;
					}
					memcpy(key->string.data, s, key->string.size);
				}
			bucket->key = state->key;
			state->key = 0;
			dtinsert(state->buckets, bucket);
			total = bucket->total;
		}
	}
	if (!state->field)
		total->count++;
	else
	{
		range = !!(state->op & STATS_RANGE);
		square = !!(state->op & STATS_DEVIATION);
		for (field = state->field; field; field = field->next)
		{
			if (cxcast(cx, &val, field->variable, cx->state->type_number, data, NiL))
				return -1;
			total->count++;
			total->value += val.value.number;
			if (range)
			{
				if (total->min > val.value.number || total->count == 1)
					total->min = val.value.number;
				if (total->max < val.value.number || total->count == 1)
					total->max = val.value.number;
			}
			if (square)
				total->square += val.value.number * val.value.number;
			total++;
		}
	}
	return 0;
}

static void
number(Sfio_t* op, Cxnumber_t n, int fw)
{
	if (n == 0 || ((n >= 0) ? n : -n) >= 1 && n >= FLTMAX_INTMAX_MIN && n <= FLTMAX_UINTMAX_MAX && n == (Cxinteger_t)n)
		sfprintf(op, " %*I*u", fw, sizeof(Cxinteger_t), (Cxinteger_t)n);
	else
	{
		if (n >= 0)
			sfputc(op, ' ');
		sfprintf(op, " %1.*I*e", fw - 7, sizeof(n), n);
	}
}

static int
list(Cx_t* cx, register State_t* state, Sfio_t* op, const char* label, register Field_t* field, register Total_t* total, Cxvalue_t* key)
{
	Dssrecord_t	record;
	Print_t		pr;
	Cxnumber_t	u;
	Cxoperand_t	arg;
	Group_t*	group;
	char*		s;
	int		r;

	do
	{
		if (state->print)
		{
			record.data = &pr;
			pr.field = field;
			pr.group = state->group;
			pr.total = total;
			pr.key = key;
			DSS(cx)->cx = state->print;
			r = dssprintf(DSS(cx), op, state->format, &record);
			DSS(cx)->cx = cx;
			if (r < 0)
				return -1;
		}
		else
		{
			sfprintf(op, "%*s", FW, field ? field->variable->name : "-");
			if (state->op & STATS_COUNT)
				number(op, (Cxinteger_t)total->count, FW);
			u = total->value / (Cxinteger_t)total->count;
			if (state->op & STATS_SUM)
				number(op, total->value, FW);
			if (state->op & STATS_AVERAGE)
				number(op, u, FW);
			if (state->op & STATS_DEVIATION)
			{
				if ((u = total->square + u * (u - 2 * total->value)) < 0)
					u = -u;
				if (total->count > 1)
					u /= (Cxinteger_t)(total->count - 1);
				number(op, (Cxnumber_t)sqrt(u), FW);
			}
			if (state->op & STATS_RANGE)
			{
				number(op, total->min, FW);
				number(op, total->max, FW);
			}
			if (label)
			{
				sfprintf(op, " %s", label);
				label = 0;
			}
			else if (key)
			{
				for (group = state->group; group; group = group->next, key++)
				{
					sfputc(op, ' ');
					if (group->string)
						s = key->string.data;
					else
					{
						arg.type = group->variable->type;
						arg.value = *key;
						if (cxcast(cx, &arg, NiL, cx->state->type_string, NiL, group->variable->format.details))
							return -1;
						s = arg.value.string.data;
					}
					sfprintf(op, "%*s", group->width, s);
				}
				key = 0;
			}
			sfprintf(op, "\n");
		}
		total++;
	} while (field && (field = field->next));
	return 0;
}

static int
stats_end(Cx_t* cx, Cxexpr_t* expr, void* data, Cxdisc_t* disc)
{
	register State_t*	state = (State_t*)expr->data;
	register Bucket_t*	bucket;
	register Group_t*	group;

	if (state->label)
		sfprintf(expr->op, "%s\n", state->label);
	if (!state->print)
	{
		sfprintf(expr->op, "%*s", FW, "FIELD");
		if (state->op & STATS_COUNT)
			sfprintf(expr->op, " %*s", FW, "COUNT");
		if (state->op & STATS_SUM)
			sfprintf(expr->op, " %*s", FW, "SUM");
		if (state->op & STATS_AVERAGE)
			sfprintf(expr->op, " %*s", FW, "AVERAGE");
		if (state->op & STATS_DEVIATION)
			sfprintf(expr->op, " %*s", FW, "DEVIATION");
		if (state->op & STATS_RANGE)
			sfprintf(expr->op, " %*s %*s", FW, "MIN", FW, "MAX");
		if (state->buckets)
			for (group = state->group; group; group = group->next)
			{
				group->width = group->variable->format.print ? group->variable->format.print : group->variable->type->format.print ? group->variable->type->format.print : FW;
				sfprintf(expr->op, " %*s", group->width, group->variable->name);
			}
		sfprintf(expr->op, "\n");
	}
	if (state->buckets)
	{
		for (bucket = (Bucket_t*)dtfirst(state->buckets); bucket; bucket = (Bucket_t*)dtnext(state->buckets, bucket))
			if (list(cx, state, expr->op, NiL, state->field, bucket->total, bucket->key))
				goto bad;
		if (state->total->count && list(cx, state, expr->op, "OVERFLOW", state->field, state->total, NiL))
			goto bad;
	}
	else if (list(cx, state, expr->op, NiL, state->field, state->total, NiL))
		goto bad;
	if (state->print)
		cxclose(state->print);
	vmclose(state->vm);
	return 0;
 bad:
	if (state->print)
		cxclose(state->print);
	vmclose(state->vm);
	return -1;
}

static Cxquery_t	queries[] =
{
	{
		"stats",
		"collect numeric field value statistics",
		CXH,
		stats_beg,
		0,
		stats_act,
		stats_end
	},
	{0}
};

static Dsslib_t		lib =
{
	"stats",
	"stats query",
	CXH,
	0,
	0,
	0,
	0,
	0,
	0,
	&queries[0]
};

Dsslib_t*
dss_lib(const char* name, Dssdisc_t* disc)
{
	return &lib;
}
