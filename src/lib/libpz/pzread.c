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

#include "pzlib.h"

/*
 * read a buffer from a pz stream
 * this code mirrors pzinflate()
 */

ssize_t
pzread(register Pz_t* pz, void* buf, size_t z)
{
	register Pzpart_t*	pp;
	register int		i;
	register int		j;
	register int		k;
	register size_t		m;
	ssize_t			n;
	ssize_t			w;
	unsigned char*		ob;
	unsigned char*		om;
	Pzwrite_f		writef;

	pp = pz->part;
	if (z >= pp->row)
		n = (z / pp->row) * pp->row;
	else if (!(pz->flags & PZ_FORCE))
		return -1;
	else
		n = z;
	if (writef = pz->disc->writef)
	{
		if (sfstrtmp(pz->str, SF_WRITE, buf, z))
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "cannot initailize tmp stream");
			return -1;
		}
		if (!pz->wrk && !(pz->wrk = vmnewof(pz->vm, 0, unsigned char, pz->mrow, 0)))
			return -1;
		ob = pz->wrk;
		w = -1;
	}
	else
	{
		ob = (unsigned char*)buf;
		om = ob + n;
	}
	if (pz->flags & PZ_FORCE)
	{
		if (writef)
		{
			m = pz->part->row;
			if (!(ob = (unsigned char*)sfreserve(pz->io, m, 0)))
			{
				if (sfvalue(pz->io))
				{
					if (pz->disc->errorf)
						(*pz->disc->errorf)(pz, pz->disc, 2, "%s: data corrupted", pz->path);
					return -1;
				}
				return 0;
			}
			if ((n = (*writef)(pz, pz->str, ob, pz->disc)) <= 0)
				return -1;
			if (z < n)
				return -1;
			w = (z / n) * n;
			while ((w -= n) > 0)
			{
				if (!(ob = (unsigned char*)sfreserve(pz->io, m, 0)))
				{
					if (sfvalue(pz->io))
					{
						if (pz->disc->errorf)
							(*pz->disc->errorf)(pz, pz->disc, 2, "%s: data corrupted", pz->path);
						return -1;
					}
					break;
				}
				if ((n = (*writef)(pz, pz->str, ob, pz->disc)) <= 0)
					return -1;
			}
			return (ssize_t)sfstrtell(pz->str);
		}
		else
			return sfread(pz->io, buf, n);
	}
	if (pz->prefix.count)
	{
		if (pz->prefix.data)
		{
			if (z < (m = pz->prefix.count))
				m = z;
			memcpy(buf, pz->prefix.data, m);
			pz->prefix.count -= m;
			pz->prefix.data += m;
			return m;
		}
		pz->prefix.count = 0;
	}
	for (;;)
	{
		if (!pz->rs.hi)
		{
			if (pz->rs.lo)
				return 0;
			if (!(m = sfgetu(pz->io)))
			{
				switch (pzfile(pz))
				{
				case 0:
					pz->rs.lo = 1;
					goto done;
				case 1:
					continue;
				default:
					return -1;
				}
			}
			if (pp->nmap)
			{
				if (m > pz->win || (m % pp->nmap) || sfread(pz->io, pz->buf, m) != m)
				{
					if (pz->disc->errorf)
						(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%s: data corrupted", pz->path);
					return -1;
				}
				n = m / pp->nmap;
				m = 0;
				j = 0;
				k = 0;
				for (i = 0; i < pp->nmap; i++)
				{
					if (i > 0 && pp->lab[i] == pp->lab[i - 1])
						j++;
					else
						j = m;
					if (!pp->value || pp->value[i] < 0)
						pp->mix[k++] = pz->buf + j;
					m += n;
				}
			}
			else if (m != 1)
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%s: data corrupted", pz->path);
				return -1;
			}
			m = sfgetu(pz->io);
			if (m < pp->row || sfread(pz->io, pz->pat, pp->row) != pp->row)
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%s: data corrupted", pz->path);
				return -1;
			}
			m -= pp->row;
			if (sfread(pz->io, pz->nxt = pz->val, m) != m)
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%s: data corrupted", pz->path);
				return -1;
			}
			pz->rs.hi = k;
			PZGETZ(pz, pz->io, pz->rs.lo, i);
		}
		for (;;)
		{
			while (pz->rs.lo > 0)
			{
				pz->rs.lo--;
				memcpy(ob, pz->pat, pp->row);
				for (i = 0; i < pz->rs.hi; i++)
				{
					ob[pp->map[i]] = *pp->mix[i];
					pp->mix[i] += pp->inc[i];
				}
				if (writef)
				{
					if ((n = (*writef)(pz, pz->str, ob, pz->disc)) <= 0)
						return -1;
					if (w < 0)
						w = (z / n) * (n - 1);
					if ((w -= n) <= 0)
						goto done;
				}
				else if ((ob += pp->row) >= om)
					goto done;
			}
			PZGETP(pz, pz->io, m, i, break);
			for (;;)
			{
				pz->pat[m - 1] = *pz->nxt++;
				PZGETP(pz, pz->io, m, i, break);
			}
			PZGETZ(pz, pz->io, pz->rs.lo, i);
		}
		pz->rs.hi = 0;
	}
 done:
	return writef ? (ssize_t)sfstrtell(pz->str) : (ob - (unsigned char*)buf);
}
