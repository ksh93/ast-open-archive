/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1995-2000 AT&T Corp.              *
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

#include "sed.h"

void docopy(unsigned char *where, int n);
int dosub(unsigned char *where, unsigned char *rp);

Text retemp;	/* holds a rewritten regex, without delimiter */

int
recomp(Text *rebuf, Text *t)
{
	static int lastre;
	int code;
	int c;
	int n;
	int delim = *t->w;
	if(!(c = *t->w) || c == '\n' || !(n = *(t->w + 1)) || n == '\n')
		syntax("unterminated regular expression");
	else if (c != n) {
		assure(rebuf, sizeof(regex_t));
		if(code=regcomp((regex_t*)rebuf->w,(char*)t->w,reflags|REG_DELIMITED|((reflags&REG_LENIENT)?0:REG_ESCAPE)))
			badre((regex_t*)rebuf->w,code);
		lastre = rebuf->w - rebuf->s;
		t->w += ((regex_t*)rebuf->w)->re_npat;
		if(*t->w==delim)
			t->w++;
		else
			syntax("unterminated regular expression");
		rebuf->w += sizeof(regex_t);
	} else if(rebuf->w == rebuf->s)
		syntax("no previous regular expression");
	else
		t->w += 2;
	return lastre;
}

int
reexec(regex_t* re, char* s, size_t nmatch, regmatch_t* match, int flags)
{
	int code;
	if((code = regexec(re, s, nmatch, match, flags)) && code != REG_NOMATCH) {
		char buf[UCHAR_MAX+1];
		regerror(code, re, buf, sizeof(buf));
		error(3, "regular expression execution error: %s", buf);
	}
	return code;
}

Text gendata;

#define NMATCH 10
regmatch_t matches[NMATCH];
#define so matches[0].rm_so
#define eo matches[0].rm_eo

int
substitute(regex_t *re, Text* data, unsigned char *rhs, int n)
{
	Text t;
	unsigned char *where = data->s;
	if(reexec(re, (char*)data->s, NMATCH, matches, 0))
		return 0;
	vacate(&gendata);
	if(n == 0)
		do {
			docopy(where, so);
			if(!dosub(where, rhs))
				return 0;
			where += eo;
			if(eo == so)
				if(where < data->w)
					docopy(where++, 1);
				else
					goto done;
		} while(reexec(re, (char*)where, NMATCH, matches, REG_NOTBOL) == 0);
	else {
		while(--n > 0) {
			where += eo;
			if(eo == so)
				if(where < data->w)
					where++;
				else
					return 0;
			if(reexec(re, (char*)where, NMATCH, matches, REG_NOTBOL))
				return 0;
		}
		docopy(data->s, where-data->s+so);
		if(!dosub(where, rhs))
			return 0;
		where += eo;
	}			
	eo = so = data->w - where;
	docopy(where, so);
done:
	exch(gendata, *data, t);
	return 1;
}

void
docopy(unsigned char *where, int n)
{
	assure(&gendata, n+1);
	memmove(gendata.w, where, n);
	gendata.w += n;
	*gendata.w = 0;
}

	/* interpretation problem: if there is no match for \1, say,
           does the substitition occur?  dosub uses a null string.
	   a change where indicated will abort the substitution */
	
int
dosub(unsigned char *where, unsigned char *rp)
{
	int c, n;
	regmatch_t *m;

	while(c = *rp++) {
		if(c == '\\') {
			c = *rp++;
			if (c >= '1' && c <= '9') {
				m = matches + c - '0';
				if(m->rm_eo == -1)
					continue;   /* or return 0 */
				n = m->rm_eo - m->rm_so;
				assure(&gendata, n);
				memmove(gendata.w,where+m->rm_so,n);
				gendata.w += n;
				continue;
			}
		} else if(c == '&') {
				assure(&gendata, eo-so);
				memmove(gendata.w,where+so,eo-so);
				gendata.w += eo-so;
				continue;
		}
		assure(&gendata, 1);
		*gendata.w++ = c;
	}
	return 1;
}
