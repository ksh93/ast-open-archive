/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2008 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
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
#include <ast.h>
#include <bgp.h>

#define TOTAL		1024
#define ALIGN		16

int
main()
{
	Bgproute_t	bgp;
	unsigned int	total;
	unsigned int	fixed;
	unsigned int	pad;
	unsigned int	data;

	total = sizeof(Bgproute_t);
	fixed = offsetof(Bgproute_t, pad);
	if (!(pad = ALIGN - fixed % ALIGN))
		pad = ALIGN;
	data = TOTAL - (fixed + pad);
	if (total == TOTAL && pad == sizeof(bgp.pad) && data == sizeof(bgp.data))
		sfprintf(sfstdout, "%u\n", total);
	else
		sfprintf(sfstdout, "edit bgp.h and adjust padding dimensions to pad[%u] and data[%u] for data aligned to %d bytes and a total size of %u\n", pad, data, ALIGN, TOTAL);
	return 0;
}
