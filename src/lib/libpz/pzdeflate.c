/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1998-2000 AT&T Corp.              *
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

#include "pzlib.h"

/*
 * pz deflate from pz->io to op
 */

int
pzdeflate(register Pz_t* pz, Sfio_t* op)
{
	register Pzpart_t*	pp;
	register int		i;
	register int		j;
	register size_t		n;
	register size_t		m;
	register unsigned char*	buf;
	register unsigned char*	wrk;
	register unsigned char*	pat;
	register unsigned char*	low;
	unsigned char*		vp;
	unsigned char*		ve;
	ssize_t			r;
	int			peek;
	Pzread_f		readf;
	Pzindex_f		indexf;
	Pzindex_t		index;
	Sfio_t*			tmp;
	
	if (!(pz->flags & PZ_WRITE))
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%s: cannot deflate -- not open for write", pz->path);
		return -1;
	}
	if (pzheadwrite(pz, op))
		return -1;
	if (pz->flags & PZ_NOPZIP)
	{
		n = pz->part->row;
		if (readf = pz->disc->readf)
			for (;;)
			{
				if (!(buf = (unsigned char*)sfreserve(op, n, 1)))
				{
					if (pz->disc->errorf)
						(*pz->disc->errorf)(pz, pz->disc, 2, "%s: output write error", pz->path);
					return -1;
				}
				if ((r = (*readf)(pz, pz->io, buf, pz->disc)) < 0)
					return -1;
				if (sfwrite(op, buf, r) != r)
				{
					if (pz->disc->errorf)
						(*pz->disc->errorf)(pz, pz->disc, 2, "%s: output write error", pz->path);
					return -1;
				}
				if (r < n)
					break;
			}
		else
			r = sfmove(pz->io, op, SF_UNBOUND, -1);
		if (r < 0 || sferror(pz->io))
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: read error", pz->path);
			return -1;
		}
		if (r > 0 && r % n)
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: last record incomplete", pz->path);
		}
		if (sfsync(op))
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: output write error", pz->path);
			return -1;
		}
		return 0;
	}
	if (pz->split.flags & PZ_SPLIT_DEFLATE)
		return pzsdeflate(pz, op);

	/*
	 * deflate each window
	 */

	pp = pz->part;
	readf = pz->disc->readf;
	indexf = (pz->disc->version >= PZ_VERSION_SPLIT) ? pz->disc->indexf : (Pzindex_f)0;
	wrk = pz->wrk;
	pat = pz->pat;
	low = pp->low;
	tmp = pz->tmp;
	peek = 0;
	for (;;)
	{
		if (peek)
		{
			peek = 0;
			r = pp->row;
		}
		else if ((r = readf ? (*readf)(pz, pz->io, pat, pz->disc) : sfread(pz->io, pat, pp->row)) != pp->row)
		{
			if (r && pz->disc->errorf)
			{
				if (r > 0)
					(*pz->disc->errorf)(pz, pz->disc, 2, "last record incomplete");
				else
					(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "read error");
			}
			break;
		}
		if (indexf)
		{
			sfraise(op, SFGZ_GETPOS, &index.block);
			index.offset = 0;
			if ((*indexf)(pz, &index, pat, pz->disc) < 0)
				break;
		}

		/*
		 * collect a window of hi frequency cols in buf
		 * and encode the lo frequency rows in tmp+pz->val
		 * lo frequency values must not exceed pp->loq
		 */

		for (i = 0; i < pp->nfix; i++)
			pat[pp->fix[i]] = pp->value[pp->fix[i]];
		ve = (vp = pz->val) + pp->loq - 2 * pp->row;
		memcpy(vp, pat, pp->row);
		vp += pp->row;
		buf = pz->buf;
		for (j = 0; j < pp->nmap; j++)
			*buf++ = pat[pp->map[j]];
		m = 1;
		for (n = 1; n < pp->col; n++)
		{
			if (readf)
			{
				if ((r = (*readf)(pz, pz->io, wrk, pz->disc)) != pp->row)
					break;
			}
			else if ((r = sfread(pz->io, wrk, pp->row)) != pp->row)
			{
				if (r && pz->disc->errorf)
				{
					if (r > 0)
						(*pz->disc->errorf)(pz, pz->disc, 2, "last record incomplete");
					else
						(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "read error");
				}
				break;
			}
			for (j = 0; j < pp->row; j++)
				if (pat[j] != wrk[j] && low[j])
				{
					if (vp >= ve)
					{
						memcpy(pat, wrk, pp->row);
						peek = 1;
						goto dump;
					}
					sfputu(tmp, m);
					sfputu(tmp, j + 1);
					*vp++ = pat[j] = wrk[j];
					m = 0;
					while (++j < pp->row)
						if (pat[j] != wrk[j] && low[j])
						{
							sfputu(tmp, j + 1);
							*vp++ = pat[j] = wrk[j];
						}
					sfputu(tmp, 0);
					break;
				}
			m++;
			for (j = 0; j < pp->nmap; j++)
				*buf++ = wrk[pp->map[j]];
			if (indexf)
			{
				index.offset += pp->row;
				if ((*indexf)(pz, &index, wrk, pz->disc) < 0)
					break;
			}
		}
	dump:
		if (pz->flags & PZ_SECTION)
			pz->count.sections++;
		else
		{
			pz->count.windows++;
			pz->count.records += n;
		}
		sfputu(tmp, m);
		sfputu(tmp, 0);

		/*
		 * transpose the hi frequency from row major to col major
		 * and write it by group to op
		 */

		if (pp->nmap)
		{
			pp->mix[0] = buf = pz->wrk;
			m = 0;
			for (j = 1; j < pp->nmap; j++)
			{
				m += n;
				pp->mix[j] = (pp->lab[j] == pp->lab[j - 1]) ?  (pp->mix[j - 1] + 1) : (buf + m);
			}
			buf = pz->buf;
			for (i = 0; i < n; i++)
				for (j = 0; j < pp->nmap; j++)
				{
					*pp->mix[j] = *buf++;
					pp->mix[j] += pp->inc[j];
				}
			m = n * pp->nmap;
			sfputu(op, m);
			buf = pz->wrk;
			for (i = 0; i < pp->ngrp; i++)
			{
				m = n * pp->grp[i];
				if (sfwrite(op, buf, m) != m || sfsync(op))
				{
					if (pz->disc->errorf)
						(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "hi frequency write error");
					return -1;
				}
				buf += m;
			}
		}
		else
		{
			/*
			 * this is a phony size that is verified on inflate
			 * 0 here would terminate the inflate loop in the
			 * first window
			 */

			sfputu(op, 1);
		}

		/*
		 * now write the lo frequency encoding
		 */

		m = vp - pz->val;
		sfputu(op, m);
		if (sfwrite(op, pz->val, m) != m || sfsync(op))
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "lo frequency value write error");
			return -1;
		}
		m = sfstrtell(tmp);
		if (sfwrite(op, sfstrset(tmp, 0), m) != m || sfsync(op))
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "lo frequency code write error");
			return -1;
		}
	}
	sfputu(op, 0);
	if (!(pz->flags & PZ_SECTION))
	{
		sfputc(op, 1);
		if (pz->disc->eventf && (*pz->disc->eventf)(pz, PZ_TAILWRITE, op, 0, pz->disc) < 0)
			return -1;
		sfputc(op, 0);
	}
	if (sfsync(op))
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "write error");
		return -1;
	}
	return 0;
}
