/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1997-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*     If you received this software without first entering     *
*       into a license with AT&T, you have an infringing       *
*           copy and cannot use it without violating           *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*             Glenn Fowler <gsf@research.att.com>              *
*                                                              *
***************************************************************/
#pragma prototyped

/*
 * cdb schema map support
 */

#include "cdblib.h"

/*
 * open a map handle and initialize from mapstr
 */

Cdbmap_t*
cdbmapopen(Cdb_t* cdb, register const char* s, Cdbmapmeth_t* meth)
{
	register Cdbmap_t*	mp;
	register Cdbformat_t*	fp;
	register int		c;
	register int		k;
	register int		n;
	int			i;
	int			m;
	char*			b;
	char*			e;
	char*			r;
	unsigned long		z;
	Cdbschema_t*		sp;
	Vmalloc_t*		vp;
	Cdbconvert_t		cvt;

	if (!(vp = vmopen(Vmdcheap, Vmlast, 0)))
		goto nospace;
	if (!(mp = vmnewof(vp, 0, Cdbmap_t, 1, (cdb->partitions - 1) * sizeof(Cdbmapschema_t))))
		goto nospace;
	mp->vm = vp;
	if (meth)
		mp->meth = *meth;

	/*
	 * strip off the map options
	 */

	if (s)
		for (;;)
		{
			switch (c = *s++)
			{
			case 'd':
				mp->flags |= CDB_MAP_DELIMIT;
				continue;
			case 'i':
				mp->flags |= CDB_MAP_IGNORE;
				continue;
			case 'n':
				mp->flags |= CDB_MAP_NATIVE;
				continue;
			case 'p':
				mp->flags |= CDB_MAP_PHYSICAL;
				continue;
			case 't':
				mp->flags |= CDB_MAP_TYPES;
				continue;
			case 'v':
				mp->flags |= CDB_MAP_VIRTUAL;
				continue;
			case 'z':
				mp->flags |= CDB_MAP_SIZED;
				continue;
			case '{':
				b = (char*)s;
				n = 1;
				for (;; sfputc(cdb->tmp, c))
				{
					switch (c = *s++)
					{
					case 0:
						if (cdb->disc->errorf)
							(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: unbalanced {...}", cdb->path);
						goto bad;
					case '{':
						n++;
						continue;
					case '}':
						if (--n <= 0)
							break;
						continue;
					default:
						continue;
					}
					break;
				}
				b = sfstruse(cdb->tmp);
				if (e = strchr(b, ':'))
					*e++ = 0;
				if (!(meth = cdbgetmap(b)))
				{
					if (cdb->disc->errorf)
						(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %s: map method not found", cdb->path, b);
					goto bad;
				}
				if (e && !(mp->details = vmstrdup(mp->vm, e)))
					goto nospace;
				continue;
			default:
				s--;
				break;
			}
			break;
		}
	if (!mp->flags)
		mp->flags = CDB_MAP_DEFAULT;
	r = (mp->flags & CDB_MAP_SIZED) ? "x-*" : "-*";
	if (!s || !*s)
		s = r;

	/*
	 * run through the remaining map string placing Cdbconvert_t
	 * items on cdb->tmp, keeping offsets in mp->schema[i].offset
	 */

	memset(&cvt, 0, sizeof(cvt));
	m = n = 0;
	sp = cdb->table[0];
	mp->schema[0].ifields = sp->fields;
	mp->schema[0].offset = 0;
	i = 1;
	for (;;)
	{
		switch (c = *s++)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			n = n * 10 + (c - '0');
			if (n > sp->fields)
			{
				if (cdb->disc->errorf)
					(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %d%s: map field too big -- %d max", cdb->path, n, s, sp->fields);
				goto bad;
			}
			continue;
		case '-':
			if (m)
			{
				if (cdb->disc->errorf)
					(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %s: invalid map range", cdb->path, s - 1);
				goto bad;
			}
			m = n ? n : (cvt.input.index + 1);
			n = 0;
			continue;
		case '*':
		case '+':
			if (!m && !n)
			{
				if (i >= cdb->partitions)
				{
					if (cdb->disc->errorf)
						(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %s: too many partitions in map -- %d max", cdb->path, s - 1, cdb->partitions);
					goto bad;
				}
				mp->schema[i - 1].ofields = cvt.output.index;
				cvt.input.index = cvt.output.index = CDB_MAP_NIL;
				sfwrite(cdb->tmp, &cvt, sizeof(cvt));
				sp = cdb->table[i];
				mp->schema[i].ifields = sp->fields;
				cvt.input.index = cvt.output.index = 0;
				mp->schema[i++].offset = sfstrtell(cdb->tmp);
				if (c == '*')
					s = r;
				continue;
			}
			c = *--s;
			/*FALLTHROUGH*/
		case 0:
		case ',':
		case '.':
		case ';':
		case ':':
		case ' ':
		case '\t':
		case '\n':
			if (m || n)
			{
				if (!n)
					n = sp->fields;
				else if (!m)
					m = n;
				if (m > n)
				{
					if (cdb->disc->errorf)
						(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: map range must be increasing", cdb->path);
					goto bad;
				}
				m--;
				n--;
				cvt.input.index = m;
				m = m < n;
				do
				{
					if (m)
					{
						fp = sp->format + cvt.input.index;
						if (fp->type == CDB_IGNORE && !(mp->flags & CDB_MAP_IGNORE))
							continue;
						if ((fp->flags & CDB_PHYSICAL) && !(mp->flags & CDB_MAP_PHYSICAL))
						{
							if (!(mp->flags & CDB_MAP_VIRTUAL))
								cvt.input.index += fp->virtuals;
							else if ((fp+1)->virtuals)
								cvt.input.index++;
							continue;
						}
						if ((fp->flags & CDB_VIRTUAL) && !(mp->flags & CDB_MAP_VIRTUAL))
							continue;
					}
					sfwrite(cdb->tmp, &cvt, sizeof(cvt));
					cvt.output.index++;
				} while (cvt.input.index++ < n);
				m = n = 0;
			}
			if (!c || c == '*' && i >= cdb->partitions)
				break;
			continue;
		case 'x':
		case 'X':
			if (m || n)
			{
				if (cdb->disc->errorf)
					(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %s: `%c' cannot be a map range endpoint", cdb->path, s - 1, c);
				goto bad;
			}
			c = cvt.input.index;
			cvt.input.index = CDB_MAP_NIL;
			sfwrite(cdb->tmp, &cvt, sizeof(cvt));
			cvt.input.index = c;
			cvt.output.index++;
			continue;
		case 'd':
		case 'D':
		case 'i':
		case 'I':
		case 'n':
		case 'N':
		case 'p':
		case 'P':
		case 't':
		case 'T':
		case 'v':
		case 'V':
		case '{':
			if (cdb->disc->errorf)
				(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %s: `%c' must appear before first field map", cdb->path, s - 1, c);
			goto bad;
		default:
			if (cdb->disc->errorf)
				(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %s: invalid character in map", cdb->path, s - 1);
			goto bad;
		}
		break;
	}
	if (i < cdb->partitions)
	{
		if (cdb->disc->errorf)
			(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: too few partitions in map -- have %d, need %d", cdb->path, i, cdb->partitions);
		goto bad;
	}
	mp->schema[i - 1].ofields = cvt.output.index;
	cvt.input.index = cvt.output.index = CDB_MAP_NIL;
	sfwrite(cdb->tmp, &cvt, sizeof(cvt));
	z = sfstrtell(cdb->tmp);
	if (!(b = (char*)vmalloc(vp, z)))
		goto nospace;
	memcpy(b, sfstrbase(cdb->tmp), z);
	sfstrset(cdb->tmp, 0);
	if (meth)
		mp->meth = *meth;
	for (i = 0; i < cdb->partitions; i++)
	{
		mp->schema[i].convert = (Cdbconvert_t*)(b + mp->schema[i].offset);
		for (k = 0; k < cdb->common; k++)
			mp->schema[i].convert[k] = mp->schema[0].convert[k];
		if (cdb->flags & CDB_VERBOSE)
		{
			sfprintf(sfstderr, "map %s record ifields=%d ofields=%d %s", cdb->table[i]->name, mp->schema[i].ifields, mp->schema[i].ofields, cdbmapflags(cdb, mp->flags));
			if (mp->meth.name)
				sfprintf(sfstderr, " using %s", mp->meth.name);
			sfputc(sfstderr, '\n');
			for (k = 0; mp->schema[i].convert[k].output.index >= 0; k++)
				sfprintf(sfstderr, "    %03d => %03d\n", mp->schema[i].convert[k].input.index, mp->schema[i].convert[k].output.index);
		}
	}
	return mp;
 nospace:
	cdbnospace(cdb);
 bad:
	if (vp)
		vmclose(vp);
	sfstrset(cdb->tmp, 0);
	return 0;
}

/*
 * close a map handle
 */

int
cdbmapclose(Cdbmap_t* map)
{
	return map ? vmclose(map->vm) : -1;
}
