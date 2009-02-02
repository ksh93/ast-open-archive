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
#include	"vchdr.h"

/*	Collection of functions to translate command line arguments,
**	aliases, etc. to list of transforms and arguments suitable for
**	data tranformation by vcsfio().
**
**	The syntax is like this:
**		Transformation	Method1,Method2,...
**		Method		method.arg1.arg2....
**		arg		value
**		arg		name=value
**		method		alphanumeric string specifying a transform
**		name		alphanumeric string specifying an argument
**		value		any string, quotable with [], C-style chars
**		
**	Written by Kiem-Phong Vo.
*/

#ifndef PATH_MAX
#define PATH_MAX	(4*1024)
#endif
#define ALIASES		"lib/vcodex/aliases"
#define VCZIPRC		".vcziprc"

typedef struct _vcalias_s	Vcalias_t;	
struct _vcalias_s
{	Vcalias_t*	next;
	char*		name;	/* name of the alias			*/
	char*		value;	/* what it should expand to		*/
};	

static char	*Dfltalias[] =
{	
	"tv = ama,table,mtf,rle.0,huffgroup",
	"tss7 = ss7,table,mtf,rle.0,huffpart",
	"tnls = ama.nls,rtable,mtf,rle.0,huffgroup",
	"tnl = ama.nl,table,mtf,rle.0,huffgroup",
	"tbdw = bdw,ama,table,mtf,rle.0,huffgroup",
	"t = table,mtf,rle.0,huffgroup",
	"rt = rtable,mtf,rle.0,huffgroup",
	"qv = ama,transpose,rle,huffman",
	"q = transpose,rle,huffman",
	"netflow = netflow,mtf,rle.0,huffgroup",
	"flatrdb = rdb,bwt,mtf,rle.0,huffgroup",
	"fixedrdb = rdb.full,table,mtf,rle.0,huffgroup",
	"dna = sieve.reverse.map=ATGCatgc,huffgroup",
	"delta = sieve.delta,bwt,mtf,rle.0,huffgroup",
	"b = bwt,mtf,rle.0,huffgroup",
	0
};

static Vcalias_t	*Alias;

/* create aliases, text lines of the form 'name = value' */
#if __STD_C
static Vcalias_t* zipalias(Vcalias_t* alias, char* s)
#else
static Vcalias_t* zipalias(alias, s)
Vcalias_t*	alias;	/* curren alias list	*/
char*		s;	/* spec of new aliases	*/
#endif
{
	Vcalias_t	*al;
	ssize_t		a, v, w, n;

	for(n = s ? strlen(s) : 0; n > 0; )
	{	/* skip starting blanks */	
		while(n > 0 && (isblank(*s) || *s == '\n') )
			{ s += 1; n -= 1; }

		if(!isalnum(*s) ) /* invalid alias specification */
			goto skip_line;

		/* get the name */
		for(a = 0; a < n; ++a)
			if(!isalnum(s[a]))
				break;

		for(v = a; v < n; ++v)
			if(!isblank(s[v]) )
				break;
		if(s[v] != '=') /* syntax is name = value */
			goto skip_line;

		/* get the value */
		for(v += 1; v < n; ++v)
			if(!isblank(s[v]) )
				break;
		for(w = v; w < n; ++w)
			if(isblank(s[w]) )
				break;
		if(w == v)
			goto skip_line;

		if(!(al = (Vcalias_t*)malloc(sizeof(Vcalias_t) + a+1 + (w-v)+1)) )
			break;

		al->name = (char*)(al+1);
		al->value = al->name + a+1;
		memcpy(al->name, s, a); al->name[a] = 0;
		memcpy(al->value, s+v, w-v); al->value[w-v] = 0;
		al->next = alias;
		alias = al;

	skip_line:
		for(; n > 0; --n, ++s)
			if(*s == '\n')
				break;
	}

	return alias;
}

/* initialize a list of aliases */
#if __STD_C
void vcaddalias(char** dflt)
#else
void vcaddalias(dflt)
char**	dflt;	/* list of default aliases */
#endif
{
	ssize_t		z;
	Sfio_t		*sf;
	char		*sp, file[PATH_MAX];
	Vcalias_t	*alias = Alias;

	if(!alias)
	{
#if _PACKAGE_ast /* AST alias convention */
		if(pathpath(file, ALIASES, "",  PATH_REGULAR) && (sf = sfopen(0, file, "")) )
		{	while((sp = sfgetr(sf, '\n', 1)) )
				alias = zipalias(alias, sp);
			sfclose(sf);
		}
#endif

		/* $HOME/.vcziprc */ 
		if((sp = getenv("HOME")) && (z = strlen(sp)) > 0 && (z+1+strlen(VCZIPRC)+1) <= PATH_MAX )
		{	memcpy(file, sp, z);
			sp[z] = '/';
			strcpy(file+z+1, VCZIPRC);

			if((sf = sfopen(0, file, "")) )
			{	while((sp = sfgetr(sf, '\n', 1)) )
					alias = zipalias(alias, sp);
				sfclose(sf);
			}
		}
		for(z = 0; sp = Dfltalias[z]; ++z)
			alias = zipalias(alias, sp);
	}

	/* other default aliases */
	if(dflt)
		for(z = 0; (sp = dflt[z]); ++z)
			alias = zipalias(alias, sp);

	Alias = alias;
}

/* map an alias. Arguments are passed onto the first method of the aliased spec */
#if __STD_C
char* vcgetalias(char* spec, char* meth, ssize_t mtsz)
#else
char* vcgetalias(spec, meth, mtsz)
char*		spec;	/* name.arg1.arg2...	*/
char*		meth;	/* buffer for methods	*/
ssize_t		mtsz;	/* buffer size		*/
#endif
{
	char		*args, *rest, name[1024];
	ssize_t		n, a, r;
	Vcalias_t	*alias;

	if(!Alias)
		vcaddalias(NIL(char**));

	if(!(alias = Alias) || !spec)
		return spec;

	/* must be of the form xxx.yyy.zzz... only */
	if(!(args = vcsubstring(spec, VC_METHSEP, name, sizeof(name), 0)) || *args != 0 )
		return spec;

	/* find the extent of the alias name */
	for(n = 0; name[n]; ++n)
		if(name[n] == 0 || name[n] == VC_ARGSEP)
			break;
	args = name[n] ? name+n+1 : name+n;
	name[n] = 0;

	/* see if that matches an alias */
	for(; alias; alias = alias->next)
		if(strcmp(alias->name, name) == 0)
			break;
	if(!alias)
		return spec;

	if(!*args || !meth || !mtsz) /* no new arguments */
		return alias->value;

	/* copy the spec of the first transform to meth[] */
	if(!(rest = vcsubstring(alias->value, VC_METHSEP, meth, mtsz, 0)) )
		return spec;

	n = strlen(meth);
	a = strlen(args);
	r = strlen(rest);
	if(n+1+a+1+r > mtsz) /* not enough room */
		return spec;

	/* copy additional arguments */
	meth[n] = VC_ARGSEP;
	strcpy(meth+n+1, args);

	if(r > 0) /* copy the rest of the alias */
	{	meth[n+1+a] = VC_METHSEP;
		strcpy(meth+n+1+a+1, rest);
	}

	return meth;
}

/* walk the list of aliases */
#if __STD_C
int vcwalkalias(Vcwalk_f walkf, Void_t* disc)
#else
int vcwalkalias(walkf, disc)
Vcwalk_f	walkf;
Void_t*		disc;
#endif
{
	Vcalias_t	*al;
	int		rv;

	if(!Alias)
		vcaddalias(NIL(char**));
	if(!walkf)
		return -1;
	for(al = Alias; al; al = al->next)
		if((rv = (*walkf)((Void_t*)0, al->name, al->value, disc)) < 0 )
			return rv;
	return 0;
}