#include	"dthdr.h"

/*	Hashing a string
**
**	Written by Kiem-Phong Vo (05/22/96)
*/
#if __STD_C
ulong dtstrhash(reg ulong h, reg char* s, reg int n)
#else
ulong dtstrhash(h,s,n)
reg ulong	h;
reg char*	s;
reg int		n;
#endif
{
	if(n <= 0)
	{	for(; (n = *((unsigned char*)s)) != 0; ++s)
			h = dtcharhash(h,n);
	}
	else
	{	reg char*	ends;
		for(ends = s+n; s < ends; ++s)
		{	n = *((unsigned char*)s);
			h = dtcharhash(h,n);
		}
	}

	return h;
}
