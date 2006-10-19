/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 2003-2006 AT&T Knowledge Ventures            *
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
/*
 * dss type library
 *
 *   David Korn
 *   AT&T Labs
 *
 */

#include	<shell.h>
#include	<dss.h>

#ifndef SH_DICT
#   define SH_DICT	"libshell"
#endif

#define vnode(np)	((Cxvariable_t*)((np)->nvname))

static int match(int, char *[], void *);
static int format(int, char *[], void *);
static Namval_t *create_dss(Namval_t*,const char*,int,Namfun_t*);
static Namval_t *create_type(Namval_t*,const char*,int,Namfun_t*);

static const Namdisc_t parent_disc;
static Dt_t		*typedict;
static Dssdisc_t	Dssdisc;
static size_t		buflen;

struct child
{
	Namfun_t	fun;
	Namval_t	*parent;
	struct parent	*dss;
};

struct type
{
	Namfun_t	fun;
	Cxtype_t	*type;
	Cx_t		*cx;
	Namval_t	*bltins[2];
	char		*format;
};

struct parent
{
	Namfun_t	fun;
	Namval_t	*np;
	Shell_t		*sh;
	Namval_t	*parent;
	Dssdisc_t	*disc;
	Cxstate_t	*state;
	Cx_t		*cx;
	Dt_t		*dict;
	char		*data;
	struct child	childfun;
	Namfun_t	parentfun;
	int		numnodes;
	size_t		namelen;
	char		*name;
	Dss_t		*dss;
	Cxtype_t	*type;
	Dssfile_t	*fp;
	Sfio_t		*out;
};

struct query
{
	Cxexpr_t	expr;
	Cxquery_t	*query;
	struct parent	*dss;
};

/*
 * the following function is added as a builtin for each query
 * function that a variable can have
 */
static int query(int argc, char *argv[], void *ptr)
{
	struct query	*pp = (struct query*)ptr;
	Cxquery_t	*qp = pp->query;
	Dss_t		*dss = pp->dss->dss;
	Cx_t		*cx = dss->cx;
	Dssrecord_t	*data;
	int		 n;
	if(!pp->expr.query || argv[1])
	{
		if(pp->expr.query && qp->end)
			(*qp->end)(cx,&pp->expr,(void*)0,cx->disc);
		memset((void*)&pp->expr,0,sizeof(pp->expr));
		if(argc==2 && strcmp(argv[1],"end")==0)
			return(0);
		pp->expr.query = pp->query;
		pp->expr.op = sfstdout;
		if(qp->beg)
		{
			if((*qp->beg)(cx,&pp->expr,(void*)argv,cx->disc))
			{
				pp->expr.query = 0;
				return(2);
			}
		}
		return(0);
	}
	if(!(data = (Dssrecord_t*)pp->dss->np->nvalue))
		return(1);
	if(qp->sel)
	{
		if((n=(*qp->sel)(cx,&pp->expr,data,cx->disc))<=0)
			return(n<0?2:1);
	}
	if(qp->act)
		return(-(*qp->act)(cx, &pp->expr,data,cx->disc));
	return(0);
}

static char *getbuf(size_t len)
{
	static char *buf;
	if(buflen < len)
	{
		if(buflen==0)
			buf = (char*)malloc(len);
		else
			buf = (char*)realloc(buf,len);
		buflen = len;
	}
	return(buf);
}

static void check_numeric(Namval_t *np, Cxtype_t *tp, Cxstate_t *sp)
{
	if(cxisnumber(tp))
	{
		nv_onattr(np,NV_LONG|NV_INTEGER|NV_DOUBLE|NV_EXPNOTE);
		nv_setsize(np,10);
	}
	else if(cxisbuffer(tp))
		nv_onattr(np,NV_BINARY);
}

static Namval_t *typenode(const char *name, int flag)
{
	int		offset = stktell(stkstd);
	Namval_t	*mp, *rp;
	sfputc(stkstd,0);
	sfprintf(stkstd,"dss.%s",name);
	sfputc(stkstd,0);
	mp = nv_search(stkptr(stkstd,offset+1),typedict,flag);
	stkseek(stkstd,offset);
	if(flag&NV_ADD)
	{
		/* create reference variable name to NV_CLASS.dss.name */
		Shell_t *shp = sh_getinterp();
		sfputc(stkstd,0);
		sfprintf(stkstd,NV_CLASS".dss.%s",name);
		sfputc(stkstd,0);
		rp = nv_open(name, shp->var_tree, NV_IDENT);
		nv_unset(rp);
		nv_putval(rp,stkptr(stkstd,offset+1),0);
		nv_setref(rp);
		stkseek(stkstd,offset);
	}
	return(mp);
}

Cxvalue_t *get_child_common(Namval_t *np, Namfun_t *fp, Cxoperand_t *op)
{
	struct child	*cp = (struct child*)fp;
	struct parent	*pp = cp->dss;
	Cxtype_t	*tp = pp->type;
	Cxvariable_t	*vp = vnode(np);
	Cxinstruction_t	in;
	if(nv_isattr(cp->parent,NV_INTEGER))
	{
		in.data.variable = vp;
		op->type = vp->type;
		op->value.number = nv_getnum(cp->parent);
		if((*tp->member->getf)(pp->cx,&in,op,NiL,NiL,NiL,pp->cx->disc)==0)
			return(&op->value);
	}
	return(0);
}

static char* get_child(register Namval_t* np, Namfun_t *fp)
{
	Cxvariable_t	*vp = vnode(np);
	Cxoperand_t	op;
	Cxvalue_t	*val;
	if(val = get_child_common(np, fp, &op))
	{
		if(cxisnumber(vp->type))
		{
			long l = nv_size(np)+8;
			char *sp = getbuf(l);
			sfsprintf(sp,l,"%.*Lg\0",nv_size(np),val->number);
			return(sp);
		}
		return(val->string.data);
	}
	return(0);
}

static char* get_mchild(register Namval_t* np, Namfun_t *fp)
{
	Cxoperand_t ret;
	struct child *pp = (struct child*)fp;
	struct parent *dp = pp->dss;
	Cxvariable_t *vp = vnode(np);
	if(!pp->parent->nvalue)
		return(0);
	if(nv_isattr(np,NV_BINARY) && nv_isattr(np,NV_RAW))
	{
		if(cxcast(dp->cx,&ret,vp,dp->state->type_buffer,pp->parent->nvalue,(char*)0)==0)
		{
			nv_setsize(np,ret.value.buffer.size);
			return(ret.value.buffer.data);
		}
	}
	else if(cxcast(dp->cx,&ret,vp,dp->state->type_string,pp->parent->nvalue,(char*)0)==0)
		return(ret.value.string.data);
	return(nv_name(np));
}

static Sfdouble_t nget_child(register Namval_t* np, Namfun_t *fp)
{
	Cxvariable_t	*vp = vnode(np);
	Cxoperand_t	op;
	Cxvalue_t	*val;
	op.type = 0;
	if(val = get_child_common(np, fp, &op))
	{
		Sfdouble_t	ld;
		if(cxisnumber(vp->type))
			return(val->number);
		else
		{
			ld = sh_strnum(val->string.data, (char**)0, 1);
			return(ld);
		}
	}
	return(0);
}

static Sfdouble_t nget_mchild(register Namval_t* np, Namfun_t *fp)
{
	Cxoperand_t ret;
	struct child *pp = (struct child*)fp;
	struct parent *dp = pp->dss;
	Cxvariable_t *vp = vnode(np);
	if(cxcast(dp->cx,&ret,vp,dp->state->type_number,pp->parent->nvalue,(char*)0)==0)
		return(ret.value.number);
	return(0.0);
}

static void put_child(Namval_t* np, const char* val, int flags, Namfun_t* fp)
{
	struct child	*cp = (struct child*)fp;
	struct parent	*pp = cp->dss;
	Cxtype_t	*tp = pp->type;
	Cxvariable_t	*vp = vnode(np);
	Cxinstruction_t	in;
	Cxoperand_t	ret,op;
	in.data.variable = vp;
	nv_onattr(np,NV_NODISC);
	nv_putval(np,val,flags);
	nv_offattr(np,NV_NODISC);
	ret.type = vp->type;
	ret.value.number = nv_getnum(cp->parent);
	op.value.number = nv_getn(np,fp);
	if((*tp->member->setf)(pp->cx,&in, &ret,&op,NiL,NiL,pp->cx->disc)==0)
		nv_putval(cp->parent,(char*)&ret.value.number,NV_INTEGER|NV_DOUBLE|NV_LONG|NV_NODISC);
}

static void put_mchild(Namval_t* np, const char* val, int flag, Namfun_t* fp)
{
}

static char *name_child(Namval_t *np, Namfun_t *fp)
{
	struct child	*pp = (struct child*)fp;
	struct parent	*dp = pp->dss;
	const char	*name = vnode(np)->name;
	char		*cp;
	size_t		l,len;
	cp = nv_name(pp->parent);
	len= (l=strlen(cp))+strlen(name)+2;
	if(dp->namelen < len)
	{
		if(dp->namelen==0)
			dp->name = (char*)malloc(len);
		else
			dp->name = (char*)realloc(dp->name,len);
		dp->namelen = len;
	}
	memcpy(dp->name,cp,l);
	dp->name[l++] = '.';
	strcpy(&dp->name[l],name);
	return(dp->name);
}

static Namval_t *type_child(register Namval_t* np, Namfun_t *fp)
{
	struct parent	*pp =  ((struct child*)fp)->dss;
	Cxstate_t	*sp = pp->state;
	Cxvariable_t	*vp = vnode(np);
	if(!pp->dss || vp->type==sp->type_number || vp->type==sp->type_string)
		return(0);
	return(typenode(vp->type->name,0));
}

/* for child variables of compound variables */
static const Namdisc_t child_disc =
{
	sizeof(struct child),
	put_child,
	get_child,
	nget_child,
	0,
	0,
	0,
	name_child,
	0,
	type_child
};

/* for child variables of methods */
static const Namdisc_t mchild_disc =
{
	sizeof(struct child),
	put_mchild,
	get_mchild,
	nget_mchild,
	0,
	0,
	0,
	name_child,
	0,
	type_child
};

static Namval_t *node(Cxvariable_t *vp,struct parent *dp)
{
	Namval_t *np;
	if(!dp->data && !(dp->data = (char*)calloc(dp->numnodes,NV_MINSZ)))
		return(0);
	np = nv_namptr(dp->data,(int)vp->data);
	if(!np->nvname)
	{
		Namval_t *mp;
		Namfun_t *fp, *nfp=0;
		nv_disc(np,&dp->childfun.fun,NV_FIRST);
		check_numeric(np, vp->type, dp->state);
		if(!dp->dss && vp->type->base && (mp=typenode(vp->type->name,0)) && (fp = nv_disc(mp, (Namfun_t*)0, NV_FIRST)))
		{
			int size = fp->dsize;
			if(size==0 || (!fp->disc || (size=fp->disc->dsize)==0)) 
				size = sizeof(Namfun_t);
			if(fp->disc && fp->disc->clonef)
				nfp = (*fp->disc->clonef)(np,mp,0,fp);
			else if(nfp = malloc(size))
			{
				memcpy((void*)nfp,(void*)fp,size);
				nfp->nofree = 0;
			}
			if(nfp)
				nv_disc(np,nfp,NV_FIRST);
		}
		np->nvname = (char*)vp;
	}
	return(np);
}

static Namfun_t *clone_parent(Namval_t* np, Namval_t *mp, int flags, Namfun_t *fp)
{
	struct parent *dp;
	size_t size = fp->dsize;
	if(size==0 && (!fp->disc || (size=fp->disc->dsize)==0)) 
		size = sizeof(Namfun_t);
	dp = (struct parent*)malloc(size);
	memcpy((void*)dp,(void*)fp,size);
	dp->childfun.parent = mp;
	dp->childfun.dss = dp;
	dp->data = 0;
	dp->namelen = 0;
	dp->np = mp;
	dp->parent = nv_lastdict();
	return(&dp->fun);
}

static void pushtype(Namval_t *np, Cxtype_t *tt, Namfun_t *fp)
{
	Namval_t *mp;
	for(; tt->base; tt = tt->base)
	{
		if(tt->member)
		{
			Namfun_t *nfp;
			mp = typenode(tt->name,0);
			if(!fp->next)
			{
				fp = nv_disc(np, fp, NV_CLONE);
			}
			nfp = nv_hasdisc(mp, &parent_disc);
			nfp = clone_parent(mp,np,0,nfp);
			nv_disc(np,nfp,NV_LAST);
		}
	}
}

static Namval_t *create_parent(Namval_t *np,const char *name,int flag,Namfun_t *fp)
{
	struct parent	*dp = (struct parent*)fp;
	Dt_t		*dict = dp->dict;
	Cxvariable_t	*vp;
	char		*last;
	if(!name)
		return(dp->parent);
	if(last=strchr(name,'.'))
		*last = 0;
	vp = (Cxvariable_t*)dtmatch(dict,name);
	if(last)
		*last = '.';
	if(vp)
	{
		np = node(vp,dp);
		if(last)
		{
			if(np->nvfun == &dp->childfun.fun)
			{
				fp = nv_disc(np,np->nvfun, NV_CLONE);
				pushtype(np, vp->type, fp);
			}
			np = (*fp->disc->createf)(np,last+1, flag, fp);
		}
		return(np);
	}
	return(0);
}

static void dss_unset(Namval_t *np, struct parent *dp)
{
	if(dp->namelen)
		free((void*)dp->name);
	if(dp->fp)
		dssfclose(dp->fp);
	if(!dp->np)
	{
		if(dp->dss)
			dssclose(dp->dss);
		else
			cxclose(dp->cx);
	}
	nv_disc(np,&dp->fun,NV_POP);
	if(dp->data)
		free((void*)dp->data);
	if(!dp->fun.nofree)
		free((void*)dp);
}

static void put_parent(Namval_t* np, const char* val, int flag, Namfun_t* fp)
{
	struct parent *pp = (struct parent*)fp;
	if(!val)
	{
		nv_putv(np,val,flag,fp);
		dss_unset(np, pp);
	}
	if(!pp->dss)
		nv_putv(np,val, flag, fp);
}

static int read_parent(register Namval_t* np, Sfio_t *iop, int delim, Namfun_t *fp)
{
	struct parent *dp = (struct parent*)np->nvfun;
	if(dp->fp && iop!=dp->fp->io)
	{
		sfprintf(sfstderr,"read on incorrect input stream iop=%x(%d) io=%x(%d)\n",iop,sffileno(iop),dp->fp->io,sffileno(dp->fp->io));
		return(1);
	}
	else if(!dp->fp)
	{
		if(!((dp->fp = dssfopen(dp->dss,(char*)0,iop,DSS_FILE_READ,0))))
		{
			sfprintf(sfstderr,"invalid dss format\n");
			return(1);
		}
	}
	np->nvalue = (char*)dssfread(dp->fp);
	return(np->nvalue==0);
}

static Namval_t *next_parent(register Namval_t* np, Dt_t *root,Namfun_t *fp)
{
	struct parent	*dp;
	Cxvariable_t	*vp;
	if(root)
	{
		if(!(fp=nv_hasdisc(np,&mchild_disc)) && !(fp=nv_hasdisc(np,&child_disc)))
			return(0);
		dp = ((struct child*)fp)->dss;
		vp = vnode(np);
		vp=(Cxvariable_t*)dtnext(dp->dict,vp);
	}
	else
	{
		if(!(fp=nv_hasdisc(np,&parent_disc)))
			return(0);
		dp = (struct parent*)fp;
		vp=(Cxvariable_t*)dtfirst(dp->dict);
	}
	if(!vp)
		return((Namval_t*)0);
	np = node(vp,dp);
	return(np);
}

/*
 * this function creates the builtin functions that can be invoked on
 * this variable
 */
static Namfun_t *clone_bltin(Namval_t* np, Namval_t *mp, int flags, Namfun_t *fp)
{
	register struct parent	*dp = (struct parent*)mp->nvfun;
	register Cxstate_t	*sp = dp->state;
	register Cxquery_t	*qp;
	register Nambfun_t	*nbf;
	register int		n;
	int			level, offset=stktell(stkstd);
	size_t			size;
	for(level=0; level<2; level++)
	{
		for (n=0, qp = (Cxquery_t*)dtfirst(sp->queries); qp; qp = (Cxquery_t*)dtnext(sp->queries, qp))
		{
			if(strcmp(qp->name,"print")==0)
				continue;
			if(strcmp(qp->name,"null")==0)
				continue;
			if(qp->method && !strmatch(qp->name,qp->method))
				continue;
			if(level)
			{
				struct query *pp = newof(NiL,struct query,1,0);
				pp->query = qp;
				pp->dss = dp;
				sfputc(stkstd,0);
				nbf->bnames[n] = qp->name;
				sfprintf(stkstd,"%s.%s",nv_name(mp),qp->name);
				sfputc(stkstd,0);
				nbf->bltins[n] =  sh_addbuiltin(stkptr(stkstd,offset+1), query,pp);
				nv_offattr(nbf->bltins[n],NV_NOFREE);
				stkseek(stkstd,offset);
			}
			n++;
		}
		if(level==1)
			break;
		size = (n+1)*sizeof(char*) + n*sizeof(Namval_t*);
		nbf = newof(NiL, Nambfun_t, 1, size);
		nbf->fun.dsize = sizeof(Nambfun_t)+size;
		nbf->fun.disc = nv_discfun(NV_DCADD);
		nbf->num = n;
		nbf->bnames = (const char**)&nbf->bltins[n+1];
		nv_disc(mp,&nbf->fun,NV_FIRST);
	}
	nv_disc(mp,&nbf->fun,NV_POP);
	return((Namfun_t*)nbf);
}

static const Namdisc_t bltin_disc =
{
	0,0,0,0,0,0,clone_bltin
};

static const Namdisc_t parent_disc =
{
	sizeof(struct parent),
	put_parent,
	0,
	0,
	0,
	create_parent,
	clone_parent,
	0,
	next_parent,
	0,
	read_parent
};

/*
 * should only get here when requesting raw binary data
 * This must come after the nv_tree discipline
 */
static char* get_parent(register Namval_t* np, Namfun_t *fp)
{
	Dssrecord_t	*rp = (Dssrecord_t*)np->nvalue;
	char		*cp;
	if(rp && rp->size>0)
	{
		size_t m, n = (4*rp->size)/3 + rp->size/45 + 8;
		m = base64encode(rp->data,rp->size, (void**)0, cp=getbuf(n), n, (void**)0);
		nv_setsize(np,m);
		return(cp);
	}
	return(0);
}

static const Namdisc_t parent2_disc =
{
	0,0,get_parent
};

/*
 * add discipline builtin given by name to type
 */
static Namval_t *add_discipline(struct type *tp, const char *name, int (*fun)(int, char*[],void*))
{
	Namval_t *mp;
	int offset = stktell(stkstd);
	sfputc(stkstd,0);
	sfprintf(stkstd,NV_CLASS".dss.%s.%s",tp->type->name,name);
	sfputc(stkstd,0);
	mp =  sh_addbuiltin(stkptr(stkstd,offset+1),fun,(void*)tp);
	stkseek(stkstd,offset);
	nv_onattr(mp,NV_BLTINOPT);
	nv_offattr(mp,NV_NOFREE);
	return(mp);
}

static void put_type(Namval_t* np, const char* val, int flag, Namfun_t* fp)
{
	struct type	*tp = (struct type*)fp;
	Cxvalue_t	cval;
	if(val && !(flag&NV_INTEGER) && tp->type->internalf && !cxisstring(tp->type))
	{
		size_t	 size = strlen(val);
		if((*tp->type->internalf)(tp->cx, tp->type, NiL, NiL, &cval, val, size, Vmregion, &Dssdisc) <0)
			errormsg(SH_DICT,ERROR_exit(1),"%s: cannot covert to type dss.%s",val,tp->type->name);
		if(cxisnumber(tp->type))
			nv_putv(np,(char*)&cval.number,flag|NV_LONG|NV_INTEGER|NV_DOUBLE,fp);
		else
		{
			nv_setsize(np,cval.buffer.size);
			nv_putv(np, (char*)cval.buffer.data, NV_RAW|NV_BINARY,fp);
		}
	}
	else
		nv_putv(np,val,flag,fp);
	if(!val)
	{
		nv_disc(np,fp,NV_POP);
		if(fp->nofree==0)
#if HUH
			free((void*)fp);
#else
			;
#endif
		else if(--fp->nofree==0)
		{
			Namval_t *mp;
			int i;
			for(i=0; i < sizeof(tp->bltins)/sizeof(Namval_t*); i++)
			{
				if(mp = tp->bltins[i])
				{
					Shell_t *shp = sh_getinterp();
					dtdelete(shp->bltin_tree,mp);
					free((void*)mp);
				}
			}
			free((void*)fp);
		}
	}
}

static char* get_type(register Namval_t* np, Namfun_t *fp)
{
	struct type	*tp = (struct type*)fp;
	Cxvalue_t	cval;
	int		i,n;
	char		*buf,*format;
	Cxvariable_t	*vp=0;
	if(!tp->type->externalf)
		return(nv_getv(np,fp));
	if(nv_isattr(np,NV_INTEGER))
		cval.number = nv_getn(np,fp);
	else
	{
		cval.string.data = nv_getv(np,fp);
		if(nv_isattr(np,NV_BINARY) && nv_isattr(np,NV_RAW))
			return(cval.string.data);
		if((n=nv_size(np))==0)
			n = strlen(cval.string.data);
		cval.string.size = n;
	}
	buf = getbuf(32);
	if(nv_hasdisc(np,&child_disc))
		vp = vnode(np);
	if(!vp || !(format=vp->format.details))
		format = tp->format;
	for(i=0; i < 2; i++)
	{
		n = (*tp->type->externalf)(tp->cx, tp->type, format, NiL, &cval, buf, buflen, &Dssdisc);
		if(n<buflen)
			break;
		buf = getbuf(n);
	}
	return(buf);
}

/*
 *  They can all share one instance of the discipline
 */
static Namfun_t *clone_type(Namval_t* np, Namval_t *mp, int flags, Namfun_t *fp)
{
	fp->nofree++;
	return(fp);
}

static char *setdisc_type(Namval_t *np, const char *event, Namval_t* action, Namfun_t
 *fp)
{
	struct type *tp = (struct type*)fp;
	int n = -1;
	const char *name = event?event:(const char*)action;
	if(name)
	{
		if(strcmp(name,"match")==0 && tp->type->match)
			n = 0;
		else if(strcmp(name,"format")==0 && tp->type->format.details)
			n = 1;
	}
	if(!event)
	{
		if(!action && tp->type->match)
			return("match");
		if(n<=0 && tp->type->format.details)
			return("format");
		n = -1;
	}
	if(n<0)
		return(nv_setdisc(np,event,action,fp));
	if(action==np)
	{
		/* clone the discipline for variable specific function */
		if(fp->nofree && fp->type!=np)
		{
			tp = (struct type*)nv_disc(np, fp, NV_CLONE);
			fp->nofree--;
		}
		action = tp->bltins[n];
	}
	else if(action)
		tp->bltins[n] = action;
	else
	{
		action = tp->bltins[n];
		tp->bltins[n] = 0;
	}
	return(action?(char*)action:"");
}

static Namval_t *create_type(Namval_t *np,const char *name,int flag,Namfun_t *fp)
{
	struct type	*tp = (struct type*)fp;
	if(!fp->next)
	{
		pushtype(np, tp->type, fp);
		fp = nv_hasdisc(np,fp->disc);
	}
	while (fp = fp->next)
		if (fp->disc && fp->disc->createf)
			return((*fp->disc->createf)(np, name, flag, fp));
	return(0);
}

static const Namdisc_t type_disc =
{
	0,
	put_type,
	get_type,
	0,
	setdisc_type,
	create_type,
	clone_type
};

static const char sh_opttype[] =
"[-1c?\n@(#)$Id: type (AT&T Research) 2005-04-22 $\n]"
USAGE_LICENSE
"[+NAME?\f?\f - set the type of variables to \b\f?\f\b]"
"[+DESCRIPTION?\b\f?\f\b sets type on each of the variables specified "
	"by \aname\a to \b\f?\f\b. If \b=\b\avalue\a is specified, "
	"the variable \aname\a is set to \avalue\a before the variable "
	"is converted to \b\f?\f\b.]"
"[+?If no \aname\as are specified then the names and values of all "
	"variables of this type are written to standard output.]" 
"[+?\b\f?\f\b is built-in to the shell as a declaration command so that "
	"field splitting and pathname expansion are not performed on "
	"the arguments.  Tilde expansion occurs on \avalue\a.]"
	"[+?The types are:]{\ftypes\f}"
"[r?Enables readonly.  Once enabled, the value cannot be changed or unset.]"
"[p?Causes the output to be in a form of \b\f?\f\b commands that can be "
	"used as input to the shell to recreate the current type of "
	"these variables.]"
"\n"
"\n[name[=value]...]\n"
"\n"
"[+EXIT STATUS?]{"
        "[+0?Successful completion.]"
        "[+>0?An error occurred.]"
"}"

"[+SEE ALSO?\breadonly\b(1), \btypeset\b(1)]"
;

static void addtype(Namval_t *np, Cxheader_t *hp)
{
	Namtype_t	*cp = newof((Namtype_t*)0,Namtype_t,1,sizeof(Dssoptdisc_t));
	Dssoptdisc_t	*dp = (Dssoptdisc_t*)(cp+1);
	Shell_t		*shp = sh_getinterp();
	Namval_t	*mp,*bp;
	cp->optstring = sh_opttype;
	memset((void*)dp,0,sizeof(*dp));
        dp->header = hp;
        dp->optdisc.infof = dssoptinfo;
	cp->optinfof = (void*)dp;
	cp->shp = (void*)shp;
	cp->np = np;
	mp = nv_search("typeset",shp->bltin_tree,0);
	bp = sh_addbuiltin(hp->name, (int(*)(int,char*[],void*))mp->nvalue, (void*)cp); 
	nv_onattr(bp,nv_isattr(mp,NV_PUBLIC));
}

static void mktype(Namval_t *np, Cxtype_t *tp, Cx_t *cx)
{
	struct type	*pp;
	pp = newof((struct type*)0, struct type,1,0);
	pp->fun.dsize = sizeof(struct type);
	pp->fun.type = np;
	pp->fun.nofree = 1;
	pp->fun.disc = &type_disc;
	pp->type = tp;
	pp->cx = cx;
	nv_disc(np,&pp->fun,NV_FIRST);
	if(tp->match)
		pp->bltins[0] = add_discipline(pp, "match", match);
	if(tp->format.details)
		pp->bltins[1] = add_discipline(pp, "format", format);
	addtype(np,(Cxheader_t*)tp);
	while(tp = tp->base)
	{
		if(tp->base && !typenode(tp->name, 0))
			create_dss((Namval_t*)0, tp->name, 0, (Namfun_t*)0);
	}
}

static Namval_t *create_dss(Namval_t *np,const char *name,int flag,Namfun_t *fp)
{
	Cxvariable_t	*vp;
	Cxstate_t	*sp;
	Cxtype_t	*tp=0;
	Cx_t		*cx;
	struct parent	*dp;
	Namval_t	*mp;
	Dss_t		*dss=0;
	Dssmeth_t	*meth;
	int		n;
	Dt_t		*dict;
	Namval_t	*parent = nv_lastdict();
	Dssdisc.errorf = (Error_f)errorf;
	if(meth = dssmeth(name,&Dssdisc))
	{
		if(!(dss = dssopen(0, 0,&Dssdisc,meth)))
			errormsg(SH_DICT,ERROR_exit(1),"dssopen failed");
		cx = dss->cx;
		sp = cx->state;
	}
	else
	{
		Dssstate_t *state = dssstate(&Dssdisc);
		if(!state || !(tp = dtmatch(state->cx->types,name)))
			errormsg(SH_DICT,ERROR_exit(1),"%s: unknown dss type",name);
		sp = state->cx;
		cx = cxopen(0,0,&Dssdisc);
	}
	mp = typenode(name,NV_ADD);
	if(dss)
		dict = dss->cx->variables;
	else
	{
		check_numeric(mp, tp, sp);
		if(!tp->member || !(dict = tp->member->members))
		{
			mktype(mp, tp, cx);
			return(mp);
		}
	}
	for (n=0,vp = (Cxvariable_t*)dtfirst(dict); vp; vp = (Cxvariable_t*)dtnext(dict, vp))
	{
		Namval_t *qp;
		vp->data = (void*)n++;
		if(vp->type==sp->type_number || vp->type==sp->type_string || vp->type==sp->type_buffer)
			continue;
		if(!(qp=typenode(vp->type->name,0)))
		{
			if(!vp->type->member && (qp=typenode(vp->type->name,NV_ADD)))
			{
				mktype(qp,vp->type,cx);
				check_numeric(qp, vp->type, sp);
			}
			else if(vp->type->member)
				qp = create_dss(np, vp->name,flag,fp);
		}
	}
	if(!(dp = newof((struct parent*)0, struct parent,1,0)))
		return(0);
	dp->numnodes = n;
	dp->fun.disc = &parent_disc;
	dp->fun.type = mp;
	dp->sh = sh_getinterp();
	dp->parent = parent;
	dp->dss = dss;
	dp->cx = cx;
	dp->dict = dict;
	dp->state = sp;
	dp->type = tp;
	dp->disc = &Dssdisc;
	dp->childfun.fun.disc = meth?&mchild_disc:&child_disc;
	dp->childfun.fun.nofree = n;
	dp->childfun.parent = mp;
	dp->childfun.dss = dp;
	dp->parentfun.disc = &parent2_disc;
	dp->parentfun.nofree = 1;
	nv_disc(mp, &dp->parentfun,NV_FIRST);
	nv_setvtree(mp);
	if(dss)
	{
		fp = newof((Namfun_t*)0, Namfun_t,1,0);
		fp->disc = &bltin_disc;
		nv_disc(mp,fp,NV_FIRST);
	}
	nv_disc(mp,&dp->fun,NV_FIRST);
	mp->nvalue = 0;
	if(dss)
		nv_onattr(mp,NV_BINARY);
	addtype(mp,meth?(Cxheader_t*)meth:(Cxheader_t*)tp);
	return(mp);
}

static char *name_dss(Namval_t *np, Namfun_t *fp)
{
	int	len = sizeof(NV_CLASS)+strlen(np->nvname)+1 ;
	char	*name = getbuf(len);
	memcpy(name,NV_CLASS,sizeof(NV_CLASS)-1);
	name[sizeof(NV_CLASS)-1] = '.';
	strcpy(&name[sizeof(NV_CLASS)],np->nvname);
	return(name);
}

static const Namdisc_t dss_disc =
{
	0,
	0,
	0,
	0,
	0,
	create_dss,
	0,
	name_dss,
};

static const char *discnames[] = { "list", "load", 0 };

static const char optlist[] =
"[-1c?\n@(#)$Id: dss.list (AT&T Research) 2003-01-10 $\n]"
USAGE_LICENSE
"[+NAME?\f?\f - list the known dss entities]"
"[+DESCRIPTION?\b\f?\f\b causes each of the specified dss entities "
	"specified as options to be written to standard output.  If no "
	"options are specified, all entities are written.]"
"[+?If multiple entities types are written, then the entity type is written "
	"before each of the names.]"
"[l?List the names of library entities.]"
"[m?List the names of method entities.]"
"[q?List the names of query entities.]"
"[t?List the names of type entities.]"
"[v?A description of each entity will be listed along with the name.]"
"\n"
"\n\n"
"\n"
"[+EXIT STATUS?]{"
        "[+0?Successful completion.]"
        "[+>0?An error occurred.]"
"}"
"[+SEE ALSO?\b"NV_CLASS".dss.load\b(1)]"
;

#define fval(x)		(1L<<(x)-'a')
static const char *listnames[] = { "library", "method", "query", "type", 0 };

static int listdss(int argc, char *argv[], void *data)
{
	Cxstate_t	*sp = cxstate((Cxdisc_t*)data);
	int		flags=0, n, delim='\n';
	Dt_t		*dict;
	char		*name;
	Cxtype_t	*tp;
	NOT_USED(argc);
	while((n = optget(argv,optlist))) switch(n)
	{
	    case 'm': case 'l': case 't': case 'q':
		flags |= fval(n);
		break;
	    case 'v':
		delim = '\t';
		break;
	    case ':':
		errormsg(SH_DICT,2, "%s", opt_info.arg);
	    case '?':
		errormsg(SH_DICT,ERROR_usage(0), "%s", opt_info.arg);
		return(2);
	}
	argv += opt_info.index;
	if(error_info.errors || *argv)
		 errormsg(SH_DICT,ERROR_usage(2),"%s",optusage((char*)0));
	if(flags==0)
		flags = fval('m')|fval('l')|fval('q')|fval('t');
	n = flags&(flags-1);
	argv = (char**)listnames;
	while(name = *argv++)
	{
		if(fval('t') && *name=='t')
			dict =  sp->types;
		else if(fval('l') && *name=='l')
			dict =  sp->libraries;
		else if(fval('q') && *name=='q')
			dict =  sp->queries;
		else if(fval('m') && *name=='m')
			dict =  sp->methods;
		else
			continue;
		for (tp = (Cxtype_t*)dtfirst(dict); tp; tp = (Cxtype_t*)dtnext(dict, tp))
		{
			if(n)
				sfprintf(sfstdout,"%s\t",name);
			sfputr(sfstdout,tp->name,delim);
			if(delim!='\n')
				sfputr(sfstdout,tp->description,'\n');
		}
	}
	return(error_info.errors);
}

static const char optload[] =
"[-1c?\n@(#)$Id: dss.load (AT&T Research) 2003-01-10 $\n]"
USAGE_LICENSE
"[+NAME?\f?\f - load a dss format library]"
"[+DESCRIPTION?\b\f?\f\b causes each of the specified dss libraries \alib\a "
	"to be loaded into the shell and its contents merged with other "
	"dss libraries.]"
"\n"
"\nlib ...\n"
"\n"
"[+EXIT STATUS?]{"
        "[+0?Successful completion.]"
        "[+>0?An error occurred.]"
"}"
"[+SEE ALSO?\b"NV_CLASS".dss.list\b(1)]"
;

static int loadlib(int argc, char *argv[], void *data)
{
	Cxdisc_t	*dp = (Cxdisc_t*)data;
	char		*name;
	int		n;
	NOT_USED(argc);
	while((n = optget(argv,optload))) switch(n)
	{
	    case ':':
		errormsg(SH_DICT,2, "%s", opt_info.arg);
	    case '?':
		errormsg(SH_DICT,ERROR_usage(0), "%s", opt_info.arg);
		return(2);
	}
	argv += opt_info.index;
	if(error_info.errors || !*argv)
		 errormsg(SH_DICT,ERROR_usage(2),"%s",optusage((char*)0));
	while(name = *argv++)
	{
		if(dssload(name, dp) < 0)
			error_info.errors = 1;
	}
	return(error_info.errors);
}

static Cxvalue_t *getvalue(Namval_t *np, Namfun_t *fp, Cxoperand_t *valp) 
{
	if(fp && fp->disc==&child_disc)
	{
		struct child *pp = (struct child*)fp;
		struct parent *dp = pp->dss;
		Cxvariable_t *vp = vnode(np);
		cxcast(dp->cx,valp,vp,vp->type,pp->parent->nvalue,(char*)0);
	}
	else
	{
		if(nv_isattr(np, NV_INTEGER))
			valp->value.number = nv_getnum(np);
		else
		{
			valp->value.string.data = nv_getval(np);
			valp->value.string.size = nv_size(np);
			if(!valp->value.string.size)
				valp->value.string.size = strlen(valp->value.string.data);
		}
	}
	return(&valp->value);
}

static const char optmatch[] =
"[-1c?\n@(#)$Id: dss.match (AT&T Research) 2003-01-15 $\n]"
USAGE_LICENSE
"[+NAME?\f?\f - match a dss type variable to a pattern]"
"[+DESCRIPTION?\b\f?\f\b causes the value of the variable whose name "
	"precedes \b.match\b in the callers name to be matched against "
	"the specified pattern \apattern\a.  The interpretation of "
	"\apattern\a depends on the type of this variable.]"
"[+?\fmatch\f]"
"[v?\apattern\a is a variable name that contains the pattern.]"
"\n"
"\npattern\n"
"\n"
"[+EXIT STATUS?]{"
        "[+0?The value of the variable matched \apattern\a.]"
        "[+1?The value of the variable did not match \apattern\a.]"
        "[+2?A usage error was found or a help request was made.]"
        "[+>2?An error occurred.]"
"}"
"[+SEE ALSO?\bdss\b(3)]"
;

static int match(int argc, char *argv[], void *ptr)
{
	Nambltin_t	*bp = (Nambltin_t*)ptr;
	struct type	*tp = (struct type*)bp->ptr;
	Cxmatch_t	*mp = tp->type->match;
	Namval_t	*np;
	Namfun_t	*fp;
	Cxoperand_t	cval;
	Cxvalue_t	*valp = &cval.value;
	Cxtype_t	*tptr=0;
	Dssoptdisc_t	disc;
	char		*cp;
	void		*comp;
	int		n, flag=0;
	if(comp=bp->data)
	{
		if(!(bp->flags&SH_END_OPTIM))
			goto exec;
		(*mp->freef)(tp->cx, comp, &Dssdisc);
		bp->data = 0;
		if(argc<=0)
			return(0);
	}
	memset((void*)&disc,0,sizeof(disc));
	disc.header = (Cxheader_t*)tp->type;
	disc.optdisc.infof = dssoptinfo;
	opt_info.disc = &disc.optdisc;
	while((n = optget(argv,optmatch))) switch(n)
	{
	    case 'v':
		flag = 'v';
		break;
	    case ':':
		errormsg(SH_DICT,2, "%s", opt_info.arg);
	    case '?':
		errormsg(SH_DICT,ERROR_usage(0), "%s", opt_info.arg);
		opt_info.disc = 0;
		return(2);
	}
	opt_info.disc = 0;
	argv += opt_info.index;
	if(error_info.errors || !(cp= *argv))
		 errormsg(SH_DICT,ERROR_usage(2),"%s",optusage((char*)0));
	if(flag)
	{
		if(!(np = nv_open(*argv, ((Shell_t*)bp->shp)->var_tree, NV_NOADD|NV_VARNAME|NV_NOASSIGN)))
			errormsg(SH_DICT,ERROR_exit(3),"%s: variable not set",*argv);
		if((fp=nv_hasdisc(np,&mchild_disc)) || (fp=nv_hasdisc(np,&child_disc)))
		{
			Namval_t *mp = (*fp->disc->typef)(np,fp);
			if(mp)
				tptr = ((struct type*)(mp->nvfun))->type;
			if(!tptr)
				fp = 0;
		}
		else if(fp=nv_hasdisc(np,&type_disc))
			tptr = ((struct type*)fp)->type;
		valp = getvalue(np, fp, &cval);
	}
	else
	{
		valp->string.data = cp;
		valp->string.size = strlen(cp);
	}
	if(!tptr)
		tptr = tp->cx->state->type_string;
	comp = (*mp->compf)(tp->cx, tp->type, tptr, valp, &Dssdisc);
	if(!comp)
		return(3);
	if(bp->flags&SH_BEGIN_OPTIM)
		bp->data = comp;
exec:
	if(bp->np)
		fp = nv_hasdisc(bp->np, &child_disc);
	valp = getvalue(bp->np, fp, &cval);
	n = (*mp->execf)(tp->cx, comp, valp, &Dssdisc);
	if(!bp->data)
		(*mp->freef)(tp->cx, comp, &Dssdisc);
	return(n<0?4:!n);
}

static const char optformat[] =
"[-1c?\n@(#)$Id: dss.format (AT&T Research) 2003-01-17 $\n]"
USAGE_LICENSE
"[+NAME?\f?\f - specify a format string for a dss type]"
"[+DESCRIPTION?\b\f?\f\b will set the format string to \aformat\a for the "
	"variable whose name precedes \b.format\b in this command name.  If "
	"the variable is a type variable, this format will be the default "
	"format for variables created of this type]"
"[+?If \aformat\a is omitted, the current format string is written onto "
	"standard output.]"
"[+?\fdetails\f]"
"\n"
"\n[format] ...\n"
"\n"
"[+EXIT STATUS?]{"
        "[+0?Successful completion.]"
        "[+>0?An error occurred.]"
"}"
"[+SEE ALSO?\btypeset\b(1)]"
;

static int format(int argc, char *argv[], void *ptr)
{
	Nambltin_t		*bp = (Nambltin_t*)ptr;
	struct type		*tp = (struct type*)bp->ptr;
	Namfun_t		*fp;
	Cxvariable_t		*vp=0;
	Dssoptdisc_t		disc;
	register Namval_t	*np = bp->np;
	register char		*format=0;
	int			n;
	NOT_USED(argc);
	memset((void*)&disc,0,sizeof(disc));
	disc.header = (Cxheader_t*)tp->type;
	disc.optdisc.infof = dssoptinfo;
	opt_info.disc = &disc.optdisc;
	while((n = optget(argv,optformat))) switch(n)
	{
	    case ':':
		errormsg(SH_DICT,2, "%s", opt_info.arg);
	    case '?':
		errormsg(SH_DICT,ERROR_usage(0), "%s", opt_info.arg);
		opt_info.disc = 0;
		return(2);
	}
	opt_info.disc = 0;
	argv += opt_info.index;
	if(error_info.errors)
		 errormsg(SH_DICT,ERROR_usage(2),"%s",optusage((char*)0));
	n = 0;
	if(!(fp=nv_hasdisc(np,&type_disc)) && (fp=nv_hasdisc(np,&child_disc)))
		vp = vnode(np);
	if(vp)
	{
		format = vp->format.details;
		n = !(vp->format.flags&CX_FREE);
	}
	else if(fp->type==np)
	{
		format = tp->type->format.details;
		n = !(tp->type->format.flags&CX_FREE);
	}
	else
		format = tp->format;
	if(*argv)
	{
		if(format && !n)
			free((void*)format);
		format = strdup(*argv);
		if(vp)
		{
			vp->format.details = format;
			vp->format.flags |= CX_FREE;
		}
		else if(fp->type==np)
		{
			tp->type->format.details = format;
			tp->type->format.flags |= CX_FREE;
		}
		else
		{
			if(fp->nofree)
			{
				fp->nofree--;
				tp = (struct type*)nv_disc(np, fp, NV_CLONE);
			}
			else
				tp = (struct type*)fp;
			tp->format = format;
		}
	}
	else
	{
		if(!format)
			format = tp->type->format.details;
		sfputr(sfstdout,sh_fmtq(format),'\n');
	}
	return(0);
}

#ifdef _BLD_dsstype
#   define init_dss lib_init
#   ifdef __EXPORT__
	__EXPORT__
#   endif
#endif
void init_dss(int flag)
{
	Shell_t *shp = sh_getinterp();
	Namval_t *np,*rp;
	Namfun_t *nfp = newof(NiL,Namfun_t,1,0);
	char tmp[sizeof(NV_CLASS)+17];
	dssinit(&Dssdisc,0);
	sfsprintf(tmp, sizeof(tmp), "%s.dss", NV_CLASS);
	np = nv_open(tmp, shp->var_tree, NV_VARNAME);
	typedict = nv_dict(np);
	nfp->disc = &dss_disc;
	nv_disc(np,nfp,NV_FIRST);
	nv_adddisc(np,discnames,0);
	sfsprintf(tmp, sizeof(tmp), "%s.dss.load", NV_CLASS);
	sh_addbuiltin(tmp, loadlib, &Dssdisc); 
	sfsprintf(tmp, sizeof(tmp), "%s.dss.list", NV_CLASS);
	sh_addbuiltin(tmp, listdss, &Dssdisc); 
	/* create reference variable dss to NV_CLASS.dss */
	rp = nv_open("dss", shp->var_tree, NV_IDENT);
	nv_unset(rp);
	nv_putval(rp,nv_name(np),NV_NOFREE);
	nv_setref(rp);
}
