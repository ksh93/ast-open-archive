/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1997-2003 AT&T Corp.                *
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
 * cdb flat file format method
 */

#include <sfio_t.h>

#include "cdblib.h"

#include <expr.h>

#define FLAT_MAGIC_BEG	"!<cdb-flat-"	/* magic string prefix		*/
#define FLAT_MAGIC_END	">\n"		/* magic string suffix		*/

#define TYPES(a,b)	((((a)&~CDB_UNSIGNED)<<8)|((b)&~CDB_UNSIGNED))

typedef struct
{
	Exdisc_t	exdisc;		/* -lexpr discipline		*/
	Expr_t*		expr;		/* expr context			*/
	Exnode_t*	identify;	/* compiled record identifier	*/
	Cdb_t*		cdb;		/* cdb handle			*/
	Exid_t		symbols[1];	/* field id symbol table	*/
} Flatid_t;

typedef struct				/* fixed record active field	*/
{
	Cdbformat_t*	format;		/* field format			*/
	size_t		index;		/* field index			*/
	size_t		offset;		/* raw record offset		*/
	size_t		size;		/* field size			*/
} Flatfield_t;

typedef struct
{
	size_t		active;		/* # active fields		*/
	Flatfield_t	field[1];	/* record id context		*/
} Flatfix_t;

typedef struct
{
	Flatid_t*	id;		/* record id context		*/
	Flatfix_t*	fix;		/* pure fixed record context	*/
} Flat_t;

#define OPT_COMMENT	1
#define OPT_SCHEMA	2

static const Namval_t	details[] =
{
	"invalid",	CDB_INVALID,
	"space",	CDB_SPACE,
	"zero",		CDB_ZERO,
	0,		0
};

static const Namval_t	headers[] =
{
	"comment",	OPT_COMMENT,
	"schema",	OPT_SCHEMA,
	0,		0
};

/*
 * called by stropt() to set method details
 * unknown details get a warning
 */

static int
setdetail(void* h, const void* p, register int n, register const char* v)
{
	register Cdb_t*	cdb = (Cdb_t*)h;
	register int	i;

	if (p)
		switch (i = (int)((Namval_t*)p)->value)
		{
		case CDB_INVALID:
		case CDB_SPACE:
		case CDB_ZERO:
			if (n)
				cdb->options |= i;
			else
				cdb->options &= ~i;
			break;
		default:
			if (cdb->disc->errorf)
				(*cdb->disc->errorf)(cdb, cdb->disc, 1, "%s: %s: unknown %s format detail", cdb->path, ((Namval_t*)p)->name, cdb->meth.name);
			break;
		}
	else if (cdb->disc->errorf)
		(*cdb->disc->errorf)(cdb, cdb->disc, 1, "%s: %s: unknown %s format detail", cdb->path, v, cdb->meth.name);
	return 0;
}

/*
 * called by stropt() to set method header info
 * unknown headers are silently ignored for compatibility
 * the major.minor consistency check handles incompatibilities
 */

static int
setheader(void* h, const void* p, register int n, register const char* v)
{
	register Cdb_t*	cdb = (Cdb_t*)h;

	if (p) switch (((Namval_t*)p)->value)
	{
	case OPT_COMMENT:
		if (n) cdb->disc->comment = vmstrdup(cdb->vm, v);
		break;
	case OPT_SCHEMA:
		if (n && cdbparse(cdb, v) < 0)
			return -1;
		break;
	/* unknown headers are silently ignored for compatibility */
	}
	return 0;
}

/*
 * return -1 if cdb->io is not a flat file
 * io position is not advanced
 */

static int
flatrecognize(register Cdb_t* cdb)
{
	register char*	s;
	register char*	e;
	register char*	m;
	register int	c;
	char*		b;
	int		r;

	if (!(s = b = (char*)sfreserve(cdb->io, SF_UNBOUND, 1)))
		return -1;
	e = s + sfvalue(cdb->io);
	r = -1;
	m = FLAT_MAGIC_BEG;
	while (c = *m++)
	{
		if (s >= e || *s++ != c)
			goto done;
	}
	for (;;)
	{
		if (s >= e)
			goto done;
		if (*s != '.' && !isdigit(*s))
			break;
		s++;
	}
	m = FLAT_MAGIC_END;
	while (c = *m++)
	{
		if (s >= e || *s++ != c)
			goto done;
	}
	r = 1;
 done:
	sfread(cdb->io, b, 0);
	return r;
}

/*
 * cache the cdbimage() prefix and string values for a record that
 * spans buffer boundaries
 */

static int
flatspan(Cdb_t* cdb, char* buf, char* end, Vmalloc_t* vp, Cdbdata_t* odp, Cdbdata_t* oep)
{
	register Cdbrecord_t*	rp;
	register Cdbdata_t*	dp;
	register Cdbdata_t*	ep;
	register char*		s;

	rp = cdb->record;
	if (buf)
	{
		rp->image.size = end - buf;
		if (!(rp->image.data = vmoldof(vp, 0, char, rp->image.size, 0)))
			return cdbnospace(cdb);
		memcpy(rp->image.data, buf, rp->image.size);
	}
	for (; rp->next; rp = rp->next)
		for (ep = (dp = rp->data) + rp->schema->strings; dp < ep; dp++)
			if ((dp->flags & (CDB_STRING|CDB_CACHED)) == CDB_STRING)
			{
				if (!(s = vmoldof(vp, 0, char, dp->string.length, 1)))
					return cdbnospace(cdb);
				dp->string.base = (char*)memcpy(s, dp->string.base, dp->string.length);
				dp->string.base[dp->string.length] = 0;
				dp->flags |= CDB_CACHED|CDB_TERMINATED;
			}
	for (dp = odp, ep = oep; dp < ep; dp++)
		if ((dp->flags & (CDB_STRING|CDB_CACHED)) == CDB_STRING)
		{
			if (!(s = vmoldof(vp, 0, char, dp->string.length, 1)))
				return cdbnospace(cdb);
			dp->string.base = (char*)memcpy(s, dp->string.base, dp->string.length);
			dp->string.base[dp->string.length] = 0;
			dp->flags |= CDB_CACHED|CDB_TERMINATED;
		}
	return 0;
}

/*
 * read and return the next record from the flat file
 */

static Cdbrecord_t*
flatrecread(Cdb_t* cdb, Cdbkey_t* key)
{
	register Cdbrecord_t*	rp;
	register Cdbformat_t*	fp;
	register Cdbdata_t*	dp;
	register Cdbdata_t*	ep;
	register Cdbschema_t*	sp;
	register char*		b;
	register char*		s;
	register char*		t;
	register char*		e;
	char*			x;
	char*			buf;
	char*			end;
	char*			sep;
	char*			vs;
	char*			ve;
	Cdbformat_t*		vf;
	int			c;
	int			d;
	int			k;
	int			n;
	int			m;
	int			q;
	int			w;
	int			op;
	int			delimterm;
	int			level;
	size_t			z;
	size_t			r;
	Cdbrecord_t*		pp;
	Vmalloc_t*		vp;
	Flatid_t*		flat = ((Flat_t*)cdb->details.data)->id;
	Extype_t		v;
	char			fmt[64];
	char			sepbuf[64];

#if !HUH19980101
	cdb->table[0]->head = cdb->table[0];
#endif
	rp = cdb->record;
	rp->next = 0;
	vmclear(vp = rp->vm);
	pp = 0;
	level = 0;
	for (;;)
	{
		rp->offset = sftell(cdb->io);
sfseek(cdb->io, rp->offset, SEEK_SET);
		if (!(buf = (char*)sfreserve(cdb->io, 0, 0)))
			goto eof;
		message((-10, "AHA %4d %5I*d %9I*d o    %9I*d t", __LINE__, sizeof(cdb->count), cdb->count + 1, sizeof(rp->offset), rp->offset, sizeof(Sfoff_t), sftell(cdb->io)));
		end = buf + (r = sfvalue(cdb->io));
		b = s = buf;
		e = end;
		vf = 0;
		sep = 0;
		sp = rp->schema = cdb->table[0];
		fp = sp->format;
		q = sp->terminator.chr;
		delimterm = sp->delimterm;
		z = sp->fixed;
		(dp = rp->data)->record = 0;
		if (op = cdb->sized)
			ep = dp + 1;
		else
		{
			op = (cdb->partitions > 1) ? 'i' : 0;
			ep = dp + cdb->common;
		}
		for (;;)
		{
			for (d = 0; dp < ep; fp++, dp++)
			{
			pop:
				dp->flags = 0;
				n = d;
				if ((d = fp->delimiter.chr) >= 0)
				{
					if ((w = fp->delimiter.dir) > 0)
					{
						if (!sep)
							sep = sepbuf;
						if (sep >= &sepbuf[elementsof(sepbuf)-1])
						{
							if (cdb->disc->errorf)
								(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: too many subfield delimiters -- %d max", cdb->path, elementsof(sepbuf) - 1);
							return 0;
						}
						*sep++ = n;
						*sep = 0;
					}
					else if (w < 0 && sep)
					{
						if (--sep == sepbuf)
							sep = 0;
						else
							*sep = 0;
					}
				}
				/*UNDENT...*/

	if (w = fp->width)
	{
	wider:
		if (w >= (e - b) + (d < 0))
		{
			if (vf)
			{
			pushed:
				if (n = fp - vf)
				{
					if (n < 0)
					{
						if (cdb->disc->errorf)
							(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %I*d: virtual field data too narrow", cdb->path, sizeof(sp->count), sp->count + 1);
						return 0;
					}
					fp -= n;
					dp -= n;
				}
				vf = 0;
				b = s = vs;
				e = ve;
				goto pop;
			}

			/*
			 * the field spans a buffer boundary:
			 * cache previous prefix and string fields and
			 * reserve the next buffer
			 */

			if (flatspan(cdb, buf, end, vp, rp->data, dp) < 0)
				return 0;
			if (!(t = vmoldof(vp, 0, char, w, 1)))
				goto nospace;
			if (w = e - b)
				memcpy(t, b, w);
			if (c = e - buf)
			{
				if (z)
					z -= c;
r = sfvalue(cdb->io);
				if (!sfreserve(cdb->io, c, 0))
					goto eof;
				message((-9, "AHA %4d %5I*d %9I*d c    %9I*d t  %4d", __LINE__, sizeof(cdb->count), cdb->count + 1, sizeof(c), c, sizeof(Sfoff_t), sftell(cdb->io), (int)r));
			}
			if (!(buf = (char*)sfreserve(cdb->io, 0, 0)))
				goto eof;
			end = buf + (r = sfvalue(cdb->io));
			e = z ? (buf + z) : end;
			c = fp->width - w;
			s = buf + c;
			memcpy(t + w, buf, c);
			b = t;
			w = fp->width;
			if (fp->flags & CDB_SPACE)
			{
				for (; isspace(*b); b++, w--);
				for (; w > 0 && isspace(b[w - 1]); w--);
			}
			b[w] = 0;
			dp->flags = (fp->flags & CDB_STRING)|CDB_CACHED|CDB_TERMINATED;
			dp->string.base = b;
			dp->string.length = w;
		}
		else
			s = b + w;
	}
	else
	{
		if (b >= e)
		{
			k = delimterm && *(s - 1) == q;
			if (flatspan(cdb, buf, end, vp, rp->data, dp) < 0)
				return 0;
			if (c = b - buf)
			{
				if (z)
					z -= c;
r = sfvalue(cdb->io);
				if (!sfreserve(cdb->io, c, 0))
					goto eof;
			}
			message((-9, "AHA %4d %5I*d %9I*d c    %9I*d t  %4d", __LINE__, sizeof(cdb->count), cdb->count + 1, sizeof(c), c, sizeof(Sfoff_t), sftell(cdb->io), (int)r));
			if (!(buf = (char*)sfreserve(cdb->io, 0, 0)))
			{
				if (!k)
					goto eof;
				buf = "\n";
				r = 1;
			}
			else
				r = sfvalue(cdb->io);
			end = buf + r;
			e = z ? (buf + z) : end;
			b = buf;
		}
		if (*(unsigned char*)b == fp->quotebegin.chr)
		{
			d = fp->escape.chr;
			n = fp->quotebegin.chr;
			k = fp->quoteend.chr;
			m = n != k;
			for (s = ++b;; s++)
			{
				if (s >= e)
				{
					if (vf)
						goto pushed;
					if (e < end)
						goto narrow;

					/*
					 * the field spans a buffer boundary
					 */

					k = delimterm && *(s - 1) == q;
					if (flatspan(cdb, buf, end, vp, rp->data, dp) < 0)
						return 0;
					if (w = s - b)
					{
						if (!(t = vmoldof(vp, 0, char, w, 0)))
							goto nospace;
						memcpy(t, b, w);
					}
					else
						t = 0;
					if (c = s - buf)
					{
						if (z)
							z -= c;
r = sfvalue(cdb->io);
						if (!sfreserve(cdb->io, c, 0))
							goto eof;
						message((-9, "AHA %4d %5I*d %9I*d c    %9I*d t  %4d r", __LINE__, sizeof(cdb->count), cdb->count + 1, sizeof(c), c, sizeof(Sfoff_t), sftell(cdb->io), (int)r));
					}
					if (!(buf = (char*)sfreserve(cdb->io, 0, 0)))
					{
						if (!k)
							goto eof;
						buf = "\n";
						r = 1;
					}
					else
						r = sfvalue(cdb->io);
					end = buf + r;
					e = z ? (buf + z) : end;
					k = fp->quoteend.chr;
					for (s = b = buf;; s++)
					{
						if (s > e)
						{
							if (cdb->disc->errorf)
								(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: record %I*d/%I*d, offset %I*d: monster record (> %ld) spans at least two buffer boundaries", cdb->path, sizeof(cdb->count), cdb->count + 1, sizeof(sp->count), sp->count + 1, sizeof(rp->offset), rp->offset, r);
							return 0;
						}
						if ((c = *(unsigned char*)s) == k)
						{
							if (--m <= 0)
							{
								w = s - b;
								break;
							}
						}
						else if (c == n)
						{
							if (m)
								m++;
						}
						else if (c == d)
						{
							if ((s + 1) < e)
								s++;
						}
					}
					c = w;
					w += s - b;
					if (!(t = vmoldof(vp, t, char, w, 1)))
						goto nospace;
					memcpy(t + c, b, s - b);
					b = t;
					if (fp->flags & CDB_SPACE)
					{
						for (; isspace(*b); b++, w--);
						for (; w > 0 && isspace(b[w - 1]); w--);
					}
					b[w] = 0;
					dp->flags = (fp->flags & CDB_STRING)|CDB_CACHED|CDB_TERMINATED;
					dp->string.base = b;
					dp->string.length = w;
					if (s < e)
						s++;
					d = fp->delimiter.chr;
					break;
				}
				if ((c = *(unsigned char*)s) == k)
				{
					if (--m <= 0)
					{
						w = s - b;
						if (s < e)
							s++;
						d = fp->delimiter.chr;
						break;
					}
				}
				else if (c == n)
				{
					if (m)
						m++;
				}
				else if (c == d)
				{
					if ((s + 1) < e)
						s++;
				}
			}
		}
		else if (d >= 0 || q >= 0)
		{
			for (s = b;; s++)
			{
				if (s >= e)
				{
					if (vf)
						goto pushed;
					if (e < end)
						goto narrow;

					/*
					 * the field spans a buffer boundary
					 */

					k = delimterm && *(s - 1) == q;
					if (flatspan(cdb, buf, end, vp, rp->data, dp) < 0)
						return 0;
					if (w = s - b)
					{
						if (!(t = vmoldof(vp, 0, char, w, 0)))
							goto nospace;
						memcpy(t, b, w);
					}
					else
						t = 0;
					if (c = s - buf)
					{
						if (z)
							z -= c;
r = sfvalue(cdb->io);
						if (!sfreserve(cdb->io, c, 0))
							goto eof;
					}
					message((-9, "AHA %4d %5I*d %9I*d c    %9I*d t  %4d r", __LINE__, sizeof(cdb->count), cdb->count + 1, sizeof(c), c, sizeof(Sfoff_t), sftell(cdb->io), r));
					if (!(buf = (char*)sfreserve(cdb->io, 0, 0)))
					{
						if (!k)
							goto eof;
						buf = "\n";
						r = 1;
					}
					else
						r = sfvalue(cdb->io);
					end = buf + r;
					e = z ? (buf + z) : end;
					for (s = b = buf;; s++)
					{
						if (s > e)
						{
							if (cdb->disc->errorf)
								(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: record %I*d/%I*d, offset %I*d: monster record (> %ld) spans at least two buffer boundaries", cdb->path, sizeof(cdb->count), cdb->count + 1, sizeof(sp->count), sp->count + 1, rp->offset, r);
							return 0;
						}
						if ((c = *(unsigned char*)s) == d || c == q || sep && strchr(sepbuf, c))
							break;
					}
					c = w;
					w += s - b;
					if (!(t = vmoldof(vp, t, char, w, 1)))
						goto nospace;
					memcpy(t + c, b, s - b);
					b = t;
					if (fp->flags & CDB_SPACE)
					{
						for (; isspace(*b); b++, w--);
						for (; w > 0 && isspace(b[w - 1]); w--);
					}
					b[w] = 0;
					dp->flags = (fp->flags & CDB_STRING)|CDB_CACHED|CDB_TERMINATED;
					dp->string.base = b;
					dp->string.length = w;
					break;
				}
				if ((c = *(unsigned char*)s) == d || c == q || sep && strchr(sepbuf, c))
				{
					w = s - b;
					break;
				}
			}
		}
	}
	if (fp->flags & (CDB_FLOATING|CDB_INTEGER|CDB_STRING))
	{
		if (fp->code != CC_NATIVE)
		{
			if (w)
			{
				if (dp->flags)
					t = b;
				else if (!(t = vmoldof(vp, 0, char, w, 1)))
					goto nospace;
				else
					dp->flags = CDB_CACHED|CDB_TERMINATED;
				b = (char*)ccmapm(t, b, w, fp->code, CC_NATIVE);
				if (fp->flags & CDB_SPACE)
				{
					for (; isspace(*b); b++, w--);
					for (; w > 0 && isspace(b[w - 1]); w--);
				}
				b[w] = 0;
			}
			if (dp->string.length = w)
				dp->flags |= CDB_STRING;
			else
			{
				dp->flags = CDB_CACHED|CDB_TERMINATED;
				b = "";
			}
			dp->string.base = b;
		}
		if (!fp->external)
			switch (fp->ultype)
			{
			case CDB_FLOATING:
				if (!w)
				{
					dp->number.floating = 0.0;
					break;
				}
				if (!dp->flags && ((c = b[w]) == '.' || isdigit(c) || c == 'e' || c == 'E'))
				{
					if (!(t = vmoldof(vp, 0, char, w, 1)))
						goto nospace;
					memcpy(t, b, w);
					t[w] = 0;
					b = t;
					dp->flags |= CDB_STRING|CDB_CACHED|CDB_TERMINATED;
					dp->string.base = b;
					dp->string.length = w;
				}
				dp->number.floating = strtod(b, &x);
				SETFLOATING(dp, fp, (x - b) != w && *x && !isspace(*x));
				break;
			case CDB_INTEGER:
				if (!w)
				{
					dp->number.integer = 0;
					break;
				}
				if (!dp->flags && (fp->base > 10 ? isalnum(b[w]) : isdigit(b[w])))
				{
					if (!(t = vmoldof(vp, 0, char, w, 1)))
						goto nospace;
					memcpy(t, b, w);
					t[w] = 0;
					b = t;
					dp->flags |= CDB_STRING|CDB_CACHED|CDB_TERMINATED;
					dp->string.base = b;
					dp->string.length = w;
				}
				dp->number.integer = strtol(b, &x, fp->base);
				SETINTEGER(dp, fp, (x - b) != w && *x && !isspace(*x));
				break;
			case CDB_UNSIGNED|CDB_INTEGER:
				if (!w)
				{
					dp->number.uinteger = 0;
					break;
				}
				if (!dp->flags && (fp->base > 10 ? isalnum(b[w]) : isdigit(b[w])))
				{
					if (!(t = vmoldof(vp, 0, char, w, 1)))
						goto nospace;
					memcpy(t, b, w);
					t[w] = 0;
					b = t;
					dp->flags |= CDB_STRING|CDB_CACHED|CDB_TERMINATED;
					dp->string.base = b;
					dp->string.length = w;
				}
				dp->number.uinteger = strtoul(b, &x, fp->base);
				SETUINTEGER(dp, fp, (x - b) != w && *x && !isspace(*x));
				break;
			case CDB_LONG|CDB_INTEGER:
				if (!w)
				{
					dp->number.linteger = 0;
					break;
				}
				if (!dp->flags && (fp->base > 10 ? isalnum(b[w]) : isdigit(b[w])))
				{
					if (!(t = vmoldof(vp, 0, char, w, 1)))
						goto nospace;
					memcpy(t, b, w);
					t[w] = 0;
					b = t;
					dp->flags |= CDB_STRING|CDB_CACHED|CDB_TERMINATED;
					dp->string.base = b;
					dp->string.length = w;
				}
				dp->number.linteger = strtoll(b, &x, fp->base);
				SETLINTEGER(dp, fp, (x - b) != w && *x && !isspace(*x));
				break;
			case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
				if (!w)
				{
					dp->number.winteger = 0;
					break;
				}
				if (!dp->flags && (fp->base > 10 ? isalnum(b[w]) : isdigit(b[w])))
				{
					if (!(t = vmoldof(vp, 0, char, w, 1)))
						goto nospace;
					memcpy(t, b, w);
					t[w] = 0;
					b = t;
					dp->flags |= CDB_STRING|CDB_CACHED|CDB_TERMINATED;
					dp->string.base = b;
					dp->string.length = w;
				}
				dp->number.winteger = strtoull(b, &x, fp->base);
				SETWINTEGER(dp, fp, (x - b) != w && *x && !isspace(*x));
				break;
			default:
				if (!dp->flags)
				{
					if (fp->flags & CDB_SPACE)
					{
						for (t = b + w; b < t && isspace(*b); b++, w--);
						for (; w > 0 && isspace(b[w - 1]); w--);
					}
					if (!(dp->string.length = w))
					{
						dp->string.base = "";
						dp->flags |= CDB_CACHED|CDB_TERMINATED;
						break;
					}
					if (!dp->flags && (cdb->flags & CDB_TERMINATED))
					{
						if (!(t = vmoldof(vp, 0, char, w, 1)))
							goto nospace;
						memcpy(t, b, w);
						t[w] = 0;
						b = t;
						dp->flags |= CDB_CACHED|CDB_TERMINATED;
					}
					dp->string.base = b;
					dp->flags |= CDB_STRING;
				}
				break;
			}
		else if ((n = (*fp->external->internalf)(cdb, fp, dp, b, w ? w : e - b, fp->external)) >= 0)
			s += (n - w);
		else if ((w = -(n + 1)) <= 0)
		{
			if (cdb->disc->errorf)
				(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %I*d: %s %s external to internal conversion error", cdb->path, sizeof(sp->count), sp->count + 1, fp->external->name, cdbtypes(cdb, fp->type));
			return 0;
		}
		else
			goto wider;
		if (fp->virtuals)
		{
			vs = s;
			ve = e;
			vf = fp + fp->virtuals + 1;
			if ((fp+1)->virtuals)
			{
				vf--;
				c = (fp->flags & CDB_UNSIGNED) ? 'u' : 'd';
				switch (TYPES(fp->ultype, (fp+1)->ultype))
				{
				case TYPES(CDB_STRING,CDB_INTEGER):
				case TYPES(CDB_STRING,CDB_LONG|CDB_INTEGER):
					dp->number.integer = strtol(dp->string.base, NiL, fp->base);
					/*FALLTHROUGH*/
				case TYPES(CDB_INTEGER,CDB_INTEGER):
				case TYPES(CDB_INTEGER,CDB_LONG|CDB_INTEGER):
				case TYPES(CDB_INTEGER,CDB_STRING):
					if ((fp+1)->width)
					{
						if ((fp+1)->base != 10)
							sfsprintf(fmt, sizeof(fmt), "%%0%d..%dlu", (fp+1)->width, (fp+1)->base);
						else
							sfsprintf(fmt, sizeof(fmt), "%%0%dl%c", (fp+1)->width, c);
					}
					else if ((fp+1)->base != 10)
						sfsprintf(fmt, sizeof(fmt), "%%..%dlu", (fp+1)->base);
					else
						sfsprintf(fmt, sizeof(fmt), "%%l%c", c);
					c = sfprintf(cdb->tmp, fmt, dp->number.integer);
					goto cache;
				case TYPES(CDB_LONG|CDB_INTEGER,CDB_INTEGER):
				case TYPES(CDB_LONG|CDB_INTEGER,CDB_LONG|CDB_INTEGER):
				case TYPES(CDB_LONG|CDB_INTEGER,CDB_STRING):
					if ((fp+1)->width)
					{
						if ((fp+1)->base != 10)
							sfsprintf(fmt, sizeof(fmt), "%%0%d..%dllu", (fp+1)->width, (fp+1)->base);
						else
							sfsprintf(fmt, sizeof(fmt), "%%0%dll%c", (fp+1)->width, c);
					}
					else if ((fp+1)->base != 10)
						sfsprintf(fmt, sizeof(fmt), "%%..%dllu", (fp+1)->base);
					else
						sfsprintf(fmt, sizeof(fmt), "%%ll%c", c);
					c = sfprintf(cdb->tmp, fmt, dp->number.linteger);
					goto cache;
				case TYPES(CDB_STRING,CDB_STRING):
					if ((fp+1)->width)
						sfsprintf(fmt, sizeof(fmt), "%%%d.%ds", (fp+1)->width, dp->string.length);
					else
						sfsprintf(fmt, sizeof(fmt), "%%.%ds", dp->string.length);
					c = sfprintf(cdb->tmp, fmt, dp->string.base);
				cache:
					if (!(t = vmoldof(vp, 0, char, c, 1)))
						goto nospace;
					(dp+1)->string.base = (char*)memcpy(t, sfstruse(cdb->tmp), ((dp+1)->string.length = c) + 1);
					(dp+1)->flags |= CDB_STRING|CDB_CACHED|CDB_TERMINATED;
					break;
				default:
					if (cdb->disc->errorf)
						(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %s => %s virtual map not supported", cdb->path, cdbtypes(cdb, fp->type), cdbtypes(cdb, (fp+1)->type));
					return 0;
				}
				dp++;
				fp++;
				vs = s;
				s = dp->string.base;
				ve = e;
				e = s + dp->string.length;
			}
			else switch (fp->type)
			{
			case CDB_FLOATING:
				s = (char*)&dp->number.floating;
				e = s + sizeof(dp->number.floating);
				break;
			case CDB_INTEGER:
				s = (char*)&dp->number.integer;
				e = s + ((fp->flags & CDB_LONG) ? sizeof(dp->number.linteger) : sizeof(dp->number.integer));
				break;
			default:
				s = dp->string.base;
				e = s + dp->string.length;
				break;
			}
			e++;
		}
	}
	else if (fp->virtuals)
	{
		dp += fp->virtuals;
		fp += fp->virtuals;
	}
	if (s < e && (c = *(unsigned char*)s) == d && (!delimterm || w || q != d || fp > sp->format && (fp-1)->delimiter.chr != d))
		s++;
	b = s;

				/*...INDENT*/
			}
			switch (op)
			{
			case 'z':
			case 'Z':
				w = b - buf;
				z = (dp-1)->number.integer;
				if (op == 'z')
					z += w;
				if (z <= (e - b))
					e = buf + z;
				ep = rp->data + cdb->common;
				op = (cdb->partitions > 1) ? 'i' : 0;
				continue;
			case 'i':
				/*
				 * enough fields have been consumed
				 * to identify the partition
				 */

				v = exeval(flat->expr, flat->identify, rp->data);
				sp = cdb->table[level]->head;
				while (sp->value.string.length)
				{
					switch (flat->identify->type)
					{
					case FLOATING:
						if (sp->value.number.floating == v.floating)
							goto found;
						break;
					case INTEGER:
						if (sp->value.number.integer == v.integer)
							goto found;
						break;
					default:
						if (strmatch(v.string, sp->value.string.base))
							goto found;
						break;
					}
					if (cdb->flags & CDB_RAW)
					{
						if (!(sp = cdb->table[++level]))
							goto noid;
					}
					else if (!(sp = sp->next))
					{
						if (level <= 1)
							goto noid;
						sp = cdb->table[--level]->head;
					}
				}
			found:
				rp->schema = sp;
#if 0
				if (pp)
				{
					if (!(rp = vmnewof(vp, rp, Cdbrecord_t, 1, sp->fields * sizeof(Cdbdata_t))))
						goto nospace;
					rp->data = (Cdbdata_t*)(rp + 1);
				}
#endif
				ep = rp->data + sp->referenced;
				fp = sp->format + (dp - rp->data);
				q = sp->terminator.chr;
				if (!cdb->sized)
					z = sp->fixed;
				if (sp->partition)
					cdb->table[++level]->head = sp->partition;
				op = 0;
				continue;
			}
			break;
		}
		if (z)
		{
			w = z;
			if (q >= 0)
				w++;
		}
		else if (delimterm && q >= 0)
		{
			e = end;
			if (w)
			{
				c = 1;
				for (;;)
				{
					if (s >= e)
					{
						if (!sfreserve(cdb->io, e - buf, 0) || !(buf = (char*)sfreserve(cdb->io, 0, 0)))
						{
							if (c)
							{
								s = buf = 0;
								break;
							}
							goto eof;
						}
						e = (s = buf) + (r = sfvalue(cdb->io));
					}
					if (*s++ == q)
					{
						if (c)
							break;
						c = 1;
					}
					else
						c = 0;
				}
			}
			else if (s < e)
				s++;
			else
				s = buf;
			w = s - buf;
		}
		else
		{
			e = end;
			do
			{
				if (s >= e)
				{
					if (flatspan(cdb, buf, end, vp, rp->data, dp) < 0)
						return 0;
r = sfvalue(cdb->io);
					if (!(s = sfreserve(cdb->io, e - buf, 0)))
						goto eof;
					message((-9, "AHA %4d %5I*d %9I*d c    %9I*d t %4d r", __LINE__, sizeof(cdb->count), cdb->count + 1, sizeof(e - buf), e - buf, sizeof(Sfoff_t), sftell(cdb->io), (int)r));
					if (!(buf = (char*)sfreserve(cdb->io, 0, 0)))
						goto eof;
					e = (s = buf) + (r = sfvalue(cdb->io));
				}
			} while (*s++ != q);
			w = s - buf;
		}
		if (w > 0)
		{
			if (w >= r && flatspan(cdb, NiL, NiL, vp, rp->data, dp) < 0)
				return 0;
			r = sfvalue(cdb->io);
			sfreserve(cdb->io, w, 0);
			message((-9, "AHA %4d %5I*d %9I*d w    %9I*d t  %4I*d r", __LINE__, sizeof(cdb->count), cdb->count + 1, sizeof(w), w, sizeof(Sfoff_t), sftell(cdb->io), sizeof(r), r));
		}
		sp->count++;
		cdb->count++;
		if (level <= 0 || cdb->partitions <= 1 || (cdb->flags & CDB_RAW))
			break;
		if (pp)
			pp->next = rp;
		pp = rp;
		if (!(rp = vmnewof(vp, 0, Cdbrecord_t, 1, cdb->fields * sizeof(Cdbdata_t))))
			goto nospace;
		rp->vm = vp;
		rp->data = (Cdbdata_t*)(rp + 1);
	}
 done:
	if (cdb->flags & CDB_DUMP)
		cdbdump(cdb, sfstderr, "read", cdb->record);
	return cdb->record;
 eof:
	if (pp)
	{
		pp->next = 0;
		goto done;
	}
	if (sfeof(cdb->io) || !sfvalue(cdb->io))
		return 0;
	if (cdb->disc->errorf)
		(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: read error", cdb->path);
	return 0;
 nospace:
	cdbnospace(cdb);
	return 0;
 narrow:
	if (cdb->disc->errorf)
		(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: record %I*d/%I*d, offset %I*d: fixed %s record length %d too narrow for field %d width %d", cdb->path, sizeof(cdb->count), cdb->count + 1, sizeof(sp->count), sp->count + 1, sizeof(rp->offset), rp->offset, sp->name, z, fp - sp->format, w);
	return 0;
 noid:
	sfseek(cdb->io, rp->offset, SEEK_SET);
	if (pp)
		goto done;
	if (cdb->disc->errorf)
		(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %I*u: cannot determine partition", cdb->path, sizeof(cdb->count), cdb->count + 1);
	return 0;
}

/*
 * read and return the next record from the flat file
 * optimized for pure delimited CDB_TERMINATED fields
 */

static Cdbrecord_t*
flatpureread(Cdb_t* cdb, Cdbkey_t* key)
{
	register Cdbformat_t*	fp;
	register Cdbdata_t*	dp;
	register Cdbdata_t*	ep;
	register char*		b;
	register char*		s;
	register char*		e;
	Cdbrecord_t*		rp;
	Cdbschema_t*		sp;
	int			d;

	rp = cdb->record;
	rp->offset = sftell(cdb->io);
	vmclear(rp->vm);
	sp = rp->schema = cdb->table[0];
	if (!(s = sfgetr(cdb->io, sp->terminator.chr, 1)))
		goto eof;

	/*
	 * NOTE: hack alert
	 *
	 * sfgetr() restores the previous record separator
	 * but the last field in that record may still be
	 * used by the caller
	 *
	 * setting cdb->io->getr=0 gets around this but makes
	 * it impossible to seek back and sfgetr() again
	 *
	 * the tests that install flatpureread under CDB_OPEN
	 * must take this into account or a monotonic increasing
	 * offset check could be used to kick in flatrecread() 
	 * just before the sfgetr() above
	 */

	cdb->io->getr = 0;

	/*
	 * NOTE: back to normal hacks
	 */

	e = s + sfvalue(cdb->io) - 1;
	fp = sp->format;
	dp = rp->data;
	ep = dp + sp->referenced;
	for (; dp < ep; fp++, dp++)
	{
		d = fp->delimiter.chr;
		for (b = s;; s++)
			if (*s == d)
			{
				d = 0;
				break;
			}
			else if (*s == 0)
			{
				if (s < e)
					d = 0;
				break;
			}
		dp->flags = 0;
		if (fp->flags & (CDB_FLOATING|CDB_INTEGER|CDB_STRING))
		{
			if (!fp->external)
				switch (fp->ultype)
				{
				case CDB_FLOATING:
					dp->number.floating = strtod(b, NiL);
					dp->flags |= CDB_FLOATING;
					break;
				case CDB_INTEGER:
					dp->number.integer = strtol(b, NiL, fp->base);
					dp->flags |= CDB_INTEGER;
					break;
				case CDB_UNSIGNED|CDB_INTEGER:
					dp->number.uinteger = strtoul(b, NiL, fp->base);
					dp->flags |= CDB_UNSIGNED|CDB_INTEGER;
					break;
				case CDB_LONG|CDB_INTEGER:
					dp->number.linteger = strtoll(b, NiL, fp->base);
					dp->flags |= CDB_LONG|CDB_INTEGER;
					break;
				case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
					dp->number.winteger = strtoull(b, NiL, fp->base);
					dp->flags |= CDB_UNSIGNED|CDB_LONG|CDB_INTEGER;
					break;
				default:
					if (fp->flags & CDB_SPACE)
					{
						for (; *b && isspace(*b); b++);
						for (; s > b && isspace(*(s - 1)); s--);
					}
					*s = 0;
					dp->string.base = b;
					dp->flags |= (dp->string.length = s - b) ? (CDB_STRING|CDB_TERMINATED) : CDB_TERMINATED;
					break;
				}
			else if ((*fp->external->internalf)(cdb, fp, dp, b, s - b, fp->external) < 0)
				return 0;
		}
		s += !d;
	}
	cdb->count++;
	sp->count++;
	if (cdb->flags & CDB_DUMP)
		cdbdump(cdb, sfstderr, "pureread", cdb->record);
	return cdb->record;
 eof:
	if (sfeof(cdb->io))
		return 0;
	if (cdb->disc->errorf)
		(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: read error", cdb->path);
	return 0;
}

/*
 * read and return the next record from the flat file
 * optimized for fixed length records
 */

static Cdbrecord_t*
flatfixread(Cdb_t* cdb, Cdbkey_t* key)
{
	Flatfix_t*		fix = ((Flat_t*)cdb->details.data)->fix;
	register Flatfield_t*	xp;
	register Flatfield_t*	ep;
	register Cdbformat_t*	fp;
	register Cdbdata_t*	dp;
	register char*		b;
	register int		w;
	char*			s;
	char*			t;
	int			c;
	Cdbrecord_t*		rp;
	Cdbschema_t*		sp;
	Vmalloc_t*		vp;

	rp = cdb->record;
	rp->offset = sftell(cdb->io);
	vmclear(vp = rp->vm);
	sp = rp->schema = cdb->table[0];
	if (key && cdb->disc->indexf && (cdb->disc->indexf)(cdb, key, cdb->disc) < 0)
		return 0;
	for (;;)
	{
		if (sp->terminator.chr >= 0)
		{
			if (!(s = sfgetr(cdb->io, sp->terminator.chr, 0)))
				goto eof;
			if (sfvalue(cdb->io) == sp->fixed)
				break;
			if (cdb->disc->errorf)
				(*cdb->disc->errorf)(cdb, cdb->disc, 1, "%s: invalid record length %d offset %I*d -- ignored", cdb->path, sfvalue(cdb->io), sizeof(rp->offset), rp->offset);
			rp->offset += sfvalue(cdb->io);
		}
		else
		{
			if (!(s = sfreserve(cdb->io, sp->fixed, 0)))
				goto eof;
			break;
		}
	}
	for (ep = (xp = fix->field) + fix->active; xp < ep; xp++)
	{
		b = s + xp->offset;
		fp = xp->format;
		w = xp->size;
		dp = rp->data + xp->index;
		dp->flags = 0;
		if (fp->code != CC_NATIVE)
		{
			if (!(t = vmoldof(vp, 0, char, w, 1)))
				goto nospace;
			b = (char*)ccmapm(t, b, w, fp->code, CC_NATIVE);
			if (fp->flags & CDB_SPACE)
			{
				for (; isspace(*b); b++, w--);
				for (; w > 0 && isspace(b[w - 1]); w--);
			}
			b[w] = 0;
			dp->flags = CDB_CACHED|CDB_TERMINATED;
			dp->string.length = w;
			dp->flags |= CDB_STRING;
			dp->string.base = b;
		}
		if (!fp->external)
			switch (fp->ultype)
			{
			case CDB_FLOATING:
				if (!dp->flags && ((c = b[w]) == '.' || isdigit(c) || c == 'e' || c == 'E'))
				{
					if (!(t = vmoldof(vp, 0, char, w, 1)))
						goto nospace;
					memcpy(t, b, w);
					t[w] = 0;
					b = t;
					dp->flags |= CDB_STRING|CDB_CACHED|CDB_TERMINATED;
					dp->string.base = b;
					dp->string.length = w;
				}
				dp->number.floating = strtod(b, &t);
				SETFLOATING(dp, fp, (t - b) != w);
				break;
			case CDB_INTEGER:
				if (!dp->flags && (fp->base > 10 ? isalnum(b[w]) : isdigit(b[w])))
				{
					if (!(t = vmoldof(vp, 0, char, w, 1)))
						goto nospace;
					memcpy(t, b, w);
					t[w] = 0;
					b = t;
					dp->flags |= CDB_STRING|CDB_CACHED|CDB_TERMINATED;
					dp->string.base = b;
					dp->string.length = w;
				}
				dp->number.integer = strtol(b, &t, fp->base);
				SETINTEGER(dp, fp, (t - b) != w);
				break;
			case CDB_UNSIGNED|CDB_INTEGER:
				if (!dp->flags && (fp->base > 10 ? isalnum(b[w]) : isdigit(b[w])))
				{
					if (!(t = vmoldof(vp, 0, char, w, 1)))
						goto nospace;
					memcpy(t, b, w);
					t[w] = 0;
					b = t;
					dp->flags |= CDB_STRING|CDB_CACHED|CDB_TERMINATED;
					dp->string.base = b;
					dp->string.length = w;
				}
				dp->number.uinteger = strtoul(b, &t, fp->base);
				SETUINTEGER(dp, fp, (t - b) != w);
				break;
			case CDB_LONG|CDB_INTEGER:
				if (!dp->flags && (fp->base > 10 ? isalnum(b[w]) : isdigit(b[w])))
				{
					if (!(t = vmoldof(vp, 0, char, w, 1)))
						goto nospace;
					memcpy(t, b, w);
					t[w] = 0;
					b = t;
					dp->flags |= CDB_STRING|CDB_CACHED|CDB_TERMINATED;
					dp->string.base = b;
					dp->string.length = w;
				}
				dp->number.linteger = strtoll(b, &t, fp->base);
				SETLINTEGER(dp, fp, (t - b) != w);
				break;
			case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
				if (!dp->flags && (fp->base > 10 ? isalnum(b[w]) : isdigit(b[w])))
				{
					if (!(t = vmoldof(vp, 0, char, w, 1)))
						goto nospace;
					memcpy(t, b, w);
					t[w] = 0;
					b = t;
					dp->flags |= CDB_STRING|CDB_CACHED|CDB_TERMINATED;
					dp->string.base = b;
					dp->string.length = w;
				}
				dp->number.winteger = strtoull(b, &t, fp->base);
				SETWINTEGER(dp, fp, (t - b) != w);
				break;
			default:
				if (!dp->flags)
				{
					if (!dp->flags && (cdb->flags & CDB_TERMINATED))
					{
						if (!(t = vmoldof(vp, 0, char, w, 1)))
							goto nospace;
						memcpy(t, b, w);
						t[w] = 0;
						b = t;
						dp->flags |= CDB_CACHED|CDB_TERMINATED;
					}
					dp->string.base = b;
					dp->string.length = w;
					dp->flags |= CDB_STRING;
				}
				break;
			}
		else
			(*fp->external->internalf)(cdb, fp, dp, b, w, fp->external);
	}
	cdb->count++;
	sp->count++;
	if (cdb->flags & CDB_DUMP)
		cdbdump(cdb, sfstderr, "fixread", cdb->record);
	return cdb->record;
 eof:
	if (sfeof(cdb->io) || !sfvalue(cdb->io))
		return 0;
	if (cdb->disc->errorf)
		(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %I*d: read error", cdb->path, sizeof(sp->count), sp->count);
	return 0;
 nospace:
	cdbnospace(cdb);
	return 0;
}

/*
 * fill in delimiters on skipped fields from pp up to fp
 */

static void
delimit(Sfio_t* op, register Cdbformat_t* pp, register Cdbformat_t* fp)
{
	register Cdbformat_t*	vp;

	for (vp = pp;; pp++)
	{
		if (pp >= fp)
		{
			pp = vp;
			break;
		}
		if (pp->delimiter.dir < 0)
			break;
	}
	for (; pp < fp; pp++)
		if (pp->delimiter.chr >= 0)
			sfputc(op, pp->delimiter.chr);
}

/*
 * write the record in data to the flat file
 */

static int
flatrecwrite(register Cdb_t* cdb, Cdbkey_t* key, Cdbrecord_t* rp)
{
	register Sfio_t*	op;
	register Cdbformat_t*	fp;
	register Cdbdata_t*	dp;
	register Cdbdata_t*	ep;
	register Cdbformat_t*	pp;
	register Cdbschema_t*	sp;
	Sfio_t*			io;
	int			sized;
	char*			s;
	int			n;
	int			c;
	size_t			z;
	char			buf[1024];

	rp->offset = sftell(cdb->io);
	if (cdb->flags & CDB_DUMP)
		cdbdump(cdb, sfstderr, "write", rp);
	do
	{
		sp = rp->schema;
		fp = sp->format;
		dp = rp->data;
		if (sized = cdb->sized)
		{
			io = cdb->cvt;
			dp++;
			fp++;
		}
		else
			io = cdb->io;
		pp = fp;
		op = io;
		ep = rp->data + sp->fields;
		for (;;)
		{
			for (; dp < ep; dp++, fp++)
			{
				if (fp->virtuals)
				{
					if (pp < fp)
						delimit(op, pp, fp);
					pp = fp + fp->virtuals;
					if ((fp+1)->virtuals)
					{
						dp++;
						fp++;
						pp++;
					}
					ep = rp->data + (pp - sp->format) + 1;
					op = cdb->tmp;
					sized = 'v';
					continue;
				}
				if (!fp->external)
					switch (fp->type)
					{
					case CDB_FLOATING:
						if (fp->width || (dp->flags & CDB_FLOATING))
						{
							if (pp < fp)
							{
								delimit(op, pp, fp);
								pp = fp;
							}
							if (fp->width)
							{
								if (fp->base < 10)
									sfsprintf(buf, sizeof(buf), "%%0%d.%df", fp->width, fp->base);
								else
									sfsprintf(buf, sizeof(buf), "%%0%dg", fp->width);
							}
							else if (fp->base != 10)
								sfsprintf(buf, sizeof(buf), "%%.%df", fp->base);
							else
								sfsprintf(buf, sizeof(buf), "%%g");
							if (fp->code == CC_NATIVE)
								sfprintf(op, buf, dp->number.floating);
							else
							{
								n = sfprintf(cdb->tmp, buf, dp->number.floating);
								s = sfstruse(cdb->tmp);
								ccmaps(s, n, CC_NATIVE, fp->code);
								sfwrite(op, s, n);
							}
						}
						break;
					case CDB_INTEGER:
						if (fp->width || (dp->flags & CDB_INTEGER))
						{
							c = (fp->flags & CDB_UNSIGNED) ? 'u' : 'd';
							s = (fp->flags & CDB_LONG) ? (sizeof(Sflong_t) > sizeof(long) ? "ll" : "l") : (sizeof(Cdbint_t) > sizeof(int) ? "l" : "");
							if (pp < fp)
							{
								delimit(op, pp, fp);
								pp = fp;
							}
							if (fp->width)
							{
								if (fp->base != 10)
									sfsprintf(buf, sizeof(buf), "%%0%d..%d%su", fp->width, fp->base, s);
								else
									sfsprintf(buf, sizeof(buf), "%%0%d%s%c", fp->width, s, c);
							}
							else if (fp->base != 10)
								sfsprintf(buf, sizeof(buf), "%%..%d%su", fp->base, s);
							else
								sfsprintf(buf, sizeof(buf), "%%%s%c", s, c);
							if (fp->flags & CDB_LONG)
							{
								if (fp->code == CC_NATIVE)
									sfprintf(op, buf, dp->number.linteger);
								else
								{
									n = sfprintf(cdb->tmp, buf, dp->number.linteger);
									s = sfstruse(cdb->tmp);
									ccmaps(s, n, CC_NATIVE, fp->code);
									sfwrite(op, s, n);
								}
							}
							else
							{
								if (fp->code == CC_NATIVE)
									sfprintf(op, buf, dp->number.integer);
								else
								{
									n = sfprintf(cdb->tmp, buf, dp->number.integer);
									s = sfstruse(cdb->tmp);
									ccmaps(s, n, CC_NATIVE, fp->code);
									sfwrite(op, s, n);
								}
							}
						}
						break;
					default:
						if (fp->width || (dp->flags & CDB_STRING) && dp->string.length)
						{
							if (pp < fp)
							{
								delimit(op, pp, fp);
								pp = fp;
							}
							if (dp->string.length)
							{
								if (fp->code == CC_NATIVE)
									sfwrite(op, dp->string.base, dp->string.length);
								else if (!(s = (char*)sfreserve(op, dp->string.length, 0)))
									return -1;
								else
									ccmapm(s, dp->string.base, dp->string.length, CC_NATIVE, fp->code);
							}
							if ((n = fp->width - dp->string.length) > 0)
							{
								c = ccmapc(' ', CC_NATIVE, fp->code);
								while (n-- > 0)
									sfputc(op, c);
							}
						}
						break;
					}
				else if ((n = (*fp->external->externalf)(cdb, fp, dp, buf, sizeof(buf), fp->external)) < 0)
				{
					if (cdb->disc->errorf)
						(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %I*d: %s %s internal to external conversion error", cdb->path, sizeof(sp->count), sp->count, fp->external->name, cdbtypes(cdb, fp->type));
					return -1;
				}
				else
				{
					if (pp < fp)
					{
						delimit(op, pp, fp);
						pp = fp;
					}
					sfwrite(op, buf, n);
					if ((n = fp->width - n) > 0)
					{
						c = ccmapc(' ', CC_NATIVE, fp->code);
						while (n-- > 0)
							sfputc(op, c);
					}
				}
			}
			switch (sized)
			{
			case 'v':
				n = sftell(op);
				s = sfstruse(op);
				op = io;
				if (pp < (fp - 1))
				{
					delimit(op, pp, fp - 1);
					pp = fp - 1;
				}
				if (n)
				{
					ccmaps(s, n, CC_NATIVE, fp->code);
					sfwrite(op, s, n);
				}
				if ((n = fp->width - n) > 0)
				{
					c = ccmapc(' ', CC_NATIVE, fp->code);
					while (n-- > 0)
						sfputc(op, c);
				}
				ep = rp->data + sp->fields;
				sized = cdb->sized;
				continue;
			case 'Z':
				pp = fp = sp->format;
				dp = rp->data;
				z = dp->number.integer = sfstrtell(op);
				ep = dp + 1;
				op = cdb->tmp;
				sized = 'X';
				continue;
			case 'X':
				pp = fp = sp->format;
				dp = rp->data;
				dp->number.integer += sfstrtell(op) + (fp->delimiter.chr >= 0);
				sfstrset(op, 0);
				op = cdb->io;
				sized = 'x';
				continue;
			case 'z':
				pp = fp = sp->format;
				dp = rp->data;
				z = dp->number.integer = sfstrtell(op);
				ep = dp + 1;
				op = cdb->io;
				sized = 'x';
				continue;
			case 'x':
				if (sp->format->delimiter.chr >= 0)
					sfputc(op, sp->format->delimiter.chr);
				sfwrite(op, sfstrbase(cdb->cvt), z);
				sfstrset(cdb->cvt, 0);
				break;
			}
			break;
		}
		if (sp->terminator.chr >= 0 && !sp->fixed)
		{
			sfputc(op, sp->terminator.chr);
			if (sp->delimterm)
				sfputc(op, sp->terminator.chr);
		}
		sp->count++;
		cdb->count++;
	} while (rp = rp->next);
	return 1;
}

/*
 * flat file record seek
 */

static Sfoff_t
flatrecseek(Cdb_t* cdb, Sfoff_t off)
{
	if (cdb->meth.flags & CDB_STACK)
	{
		if (cdb->disc->errorf)
			(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %s seek not implemented", cdb->path, cdb->meth.name);
		return -1;
	}
	if (off < 0)
		return cdb->record->offset;
	return sfseek(cdb->io, off, SEEK_SET);
}

/*
 * read the flat file header
 */

static int
flathdrread(register Cdb_t* cdb)
{
	register char*	s;
	char*		e;

	/*
	 * check for optional magic number and header info
	 */

	if (flatrecognize(cdb) > 0)
	{
		if (!(s = sfgetr(cdb->io, '\n', 0)))
			goto badheader;
		s += sizeof(FLAT_MAGIC_BEG) - 1;
		cdb->major = strtol(s, &e, 10);
		s = e + 1;
		cdb->minor = strtol(s, &e, 10);
		if (cdb->major != CDB_MAJOR)
		{
			if (cdb->disc->errorf)
				(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %s version %d.%d data not supported by implementation version %d.%d", cdb->meth.name, cdb->path, cdb->major, cdb->minor, CDB_MAJOR, CDB_MINOR);
			return -1;
		}
		for (;;)
		{
			if (!(s = sfgetr(cdb->io, '\n', 1)))
				goto badheader;
			if (!*s)
				break;
			if (stropt(s, headers, sizeof(*headers), setheader, cdb) < 0)
				return -1;
		}
		cdb->flags |= CDB_HEADER;
	}
	return 0;
 badheader:
	if (cdb->disc->errorf)
		(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: bad header", cdb->path);
	return -1;
}

/*
 * write the flat header if requested
 */

static int
flathdrwrite(register Cdb_t* cdb)
{
	if (cdb->flags & CDB_HEADER)
	{
		sfprintf(cdb->io, "%s%d.%d%s", FLAT_MAGIC_BEG, CDB_MAJOR, CDB_MINOR, FLAT_MAGIC_END);
		sfprintf(cdb->io, "comment='%s'\n", cdb->comment);
		sfprintf(cdb->io, "schema='%s'\n", cdb->schema);
		/* optional headers here */
		sfputc(cdb->io, '\n');
		if (sferror(cdb->io))
			return -1;
	}
	return 0;
}

/*
 * return value for symbol sym
 */

static Extype_t
flatgetval(Expr_t* pgm, Exnode_t* node, Exid_t* sym, Exref_t* ref, void* env, int elt, Exdisc_t* disc)
{
	register Cdb_t*		cdb = ((Flatid_t*)disc)->cdb;
	register Cdbdata_t*	dp = (Cdbdata_t*)env + sym->index;
	register char*		s;
	Extype_t		v;

	switch (sym->type)
	{
	case FLOATING:
		v.floating = dp->number.floating;
		break;
	case INTEGER:
		v.integer = dp->number.integer;
		break;
	default:
		if (!(dp->flags & CDB_TERMINATED))
		{
			if (s = vmoldof(cdb->record->vm, 0, char, dp->string.length, 1))
			{
				dp->string.base = (char*)memcpy(s, dp->string.base, dp->string.length);
				dp->string.base[dp->string.length] = 0;
				dp->flags |= CDB_CACHED|CDB_TERMINATED;
			}
			else
				cdbnospace(cdb);
		}
		v.string = dp->string.base;
		break;
	}
	return v;
}

/*
 * handle method events
 */

static int
flatevent(register Cdb_t* cdb, int op)
{
	register Flatid_t*	fid;
	register Flatfix_t*	fix;
	register int		i;
	register Exid_t*	ip;
	register Cdbformat_t*	fp;
	register Cdbschema_t*	rp;
	Cdbformat_t*		vp;
	Cdbschema_t**		tp;
	Flat_t*			flat;
	size_t			n;
	int			simple;

	switch (op)
	{
	case CDB_METH:
		if (cdb->disc->details)
		{
			if (cdb->defopts)
			{
				cdb->defopts = 0;
				cdb->options = 0;
			}
			if (stropt(cdb->disc->details, details, sizeof(*details), setdetail, cdb) < 0)
				return -1;
		}
		break;
	case CDB_INIT:
		if (!(flat = vmnewof(cdb->vm, 0, Flat_t, 1, 0)))
			goto nospace;
		cdb->details.data = (void*)flat;
		if (cdb->identify)
		{
			/*
			 * compile the record identification function
			 */

			if (!(fid = vmnewof(cdb->vm, 0, Flatid_t, 1, sizeof(Exid_t) * cdb->common)))
				goto nospace;
			flat->id = fid;
			fid->cdb = cdb;
			for (i = 0, ip = fid->symbols, fp = cdb->table[0]->format; i < cdb->common; i++, ip++, fp++)
			{
				sfsprintf(ip->name, sizeof(ip->name), "$%d", i + 1);
				ip->lex = ID;
				ip->index = i;
				switch (fp->type)
				{
				case CDB_FLOATING:
					ip->type = FLOATING;
					break;
				case CDB_INTEGER:
					ip->type = INTEGER;
					break;
				default:
					ip->type = STRING;
					break;
				}
			}
			fid->exdisc.version = EX_VERSION;
			fid->exdisc.flags = EX_CHARSTRING|EX_FATAL|EX_UNDECLARED;
			fid->exdisc.symbols = fid->symbols;
			fid->exdisc.errorf = cdb->disc->errorf;
			fid->exdisc.getf = flatgetval;
			if (!(fid->expr = exopen(&fid->exdisc)))
				goto nospace;
			if (excomp(fid->expr, NiL, 0, cdb->identify, NiL) || !(fid->identify = exexpr(fid->expr, NiL, NiL, 0)))
			{
				if (cdb->disc->errorf)
					(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: record identification expression compile error", cdb->path);
				return -1;
			}
			for (tp = cdb->table; rp = *tp; tp++)
				switch (fid->identify->type)
				{
				case FLOATING:
					rp->value.number.floating = strtod(rp->value.string.base, NiL);
					break;
				case INTEGER:
					rp->value.number.integer = strtol(rp->value.string.base, NiL, 10);
					break;
				}
		}
		break;
	case CDB_OPEN:
		/*
		 * if any virtual fields are accessed then
		 * the original field must be too
		 */

		simple = cdb->partitions == 1;
		for (tp = cdb->table; rp = *tp; tp++)
		{
			fp = rp->format;
			if (cdb->sized)
				fp->flags |= fp->type;
			for (; fp < rp->format + rp->fields; fp++)
				if (fp->virtuals)
				{
					simple = 0;
					if (!(fp->flags & (CDB_FLOATING|CDB_INTEGER|CDB_STRING)))
					{
						vp = fp + 1;
						if (vp->virtuals)
							vp->flags &= ~(CDB_FLOATING|CDB_INTEGER|CDB_STRING);
						for (vp = fp + 1; vp < fp + fp->virtuals + 1; vp++)
							if (!vp->virtuals && (vp->flags & (CDB_FLOATING|CDB_INTEGER|CDB_STRING)))
							{
								fp->flags |= fp->type;
								vp = fp + 1;
								if (vp->virtuals)
									vp->flags |= vp->type;
								break;
							}
					}
				}
		}
		if (simple)
		{
			if (!(cdb->flags & CDB_TEST2) && cdb->table[0]->fixed)
			{
				flat = (Flat_t*)cdb->details.data;
				if (!flat->id)
				{
					rp = cdb->table[0];
					i = 0;
					for (fp = rp->format; fp < rp->format + rp->fields; fp++)
						if (fp->flags)
							i++;
					if (!(fix = vmnewof(cdb->vm, 0, Flatfix_t, 1, i * sizeof(Flatfield_t))))
						goto nospace;
					fix->active = i;
					flat->fix = fix;
					i = 0;
					n = 0;
					for (fp = rp->format; fp < rp->format + rp->fields; fp++)
					{
						if (fp->flags)
						{
							fix->field[i].format = fp;
							fix->field[i].index = fp - rp->format;
							fix->field[i].offset = n;
							fix->field[i].size = fp->width;
							i++;
						}
						n += fp->width;
					}
					cdb->meth.recreadf = flatfixread;
				}
			}
			else if ((cdb->flags & CDB_TEST2) && cdb->table[0]->variable)
			{
				if (cdb->strings && !cdb->sized && (cdb->flags & CDB_TERMINATED))
					cdb->meth.recreadf = flatpureread;
			}
		}
		break;
	case CDB_CLOSE:
		if (cdb->details.data && (fid = ((Flat_t*)cdb->details.data)->id) && fid->expr)
			exclose(fid->expr, 1);
		break;
	}
	return 0;
 nospace:
	cdbnospace(cdb);
	return -1;
}

Cdbmeth_t	_Cdbflat =
{
	"flat",
	"flat file database",
	".db",
	0,
	flatevent,
	flatrecognize,
	flathdrread,
	flathdrwrite,
	flatrecread,
	flatrecwrite,
	flatrecseek,
	0,
	&_Cdbvdelta
};

__DEFINE__(Cdbmeth_t*, Cdbflat, &_Cdbflat);
