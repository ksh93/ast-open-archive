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
 * dss printf implementation
 */

#include "dsshdr.h"

#include <ast_float.h>

struct Arg_s; typedef struct Arg_s Arg_t;

struct Arg_s
{
	Cxvariable_t*		variable;
	Cxtype_t*		cast;
	char*			details;
	char*			qb;
	char*			qe;
	unsigned short		type;
	unsigned short		fmt;
	unsigned char		flags;
};

struct Format_s
{
	Format_t*		next;
	char*			oformat;
	char*			nformat;
	Arg_t			arg[1];
};

typedef struct Fmt_s
{
	Sffmt_t			fmt;
	Dss_t*			dss;
	void*			data;
	int			errors;
	Arg_t*			ap;
} Fmt_t;

typedef union
{
	char**			p;
	char*			s;
	Sflong_t		q;
	long			l;
	int			i;
	short			h;
	char			c;
	double			f;
} Value_t;

#define DSS_FORMAT_char		1
#define DSS_FORMAT_float	2
#define DSS_FORMAT_int		3
#define DSS_FORMAT_long		4
#define DSS_FORMAT_string	5

#define DSS_FORMAT_fmt		0x01

/*
 * sfio %! extension function
 */

static int
getfmt(Sfio_t* sp, void* vp, Sffmt_t* dp)
{
	register Fmt_t*	fp = (Fmt_t*)dp;
	register Arg_t*	ap = fp->ap++;
	Value_t*	value = (Value_t*)vp;
	Cxoperand_t	ret;

	if (cxcast(fp->dss->cx, &ret, ap->variable, ap->cast, fp->data, ap->details))
	{
		fp->errors++;
		return -1;
	}
	fp->fmt.flags |= SFFMT_VALUE;
	switch (ap->type)
	{
	case DSS_FORMAT_char:
		fp->fmt.size = sizeof(int);
		if (ret.value.number < 1)
			value->c = 0;
		else if (ret.value.number > UCHAR_MAX)
			value->c = UCHAR_MAX;
		else
			value->c = (unsigned char)ret.value.number;
		break;
	case DSS_FORMAT_float:
		fp->fmt.size = sizeof(double);
		value->f = ret.value.number;
		break;
	case DSS_FORMAT_int:
		fp->fmt.size = sizeof(int);
		if (((ret.value.number >= 0) ? ret.value.number : -ret.value.number) < 1)
			value->i = 0;
		else if (ret.value.number > UINT_MAX)
			value->i = INT_MAX;
		else if (ret.value.number < INT_MIN)
			value->i = INT_MAX;
		else
			value->i = (unsigned int)ret.value.number;
		break;
	case DSS_FORMAT_long:
		fp->fmt.size = sizeof(Sflong_t);
		if (((ret.value.number >= 0) ? ret.value.number : -ret.value.number) < 1)
			value->q = 0;
		else if (ret.value.number > FLTMAX_UINTMAX_MAX)
			value->q = FLTMAX_INTMAX_MAX;
		else if (ret.value.number < FLTMAX_INTMAX_MIN)
			value->q = FLTMAX_INTMAX_MAX;
		else
			value->q = (Sfulong_t)((Sflong_t)ret.value.number);
		break;
	case DSS_FORMAT_string:
		value->s = ret.value.string.data;
		fp->fmt.size = ret.value.string.size;
		if (ap->flags & DSS_FORMAT_fmt)
			fp->fmt.size = strlen(value->s = fmtquote(value->s, ap->qb, ap->qe, fp->fmt.size, ap->fmt));
		break;
	}
	return 0;
}

/*
 * printf
 */

int
dssprintf(Dss_t* dss, Sfio_t* sp, const char* format, Dssrecord_t* record)
{
	register char*	s;
	register char*	t;
	register char*	d;
	register char*	v;
	register int	n;
	register int	q;
	register Arg_t*	ap;
	int		l;
	char*		f;
	Format_t*	fp;
	Fmt_t		fmt;

	for (fp = dss->print; fp && fp->oformat != (char*)format; fp = fp->next);
	if (!fp)
	{
		f = s = (char*)format;
		d = 0;
		l = 0;
		n = 0;
		q = 0;
		for (;;)
		{
			switch (*s++)
			{
			case 0:
				if (q)
				{
					if (dss->disc->errorf)
						(*dss->disc->errorf)(NiL, dss->disc, 2, "%s: format character omitted", f);
					return -1;
				}
				break;
			case '%':
				if (*s != '%')
				{
					q = 1;
					n++;
					f = s - 1;
				}
				continue;
			case '(':
				if (q == 1)
				{
					q++;
					for (;;)
					{
						switch (*s++)
						{
						case 0:
							s--;
							break;
						case '(':
							q++;
							continue;
						case ')':
							if (--q == 1)
								break;
							continue;
						case ':':
							if (!d)
								d = s;
							continue;
						default:
							continue;
						}
						break;
					}
					if (d)
					{
						l += s - d + 1;
						d = 0;
					}
				}
				continue;
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'o':
			case 's':
			case 'u':
			case 'x':
				if (q == 1)
					q = 0;
				continue;
			default:
				continue;
			}
			break;
		}
		if (!(fp = vmnewof(dss->vm, 0, Format_t, 1, (n - 1) * sizeof(Arg_t) + strlen(format) + 2 * n + l + 2)))
		{
			if (dss->disc->errorf)
				(*dss->disc->errorf)(NiL, dss->disc, ERROR_SYSTEM|2, "out of space");
			return -1;
		}
		fp->oformat = (char*)format;
		fp->next = dss->print;
		dss->print = fp;
		ap = &fp->arg[0];
		s = t = fp->nformat = (char*)(&fp->arg[n]);
		strcpy(t, format);
		f = t + strlen(format) + 2 * n + 1;
		q = 0;
		d = 0;
		l = 0;
		for (;;)
		{
			switch (*t++ = *s++)
			{
			case 0:
				*(t - 1) = '\n';
				*t = 0;
				break;
			case '%':
				if (*s == '%')
					*t++ = *s++;
				else
					q = 1;
				continue;
			case '(':
				if (q == 1)
				{
					q++;
					t--;
					v = s;
					for (;;)
					{
						switch (*s++)
						{
						case 0:
							if (dss->disc->errorf)
								(*dss->disc->errorf)(NiL, dss->disc, 2, "%s: %(...) imbalance", fp->oformat);
							return -1;
						case '(':
							if (!d)
								d = s;
							q++;
							continue;
						case ')':
							if (--q == 1)
								break;
							if (!d)
								d = s;
							continue;
						case ':':
							if (!d)
								d = s;
							continue;
						default:
							continue;
						}
						break;
					}
					if (d)
						*(d - 1) = 0;
					*(s - 1) = 0;
					if (!(ap->variable = cxvariable(dss->cx, v, NiL, dss->cx->disc)))
						return -1;
				}
				continue;
			case 'c':
				if (q == 1)
				{
					ap->type = DSS_FORMAT_char;
					ap->cast = dss->cx->state->type_number;
					goto set;
				}
				continue;
			case 'd':
			case 'o':
			case 'u':
			case 'x':
				if (q == 1)
				{
					if (l > 1 || ap->variable->format.width == 8 || ap->variable->type->format.width == 8)
					{
						n = *(t - 1);
						*(t - 1) = 'l';
						*t++ = 'l';
						*t++ = n;
						ap->type = DSS_FORMAT_long;
					}
					else
						ap->type = DSS_FORMAT_int;
					ap->cast = dss->cx->state->type_number;
					goto set;
				}
				continue;
			case 'e':
			case 'f':
			case 'g':
				if (q == 1)
				{
					ap->type = DSS_FORMAT_float;
					ap->cast = dss->cx->state->type_number;
					goto set;
				}
				continue;
			case 'h':
				if (q == 1)
					t--;
				continue;
			case 'l':
				if (q == 1)
				{
					t--;
					l++;
				}
				continue;
			case 's':
				if (q == 1)
				{
					ap->type = DSS_FORMAT_string;
					ap->cast = dss->cx->state->type_string;
				set:
					if (!ap->variable)
					{
						if (dss->disc->errorf)
						{
							*t = 0;
							(*dss->disc->errorf)(NiL, dss->disc, 2, "%s: (variable) omitted in format", fp->nformat);
						}
						return -1;
					}
					l = 0;
					q = 0;
					if (d)
					{
						ap->fmt = FMT_ALWAYS|FMT_ESCAPED;
						while (*d)
						{
							v = d;
							while (*d)
								if (*d++ == ':')
								{
									*(d - 1) = 0;
									break;
								}
							if (strneq(v, "endquote=", 8))
							{
								ap->qe = v += 8;
								while (*f++ = *v++);
							}
							else if (streq(v, "escape"))
								ap->fmt &= ~FMT_ESCAPED;
							else if (strneq(v, "opt", 3))
								ap->fmt &= ~FMT_ALWAYS;
							else if (streq(v, "quote") || strneq(v, "quote=", 6))
							{
								if (v[5])
								{
									ap->qb = v += 6;
									while (*f++ = *v++);
								}
								else
									ap->qb = "\"";
								if (!ap->qe)
									ap->qe = ap->qb;
							}
							else if (streq(v, "shell") || strneq(v, "shell=", 6))
							{
								ap->fmt |= FMT_SHELL;
								if (v[5])
								{
									ap->qb = v += 6;
									while (*f++ = *v++);
								}
								else
									ap->qb = "$'";
								if (!ap->qe)
									ap->qe = "'";
							}
							else if (streq(v, "wide"))
								ap->fmt |= FMT_WIDE;
							else
							{
								if (*d)
									*(d - 1) = ':';
								d = v;
								break;
							}
							ap->flags |= DSS_FORMAT_fmt;
						}
						ap->details = f;
						while (*f++ = *d++);
						d = 0;
					}
					ap++;
				}
				continue;
			case 'L':
				if (q == 1)
				{
					t--;
					l += 2;
				}
				continue;
			default:
				continue;
			}
			break;
		}
	}
	memset(&fmt, 0, sizeof(fmt));
	fmt.fmt.version = SFIO_VERSION;
	fmt.fmt.form = fp->nformat;
	fmt.fmt.extf = getfmt;
	fmt.dss = dss;
	fmt.data = record;
	fmt.ap = &fp->arg[0];
	n = sfprintf(sp, "%!", &fmt);
	return (fmt.errors || n <= 0 && sferror(sp)) ? -1 : n;
}
