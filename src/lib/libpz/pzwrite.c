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
 * write a buffer to a pz stream
 */

ssize_t
pzwrite(register Pz_t* pz, Sfio_t* op, const void* buf, size_t n)
{
	register int		i;
	register int		j;
	register unsigned char*	bp;
	register Pzpart_t*	pp;
	register unsigned char*	pat;
	register unsigned char*	low;
	unsigned char*		be;
	size_t			k;
	ssize_t			r;
	Sfio_t*			tmp;

	if (!(pz->flags & PZ_WRITE))
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, 2, "%s: cannot deflate -- not open for write", pz->path);
		return -1;
	}
	if (!n)
		return 0;
	pp = pz->part;

	/*
	 * this could be relaxed with a side buffer that caches partial
	 * rows between pzwrite() calls -- for now we take the easy way
	 */

	if (n % pp->row)
	{
		if (pz->disc->errorf)
			(*pz->disc->errorf)(pz, pz->disc, 2, "%s: write buffer size %I*u must be a multiple of the row size %I*u", pz->path, sizeof(n), n, sizeof(pp->row), pp->row);
		return -1;
	}
	if (pzheadwrite(pz, op))
		return -1;
	if (pz->flags & PZ_NOPZIP)
	{
		if ((r = sfwrite(op, buf, n)) != n)
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: write error", pz->path);
			return -1;
		}
		return r;
	}
	bp = (unsigned char*)buf;
	be = bp + n;
	pat = pz->pat;
	tmp = pz->tmp;
	low = pp->low;
	for (;;)
	{
		if (!pz->ws.bp)
		{
			/*
			 * initialize for a new window
			 */

			pz->ws.io = op;
			memcpy(pat, bp, pp->row);
			bp += pp->row;
			for (i = 0; i < pp->nfix; i++)
				pat[pp->fix[i]] = pp->value[pp->fix[i]];
			pz->ws.ve = (pz->ws.vp = pz->val) + pp->loq - 2 * pp->row;
			memcpy(pz->ws.vp, pat, pp->row);
			pz->ws.vp += pp->row;
			pz->ws.bp = pz->buf;
			for (j = 0; j < pp->nmap; j++)
				*pz->ws.bp++ = pat[pp->map[j]];
			pz->ws.rep = pz->ws.row = 1;
		}

		/*
		 * collect a window of hi frequency cols in buf
		 * and encode the lo frequency rows in tmp+pz->val
		 * lo frequency values must not exceed pp->loq
		 */

		k = pz->ws.row + (be - bp) / pp->row;
		if (k > pp->col)
			k = pp->col;
		while (pz->ws.row < k)
		{
			for (j = 0; j < pp->row; j++)
				if (pat[j] != bp[j] && low[j])
				{
					if (pz->ws.vp >= pz->ws.ve)
						goto dump;
					sfputu(tmp, pz->ws.rep);
					sfputu(tmp, j + 1);
					*pz->ws.vp++ = pat[j] = bp[j];
					pz->ws.rep = 0;
					while (++j < pp->row)
						if (pat[j] != bp[j] && low[j])
						{
							sfputu(tmp, j + 1);
							*pz->ws.vp++ = pat[j] = bp[j];
						}
					sfputu(tmp, 0);
					break;
				}
			for (j = 0; j < pp->nmap; j++)
				*pz->ws.bp++ = bp[pp->map[j]];
			pz->ws.rep++;
			pz->ws.row++;
			bp += pp->row;
		}
		if (bp < be || k >= pp->col)
			break;
	dump:
		if (pzsync(pz))
			return -1;
	}
	return n;
}
