#include <nval.h>

#define nv_opensub(nv)	(nv->nvalue ? ((Assoc *)((nv)->nvalue))->namval : NULL)
/* nv->nvalue ? was added because of a crash in var.c:473 = tksh_arrputval */

#define nv_getvalue(np, val)	do					\
				{					\
					Namfun_t *nf;			\
					nf = (np)->nvfun;		\
					(np)->nvfun = NULL;		\
					(val) = nv_getval(np);		\
					(np)->nvfun = nf;		\
				} while (0)

#define nv_putvalue(np, val,f)	do					\
				{					\
					Namfun_t *nf;			\
					nf = (np)->nvfun;		\
					(np)->nvfun = NULL;		\
					nv_putval(np,val,f);		\
					(np)->nvfun = nf;		\
				} while (0)

#define nv_stopdisc(np)	do						\
			{						\
				Namfun_t *nf = (Namfun_t *)		\
					malloc(sizeof(Namfun_t));	\
				nf->disc = & tksh_trace_stop;		\
				nv_stack(np, nf);			\
			} while (0)

#define nv_resumedisc(np)	(free (nv_stack(np, NULL)))

#define ov_return(msg)	do { errmsg = (msg) ; goto scalar; } while(0)


#define nv_scanfrom(nv, name)   do {					\
				   nv_putsub((nv),(name),ARRAY_SCAN);	\
				   if (name) {				\
				   while (nv_getsub(nv)) {		\
					if (strcmp((name),nv_getsub(nv))==0) \
						break; 			\
					nv_nextsub(nv);			\
					 }				\
				   }					\
				} while(0)

#define nv_makearray(nv)	do {					\
					Namfun_t *nf;			\
					nf = namval->nvfun;		\
					namval->nvfun = NULL;		\
					nv_setarray(namval, nv_associative); \
					namval->nvfun = nf;		\
				} while(0)

#ifndef NEWKSH
#define nv_isarray(nv) (nv_isattr((nv), NV_ARRAY))
#endif

#define nv_inscan(nv) (((Namarr_t *) (nv)->nvalue)->nelem & ARRAY_SCAN)
#define nv_notsub(np,sub) ( (!nv_putsub((np),(sub),0)) || (!nv_getsub(np)) )
#define nv_setsub(np,sub) (nv_putsub((np),(sub),ARRAY_ADD) && nv_getsub(np) && \
				(! nv_isnull(nv_opensub(np))) )
#undef nv_isnull
#define nv_isnull(nv)	((nv)->nvalue == NULL )
#define nv_subnull(np)	  ( (!nv_getsub(np)) || nv_isnull(nv_opensub(np)))
#define nv_subnullf(np)	  (nv_isnull(nv_opensub(np))&& !(nv_opensub(np)->nvfun))

#ifndef nv_onattr
#define nv_onattr(np,f)         ((np)->nvflag |= (f))
#define nv_offattr(np,f)        ((np)->nvflag &= (~(f)))
#endif

typedef struct Assoc
{
	char		pad[offsetof(Namval_t, nvalue)];
	Namval_t	*namval;
} Assoc;


#ifndef NV_NODISC
#define NV_NODISC     NV_IDENT
#endif
