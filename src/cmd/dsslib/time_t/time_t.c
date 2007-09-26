/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 2002-2007 AT&T Knowledge Ventures            *
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
 * dss time type library
 *
 * Glenn Fowler
 * AT&T Research
 */

static const char id[] = "\n@(#)$Id: dss time type library (AT&T Research) 2003-08-11 $\0\n";

#include <dsslib.h>
#include <tm.h>

#define NS			1e9
#define SS			4.294967296e9

typedef struct Precise_s
{
	Cxnumber_t		seconds;
	const char*		format;
	size_t			size;
} Precise_t;

#if _typ_int64_t
typedef uint64_t Nstime_t;
#else
typedef double Nstime_t;
#endif

static ssize_t
time_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	char*	s;
	time_t	t;

	if (!size)
		return 40;
	t = value->number;
	s = tmfmt(buf, size, CXDETAILS(details, format, type, "%K"), &t);
	if (s == (buf + size - 1))
		return 2 * size;
	return s - buf;
}

static ssize_t
time_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*	e;
	char*	f;

	if (CXDETAILS(details, format, type, 0))
	{
		value->number = tmscan(buf, &e, details, &f, NiL, 0);
		if (!*f && e > (char*)buf)
			return e - (char*)buf;
	}
	value->number = tmdate(buf, &e, NiL);
	return e - (char*)buf;
}

static void*
ns_init(void* data, Cxdisc_t* disc)
{
	Precise_t*	precise;

	if (!(precise = newof(0, Precise_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	precise->seconds = NS;
	precise->format = ".%09lu";
	precise->size = 40;
	return precise;
}

static void*
stamp_init(void* data, Cxdisc_t* disc)
{
	Precise_t*	precise;

	if (!(precise = newof(0, Precise_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	precise->seconds = SS;
	precise->format = ".%010lu";
	precise->size = 41;
	return precise;
}

static ssize_t
precise_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	char*		s;
	time_t		t;
	Precise_t*	precise = (Precise_t*)type->data;

	if (!size)
		return precise->size;
	t = value->number / precise->seconds;
	s = tmfmt(buf, size, CXDETAILS(details, format, type, "%K"), &t);
	s += sfsprintf(s, size - (s - buf), precise->format, (unsigned long)(value->number - ((Cxnumber_t)t * precise->seconds)));
	if (s == (buf + size - 1))
		return 2 * size;
	return s - buf;
}

static ssize_t
precise_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*		e;
	char*		f;
	Precise_t*	precise = (Precise_t*)type->data;

	if (CXDETAILS(details, format, type, 0))
	{
		value->number = tmscan(buf, &e, details, &f, NiL, 0);
		if (*f || e == (char*)buf)
			value->number = tmdate(buf, &e, NiL);
	}
	else
		value->number = tmdate(buf, &e, NiL);
	value->number *= precise->seconds;
	if (*e == '.')
		value->number += strtoul(e + 1, &e, 10);
	return e - (char*)buf;
}

static ssize_t
elapsed_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	char*	s;
	ssize_t	n;

	s = fmtelapsed((unsigned long)value->number, 1000);
	n = strlen(s);
	if ((n + 1) > size)
		return n + 1;
	memcpy(buf, s, n + 1);
	return n;
}

static ssize_t
elapsed_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*	e;

	value->number = strelapsed(buf, &e, 1000);
	if (e == (char*)buf)
		return -1;
	return e - (char*)buf;
}

static ssize_t
tm_hour_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	char*	s;
	int	v;
	ssize_t	n;

	v = value->number;
	CXDETAILS(details, format, type, "%d");
	if (strchr(details, 's'))
	{
		s = tm_info.format[TM_MERIDIAN + (v >= 12)];
		if (v > 12)
			v -= 12;
		n = strlen(s) + (v >= 10) + 2;
		if ((n + 1) > size)
			return n + 1;
		n = sfsprintf(buf, size, "%d%s", v, s);
	}
	else
	{
		n = sfsprintf(buf, size, details, v);
		if ((n + 1) > size)
			n++;
	}
	return n;
}

static ssize_t
tm_hour_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*	e;

	value->number = strntol(buf, size, &e, 10);
	if (e == (char*)buf)
		return -1;
	if (tmlex(e, &e, tm_info.format + TM_MERIDIAN, TM_UT - TM_MERIDIAN, NiL, 0) == 1)
		value->number += 12;
	return e - (char*)buf;
}

static ssize_t
tm_mon_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	char*	s;
	int	v;
	ssize_t	n;

	v = value->number;
	if (v <= 0)
		v = 0;
	else
		v %= 12;
	CXDETAILS(details, format, type, "%d");
	if (strchr(details, 's'))
	{
		s = tm_info.format[TM_MONTH + v];
		n = strlen(s);
		if ((n + 1) > size)
			return n + 1;
		strcpy(buf, s);
	}
	else
	{
		n = sfsprintf(buf, size, details, v + 1);
		if ((n + 1) > size)
			n++;
	}
	return n;
}

static ssize_t
tm_mon_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*	e;
	int	v;

	v = (int)strntol(buf, size, &e, 10);
	if (e != (char*)buf)
	{
		if (v < 1 || v > 12)
			return -1;
		v--;
	}
	else if ((v = tmlex(buf, &e, tm_info.format + TM_MONTH_ABBREV, TM_DAY_ABBREV - TM_MONTH_ABBREV, NiL, 0)) < 0)
		return -1;
	else if (v >= 12)
		v -= 12;
	value->number = v;
	return e - (char*)buf;
}

static ssize_t
tm_wday_external(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, char* buf, size_t size, Cxdisc_t* disc)
{
	char*	s;
	int	v;
	ssize_t	n;

	v = value->number;
	if (v <= 0)
		v = 0;
	else
		v %= 7;
	CXDETAILS(details, format, type, "%d");
	if (strchr(details, 's'))
	{
		s = tm_info.format[TM_DAY + v];
		n = strlen(s);
		if ((n + 1) > size)
			return n + 1;
		strcpy(buf, s);
	}
	else
	{
		n = sfsprintf(buf, size, details, v + 1);
		if ((n + 1) > size)
			n++;
	}
	return n;
}

static ssize_t
tm_wday_internal(Cx_t* cx, Cxtype_t* type, const char* details, Cxformat_t* format, Cxvalue_t* value, const char* buf, size_t size, Vmalloc_t* vm, Cxdisc_t* disc)
{
	char*	e;
	int	v;

	v = (int)strntol(buf, size, &e, 10);
	if (e != (char*)buf)
	{
		if (v < 1 || v > 7)
			return -1;
		v--;
	}
	else if ((v = tmlex(buf, &e, tm_info.format + TM_DAY_ABBREV, TM_TIME - TM_DAY_ABBREV, NiL, 0)) < 0)
		return -1;
	else if (v >= 7)
		v -= 7;
	value->number = v;
	return e - (char*)buf;
}

#define TIME_T_sec		1
#define TIME_T_min		2
#define TIME_T_hour		3
#define TIME_T_mday		4
#define TIME_T_mon		5
#define TIME_T_year		6
#define TIME_T_wday		7
#define TIME_T_yday		8
#define TIME_T_isdst		9
#define TIME_T_ns		10

static Cxvariable_t tm_struct[] =
{
CXV("sec",   "number",   TIME_T_sec,   "Seconds after the minute [0-61].")
CXV("min",   "number",   TIME_T_min,   "Minutes after the hour [0-59].")
CXV("hour",  "tm_hour_t",TIME_T_hour,  "Hour since midnight [0-23].")
CXV("mday",  "number",   TIME_T_mday,  "Day of the month [1-31].")
CXV("mon",   "tm_mon_t", TIME_T_mon,   "Months since January [0-11].")
CXV("year",  "number",   TIME_T_year,  "4-digit year [1969-2038].")
CXV("wday",  "tm_wday_t",TIME_T_wday,  "Days since Sunday [0-6].")
CXV("yday",  "number",   TIME_T_yday,  "Days since January 1 [0-365].")
CXV("isdst", "number",   TIME_T_isdst, "Daylight savings time in effect [0-1].")
CXV("ns",    "number",   TIME_T_ns,    "Residual nanoseconds [0-999999999].")
{0}
};

typedef struct Tm_state_s
{
	time_t		t;
	Tm_t		tm;
} Tm_state_t;

static void*
tm_init(void* data, Cxdisc_t* disc)
{
	Tm_state_t*	state;

	if (!(state = newof(0, Tm_state_t, 1, 0)))
	{
		if (disc->errorf)
			(*disc->errorf)(NiL, disc, ERROR_SYSTEM|2, "out of space");
		return 0;
	}
	state->tm = *tmmake(&state->t);
	return state;
}

static int
tm_get(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	Tm_state_t*	state = (Tm_state_t*)pc->data.variable->member->data;
	Cxnumber_t	ns;
	time_t		t;

	if (r->type && r->type->data)
		ns = ((Precise_t*)r->type->data)->seconds;
	else if (b && b->type && b->type->data)
		ns = ((Precise_t*)b->type->data)->seconds;
	else
		ns = 0;
	t = ns ? (r->value.number / ns) : r->value.number;
	if (state->t != t)
	{
		state->t = t;
		state->tm = *tmmake(&t);
	}
	switch (pc->data.variable->index)
	{
	case TIME_T_sec:
		r->value.number = state->tm.tm_sec;
		break;
	case TIME_T_min:
		r->value.number = state->tm.tm_min;
		break;
	case TIME_T_hour:
		r->value.number = state->tm.tm_hour;
		break;
	case TIME_T_mday:
		r->value.number = state->tm.tm_mday;
		break;
	case TIME_T_mon:
		r->value.number = state->tm.tm_mon;
		break;
	case TIME_T_year:
		r->value.number = 1900 + state->tm.tm_year;
		break;
	case TIME_T_wday:
		r->value.number = state->tm.tm_wday;
		break;
	case TIME_T_yday:
		r->value.number = state->tm.tm_yday;
		break;
	case TIME_T_isdst:
		r->value.number = state->tm.tm_isdst;
		break;
	case TIME_T_ns:
		if (ns)
			r->value.number -= ((Cxnumber_t)t * ns);
		else
			r->value.number = 0;
		break;
	default:
		return -1;
	}
	return 0;
}

static int
tm_set(Cx_t* cx, Cxinstruction_t* pc, Cxoperand_t* r, Cxoperand_t* a, Cxoperand_t* b, void* data, Cxdisc_t* disc)
{
	Tm_state_t*	state = (Tm_state_t*)pc->data.variable->member->data;
	Cxnumber_t	ns;
	Cxinteger_t	n;
	time_t		t;
	int		i;

	if (r->type && r->type->data)
		ns = ((Precise_t*)r->type->data)->seconds;
	else if (b && b->type && b->type->data)
		ns = ((Precise_t*)b->type->data)->seconds;
	else
		ns = 0;
	t = ns ? (r->value.number / ns) : r->value.number;
	if (state->t != t)
	{
		state->t = t;
		state->tm = *tmmake(&t);
	}
	switch (pc->data.variable->index)
	{
	case TIME_T_sec:
		state->tm.tm_sec = a->value.number;
		break;
	case TIME_T_min:
		state->tm.tm_min = a->value.number;
		break;
	case TIME_T_hour:
		state->tm.tm_hour = a->value.number;
		break;
	case TIME_T_mday:
		state->tm.tm_mday = a->value.number;
		break;
	case TIME_T_mon:
		state->tm.tm_mon = a->value.number;
		break;
	case TIME_T_year:
		if ((state->tm.tm_year = a->value.number) >= 1900)
			state->tm.tm_year -= 1900;
		break;
	case TIME_T_wday:
		i = a->value.number;
		if ((i -= state->tm.tm_wday) < 0)
			i += 7;
		state->tm.tm_mday += i;
		state->tm.tm_wday = a->value.number;
		break;
	case TIME_T_yday:
		i = a->value.number;
		if ((i -= state->tm.tm_yday) < 0)
			i += 365 + tmisleapyear(state->tm.tm_year);
		state->tm.tm_mday += i;
		state->tm.tm_yday = a->value.number;
		break;
	case TIME_T_isdst:
		state->tm.tm_isdst = a->value.number;
		break;
	case TIME_T_ns:
		if (ns)
		{
			n = r->value.number;
			n = (n / ns) * ns;
			n += a->value.number;
			r->value.number = n;
		}
		return 0;
	default:
		return -1;
	}
	r->value.number = state->t = tmtime(&state->tm, TM_LOCALZONE);
	if (ns)
		r->value.number *= ns;
	return 0;
}

static Cxmember_t	tm_member =
{
	tm_get,
	tm_set,
	(Dt_t*)&tm_struct[0]
};

static Cxtype_t types[] =
{
	{ "tm_hour_t",	"Hour since midnight with optional meridian (AM/PM).", CXH, (Cxtype_t*)"number", 0, tm_hour_external, tm_hour_internal, 0, 0, { "The format details string is a \bprintf\b(3) format string.", "%d", CX_UNSIGNED|CX_INTEGER, 1 } },
	{ "tm_mon_t",	"Month name represented as a number [0-11], starting at January.", CXH, (Cxtype_t*)"number", 0, tm_mon_external, tm_mon_internal, 0, 0, { "The format details string is a \bprintf\b(3) format string.", "%s", CX_UNSIGNED|CX_INTEGER, 1 } },
	{ "tm_wday_t",	"Weekday name represented as a number [0-6], starting at Sunday.", CXH, (Cxtype_t*)"number", 0, tm_wday_external, tm_wday_internal, 0, 0, { "The format details string is a \bprintf\b(3) format string.", "%s", CX_UNSIGNED|CX_INTEGER, 1 } },
	{ "tm_t",	"Time parts.", CXH, (Cxtype_t*)"number", tm_init, 0, 0, 0, 0, { 0, 0, CX_UNSIGNED|CX_INTEGER, 4 }, 0, &tm_member	},
	{ "elapsed_t",	"Elapsed time in milliseconds.", CXH, (Cxtype_t*)"number", 0, elapsed_external, elapsed_internal, 0, 0, { 0, 0, CX_INTEGER, 4 }	},
	{ "ns_t",	"64 bit nanoseconds since the epoch.", CXH, (Cxtype_t*)"tm_t", ns_init, precise_external, precise_internal, 0, 0, { "The format details string is a \bstrftime\b(3)/\bstrptime\b(3) format string.", "%K", CX_UNSIGNED|CX_INTEGER, 8 } },
	{ "stamp_t",	"64 bit 1/2**32 seconds since the epoch.", CXH, (Cxtype_t*)"tm_t", stamp_init, precise_external, precise_internal, 0, 0, { "The format details string is a \bstrftime\b(3)/\bstrptime\b(3) format string.", "%K", CX_UNSIGNED|CX_INTEGER, 8 } },
	{ "time_t",	"32 bit seconds since the epoch.", CXH, (Cxtype_t*)"tm_t", 0, time_external, time_internal, 0, 0, { "The format details string is a \bstrftime\b(3)/\bstrptime\b(3) format string.", "%K", CX_UNSIGNED|CX_INTEGER, 4 } },
	{ 0, 0 }
};

Dsslib_t dss_lib_time_t =
{
	"time_t",
	"time type support",
	CXH,
	0,
	0,
	&types[0],
};
