/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1998-2002 AT&T Corp.                *
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
	size_t			x;
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
	if (pzheadwrite(pz, op))
		return -1;
	if (pz->flags & PZ_NOPZIP)
	{
		if ((r = sfwrite(op, buf, n)) < 0)
		{
			if (pz->disc->errorf)
				(*pz->disc->errorf)(pz, pz->disc, 2, "%s: write error", pz->path);
			return -1;
		}
		return r;
	}
	pp = pz->part;
	if (pz->ws.pc || n < pp->row)
	{
		if (!pz->ws.pb && !(pz->ws.pb = vmnewof(pz->vm, 0, unsigned char, pp->row, 0)))
			return -1;
		x = pp->row - pz->ws.pc;
		if (x > n)
			x = n;
		memcpy(pz->ws.pb + pz->ws.pc, buf, x);
		if ((pz->ws.pc += x) < pp->row)
			return x;
		pz->ws.pc = 0;
		if (pzwrite(pz, op, pz->ws.pb, pp->row) != pp->row)
			return -1;
		if (!(n -= x))
			return x;
	}
	else
		x = 0;
	bp = (unsigned char*)buf + x;
	be = bp + n;
	if (k = n % pp->row)
	{
		if (!pz->ws.pb && !(pz->ws.pb = vmnewof(pz->vm, 0, unsigned char, pp->row, 0)))
			return -1;
		x += k;
		n -= k;
		be -= k;
		memcpy(pz->ws.pb + pz->ws.pc, be, k);
		pz->ws.pc += k;
	}
	pat = pz->pat;
	tmp = pz->tmp;
	low = pp->low;
	while (bp < be)
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
		if (k < pp->col)
			continue;
	dump:
		if (pzsync(pz))
			return -1;
	}
	return n + x;
}
