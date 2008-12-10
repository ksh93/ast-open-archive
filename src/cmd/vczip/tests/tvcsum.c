/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2008 AT&T Intellectual Property          *
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
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#include	"vctest.h"
#include	"vccrypto.h"

/* hold test strings */
typedef struct _pair_s
{	Vcchar_t*	str;
	Vcchar_t*	sum;
} Pair_t;

static Pair_t	Md5[] =
{	{ "0123456789", "781E5E245D69B566979B86E28D23F2C7" },
	{ "abcdefghij", "A925576942E94B2EF57A066101B48876" },
	{ 0, 0 }
};

main()
{
	ssize_t		k, n, h;
	Vcchar_t	*sum, hex[1024];
	Vcx_t		xx;

	if(vcxinit(&xx, Vcxmd5sum, 0, 0) < 0)
		terror("Initializing md5 handle");
	for(k = 0; Md5[k].str; ++k)
	{	if((n = vcxencode(&xx, Md5[k].str, strlen(Md5[k].str), &sum)) < 0 )
			terror("Encoding data");
		if(n != 16)
			terror("Bad md5 digest length");
		if((h = vchexcode(sum, n, hex, sizeof(hex), 1)) != 32)
			terror("Bad md5 hex coding length");
		if(strcmp(Md5[k].sum, hex) != 0)
			terror("Bad md5 digest");
	}
	vcxstop(&xx);

	exit(0);
}
