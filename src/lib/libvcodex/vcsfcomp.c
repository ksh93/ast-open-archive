/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*                  Copyright (c) 2003-2006 AT&T Corp.                  *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                            by AT&T Corp.                             *
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
#define _VCSFALIAS_PRIVATE_	Vcsfalias_t* next;

#if _PACKAGE_ast
#include <ast.h>
#endif

#include <sfio.h>
#include <vcsfio.h>

#include "vchdr.h"

#define ALIASES		"lib/vcodex/aliases"
#define VCZIPRC		".vcziprc"
#define DFLTZIP		"delta,huffgroup"

#ifndef NIL
#define NIL(type)	((type)0)
#endif

#ifndef elementsof
#define elementsof(x)	(sizeof(x)/sizeof(x[0]))
#endif

#if !__STD_C && !defined(const)
#define const
#endif

static Vcsfalias_t	aliases[] =
{
#if VCTABLE || _PACKAGE_ast
	{ "-b",		"bwt,mtf,rle.0,huffgroup",		&aliases[1]  },
	{ "-t",		"table,mtf,rle.0,huffgroup",		&aliases[2]  },
	{ "-tv",	"ama,table,mtf,rle.0,huffgroup",	&aliases[3]  },
	{ "-tama",	"ama,table,mtf,rle.0,huffpart",		&aliases[4]  },
	{ "-tnl",	"ama.nl,table,mtf,rle.0,huffgroup",	&aliases[5]  },
	{ "-tss7",	"ss7,table,mtf,rle.0,huffpart",		&aliases[6]  },
	{ "-tss7.sid",	"ss7.sid,table,mtf,rle.0,huffpart",	&aliases[7]  },
	{ "-tbdw",	"bdw,ama,table,mtf,rle.0,huffgroup",	&aliases[8]  },
	{ "-q",		"transpose,rle,huffman",		&aliases[9]  },
	{ "-qv",	"ama,transpose,rle,huffman",		&aliases[10] },
	{ "-qama",	"ama,transpose,rle,huffpart",		&aliases[11] },
	{ "-qss7",	"ss7,transpose,rle,huffpart",		&aliases[12] },
	{ "-qss7.sid",	"ss7.sid,transpose,rle,huffpart",	0            }
#else
	{ "-b",		"bwt,mtf,rle.0,huffgroup",		0            }
#endif
};

static struct State_s
{
	Vcsfalias_t*	aliases;
} state;

#if __STD_C
static void loadalias(const char* file)
#else
static void loadalias(file)
const char*	file;
#endif
{
	Sfio_t		*f;
	char		*s;
	int		n, k;
	Vcsfalias_t	*ap;

	if(!(f = sfopen(0, file, "r")))
		return;
	while((s = sfgetr(f, '\n', 1)) )
	{	n = sfvalue(f);

		for(; ISBLANK(*s); ++s)
			n -= 1;

		for(k = 0; k < n; ++k)
			if(s[k] == '=')
				break;
		if(k == 0 || k == n)
			continue;

		if(!(ap = (Vcsfalias_t*)malloc(sizeof(Vcsfalias_t)+n)) )
			break;

		ap->name = (char*)(ap+1);
		ap->meth = ap->name+k+1;
		memcpy(ap->name, s, n);

		ap->name[k] = ' ';
		for(k = 0; !ISBLANK(ap->name[k]); ++k)
			;
		ap->name[k] = 0;

		while(ISBLANK(ap->meth[0]) )
			ap->meth += 1;

		ap->next = state.aliases;
		state.aliases = ap;
	}
	sfclose(f);
}

/* return alias given name, name==0 returns alias list head */
#if __STD_C
Vcsfalias_t* vcsfgetalias(const char* name, char** next)
#else
Vcsfalias_t* vcsfgetalias(name, next)
char*	name;
char**	next;
#endif
{
	Vcsfalias_t	*ap;
	const char	*a, *s;
#if _PACKAGE_ast
	char		file[PATH_MAX];
#else
	char		*ep, *np, *path, file[4096];
#endif

	if(!state.aliases)
	{	state.aliases = aliases; /* start with these */

		/* insert system alias files */
#if _PACKAGE_ast
		if(pathpath(file, ALIASES, "", PATH_REGULAR))
			loadalias(file);
#else
		if(!(path = getenv("PATH")) )
			path = "";
		for(; path[0]; path = np)
		{	while(*path == ':')
				path += 1;
			if(!path[0])
				break;
			for(np = path; *np; ++np)
				if(*np == ':')
					break;
			for(ep = np-1; ep >= path; --ep)
				if(*ep != '/')
					break;	
			sfsprintf(file, sizeof(file), "%-.*s/%s", (ep-path)+1, path, ALIASES);
			loadalias(file);
		}

		/* insert personal VCZIPRC file */
		if(!(path = getenv("HOME")) || !path[0] )
			path = ".";
		sfsprintf(file, sizeof(file), "%s/%s", path, VCZIPRC);
		loadalias(file);
#endif
	}
	if(!name)
		return state.aliases;
	if(*name == '-')
		name++;
	for(ap = state.aliases; ap; ap = ap->next)
	{	s = name;
		a = (const char*)ap->name;
		if(*a == '-')
			a++;
		do
		{	if(ISPUNCT(*s) || ISEOS(*s))
			{	if(ISEOS(*a))
				{	if(next)
						*next = (char*)s;
					return ap;
				}
				break;
			}
		} while (*s++ == *a++);
	}
	return 0;
}

#if __STD_C
Vcsfalias_t* vcsfnextalias(Vcsfalias_t* ap)
#else
Vcsfalias_t* vcsfnextalias(ap)
Vcsfalias_t*	ap;
#endif
{
	if(!ap)
		return vcsfgetalias(NIL(char*), NIL(char**));
	return ap->next;
}

static struct { const char* id; } handle = { "vcodex" }; /* ast errorf handle convention */

/* compute list of methods passable to vcsfio() -- return value may be free()'d */
#if __STD_C
Vcsfmeth_t* vcsfcomp(const char* meth, ssize_t* nmeth, char** pnext, Vcsferror_f errorf)
#else
Vcsfmeth_t* vcsfcomp(meth, nmeth, pnext, errorf)
char*		meth;
ssize_t*	nmeth;
Vcsferror_f	errorf;
char**		pnext;
#endif
{
	Vcsfmeth_t	*list;
	Vcsfalias_t	*ap;
	size_t		nlist, ndata;
	int		k;
	int		level;
	const char	*stack[8];
	char		*data, *next;

	list = 0;
	k = nlist = 0;
	ndata = 0;
	level = 0;
	for(;;)
	{	while(ISPUNCT(*meth))
			++meth;
		if(ISEOS(*meth))
		{	if(level == 0)
				break;
			meth = stack[--level];
			continue;
		}
		if(ap = vcsfgetalias(meth, &next))
		{	if(level >= elementsof(stack))
			{	if(errorf)
					(*errorf)((Void_t*)&handle, (Void_t*)&handle, 2, "%s: method alias nesting too deep", ap->meth);
				goto drop;
			}
			stack[level++] = (const char*)next;
			meth = (const char*)ap->meth;
			continue;
		}
		if(k >= nlist)
		{	nlist += 8;
			if(!(list = (Vcsfmeth_t*)realloc(list, sizeof(Vcsfmeth_t)*nlist)))
				goto nospace;
		}
		if(!(list[k].meth = vcgetmeth(meth, 0, &next)) )
		{	if (errorf)
				(*errorf)((Void_t*)&handle, (Void_t*)&handle, 2, "%s: unknown method", meth);
			goto drop;
		}
		meth = (const char*)next;
		if(*meth == '.')
		{	for(list[k].data = (Void_t*)++meth; *meth && !ISPUNCT(*meth); ++meth);
			if(list[k].size = (char*)meth - (char*)list[k].data)
				ndata += list[k].size;
			else
				list[k].data = NIL(Void_t*);
		}
		else
		{	list[k].data = NIL(Void_t*);
			list[k].size = 0;
		}
		k += 1;
	}

	if(k == 0)
		goto drop;

	if(ndata || k < nlist)
	{	if(!(list = (Vcsfmeth_t*)realloc(list, sizeof(Vcsfmeth_t)*k + ndata)))
			goto nospace;
		if(ndata)
		{	data = (Void_t*)&list[k];
			for(nlist = 0; nlist < k; nlist++)
				if(list[nlist].data)
				{	memcpy(data, list[nlist].data, list[nlist].size);
					list[nlist].data = data;
					data += list[nlist].size;
				}
		}
	}
	if(pnext)
		*pnext = (char*)meth;
	*nmeth = k;
	return list;
 nospace:
	if (errorf)
		(*errorf)((Void_t*)&handle, (Void_t*)&handle, 2, "out of space");
 drop:
	if(list)
		free(list);
	return 0;
}

/* compute optimal list of methods passable to vcsfio() -- return value may be free()'d */
#if __STD_C
Vcsfmeth_t* vcsfbest(const char* meth, ssize_t* nmeth, int usetime, ssize_t window, char** pmeth, Vcsferror_f errorf)
#else
Vcsfmeth_t* vcsfbest(meth, nmeth, usetime, window, pmeth, errorf)
char*		meth;
ssize_t*	nmeth;
int		usetime;
ssize_t		window;
char**		pmeth;
Vcsferror_f	errorf;
#endif
{
	Sfio_t		*sf;
	Vcsfio_t	*vcsf;
	Vcchar_t	*dt;
	ssize_t		n;
	double		begtm, endtm, tm, rate;
	const char	*bestmeth;
	char		*next;
	Vcsfmeth_t	*list, *best;
	ssize_t		nlist, nbest;
#if _lib_times && _sys_times
	struct tms	t1, t2;
#else
	usetime = 0;
#endif

	if(!(list = vcsfcomp(meth, &nlist, &next, errorf)))
		return 0;
	if(*next != ':')
	{	/* only one method -- use it */
		if(pmeth)
			*pmeth = (char*)meth;
		*nmeth = nlist;
		return list;
	}

	/* initialize for error cleanup */
	best = NIL(Vcsfmeth_t*);
	sf = NIL(Sfio_t*);
	vcsf = NIL(Vcsfio_t*);

	/* get training data */
	if(window <= 0 )
		window = VCSF_TRAINSIZE;
	if(!(dt = sfreserve(sfstdin, -window, SF_LOCKR)) )
	{	if((window = (ssize_t)sfvalue(sfstdin)) <= 0)
		{	if(errorf)
				(*errorf)((Void_t*)&handle, (Void_t*)&handle, 2, "no training data in standard input");
			goto drop;
		}
		if(!(dt = sfreserve(sfstdin, window, SF_LOCKR)) )
		{	if(errorf)
				(*errorf)((Void_t*)&handle, (Void_t*)&handle, 2, "cannot read training data");
			goto drop;
		}
	}

	rate = 0.;
	tm = 0.;

	do
	{
		if(!(sf = sftmp(0)) )
		{	if(errorf)
				(*errorf)((Void_t*)&handle, (Void_t*)&handle, 2, "cannot create training temporary file");
			goto drop;
		}
		if(!(vcsf = vcsfio(sf, list, nlist, VC_ENCODE)) )
		{	if(errorf)
				(*errorf)((Void_t*)&handle, (Void_t*)&handle, 2, "cannot create training test stream");
			goto drop;
		}

		/* compute compressed size and time */
#if _lib_times && _sys_times
		begtm = (double)times(&t1);
#else
		begtm = 0.;
#endif
		if(vcsfwrite(vcsf, dt, window) != window)
		{	if(errorf)
				(*errorf)((Void_t*)&handle, (Void_t*)&handle, 2, "cannot encode training data");
			goto drop;
		}
		vcsfsync(vcsf);
		n = (ssize_t)lseek(sffileno(sf), (off_t)0, SEEK_END);
		vcsfclose(vcsf);
		vcsf = NIL(Vcsfio_t*);
		sfclose(sf);
		sf = NIL(Sfio_t*);

#if _lib_times && _sys_times
		endtm = (double)times(&t2);
#else
		endtm = 0.;
#endif

		if(!usetime)
			tm = 1.;
		else
		{	if((endtm-begtm) > tm)
				tm = endtm - begtm;
			if(tm <= 0.)
				tm = 1.;
		}

		if(!best || (n*tm)/window < rate)
		{	if(best)
				free(best);
			bestmeth = meth;
			best = list;
			nbest = nlist;
			rate  = (n*tm)/window;
		}
		else
			free(list);
		meth = (const char*)next;
	} while(list = vcsfcomp(meth, &nlist, &next, errorf));

	/* release reserved data */
	sfread(sfstdin, dt, 0);

	if(pmeth)
		*pmeth = (char*)bestmeth;
	*nmeth = nbest;
	return best;
 drop:
	if(dt)
		sfread(sfstdin, dt, 0);
	if(vcsf)
		vcsfclose(vcsf);
	if(sf)
		sfclose(sf);
	if(list)
		free(list);
	if(best)
		free(best);
	return 0;
}

/* duplicate method list for use by another stream */
#if __STD_C
Vcsfmeth_t* vcsfdup(Vcsfmeth_t* meth, ssize_t nmeth)
#else
Vcsfmeth_t* vcsfdup(meth, nmeth)
Vcsfmeth_t*	meth;
ssize_t		nmeth;
#endif
{
	Vcsfmeth_t*	m;
	char*		d;
	ssize_t		z;
	int		i;

	z = nmeth * sizeof(meth[0]);
	for (i = 0; i < nmeth; i++)
		z += meth[i].size;
	if(!(m = (Vcsfmeth_t*)malloc(z)))
		return 0;
	d = (char*)(m + nmeth);
	for (i = 0; i < nmeth; i++)
	{	m[i].meth = meth[i].meth;
		if(z = m[i].size = meth[i].size)
		{	memcpy(m[i].data = (Void_t*)d, meth[i].data, z);
			d += z;
		}
		else
			m[i].data = 0;
	}
	return m;
}

/* syntax for window size is "[0-9]+[mMkK]" */
#if __STD_C
ssize_t vcsfgetwindow(const char* spec, Vcwmethod_t** vcwm, char** next, Vcsferror_f errorf)
#else
ssize_t vcsfgetwindow(spec, vcwm, next, errorf)
const char*	spec;
Vcwmethod_t**	vcwm;
char**		next;
Vcsferror_f	errorf;
#endif
{
	ssize_t		size;
	Vcwmethod_t*	wm;

	size = (ssize_t)atoi(spec);
	while(ISDIGIT(*spec))
		spec += 1;
	if(*spec == 'k' || *spec == 'K')
		size *= 1024;
	else if(*spec == 'm' || *spec == 'M')
		size *= 1024*1024;

	wm = 0;
	while(*spec)
		if(ISWINSEP(*spec))
		{	spec += 1;
			for(wm = vcwgetmeth(NIL(char*)); wm && *spec != wm->name[0]; wm = vcwnextmeth(wm));
			if(*spec)
				spec += 1;
			break;
		}
		else	spec += 1;
	*vcwm = wm;

	if(next)
		*next = (char*)spec;
	return size;
}
