/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1989-2001 AT&T Corp.                *
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
*******************************************************************/
#pragma prototyped

/*
 * locale command
 */

static const char usage[] =
"[-?\n@(#)$Id: locale (AT&T Labs Research) 2001-05-25 $\n]"
USAGE_LICENSE
"[+NAME?locale - get locale-specific information]"
"[+DESCRIPTION?\blocale\b writes information about the current locale to"
"	the standard output. If no options or operands are specified then the"
"	environment for each locale category is summarized. If operands are"
"	spcified then information is written about each operand: a locale"
"	category name operand (\fcategories\f) selects all keywords in that"
"	category; other operands name keywords within a category. Keyword"
"	names are unique across all categories, so there is no ambiguity."
"	\acategory=value\a operands set the corresponding locale \acategory\a"
"	to \avalue\a by a call to \bsetlocale\b(3). A \avalue\a of \b-\b is"
"	interpreted as \bNULL\b, allowing \acategory\a to be queried. The"
"	\bTEST\b category converts the \avalue\a locale name to a canonical"
"	form and lists the converted name on the standard output.]"
"[+?If the \b--all\b or \b--undefined\b options are specified and no operands"
"	are specified then the names of all defined locales are listed on the"
"	standard output. A \adefined\a locale name should work as an argument"
"	to \bsetlocale\b(3). An \aundefined\a name is valid but not supported"
"	on the local system. Locale names have the general form"
"	\alanguage\a_\aterritory\a.\acharset\a@\aattribute\a[,\aattribute\a]]*"
"	At least one of \alanguage\a or \aterritory\a is always specified, the"
"	other parts are optional. The \b--abbreviated\b, \b--qualified\b,"
"	\b--verbose\b and \b--local\b options determine the locale name"
"	listing style. The default is \b--abbreviated\b. If multiple styles"
"	are specified then the names are listed in columns in this order:"
"	\babbreviated\b, \bqualified\b, \bverbose\b and \blocal\b.]"

"[a:all?List all defined locale names on the standard output.]"
"[b:abbreviated?List abbreviated locale names: \alanguage\a and \aterritory\a"
"	as the two character ISO codes; non-default \acharset\a and"
"	\aattributes\a. This is the default.]"
"[c:category?List the category names for each operand on the standard output.]"
"[e:element?The operands are interpreted as collation elements. Each element"
"	name is listed on the standard output followed by a \btab\b character,"
"	the character width, the \bstrxfrm\b(3) width, and the \bstrxfrm\b(3)"
"	collation weights.]"
"[i:indent?Indent keyword output lines for readability.]"
"[k:keyword?List the keyword name for each operand on the standard output.]"
"[l:local?List the locale names returned by the local system \bsetlocale\b(3)."
"	NOTE: these names may contain embedded space.]"
"[m:charmaps?List the names of available charmaps.]"
"[q:qualified?List qualified locale names: \alanguage\a and \aterritory\a"
"	as the two character ISO codes; default and non-default \acharset\a and"
"	\aattributes\a.]"
"[u:undefined?List all undefined locale names on the standard output.]"
"[v:verbose?List verbose locale names: \alanguage\a and \aterritory\a"
"	as the long English strings; non-default \acharset\a and"
"	\aattributes\a.]"

"\n"
"\n[ name | name=value ... ]\n"
"\n"

"[+SEE ALSO?\blocaleconv\b(3), \bnl_langinfo\b(3), \bsetlocale\b(3)]"
;

#include <ast.h>
#include <cdt.h>
#include <ctype.h>
#include <error.h>
#include <lc.h>
#include <tm.h>
#include <regex.h>

#include "FEATURE/locales"

#define TEST		(AST_LC_COUNT+1)

#if _hdr_nl_types && _hdr_langinfo

#include <nl_types.h>
#include <langinfo.h>

#else

#undef	_hdr_nl_types
#undef	_hdr_langinfo

#endif

typedef struct Keyword_s
{
	const char*	name;
	int		index;
	int		type;
	int		elements;
	int		offset;
	Dtlink_t	link;
} Keyword_t;

#define LC_category		(LC_user<<1)
#define LC_indent		(LC_user<<2)
#define LC_keyword		(LC_user<<3)
#define LC_proper		(LC_user<<4)
#define LC_quote		(LC_user<<5)
#define LC_recursive		(LC_user<<6)
#define LC_upper		(LC_user<<7)

#define C			1
#define I			4
#define S			0

#define CV_collate		1

#define CV_charset		1
#define CV_mb_cur_max		2
#define CV_mb_cur_min		3

#ifdef NOEXPR
#define CV_noexpr		NOEXPR
#else
#define CV_noexpr		(-1)
#endif
#ifdef NOSTR
#define CV_nostr		NOSTR
#else
#define CV_nostr		(-1)
#endif
#ifdef YESEXPR
#define CV_yesexpr		YESEXPR
#else
#define CV_yesexpr		(-1)
#endif
#ifdef YESSTR
#define CV_yesstr		YESSTR
#else
#define CV_yesstr		(-1)
#endif

#if _mem_credit_sign_lconv
#define CV_credit_sign		offsetof(struct lconv,credit_sign)
#else
#define CV_credit_sign		(-1)
#endif
#if _mem_currency_symbol_lconv
#define CV_currency_symbol	offsetof(struct lconv,currency_symbol)
#else
#define CV_currency_symbol	(-1)
#endif
#if _mem_debit_sign_lconv
#define CV_debit_sign		offsetof(struct lconv,debit_sign)
#else
#define CV_debit_sign		(-1)
#endif
#if _mem_frac_digits_lconv
#define CV_frac_digits		offsetof(struct lconv,frac_digits)
#else
#define CV_frac_digits		(-1)
#endif
#if _mem_int_curr_symbol_lconv
#define CV_int_curr_symbol	offsetof(struct lconv,int_curr_symbol)
#else
#define CV_int_curr_symbol	(-1)
#endif
#if _mem_int_frac_digits_lconv
#define CV_int_frac_digits	offsetof(struct lconv,int_frac_digits)
#else
#define CV_int_frac_digits	(-1)
#endif
#if _mem_left_parenthesis_lconv
#define CV_left_parenthesis	offsetof(struct lconv,left_parenthesis)
#else
#define CV_left_parenthesis	(-1)
#endif
#if _mem_mon_decimal_point_lconv
#define CV_mon_decimal_point	offsetof(struct lconv,mon_decimal_point)
#else
#define CV_mon_decimal_point	(-1)
#endif
#if _mem_mon_grouping_lconv
#define CV_mon_grouping		offsetof(struct lconv,mon_grouping)
#else
#define CV_mon_grouping		(-1)
#endif
#if _mem_mon_thousands_sep_lconv
#define CV_mon_thousands_sep	offsetof(struct lconv,mon_thousands_sep)
#else
#define CV_mon_thousands_sep	(-1)
#endif
#if _mem_n_cs_precedes_lconv
#define CV_n_cs_precedes	offsetof(struct lconv,n_cs_precedes)
#else
#define CV_n_cs_precedes	(-1)
#endif
#if _mem_n_sep_by_space_lconv
#define CV_n_sep_by_space	offsetof(struct lconv,n_sep_by_space)
#else
#define CV_n_sep_by_space	(-1)
#endif
#if _mem_n_sign_posn_lconv
#define CV_n_sign_posn		offsetof(struct lconv,n_sign_posn)
#else
#define CV_n_sign_posn		(-1)
#endif
#if _mem_negative_sign_lconv
#define CV_negative_sign	offsetof(struct lconv,negative_sign)
#else
#define CV_negative_sign	(-1)
#endif
#if _mem_p_cs_precedes_lconv
#define CV_p_cs_precedes	offsetof(struct lconv,p_cs_precedes)
#else
#define CV_p_cs_precedes	(-1)
#endif
#if _mem_p_sep_by_space_lconv
#define CV_p_sep_by_space	offsetof(struct lconv,p_sep_by_space)
#else
#define CV_p_sep_by_space	(-1)
#endif
#if _mem_p_sign_posn_lconv
#define CV_p_sign_posn		offsetof(struct lconv,p_sign_posn)
#else
#define CV_p_sign_posn		(-1)
#endif
#if _mem_positive_sign_lconv
#define CV_positive_sign	offsetof(struct lconv,positive_sign)
#else
#define CV_positive_sign	(-1)
#endif
#if _mem_right_parenthesis_lconv
#define CV_right_parenthesis	offsetof(struct lconv,right_parenthesis)
#else
#define CV_right_parenthesis	(-1)
#endif

#if _mem_decimal_point_lconv
#define CV_decimal_point	offsetof(struct lconv,decimal_point)
#else
#define CV_decimal_point	(-1)
#endif
#if _mem_grouping_lconv
#define CV_grouping		offsetof(struct lconv,grouping)
#else
#define CV_grouping		(-1)
#endif
#if _mem_thousands_sep_lconv
#define CV_thousands_sep	offsetof(struct lconv,thousands_sep)
#else
#define CV_thousands_sep	(-1)
#endif

#ifndef MB_CUR_MIN
#define MB_CUR_MIN		1
#endif

static Keyword_t	keywords[] =
{
{"TEST",		TEST,    	S,1,0},
{"collate",		AST_LC_COLLATE, S,1,CV_collate},
{"charset",		AST_LC_CTYPE,   S,1,CV_charset},
{"mb_cur_max",		AST_LC_CTYPE,   I,1,CV_mb_cur_max},
{"mb_cur_min",		AST_LC_CTYPE,   I,1,CV_mb_cur_min},
{"yesexpr",		AST_LC_MESSAGES,S,1,CV_yesexpr},
{"noexpr",		AST_LC_MESSAGES,S,1,CV_noexpr},
{"yesstr",		AST_LC_MESSAGES,S,1,CV_yesstr},
{"nostr",		AST_LC_MESSAGES,S,1,CV_nostr},
{"credit_sign",		AST_LC_MONETARY,S,1,CV_credit_sign},
{"currency_symbol",	AST_LC_MONETARY,S,1,CV_currency_symbol},
{"debit_sign",		AST_LC_MONETARY,S,1,CV_debit_sign},
{"frac_digits",		AST_LC_MONETARY,C,1,CV_frac_digits},
{"int_curr_symbol",	AST_LC_MONETARY,S,1,CV_int_curr_symbol},
{"int_frac_digits",	AST_LC_MONETARY,C,1,CV_int_frac_digits},
{"left_parenthesis",	AST_LC_MONETARY,S,1,CV_left_parenthesis},
{"mon_decimal_point",	AST_LC_MONETARY,S,1,CV_mon_decimal_point},
{"mon_grouping",	AST_LC_MONETARY,S,1,CV_mon_grouping},
{"mon_thousands_sep",	AST_LC_MONETARY,S,1,CV_mon_thousands_sep},
{"n_cs_precedes",	AST_LC_MONETARY,C,1,CV_n_cs_precedes},
{"n_sep_by_space",	AST_LC_MONETARY,C,1,CV_n_sep_by_space},
{"n_sign_posn",		AST_LC_MONETARY,C,1,CV_n_sign_posn},
{"negative_sign",	AST_LC_MONETARY,S,1,CV_negative_sign},
{"p_cs_precedes",	AST_LC_MONETARY,C,1,CV_p_cs_precedes},
{"p_sep_by_space",	AST_LC_MONETARY,C,1,CV_p_sep_by_space},
{"p_sign_posn",		AST_LC_MONETARY,C,1,CV_p_sign_posn},
{"positive_sign",	AST_LC_MONETARY,S,1,CV_positive_sign},
{"right_parenthesis",	AST_LC_MONETARY,S,1,CV_right_parenthesis},
{"decimal_point",	AST_LC_NUMERIC, S,1,CV_decimal_point},
{"grouping",		AST_LC_NUMERIC, S,1,CV_grouping},
{"thousands_sep",	AST_LC_NUMERIC, S,1,CV_thousands_sep},
{"abday",		AST_LC_TIME,    S,7,TM_DAY_ABBREV*sizeof(char*)},
{"abmon",		AST_LC_TIME,    S,12,TM_MONTH_ABBREV*sizeof(char*)},
{"alt_digits",		AST_LC_TIME,    S,10,TM_DIGITS*sizeof(char*)},
{"am_pm",		AST_LC_TIME,    S,2,TM_MERIDIAN*sizeof(char*)},
{"d_fmt",		AST_LC_TIME,    S,1,TM_DATE*sizeof(char*)},
{"d_t_fmt",		AST_LC_TIME,    S,1,TM_DEFAULT*sizeof(char*)},
{"day",			AST_LC_TIME,    S,7,TM_DAY*sizeof(char*)},
{"era",			AST_LC_TIME,    S,1,TM_ERA*sizeof(char*)},
{"era_d_fmt",		AST_LC_TIME,    S,1,TM_ERA_DATE*sizeof(char*)},
{"era_d_t_fmt",		AST_LC_TIME,    S,1,TM_ERA_DEFAULT*sizeof(char*)},
{"era_t_fmt",		AST_LC_TIME,    S,1,TM_ERA_TIME*sizeof(char*)},
{"era_year",		AST_LC_TIME,    S,1,TM_ERA_YEAR*sizeof(char*)},
{"m_d_old",		AST_LC_TIME,    S,1,TM_DISTANT*sizeof(char*)},
{"m_d_recent",		AST_LC_TIME,    S,1,TM_RECENT*sizeof(char*)},
{"mon",			AST_LC_TIME,    S,12,TM_MONTH*sizeof(char*)},
{"t_fmt",		AST_LC_TIME,    S,1,TM_TIME*sizeof(char*)},
{"t_fmt_ampm",		AST_LC_TIME,    S,1,TM_MERIDIAN_TIME*sizeof(char*)},
};

static struct State_s
{
	Lc_category_t*	categories;
	struct lconv*	conv;
	Dt_t*		dict;
	Dtdisc_t	disc;
} state;

/*
 * list the locale name(s) for lc accoding to flags
 */

static void
list_locale(Sfio_t* sp, Keyword_t* key, Lc_t* lc, unsigned int flags)
{
	register int	i;
	int		n;
	char*		fmt;
	char*		sep;
	char		buf[256];
	
	static unsigned long	types[] = { LC_abbreviated, LC_qualified, LC_verbose, LC_local };

	n = 0;
	for (i = 0; i < elementsof(types); i++)
		if (flags & types[i])
			n++;
	if (!n)
	{
		n++;
		flags |= LC_abbreviated;
	}
	n = n == 1;
	if (key)
	{
		if (flags & LC_indent)
		{
			sfputc(sp, '\t');
			n = 0;
		}
		if (flags & (LC_category|LC_keyword))
		{
			sfprintf(sp, "%s=", key->name);
			n = 0;
		}
	}
	fmt = n ? "%s%s" : "%s\"%s\"";
	sep = "";
	for (i = 0; i < elementsof(types); i++)
		if (flags & types[i])
		{
			lccanon(lc, types[i], buf, sizeof(buf));
			if (n && streq(buf, "-"))
				return;
			sfprintf(sp, fmt, sep, buf);
			sep = ";";
		}
	sfputc(sp, '\n');
}

/*
 * print the numeric value i for key
 */

static void
number(Sfio_t* sp, register Keyword_t* key, int i, unsigned int flags)
{
	if (flags & LC_indent)
		sfputc(sp, '\t');
	if (flags & LC_keyword)
		sfprintf(sp, "%s=%d\n", key->name, i);
	else
		sfprintf(sp, "%d\n", i);
}

/*
 * print one string value, possibly quoted and converted to upper case
 */

static void
value(Sfio_t* sp, register const char* s, register unsigned int flags)
{
	register int	c;
	register int	u;

	if (flags & LC_quote)
		sfputc(sp, '"');
	if (flags & LC_upper)
		while (c = *s++)
		{
			if (islower(c))
				c = toupper(c);
			sfputc(sp, c);
		}
	else if (flags & LC_proper)
	{
		u = 1;
		while (c = *s++)
		{
			if (!isalnum(c))
				u = 1;
			else
			{
				if (u && islower(c))
					c = toupper(c);
				u = 0;
			}
			sfputc(sp, c);
		}
	}
	else
		sfprintf(sp, "%s", s);
	if (flags & LC_quote)
		sfputc(sp, '"');
}

/*
 * print the string value(s) v[n] for key
 */

static void
string(Sfio_t* sp, register Keyword_t* key, char** v, int n, unsigned int flags)
{
	char**	e = v + n - 1;

	if (flags & LC_indent)
		sfputc(sp, '\t');
	if (flags & LC_keyword)
		sfprintf(sp, "%s=", key->name);
	if ((flags & LC_keyword) || v < e)
		flags |= LC_quote;
	value(sp, *v, flags);
	flags |= LC_quote;
	while (v++ < e)
	{
		sfputc(sp, ';');
		value(sp, *v, flags);
	}
	sfputc(sp, '\n');
}

/*
 * extact and list info for key with base info at data
 */

static void
extract(Sfio_t* sp, register Keyword_t* key, void* data, unsigned int flags)
{
	register int	i;
	char*		s;
	char**		v;

	switch (key->type)
	{
	case C:
		if (key->offset >= 0)
			i = *((char*)data + key->offset);
		else
			i = CHAR_MAX;
		if (i == CHAR_MAX)
			i = -1;
		number(sp, key, i, flags);
		break;
	case I:
		if (key->offset >= 0)
			i = *(int*)((char*)data + key->offset);
		else
			i = -1;
		number(sp, key, i, flags);
		break;
	case S:
		if (key->offset >= 0)
		{
			v = (char**)((char*)data + key->offset);
			i = key->elements;
		}
		else
		{
			s = "";
			v = &s;
			i = 1;
		}
		string(sp, key, v, i, flags);
		break;
	}
}

/*
 * list LC_ALL info
 */

static void
list_all(Sfio_t* sp, register Lc_t* lc, unsigned long flags)
{
	register const Lc_attribute_list_t*	ap;

	static int				sep;
	
	if (!lc)
		lc = (Lc_t*)lcinfo(LC_CTYPE)->lc;
	if (!sep)
		sep = 1;
	else
		sfputc(sp, '\n');
	if (flags & LC_category)
		sfprintf(sp, "LC_ALL\n");
	if (flags & LC_indent)
		sfputc(sp, '\t');
	if (flags & LC_keyword)
	{
		flags |= LC_quote;
		sfprintf(sp, "locale=");
	}
	value(sp, lc->code, flags);
	sfputc(sp, '\n');
	if (lc->language)
	{
		if (flags & LC_indent)
			sfputc(sp, '\t');
		if (flags & LC_keyword)
			sfprintf(sp, "language=");
		value(sp, lc->language->name, flags|LC_proper);
		sfputc(sp, '\n');
	}
	if (flags & LC_indent)
		sfputc(sp, '\t');
	if (flags & LC_keyword)
		sfprintf(sp, "territory=");
	value(sp, lc->territory->name, flags|LC_proper);
	sfputc(sp, '\n');
	if (ap = lc->attributes)
	{
		if (flags & LC_indent)
			sfputc(sp, '\t');
		if (flags & LC_keyword)
			sfprintf(sp, "attributes=");
		else if (ap->next)
			flags |= LC_quote;
		value(sp, ap->attribute->name, flags);
		while (ap = ap->next)
		{
			sfputc(sp, ';');
			value(sp, ap->attribute->name, flags);
		}
		sfputc(sp, '\n');
	}
}

static int	scan(Sfio_t*, Keyword_t*, unsigned long);

/*
 * list info for key
 */

static void
list_keyword(Sfio_t* sp, register Keyword_t* key, char* value, unsigned int flags)
{
	register int		i;
	register int		n;
	register unsigned int	f;
	char*			s;

	if ((flags & LC_category) && key->index != AST_LC_ALL)
		sfprintf(sp, "%s\n", state.categories[key->index].name);
	switch (key->index)
	{
	case AST_LC_COLLATE:
		s = mbcoll() ? "strcoll" : "strcmp";
		string(sp, key, &s, 1, flags);
		break;
	case AST_LC_CTYPE:
		switch (key->offset)
		{
		case CV_charset:
			s = (char*)lcinfo(LC_CTYPE)->lc->charset->code;
			string(sp, key, &s, 1, flags|LC_upper);
			break;
		case CV_mb_cur_max:
			number(sp, key, ast.mb_cur_max, flags);
			break;
		case CV_mb_cur_min:
			number(sp, key, MB_CUR_MIN, flags);
			break;
		}
		break;
	case AST_LC_MESSAGES:
#if _lib_nl_langinfo
		if (key->offset >= 0)
			s = nl_langinfo(key->offset);
		else
#endif
			s = "";
		string(sp, key, &s, 1, flags);
		break;
	case AST_LC_MONETARY:
	case AST_LC_NUMERIC:
		if (!state.conv)
			state.conv = localeconv();
		extract(sp, key, state.conv, flags);
		break;
	case AST_LC_TIME:
		extract(sp, key, tmlocale(), flags);
		break;
	case AST_LC_ALL:
		if (value)
		{
			if (streq(value, "-"))
				value = 0;
			if (!setlocale(key->offset, value))
				error(1, "%s: invalid locale", value);
			state.conv = 0;
		}
		else
		{
			if (key->type == AST_LC_ALL)
			{
				if ((flags & (LC_defined|LC_recursive)) == LC_defined)
				{
					scan(sp, key, flags|LC_recursive);
					break;
				}
				i = 1;
				n = AST_LC_COUNT - 1;
			}
			else
				i = n = key->type;
			if (flags & (LC_category|LC_defined|LC_keyword))
				list_all(sp, NiL, flags);
			for (; i <= n; i++)
			{
				f = flags;
				for (key = (Keyword_t*)dtfirst(state.dict); key; key = (Keyword_t*)dtnext(state.dict, key))
					if (key->index == i)
					{
						list_keyword(sp, key, NiL, f);
						f &= ~LC_category;
					}
			}
		}
		break;
	case TEST:
		if (!value)
			error(2, "%s: value expected", key->name);
		else
		{
			if (streq(value, "-"))
				value = 0;
			list_locale(sfstdout, key, lcmake(value), flags);
		}
		break;
	}
}

/*
 * scan all locales matching flags
 */

static int
scan(Sfio_t* sp, Keyword_t* key, unsigned long flags)
{
	register Lc_t*	lc = 0;
	
	while (lc = lcscan(lc))
	{
		switch (flags & (LC_defined|LC_undefined))
		{
		case LC_defined:
			if (!lc->index && !setlocale(LC_MONETARY, lc->code))
				continue;
			break;
		case LC_undefined:
			if (lc->index || setlocale(LC_MONETARY, lc->code))
				continue;
			break;
		}
		if (!key)
			list_locale(sp, NiL, lc, flags);
		else if (setlocale(LC_ALL, lc->code))
			list_keyword(sp, key, NiL, flags&~LC_quote);
	}
	return 0;
}

main(int argc, char** argv)
{
	register char*		name;
	register char*		s;
	register int		i;
	register unsigned int	flags;
	int			collate;
	char*			value;
	Keyword_t*		key;
	char			buf[64];
	char			col[64];
	char			dip[64];

	error_info.id = "locale";
	flags = 0;
	collate = 0;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'a':
			flags |= LC_defined;
			continue;
		case 'b':
			flags |= LC_abbreviated;
			continue;
		case 'c':
			flags |= LC_category;
			continue;
		case 'e':
			collate = 1;
			continue;
		case 'i':
			flags |= LC_indent;
			continue;
		case 'k':
			flags |= LC_keyword;
			continue;
		case 'l':
			flags |= LC_local;
			continue;
		case 'm':
			return execv("/usr/bin/locale", argv);
		case 'q':
			flags |= LC_qualified;
			continue;
		case 'u':
			flags |= LC_undefined;
			continue;
		case 'v':
			flags |= LC_verbose;
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		}
		break;
	}
	if (error_info.errors)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	argv += opt_info.index;
	if (collate)
	{
		while (name = *argv++)
		{
			sfprintf(sfstdout, "%s\t", name);
			sfsprintf(col, sizeof(col), ".%s.]", name);
			if ((i = regcollate(col, NiL, buf, sizeof(buf))) < 0)
			{
				sfprintf(sfstdout, "ERROR\n");
				continue;
			}
			if (!(collate = mbxfrm(col, buf, sizeof(col))))
			{
				if (i > 1)
				{
					sfprintf(sfstdout, "INVALID\n");
					continue;
				}
				collate = i;
				memcpy(col, buf, i);
			}
			else if (i > 1)
			{
				buf[1] = 0;
				if (mbxfrm(dip, buf, sizeof(dip)) != collate)
				{
					sfprintf(sfstdout, "UNDEFINED\n");
					continue;
				}
			}
			sfprintf(sfstdout, "%d %d", i, collate);
			for (i = 0; i < collate; i++)
				sfprintf(sfstdout, " %02x", ((unsigned char*)col)[i]);
			sfputc(sfstdout, '\n');
		}
		return error_info.errors != 0;
	}
	state.categories = lccategories();
	if (!*argv)
	{
		if (flags & (LC_undefined|LC_defined))
		{
			if (!(flags & (LC_abbreviated|LC_qualified|LC_local|LC_verbose)))
				flags |= LC_abbreviated;
			return scan(sfstdout, NiL, flags);
		}
		if (!flags)
		{
			name = "LANG";
			sfprintf(sfstdout, "%s=", name);
			if (!(s = getenv("LANG")))
				s = "POSIX";
			sfprintf(sfstdout, "%s\n", fmtquote(s, "'", NiL, strlen(s), 0));
			value = getenv(state.categories[AST_LC_ALL].name);
			for (i = 1; i < AST_LC_COUNT; i++)
			{
				s = setlocale(state.categories[i].external, NiL);
				sfprintf(sfstdout, "%s=%s\n", state.categories[i].name, fmtquote(s, "\"", "\"", strlen(s), value || !getenv(state.categories[i].name)));
			}
			sfprintf(sfstdout, "%s=", state.categories[0].name);
			if (value)
			{
				s = setlocale(state.categories[0].external, NiL);
				sfprintf(sfstdout, "%s", fmtquote(s, "\"", "\"", strlen(s), 0));
			}
			sfputc(sfstdout, '\n');
			return 0;
		}
	}
	state.disc.key = offsetof(Keyword_t, name);
	state.disc.size = -1;
	state.disc.link = offsetof(Keyword_t, link);
	if (!(state.dict = dtopen(&state.disc, Dttree)))
		error(3, "out of space [dictionary]");
	for (i = 0; i < elementsof(keywords); i++)
		dtinsert(state.dict, keywords + i);
	for (i = 0; i < AST_LC_COUNT; i++)
	{
		if (!(key = newof(0, Keyword_t, 1, 0)))
			error(3, "out of space [keyword]");
		key->name = state.categories[i].name;
		key->index = AST_LC_ALL;
		key->type = state.categories[i].internal;
		key->offset = state.categories[i].external;
		dtinsert(state.dict, key);
	}
	while (name = *argv++)
	{
		if (value = strchr(name, '='))
			*value++ = 0;
		if (!(key = (Keyword_t*)dtmatch(state.dict, name)))
		{
			if (name[0] == 'L' && name[1] == 'C' && name[2] == '_')
				error(2, "%s: unknown category", name);
			else
				error(2, "%s: unknown keyword", name);
		}
		else
			list_keyword(sfstdout, key, value, flags);
	}
	return error_info.errors != 0;
}
