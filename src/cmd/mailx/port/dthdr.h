#ifndef _DTHDR_H
#define _DTHDR_H	1
#define _KPV_ONLY	1

/*	Internal definitions for libcdt.
**	Written by Kiem-Phong Vo (07/15/95)
*/

#include	"cdt.h"

/* short-hand notations */
#define reg	register
#define ulong	unsigned long
#define left	hl._left
#define hash	hl._hash
#define htab	hh._htab
#define head	hh._head

/* this must be disjoint from DT_METHODS */
#define DT_FLATTEN	010000	/* dictionary already flattened	*/

/* bits used for secondary hashing */
#define HIGHBIT		(~(((ulong)~0L) >> 1))
#define SETVISIT(h)	((h) | HIGHBIT)
#define CLRVISIT(h)	((h) & ~HIGHBIT)
#define ISVISIT(h)	((h) & HIGHBIT)
#define REHASH(h)	(((h)<<5) - (h))

/* hash start size and load factor */
#define HSLOT		(32)
#define HRESIZE(n)	((n) << 1)
#define HLOAD(s)	((s) << 1)
#define HINDEX(n,h)	((h)&((n)-1))

#define UNFLATTEN(dt) \
		((dt->data->type&DT_FLATTEN) ? dtrestore(dt,NIL(Dtlink_t*)) : 0)

/* the pointer to the actual object */
#define ELT(o,d)	((Dtlink_t*)((char*)(o) + d->link) )
#define OBJ(e,d)	(d->link == 0 ? (Void_t*)(e) : \
			 d->link <  0 ? ((Dthold_t*)(e))->obj : \
					(Void_t*)((char*)(e) - d->link) )
#define KEY(o,d)	(d->size < 0 ?	*((char**)((char*)(o) + d->key)) : \
					((char*)(o) + d->key) )

/* compare function */
#define CMP(dt,k1,k2,d) (d->comparf ? (*d->comparf)(dt,k1,k2,d) : \
		 	 d->size <= 0 ? strcmp(k1,k2) : memcmp(k1,k2,d->size) )

/* hash function */
#define HASH(dt,k,d)	(d->hashf ? (*d->hashf)(dt,k,d) : dtstrhash(0,k,d->size) )

/* tree rotation functions */
#define RROTATE(x,y)	((x)->left = (y)->right, (y)->right = (x), (x) = (y))
#define LROTATE(x,y)	((x)->right = (y)->left, (y)->left = (x), (x) = (y))
#define RLINK(r,x)	(r = r->left = x )
#define LLINK(l,x)	(l = l->right = x )

_BEGIN_EXTERNS_

#if defined(extern)
#undef extern
#endif
#if !_PACKAGE_ast && _BLD_DLL && defined(_WIN32)
#define extern	 	__declspec(dllimport)
#endif

#if !_PACKAGE_ast
extern Void_t*	malloc _ARG_((int));
extern void	free _ARG_((Void_t*));
extern int	memcmp _ARG_((const Void_t*, const Void_t*, size_t));
extern int	strcmp _ARG_((const char*, const char*));
#endif

#if defined(extern)
#undef extern
#endif
#if _PACKAGE_ast && defined(__EXPORT__)
#define extern		__EXPORT__
#endif

_END_EXTERNS_

#endif /* _DTHDR_H */
