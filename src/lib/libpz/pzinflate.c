/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1998-2001 AT&T Corp.                *
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

#include "pzlib.h"

/*
 * restore lo+hi into ob
 */

static int
restore(register Pz_t* pz, Pzpart_t* pp, register Sfio_t* ip, Sfio_t* op, register unsigned char* pat, register unsigned char* buf, size_t row, size_t m, register size_t* map, register unsigned char** mix, register size_t* inc)
{
	register size_t	z;
	register int	i;
	Pzwrite_f	writef;

	writef = pz->disc->writef;
	for (;;)
	{
		PZGETZ(pz, ip, z, i);
		do
		{
			memcpy(buf, pat, row);
			for (i = 0; i < m; i++)
			{
				buf[map[i]] = *mix[i];
				mix[i] += inc[i];
			}
			if (writef)
			{
				if ((*writef)(pz, op, buf, pz->disc) < 0)
					return -1;
			}
			else if (sfwrite(op, buf, row) != row)
			{
				if (pz->disc->errorf)
					(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "write error");
				return -1;
			}
		} while (--z);
		PZGETP(pz, ip, z, i, break);
		for (;;)
		{
			pat[z - 1] = *pz->nxt++;
			PZGETP(pz, ip, z, i, break);
		}
	}
	return 0;
}

/*
 * pz inflate from pz->io to op
 */

int
pzinflate(register Pz_t* pz, Sfio_t* op)
{
	register Pzpart_t*	pp;
	register int		i;
	register int		j;
	register int		k;
	register size_t		n;
	register size_t		m;
	register unsigned char*	pat;
	ssize_t			r;
	Pzwrite_f		writef;

	if (!(pz->flags & PZ_READ))
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "%s: cannot inflate -- not open for read", pz->path);
		return -1;
	}
	if (pz->flags & PZ_SPLIT)
		return pzssplit(pz);
	if (pz->flags & PZ_FORCE)
	{
		if (writef = pz->disc->writef)
		{
			n = pz->part->row;
			do
			{
				if (!(pat = (unsigned char*)sfreserve(pz->io, n, 0)))
				{
					if (sfvalue(pz->io))
					{
						if (pz->disc->errorf)
							(*pz->disc->errorf)(pz, pz->disc, 2, "%s: data corrupted", pz->path);
						return -1;
					}
					break;
				}
			} while ((r = (*writef)(pz, op, pat, pz->disc)) >= 0);
			if (r < 0)
				return -1;
		}
		else if (sfmove(pz->io, op, SF_UNBOUND, -1) < 0 || sferror(pz->io))
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: data corrupted", pz->path);
			return -1;
		}
		if (sfsync(op))
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: output write error", pz->path);
			return -1;
		}
		return 0;
	}

	/*
	 * copy the prefix
	 */

	if (pz->prefix.count)
	{
		if (!pz->prefix.skip && pz->prefix.data && sfwrite(op, pz->prefix.data, pz->prefix.count) != pz->prefix.count)
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: output write error", pz->path);
			return -1;
		}
		pz->prefix.count = 0;
	}
	if (pz->split.flags & PZ_SPLIT_INFLATE)
		return pzsinflate(pz, op);

	/*
	 * inflate each file
	 */

	do
	{
		/*
		 * inflate each window
		 */

		pp = pz->part;
		pat = pz->pat;
		while (m = sfgetu(pz->io))
		{
			/*
			 * hi frequency data in pz->buf
			 */

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

			/*
			 * lo frequency
			 */

			m = sfgetu(pz->io);
			if (m < pp->row || sfread(pz->io, pat, pp->row) != pp->row)
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

			/*
			 * restore lo+hi on op
			 */

			if (restore(pz, pp, pz->io, op, pat, pz->wrk, pp->row, k, pp->map, pp->mix, pp->inc))
				return -1;
		}
		if (sfsync(op))
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, ERROR_SYSTEM|2, "write error");
			return -1;
		}
	} while ((i = !(pz->flags & PZ_SECTION)) && (i = pzfile(pz)) > 0);
	return i;
}
