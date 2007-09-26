/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*              Copyright (c) 2007 AT&T Knowledge Ventures              *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                      by AT&T Knowledge Ventures                      *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                  David Korn <dgk@research.att.com>                   *
*                                                                      *
***********************************************************************/
#pragma prototyped

#include	<shell.h>
#include	"dbmlib.h"

static const char dbm_usage[] =
"[-?@(#)$Id: Dbm_t (AT&T Research) 2007-09-07 $\n]"
USAGE_LICENSE
"[+NAME?Dbm_t - create an associative array containing contents of a dbm file]"
"[+DESCRIPTION?\bDbm_t\b is a declaration command that creates an associative "
	"array corresponding to the dbm file whose name is the value of the "
	"variable \avarname\a.  The variable \avarname\a becomes an associative "
	"array with subscripts corresponding to keys in the dbm file.]"
"[+?The keys in the file cannot contain the NUL character, \b\\0\b, except as "
	"the last character.  In this case all keys must have \b\\0\b as the "
	"last characer.  The \b-z\b option adds a trailing NUL to each key.]"
"[+?If no options are specified, \bDbm_t\b defaults to \b-r\b.]"
"[c:create?Clear the dbm file if it already exists or create it.]"
"[e:exclusive?Open for exclusive access.]"
"[r:read?Open for read access only.  \avarname\a becomes a readonly variable.]"
"[w:write?Open for read and write access.]"
"[z:zero?When used with \b-c\b, a \b\\0\b byte is appended to each key.]"
"\n"
"\nvarname=file\n"
"\n"
"[+EXIT STATUS]"
    "{"
        "[+0?Successful completion.]"
        "[+>0?An error occurred.]"
    "}"
"[+SEE ALSO?\bksh\b(1), \bdbm\b(3)]"
;

struct dbm_array
{
	Namarr_t	header;
	DBM		*dbm;
	Namval_t	*pos;
	char		*name;
	Namval_t	*cur;
	size_t		namlen;
	Namval_t	node;
	datum		key;
	int		addzero;
};

extern Namarr_t		*nv_arrayptr(Namval_t*);
#define NV_ASETSUB       8

static const Namdisc_t	*array_disc(Namval_t *np)
{
	Namarr_t	*ap;
	const Namdisc_t	*dp;
	nv_putsub(np, (char*)0, 1);
	ap = nv_arrayptr(np);
	dp = ap->hdr.disc;
	nv_disc(np,&ap->hdr,NV_POP);
	return(dp);
}


static size_t check_size(char **buff, size_t olds, size_t news)
{
	if(news>=olds)
	{
		if(olds)
		{
			while((olds*=2) <= news);
			*buff = (char*)realloc(*buff,olds);
			
		}
		else
			*buff = (char*)malloc(olds=news+1);
	}
	return(olds);
}

static void dbm_setname(struct dbm_array *ap)
{
	if(((char*)ap->key.dptr)[ap->key.dsize-ap->addzero])
	{
		ap->namlen = check_size(&ap->name,ap->namlen, ap->key.dsize);
		memcpy(ap->name,ap->key.dptr,ap->key.dsize);
		ap->name[ap->key.dsize] = 0;
		ap->node.nvname = ap->name;
	}
	else
		ap->node.nvname = ap->key.dptr;
}

static void dbm_get(struct dbm_array *ap)
{
	datum data;
	dbm_clearerr(ap->dbm);
	data = dbm_fetch(ap->dbm,ap->key);
	if(data.dptr)
	{
		ap->node.nvsize = check_size(&ap->node.nvalue,ap->node.nvsize,data.dsize);
		memcpy(ap->node.nvalue,data.dptr,data.dsize);
		ap->node.nvalue[data.dsize] = 0;
		ap->cur = &ap->node;
	}
	else
	{
		int err;
		ap->cur = 0;
		if(err=dbm_error(ap->dbm))
		{
			dbm_clearerr(ap->dbm);
			error(ERROR_system(err),"Unable to get key %.*s",ap->key.dsize-ap->addzero,ap->key.dptr);
		}
	}
}

static void *dbm_associative(register Namval_t *np,const char *sp,int mode)
{
	register struct dbm_array *ap = (struct dbm_array*)nv_arrayptr(np);
	switch(mode)
	{
	    case NV_AINIT:
	    {
		DBM *dp = (DBM*)np->nvalue;
		np->nvalue = 0;
		if(ap = (struct dbm_array*)calloc(1,sizeof(struct dbm_array)))
		{
			ap->header.hdr.disc = array_disc(np);
			nv_disc(np,(Namfun_t*)ap, NV_FIRST);
			ap->dbm = dp;
			ap->header.hdr.nofree = 1;
			ap->node.nvalue = (char*)malloc(ap->node.nvsize=40);
			if(nv_isattr(np, NV_ZFILL))
			{
				nv_offattr(np,NV_ZFILL);
				ap->addzero=1;
			}
		}
		return((void*)ap);
	    }
	    case NV_ADELETE:
		if(ap->pos)
			ap->header.nelem = 1;
		else if(ap->cur)
			dbm_delete(ap->dbm,ap->key);
		ap->pos = ap->cur = 0;
		return((void*)ap);
	    case NV_AFREE:
		ap->cur = ap->pos = 0;
		if(ap->name)
		{
			free((void*)ap->name);
			ap->namlen = 0;
		}
		free((void*)ap->node.nvalue);
		ap->node.nvalue = 0;
		ap->node.nvsize = 0;
		dbm_close(ap->dbm);
		ap->dbm = 0;
		return((void*)ap);
	    case NV_ANEXT:
		if(!ap->pos)
		{
			ap->pos = &ap->node;
			ap->key = dbm_firstkey(ap->dbm);
		}
		else
			ap->key = dbm_nextkey(ap->dbm);
		if(ap->key.dptr)
		{
			ap->cur = ap->pos;
			dbm_setname(ap);
			return((void*)ap->cur);
		}
		else
			ap->pos = 0;
		return((void*)0);
	    case NV_ASETSUB:
		ap->key.dsize = strlen(sp)+ap->addzero;
		ap->namlen = check_size(&ap->name,ap->namlen, ap->key.dsize);
		ap->key.dptr = memcpy(ap->name,sp,ap->key.dsize+1);
		ap->node.nvname = ap->key.dptr;
		ap->cur = (Namval_t*)sp;
		/* FALL THROUGH*/
	    case NV_ACURRENT:
		if(ap->pos)
			dbm_get(ap);
		return((void*)ap->cur);
	    case NV_ANAME:
		if(ap->cur && ap->cur!= &ap->node)
			ap->cur = &ap->node;
		if(ap->cur)
			return((void*)nv_name(ap->cur));
		return((void*)0);
	    default:
		if(sp)
		{
			if(sp==(char*)np)
			{
				ap->cur = 0;
				return(0);
			}
			else if(!(ap->header.nelem&ARRAY_SCAN))
				ap->pos = 0;
			ap->key.dsize = strlen(sp)+ap->addzero;
			if(mode==NV_AADD && ap->key.dsize==1 && *sp=='0' && ap->header.nelem==0 && !ap->addzero) 
			{
				/* only happens during initialization */
				ap->key = dbm_firstkey(ap->dbm);
				if(ap->key.dptr && ((char*)ap->key.dptr)[ap->key.dsize-1]==0)
					ap->addzero = 1;
				while(ap->key.dptr)
				{
					ap->header.nelem++;
					ap->key = dbm_nextkey(ap->dbm);
				}
				return((void*)(&ap->cur));
			}
			ap->namlen = check_size(&ap->name,ap->namlen, ap->key.dsize);
			ap->key.dptr = memcpy(ap->name,sp,ap->key.dsize+1);
			dbm_get(ap);
			if(ap->cur)
				dbm_setname(ap);
		}
		if(ap->cur)
		{
			ap->cur = &ap->node;
			return((void*)(&ap->cur->nvalue));
		}
		else
			return((void*)(&ap->cur));
	}
}

static void put_dbm(Namval_t* np, const char* val, int flag, Namfun_t* fp)
{
	struct dbm_array *ap = (struct dbm_array*)nv_arrayptr(np);
	datum data;
	if(val)
	{
		data.dptr=(char*)val;
		data.dsize = strlen(val)+ap->addzero;
		dbm_store(ap->dbm,ap->key,data,DBM_REPLACE);
	}
	else if(!ap)
		np->nvalue = 0;
}

static const Namdisc_t dbm_disc =
{
	0,
	put_dbm,
};

static Namfun_t *clone_dbm(Namval_t* np, Namval_t *mp, int flags, Namfun_t *fp)
{
	Namfun_t *pp;
	pp = (Namfun_t*)calloc(1,sizeof(Namfun_t));
	pp->dsize = sizeof(Namfun_t);
	pp->type = np;
	pp->disc = &dbm_disc;
	return(pp);
}
static Namval_t *create_dbm(Namval_t *np,const char *name,int flag,Namfun_t *fp)
{
	return(0);
}

static const Namdisc_t Dbm_disc =
{
	0,
	0,
	0,
	0,
	0,
	create_dbm,
	clone_dbm,
};

static int dbm_create(int argc, char** argv, void* context)
{
	int		oflags = 0, zflag=0;
	Namval_t	*np;
	Namfun_t	*fp;
	Namtype_t	*tp = (Namtype_t*)context;
	Namarr_t	*ap;
	char		*dbfile;
	DBM		*db;
	int		fds[10],n=0;

	cmdinit(argc, argv, context, ERROR_CATALOG, ERROR_NOTIFY);
#if _use_ndbm
	for (;;)
	{
		switch (optget(argv, dbm_usage))
		{
		case 'c':
			oflags |= O_CREAT|O_TRUNC|O_RDWR;
			continue;
		case 'e':
			oflags |= O_EXCL;
			continue;
		case 'r':
			continue;
		case 'w':
			oflags |= O_RDWR;
			continue;
		case 'z':
			zflag = 1;
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			break;
		case ':':
			error(2, "%s", opt_info.arg);
			break;
		}
		break;
	}
	argv += opt_info.index;
	if (error_info.errors || !*argv || *(argv + 1))
	{
		error(ERROR_USAGE|2, "%s", optusage(NiL));
		return 1;
	}
	if(oflags==0)
		oflags = O_RDONLY;
	if(!(np = nv_open(*argv, (void*)0, NV_VARNAME|NV_NOADD)) || !(dbfile=nv_getval(np)))
		error(3, "%s must contain the name of a dbm file");
	while((fds[n] = open("/dev/null",NV_RDONLY)) < 10)
		n++;
	if (!error_info.errors && !(db=dbm_open(dbfile, oflags, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)))
	{
		error(ERROR_SYSTEM|3, "%s: cannot open db",dbfile);
		return 1;
	}
	while(n>0)
		close(fds[--n]);
	nv_unset(np);
	np->nvalue = (char*)db;
	if(zflag && (oflags&O_CREAT))
		nv_onattr(np,NV_ZFILL);
	if((ap=nv_setarray(np, dbm_associative)) && ap->nelem>0)
		ap->nelem--;
	fp = clone_dbm(tp->np,np, 0, tp->np->nvfun);
	nv_disc(np,fp,NV_LAST);
	if(!(oflags&O_RDWR))
		nv_onattr(np,NV_RDONLY);
	return error_info.errors != 0;
#else
	error(2, "ndbm library required");
	return 1;
#endif
}

void lib_init(int flag, void* context)
{
	Shell_t		*shp = ((Shbltin_t*)context)->shp;
	Namval_t	*mp,*np,*bp;
	Namfun_t	*nfp = newof(NiL,Namfun_t,1,0);
	Namtype_t	*tp;
	char tmp[sizeof(NV_CLASS)+17];
	sfsprintf(tmp, sizeof(tmp), "%s.Dbm_t", NV_CLASS);
	np = nv_open(tmp, shp->var_tree, NV_VARNAME);
	nfp->type = np;
	nfp->disc = &Dbm_disc;
	nv_disc(np,nfp,NV_FIRST);
	tp = newof(NiL,Namtype_t,1,0);
	tp->optstring = dbm_usage;
	tp->shp = (void*)shp;
	tp->np = np;
	bp = sh_addbuiltin("Dbm_t", dbm_create, (void*)tp); 
	mp = nv_search("typeset",shp->bltin_tree,0);
	nv_onattr(bp,nv_isattr(mp,NV_PUBLIC));
}
