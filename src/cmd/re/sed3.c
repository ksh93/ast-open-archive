/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1995-2003 AT&T Corp.                *
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

#include "sed.h"

#include <sfstr.h>

Text retemp;	/* holds a rewritten regex, without delimiter */

int
recomp(Text *rebuf, Text *t, int sub)
{
	static int lastre;
	int code;
	int c;
	int n;
	if(!(c = *t->w) || c == '\n' || !(n = *(t->w + 1)) || n == '\n')
		syntax("unterminated regular expression");
	else if (c != n) {
		assure(rebuf, sizeof(regex_t));
		if (code = regcomp((regex_t*)rebuf->w,(char*)t->w,reflags|REG_DELIMITED|REG_MUSTDELIM|((reflags&REG_LENIENT)?0:REG_ESCAPE)))
			badre((regex_t*)rebuf->w,code);
		lastre = rebuf->w - rebuf->s;
		t->w += ((regex_t*)rebuf->w)->re_npat;
		rebuf->w += sizeof(regex_t);
	} else if(rebuf->w == rebuf->s)
		syntax("no previous regular expression");
	else {
		if (sub) {
			assure(rebuf, sizeof(regex_t));
			if (code = regdup(readdr(lastre), (regex_t*)rebuf->w))
				badre((regex_t*)rebuf->w,code);
			lastre = rebuf->w - rebuf->s;
			rebuf->w += sizeof(regex_t);
		}
		t->w += 2;
	}
	return lastre;
}

void
reerror(regex_t* re, int code)
{
	if(code && code != REG_NOMATCH) {
		char buf[UCHAR_MAX+1];
		regerror(code, re, buf, sizeof(buf));
		error(3, "regular expression execution error: %s", buf);
	}
}

int
reexec(regex_t* re, char* s, size_t nmatch, regmatch_t* match, int flags)
{
	int code;
	if((code = regexec(re, s, nmatch, match, flags)) && code != REG_NOMATCH)
		reerror(re, code);
	return code;
}

int
substitute(regex_t *re, Text *data)
{
	int n;
	regmatch_t matches[10];
	if(reexec(re, (char*)data->s, elementsof(matches), matches, 0))
		return 0;
	if(n = regsubexec(re, (char*)data->s, elementsof(matches), matches)) {
		reerror(re, n);
		return 0;
	}
	n = re->re_sub->re_len;
	assure(data, n+1);
	memcpy(data->s, re->re_sub->re_buf, n+1);
	data->w = data->s + n;
	return 1;
}
