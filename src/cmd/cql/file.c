/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1991-2002 AT&T Corp.                *
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
/*
 * Glenn Fowler
 * AT&T Research
 *
 * cql file and record support
 */

#include "cql.h"

#define ROTATE(p,l,r,t)	((t)=(p)->l,(p)->l=(t)->r,(t)->r=(p),(p)=(t))

/*
 * free cdt table entry
 */

static void
cdtobjfree(Dt_t* dt, void* obj, Dtdisc_t* disc)
{
	NoP(dt);
	NoP(disc);
	free(obj);
}

/*
 * return update for offset
 */

static Update_t*
search(Update_t** rootp, unsigned long offset, int fields)
{
	register Update_t*	t;
	register Update_t*	left;
	register Update_t*	right;
	register Update_t*	root;
	register Update_t*	lroot;
	register Update_t*	rroot;

	register long		c;

	root = *rootp;
	left = right = lroot = rroot = 0;
	while (root)
	{
		if (!(c = offset - root->offset)) break;
		if (c < 0)
		{
			if (root->left && (c = offset - root->left->offset) <= 0)
			{
				ROTATE(root, left, right, t);
				if (!c) break;
			}
			if (right) right->left = root;
			else rroot = root;
			right = root;
			root = root->left;
			right->left = 0;
		}
		else
		{
			if (root->right && (c = offset - root->right->offset) >= 0)
			{
				ROTATE(root, right, left, t);
				if (!c) break;
			}
			if (left) left->right = root;
			else lroot = root;
			left = root;
			root = root->right;
			left->right = 0;
		}
	}
	if (root)
	{
		if (right) right->left = root->right;
		else rroot = root->right;
		if (left) left->right = root->left;
		else lroot = root->left;
	}
	else if (fields && (root = newof(0, Update_t, 1, (fields - 1) * sizeof(char**))))
		root->offset = offset;
	if (root)
	{
		root->left = lroot;
		root->right = rroot;
		*rootp = root;
	}
	else if (left)
	{
		left->right = rroot;
		*rootp = lroot;
	}
	else if (right)
	{
		right->left = lroot;
		*rootp = rroot;
	}
	return root;
}

static int	partition;		/* partition check		*/

/*
 * read and split cdb record
 */

static void*
hix_split(Hix_t* hix, Sfio_t* ip, char* path, char* tag, Hixdisc_t* disc)
{
	register File_t*	f = HIXDISC2FILE(disc);
	register Cdbrecord_t*	r;
	Sfoff_t			cur;
	Sfoff_t			nxt;

	f->cdb->io = ip;
	f->cdb->path = path;
	cur = sftell(ip);
	if (r = cdbread(f->cdb, f->cdbkey))
	{
		nxt = sftell(ip);
		hix->size = nxt - cur;
		return (void*)r;
	}
	hix->size = 0;
	return 0;
}

/*
 * data for cdb.disc.indexf
 */

typedef struct Pval_s
{
	struct Pval_s*	next;
	char*		val;
} Pval_t;

typedef struct Psym_s
{
	struct Psym_s*	next;
	Exid_t*		sym;
	Pval_t*		val;
	size_t		num;
} Psym_t;

typedef struct
{
	Psym_t*		sym;
	Psym_t*		sorted;
	int		indexed;
	Vmalloc_t*	vm;
} Prune_t;

typedef struct
{
	const char*	val;
	size_t		off;
	size_t		len;
} Kval_t;

struct Cdbkey_s
{
	size_t		fixed;
	size_t		num;
	Kval_t*		kv;
	Kval_t		val[1];
};

/*
 * generate prune info for cdb.disc.indexf
 */

static int
prune(register Hix_t* hix, register Exnode_t* x, Prune_t* pp)
{
	register Exnode_t*	l;
	register Exnode_t*	r;
	register Extype_t**	v;
	Exid_t*			sym;
	Psym_t*			ps;
	Pval_t*			pv;

	for (;;) switch (x->op)
	{
	case AND:
		if (prune(hix, x->data.operand.left, NiL))
		{
			if (pp)
			{
				prune(hix, x->data.operand.left, pp);
				if (prune(hix, x->data.operand.right, NiL))
					prune(hix, x->data.operand.right, pp);
			}
			return 1;
		}
		x = x->data.operand.right;
		break;
	case OR:
		if (prune(hix, x->data.operand.left, NiL) && prune(hix, x->data.operand.right, NiL))
		{
			if (pp)
			{
				prune(hix, x->data.operand.left, pp);
				prune(hix, x->data.operand.right, pp);
			}
			return 1;
		}
		return 0;
	case EQ:
		l = x->data.operand.left;
		if (l->op != ID)
		{
			r = l;
			l = x->data.operand.right;
			if (l->op != ID)
				return 0;
		}
		else
			r = x->data.operand.right;
		if (l->data.variable.reference || r->op != CONSTANT || r->type != STRING || strmatch(r->data.constant.value.string, MATCHPATTERN))
			return 0;
		sym = l->data.variable.symbol;
		if (GETINDEX(sym))
		{
			if (pp)
				pp->indexed = 1;
			return 0;
		}
		if (pp)
		{
			for (ps = pp->sym; ps && ps->sym != sym; ps = ps->next);
			if (!ps)
			{
				if (!(ps = vmnewof(pp->vm, 0, Psym_t, 1, 0)))
					error(ERROR_SYSTEM|3, "out of space [prune]");
				ps->next = pp->sym;
				pp->sym = ps;
				ps->sym = sym;
				if (sym->local.number & S_sorted)
					pp->sorted = ps;
			}
			if (!(pv = vmnewof(pp->vm, 0, Pval_t, 1, 0)))
				error(ERROR_SYSTEM|3, "out of space [prune]");
			ps->num++;
			pv->next = ps->val;
			ps->val = pv;
			pv->val = r->data.constant.value.string;
		}
		return 1;
	case SWITCH:
		l = x->data.operand.left;
		if (l->op != ID || l->data.variable.reference)
			return 0;
		sym = l->data.variable.symbol;
		if (GETINDEX(sym))
		{
			if (pp)
				pp->indexed = 1;
			return 0;
		}
		x = x->data.operand.right;
		if (x->data.operand.left)
			return 0;
		if (pp)
		{
			for (ps = pp->sym; ps && ps->sym != sym; ps = ps->next);
			if (!ps)
			{
				if (!(ps = vmnewof(pp->vm, 0, Psym_t, 1, 0)))
					error(ERROR_SYSTEM|3, "out of space [prune]");
				ps->next = pp->sym;
				pp->sym = ps;
				ps->sym = sym;
				if (sym->local.number & S_sorted)
					pp->sorted = ps;
			}
		}
		x = x->data.operand.right;
		for (v = x->data.select.constant; *v; v++)
		{
			if (l->type != STRING || strmatch((*v)->string, MATCHPATTERN))
				return 0;
			if (pp)
			{
				if (!(pv = vmnewof(pp->vm, 0, Pval_t, 1, 0)))
					error(ERROR_SYSTEM|3, "out of space [prune]");
				ps->num++;
				pv->next = ps->val;
				ps->val = pv;
				pv->val = (*v)->string;
			}
		}
		return 1;
	default:
		return 0;
	}
}

/*
 * cdb fixed sorted indexf
 */

static int
cdb_fs_index(Cdb_t* cdb, register Cdbkey_t* kp, Cdbdisc_t* disc)
{
	register unsigned char*	s;
	register unsigned char*	e;
	register Sfio_t*	ip;
	register Kval_t*	kv;

	unsigned char*		b;
	size_t			n;
	size_t			k;
	int			r;
	int			c;
	int			l;
	Kval_t*			ke;

	k = kp->fixed;
	if ((kv = kp->kv) >= (ke = kp->val + kp->num))
		return -1;
	ip = cdb->io;
	while (s = (unsigned char*)sfreserve(ip, SF_UNBOUND, l = 1))
	{
		b = s;
		n = sfvalue(ip);
		if (n < k)
		{
			sfread(ip, b, 0);
			if (!(s = (unsigned char*)sfreserve(ip, n = k, l = 0)))
			{
				if (disc->errorf)
					(*disc->errorf)(cdb, disc, 2, "%s: last record incomplete", cdb->path);
				return -1;
			}
		}
		r = n % k;
		e = s + n - r;
		if (n <= 2 * k || memcmp(kv->val, e - k + kv->off, kv->len) <= 0)
			for (; s < e; s += k)
			{
				while ((c = memcmp(kv->val, s + kv->off, kv->len)) < 0)
					if (++kv >= ke)
					{
						kp->kv = kv;
						return -1;
					}
				if (c == 0)
				{
					kp->kv = kv;
					if (l)
						sfread(ip, b, s - b);
					else
						sfseek(ip, (Sfoff_t)(-k), SEEK_CUR);
					return 0;
				}
			}
		if (l)
			sfread(ip, b, e - b);
	}
	if (sfvalue(ip) && disc->errorf)
		(*disc->errorf)(cdb, disc, ERROR_SYSTEM|2, "%s: read error", cdb->path);
	return -1;
}

/*
 * match val against sorted list in lo
 */

static int
match(register Kval_t* lo, size_t num, unsigned char* val)
{
	register Kval_t*	hi = lo + num - 1;
	register Kval_t*	mid;
	register unsigned char*	s;
	register int		c;
	register int		v;

	c = *val;
	while (lo <= hi)
	{
		mid = lo + (hi - lo) / 2;
		if (!(v = c - *(s = (unsigned char*)mid->val)) && !(v = memcmp(val, (char*)s, mid->len)))
			return 1;
		if (v > 0)
			lo = mid + 1;
		else
			hi = mid - 1;
	}
	return 0;
}

/*
 * cdb fixed indexf
 */

static int
cdb_f_index(Cdb_t* cdb, Cdbkey_t* kp, Cdbdisc_t* disc)
{
	register unsigned char*	s;
	register unsigned char*	e;
	register Sfio_t*	ip;

	unsigned char*		b;
	size_t			n;
	size_t			k;
	int			r;
	int			l;

	k = kp->fixed;
	ip = cdb->io;
	while (s = (unsigned char*)sfreserve(ip, SF_UNBOUND, l = 1))
	{
		b = s;
		n = sfvalue(ip);
		if (n < k)
		{
			sfread(ip, b, 0);
			if (!(s = (unsigned char*)sfreserve(ip, n = k, l = 0)))
			{
				if (disc->errorf)
					(*disc->errorf)(cdb, disc, 2, "%s: last record incomplete", cdb->path);
				return -1;
			}
		}
		r = n % k;
		for (e = s + n - r; s < e; s += k)
			if (match(kp->val, kp->num, s + kp->val->off))
			{
				if (l)
					sfread(ip, b, s - b);
				else
					sfseek(ip, (Sfoff_t)(-k), SEEK_CUR);
				return 0;
			}
		if (l)
			sfread(ip, b, e - b);
	}
	if (sfvalue(ip) && disc->errorf)
		(*disc->errorf)(cdb, disc, ERROR_SYSTEM|2, "%s: read error", cdb->path);
	return -1;
}

/*
 * order Kval_t by val
 */

static int
byval(const void* va, const void* vb)
{
	return strcmp(((Kval_t*)va)->val, ((Kval_t*)vb)->val);
}

/*
 * cdb event handler
 */

static int
cdb_event(Cdb_t* cdb, int op, void* arg, Cdbdisc_t* disc)
{
	register File_t*	f = CDBDISC2FILE(disc);
	register int		i;
	register Record_t*	r;
	Cdbmeth_t**		mp;


	switch (op)
	{
	case CDB_METH:
		/*
		 * assume Cdbflat if data does not self identify
		 */

		mp = (Cdbmeth_t**)arg;
		if (mp && !*mp)
			*mp = Cdbflat;
		break;
	case CDB_OPEN:
		if (!(state.test & 000040))
		{
			/*
			 * generate data for referenced fields only
			 */

			f->fixed = cdb->table[0]->fixed;
			if ((!f->index || *f->index == hixend) && (!f->force || f->fixed))
			{
				r = f->record;
				for (i = 0; i < r->fields; i++)
					if (!r->format[i].referenced && !r->member[i].index)
					{
						cdb->table[0]->format[i].flags = 0;
						if (state.test & 004000)
							error(0, "skip %s%s%s.%s", r->symbol->name, r->member[i].record ? "." : "", r->member[i].record ? r->member[i].record->symbol->name : "", r->member[i].symbol->name);
					}
					else if (state.test & 004000)
						error(0, "keep %s%s%s.%s [ referenced=%d index=%d ]", r->symbol->name, r->member[i].record ? "." : "", r->member[i].record ? r->member[i].record->symbol->name : "", r->member[i].symbol->name, r->format[i].referenced, r->member[i].index);
			}
		}
		if (state.loop[SELECT].body && cdb->table[0]->fixed)
		{
			Prune_t		pp;
			Psym_t*		ps;
			Psym_t*		bs;
			Pval_t*		pv;
			Cdbkey_t*	kp;
			Kval_t*		kv;
			Cdbformat_t*	fp;
			size_t		n;

			memset(&pp, 0, sizeof(pp));
			if (!(pp.vm = vmopen(Vmdcheap, Vmlast, 0)))
				error(ERROR_SYSTEM|3, "out of space [prune]");
			prune(NiL, state.loop[SELECT].body, &pp);
			if (!(bs = pp.sorted))
				for (ps = pp.sym; ps; ps = ps->next)
					if (!bs || ps->num < bs->num)
						bs = ps;
			if (bs)
			{
				if (!(kp = vmnewof(cdb->vm, 0, Cdbkey_t, 1, (bs->num - 1) * sizeof(Kval_t))))
					error(ERROR_SYSTEM|3, "out of space [index]");
				kp->num = bs->num;
				kp->kv = kp->val;
				kp->fixed = f->fixed;
				r = f->record;
				n = 0;
				fp = cdb->table[0]->format;
				for (i = 0; i < r->fields && r->member[i].symbol != bs->sym; i++)
					n += fp[i].width;
				for (pv = bs->val, kv = kp->val; pv; pv = pv->next, kv++)
				{
					kv->val = pv->val;
					kv->off = n;
					kv->len = strlen(kv->val);
				}
				qsort(kp->val, kp->num, sizeof(kp->val[0]), byval);
				f->cdbkey = kp;
				disc->indexf = pp.sorted ? cdb_fs_index : cdb_f_index;
				if (state.test & 004000)
					error(0, "indexf%s on %u value%s", pp.sorted ? " sorted" : "", kp->num, kp->num == 1 ? "" : "s");
			}
			vmclose(pp.vm);
		}
		break;
	}
	return 0;
}

/*
 * add cdb schema string value to sp
 */

static void
schemaval(Sfio_t* sp, int op, int dir, int val)
{
	char	esc[2];

	if (val >= 0)
	{
		esc[0] = val;
		esc[1] = 0;
		sfprintf(sp, "{%s}", fmtesc(esc));
		if (dir)
			sfputc(sp, dir);
	}
	sfputc(sp, op);
}

/*
 * hix event handler
 */

int
hix_event(Hix_t* hix, int op, void* arg, Hixdisc_t* disc)
{
	register File_t*	f = HIXDISC2FILE(disc);
	register Record_t*	r = f->record;
	register int		i;
	register int		n;
	register int		c;
	register int		delimiter;
	Sfio_t*			ip = (Sfio_t*)arg;
	Sfio_t*			sp;

	switch (op)
	{
	case HIX_OPEN:
		/*
		 * set up the hix split discipline
		 * cdb handles the details
		 */

		if (!f->schema)
		{
			/*
			 * generate the cdb schema from the cql schema
			 */

			if (!(sp = sfstropen()))
				error(3, "out of space [split]");
			if (f->delimiter == DELDEF)
				f->delimiter = DELIMITER;
			if (f->terminator == DELDEF)
				f->terminator = TERMINATOR;
			if (f->termset || f->terminator != CDB_TERMINATOR[0])
				schemaval(sp, 't', 0, f->terminator);
			if (r->format[0].delimiter == DELDEF)
				r->format[0].delimiter = f->delimiter;
			delimiter = r->format[0].delimiter;
			schemaval(sp, 'd', 0, delimiter);
			if (f->permanent)
				sfprintf(sp, "%dp", f->permanent);
			for (n = 0; n < r->subfields; n++)
			{
				if (r->format[n].referenced)
					r->format[n].referenced = F_STRING;
				if (r->format[n].format)
				{
					if (r->format[n].elements > 0 && !strchr(r->format[n].format, '*'))
						sfprintf(sp, "%d*", r->format[n].elements);
					sfputr(sp, r->format[n].format, -1);
					continue;
				}
				if (r->format[n].string && r->format[n].delimiter == DELDEF)
					r->format[n].delimiter = delimiter;
				if (r->format[n].delimiter != delimiter)
					schemaval(sp, 'd', r->format[n].direction, delimiter = r->format[n].delimiter);
				if (i = r->format[n].width)
					sfprintf(sp, "%d", i);
				if (r->format[n].base && r->format[n].base != 10)
					sfprintf(sp, ".%d", r->format[n].base);
				if (r->format[n].string)
					c = 's';
				else
					switch (r->format[n].type)
					{
					case FLOATING:
						c = 'f';
						break;
					case INTEGER:
						c = 'l';
						break;
					case UNSIGNED:
						c = 'w';
						break;
					default:
						c = 's';
						break;
					}
				if (i && r->format[n].delimiter == DELOFF)
					c = toupper(c);
				sfputc(sp, c);
			}
			if (!(f->schema = strdup(sfstruse(sp))))
				error(ERROR_SYSTEM|3, "out of space [schema]");
			sfclose(sp);
		}
		f->cdbdisc.version = CDB_VERSION;
		f->cdbdisc.schema = f->schema;
		f->cdbdisc.details = f->details ? f->details :
			((Local_t*)f->record->symbol->local.pointer)->file ?
			((Local_t*)f->record->symbol->local.pointer)->file->details : (char*)0;
		f->cdbdisc.errorf = (Cdberror_f)errorf;
		f->cdbdisc.eventf = cdb_event;
		message((-1, "cdb: open %s f=%p fp=%p schema=%s offset=%lld flags=%s details=%s", hix->name, f, ip, f->cdbdisc.schema, sftell(ip), cdbflags(NiL, CDB_READ|CDB_TERMINATED|state.cdb_flags|((error_info.trace<=-8)?((error_info.trace<=-11)?(CDB_DUMP|CDB_VERBOSE):CDB_DUMP):0)), f->cdbdisc.details));
		f->cdb = cdbopen(NiL, &f->cdbdisc, f->format, ip, hix->name, CDB_READ|CDB_TERMINATED|state.cdb_flags|((error_info.trace<=-8)?((error_info.trace<=-11)?(CDB_DUMP|CDB_VERBOSE):CDB_DUMP):0));
		if (!f->cdb)
			return -1;
		disc->splitf = hix_split;
		break;
	case HIX_CLOSE:
		if (f->cdb)
			return cdbclose(f->cdb);
		break;
	}
	return 0;
}

/*
 * split and return the record in f for index key k with hash h and index i
 * if k==0 then all records returned
 * if c==0 then caching turned off
 * if p>0 then partition restricted
 * if p<0 then saved partition number not updated
 */

Field_t*
record(register File_t* f, char* k, long h, int i, int c, int p)
{
	register Cache_t*	q;
	Cache_t*		w;
	Update_t*		x;
	int			d;
	int			n;
	Mark_t*			kp;

	q = f->cache;
	if (p > 0)
		p = partition;
	if (k)
	{
		if (c)
		{
			/*
			 * check for cache hit
			 */

			w = 0;
			if (d = (++f->decay >= DECAY))
				f->decay = 1;
			do
			{
				if (q->hash == h && q->index == i && (q->partition == p || q->partition <= 0 || p <= 0) && q->record)
				{
					/*
					 * cache hit
					 */

					q->hit++;
					q->sequence = state.record;
					f->cache = q;
					f->decay = 0;
					if (p >= 0)
						partition = q->partition;
					message((-8, "hit: cache=%s hit=%d key=%s hash=0x%08x index=%d partition=%d", filename(f), q->hit, k, h, i, q->partition));
					return q->record->data;
				}
				if (d && q->hit > 0)
					q->hit--;
				if ((!w || q->hit <= w->hit) && q->sequence != state.record)
					w = q;
			} while ((q = q->next) != f->cache);

			/*
			 * allocate a new cache record or reuse the worst
			 */

			if (!w || w->hit > f->limit)
			{
				message((-8, "add: cache=%s limit=%d hit=%d key=%s hash=0x%08x index=%d partition=%d", filename(f), f->limit, w ? w->hit : 0, k, h, i, w ? w->partition : 0));
				f->limit++;
				if (!(q = newof(0, Cache_t, 1, 0)))
					error(3, "out of space [cache]");
				q->next = f->cache->next;
				f->cache->next = q;
			}
			else
				(q = w)->hit = 0;
			f->cache = q;
		}

		/*
		 * reset the selector if different from last time
		 */

		if (f->field->hash != h || f->field->index != i)
		{
			hixset(f->hix, 0);
			hixeq(f->hix, i, h);
		}
		q->hash = h;
		q->index = i;
		q->sequence = state.record;
		f->field = q;
	}
	if (q->cached)
	{
		q->cached = 0;
		cdbdrop(f->cdb, q->record);
	}
	while (q->record = (Cdbrecord_t*)hixget(f->hix, p))
	{
		if (f->scanlimit && f->hix->partition > f->scanlimit)
			return 0;
		if (!k || q->record->data[i].string.length == strlen(k) && !memcmp(q->record->data[i].string.base, k, q->record->data[i].string.length))
		{
			if (f->update && (x = search(&f->update, f->hix->offset, 0)))
			{
				register char***	up;
				register Field_t*	fp;
				register Field_t*	ep;

				up = x->value;
				for (ep = (fp = q->record->data) + f->maxfield; fp < ep; fp++, up++)
					if (*up)
					{
						fp->f_converted = CDB_STRING|CDB_CACHED|CDB_TERMINATED;
						fp->f_string = **up;
						fp->f_size = strlen(fp->f_string);
					}
			}
			if (!k && f->overlay)
			{
				if (f->hix->partition <= 1)
					kp = 0;
				else if ((kp = (Mark_t*)dtmatch(f->overlay, (q->record->data + f->record->key)->f_string)) && kp->value > 0 && kp->value < f->hix->partition)
				{
					message((-9, "dat: %s partition=%d overlayed by partition=%d", (q->record->data + f->record->key)->f_string, f->hix->partition, f->overlay));
					continue;
				}
				if (f->hix->partition < f->hix->partitions)
				{
					if (!kp)
					{
						k = (q->record->data + f->record->key)->f_string;
						n = strlen(k);
						if (!(kp = newof(0, Mark_t, 1, n)))
							error(ERROR_SYSTEM|3, "out of space [overlay]");
						memcpy(kp->name, k, n);
						dtinsert(f->overlay, kp);
					}
					kp->value = f->hix->partition;
				}
			}
			if (p >= 0)
				partition = f->hix->restricted;
			if (c)
			{
				if (!(q->record = cdbcache(f->cdb, q->record)))
					return 0;
				q->cached = 1;
				q->partition = f->hix->restricted;
			}
			return q->record->data;
		}
	}
	if (!state.active)
		f->field->index = -1;
	return 0;
}

/*
 * copy an image of the current record in f to op
 */

void
image(register File_t* f, Sfio_t* op, int tc)
{
	if (f->cdb && f->cache && f->cache->record)
		cdbimage(f->cdb, f->cache->record, op, tc);
}

/*
 * generate indices for f and sym from file
 */

void
generate(Expr_t* prog, register File_t* f, const char* file, Exid_t* sym)
{
	register Field_t*	field;
	register int		n;
	register long*		h;
	register int*		g;
	register Record_t*	r = f->record;
	int			i = 0;
	const char*		secondary;
	const char*		name;
	Extype_t		val;

	if (f->hix)
		return;
	if (f->access == ASSOCIATIVE)
		f->access = 0;
	if (!(f->index = newof(0, int, r->fields + 2, 0)))
		error(3, "out of space [index list]");
	if (!file && r->symbol->local.pointer && ((Local_t*)r->symbol->local.pointer)->file)
		file = ((Local_t*)r->symbol->local.pointer)->file->name;
	if (state.schema != r->symbol && state.schema->local.pointer && ((Local_t*)state.schema->local.pointer)->file)
		secondary = ((Local_t*)state.schema->local.pointer)->file->name;
	else
		secondary = 0;
	if (!(name = file) && !(name = secondary))
		name = r->symbol->name;
	g = f->index;
	if (!(state.hix.flags & HIX_READONLY) && (!f->access || f->access == INDEX))
		for (n = 0; n < r->fields; n++)
			if (r->member[n].index)
				*g++ = n;
	*g = hixend;
	if (*(g = f->index) == hixend)
	{
		free(f->index);
		g = f->index = 0;
		f->access = NONE;
	}
	else
	{
		if (!(f->hash = newof(0, long, n + 2, 0)))
			error(3, "out of space [hash list]");
		f->access = INDEX;
	}
	for (;;)
	{
		message((-7, "fil: name=%s access=%c primary=%s secondary=%s info=%s%s%s", f->name, -f->access, file, secondary, r->symbol->name, f->hix ? " REPEAT" : "", g ? " GENERATE" : ""));
		f->hixdisc = state.hix;
		if (f->hix)
			f->hixdisc.flags &= ~HIX_REGENERATE;
		if (!(f->hix = hixopen(file, secondary, r->symbol->name, g, &f->hixdisc)))
			break;
		if ((state.test & 2) && !f->name)
			f->name = f->hix->name;
		if (f->hix->partitions > 1 && !f->hix->restricted)
		{
			f->overdisc.key = offsetof(Mark_t, name);
			f->overdisc.freef = cdtobjfree;
			if (!(f->overlay = dtopen(&f->overdisc, Dtset)))
				error(ERROR_SYSTEM|3, "%s: (%s*)%s: cannot overlay table", name, r->symbol->name, sym->name);
		}
		if (f->hix->delimiter)
			f->delimiter = f->hix->delimiter;
		if ((n = (sym == state.schema && !state.generate)) || !g || *g == hixend)
		{
			state.generate = 0;
			if (!n)
				f->limit = 1 - CACHE;
			if (!f->name)
				f->name = strdup(name);
			message((-6, "raw: file=%s sym=%s", f->hix->name, sym->name));
			return;
		}
		if (!i++)
		{
			if (sym != state.schema && (file && streq(name, ((Local_t*)state.schema->local.pointer)->file->name) || !file && sym == ((Local_t*)state.schema->local.pointer)->file->record->symbol))
				error(ERROR_PANIC, "%s: %s index generation clashes with %s", name, sym->name, state.schema->name);
			message((-6, "gen: file=%s sym=%s", name, sym->name));
			while (field = record(f, NiL, 0, 0, 0, -1))
			{
				h = f->hash;
				if (g)
				{
					while ((n = *g++) != hixend)
					{
						if (r->member[n].index > 1 || r->member[n].format.type == STRING) val.integer = strsum((field + n)->f_string, 0L);
						else val = value(prog, NiL, r->member[n].symbol, NiL, field, 0, &state.expr);
						message((-8, "put: #3 offset=%05lld field=%d hash=0x%08llx value=%s index=%d type=%d", (Sflong_t)f->hix->offset, n, val.integer, (field + n)->f_string, r->member[n].index, r->member[n].format.type));
						*h++ = val.integer;
					}
					g = f->index;
				}
				if (hixput(f->hix, f->hash)) break;
			}
			if (!hixclose(f->hix)) continue;
		}
		break;
	}
	error(ERROR_SYSTEM|3, "%s: (%s*)%s: %s", name, r->symbol->name, sym->name, hixerror(f->hix));
}

/*
 * load the indices for f referenced as sym
 *
 * NOTE: (1) the reference key is restricted to the first field
 *	 (2) reference keys assumed to be in canonical form
 */

void
load(Expr_t* prog, register File_t* f, Exid_t* sym)
{
	register char*		s;
	register Field_t*	p;
	register Field_t*	q;
	register Field_t*	e;
	register int		d;
	register int		i;
	int			k;
	int			m;
	int			n;
	char*			t;
	Field_t*		x;
	struct stat		st;

	if (f->field)
	{
		if (f->hix)
			hixseek(f->hix, 0);
		return;
	}
	if (!f->record && (!sym || !(f->record = ((Local_t*)sym->local.pointer)->record)))
	{
		error(1, "%s: no schema", f->name);
		return;
	}
	if (!f->record->subfields)
		subfields(f->record);
	if ((n = f->maxfield) < 0)
		n = f->record->subfields - 1;
	if (!(f->field = newof(0, Cache_t, 1, n * sizeof(Field_t))))
		error(3, "out of space [field cache]");
	f->field->last = f->field->first + n;
	f->field->index = -1;
	f->cache = f->field;
	f->cache->next = f->cache;
	switch (f->access)
	{
	case DIRECT:
	case NONE:
		if (f->delimiter == DELDEF && (!f->record->symbol || !f->record->symbol->local.pointer || !((Local_t*)f->record->symbol->local.pointer)->file || (f->delimiter = ((Local_t*)f->record->symbol->local.pointer)->file->delimiter) == DELDEF))
			f->delimiter = DELIMITER;
		if (f->terminator == DELDEF && (!f->record->symbol || !f->record->symbol->local.pointer || !((Local_t*)f->record->symbol->local.pointer)->file || (f->terminator = ((Local_t*)f->record->symbol->local.pointer)->file->terminator) == DELDEF))
			f->terminator = TERMINATOR;
		if (f->access == DIRECT)
			f->name = 0;
		return;
	}
	if (f->delimiter && (!f->record->symbol || !f->record->symbol->local.pointer || !((Local_t*)f->record->symbol->local.pointer)->file || (f->delimiter = ((Local_t*)f->record->symbol->local.pointer)->file->delimiter) == DELDEF) && (f->delimiter = ((Local_t*)state.schema->local.pointer)->file->delimiter) == DELDEF)
		f->delimiter = DELIMITER;
	if (f->terminator == DELDEF && (!f->record->symbol || !f->record->symbol->local.pointer || !((Local_t*)f->record->symbol->local.pointer)->file || (f->terminator = ((Local_t*)f->record->symbol->local.pointer)->file->terminator) == DELDEF) && (f->terminator = ((Local_t*)state.schema->local.pointer)->file->terminator) == DELDEF)
		f->terminator = TERMINATOR;
	if (!f->scanlimit && (!f->record->symbol || !f->record->symbol->local.pointer || !((Local_t*)f->record->symbol->local.pointer)->file || !(f->scanlimit = ((Local_t*)f->record->symbol->local.pointer)->file->scanlimit)) && !(f->scanlimit = ((Local_t*)state.schema->local.pointer)->file->scanlimit))
		f->scanlimit = SCANLIMIT;
	if (!sym) return;
	if (m = f->here)
	{
		s = f->name;
		f->name = "{}";
	}
	else if (f->name)
	{
		if (streq(f->name, "-") || streq(f->name, "/dev/fd/0") || streq(f->name, "/dev/stdin"))
		{
			f->name = "/dev/stdin";
			if (state.hix.flags & HIX_READONLY)
				goto gen;
			m = PIPECHUNK;
			if (!(s = oldof(0, char, m + 2, 0)))
				goto gen;
			sfsetbuf(sfstdin, s, m);
			if (!(s = sfreserve(sfstdin, m, 0)))
				goto gen;
			m = sfvalue(sfstdin);
			if (state.test & 000400)
				f->access = ASSOCIATIVE;
		}
		else if (stat(f->name, &st) || st.st_size > FILECHUNK && f->access != ASSOCIATIVE || (state.test & 010000))
			goto gen;
		else
		{
			m = st.st_size;
			if (!(s = oldof(0, char, m + 2, 0)))
				error(3, "out of space [data]");
			if ((n = open(f->name, O_RDONLY|O_BINARY)) < 0)
				error(ERROR_SYSTEM|3, "%s: cannot read", f->name);
			if ((k = read(n, s, m)) < 0)
				error(ERROR_SYSTEM|3, "%s: read error", f->name);
			if (k != m)
				error(3, "%s: read %d -- expected %d", f->name, k, m);
			close(n);
		}
		state.term[s[m] = f->terminator]++;
		s[m + 1] = 0;
		state.term[f->delimiter]++;
		for (t = s; !state.term[*(unsigned char*)t]; t++);
		state.term[f->delimiter]--;
		state.term[f->terminator]--;
		if (*t != f->delimiter && *t != f->terminator)
			goto gen;
	}
	else
	{
	gen:
		message((-3, "gen: record=%s file=%s delimiter='%c' access=%c offset=%d records=%d scanlimit=%d", sym->name, filename(f), f->delimiter, -f->access, f->offset, f->records, f->scanlimit));
		generate(prog, f, f->name, sym);
		return;
	}
	n = f->record->subfields;
	m /= 40;
	if (m < FIELDCHUNK) m = FIELDCHUNK;
	if (!(f->field = newof(f->field, Cache_t, 1, m * n * sizeof(Field_t))))
		error(3, "out of space [field cache]");
	f->cache = f->field;
	f->cache->next = f->cache;
	state.term[d = f->delimiter]++;
	state.term[f->terminator]++;
	i = 0;
	k = (f->access == ASSOCIATIVE) ? 0 : -1;
	x = INDIRECT(f, 0);
	p = x - f->key;
	q = p + m;
	for (;;)
	{
		if (p >= q)
		{
			m = (5 * m) / 4;
			if (!(f->field = newof(f->field, Cache_t, 1, m * n * sizeof(Field_t))))
				error(3, "out of space [data]");
			f->cache = f->field;
			f->cache->next = f->cache;
			x = INDIRECT(f, i);
			p = x - f->key;
			q = p + m;
		}
		e = p + n;
		for (;;)
		{
			if (p < e) (p++)->f_string = s;
			while (!state.term[*((unsigned char*)s)]) s++;
			if (*s != d)
			{
				while (p < e) (p++)->f_string = s;
				if (!*s) goto eof;
				*s++ = 0;
				break;
			}
			*s++ = 0;
		}
		if (k >= 0)
		{
			if (!i)
			{
				f->offset = k = strtol(x->f_string, NiL, 0);
				x += n;
			}
			else if (k == strtol(x->f_string, NiL, 0) + 1)
			{
				k++;
				x += n;
			}
			else k = -1;
		}
		i++;
	}
 eof:
	state.term[d]--;
	state.term[f->terminator]--;
	f->records = i;
	f->field->last = p - 1;
	if (k >= 0)
	{
		f->access = RECORD;
		sym->type = INTEGER;
	}
	else
	{
		f->offset = 0;
		if (f->access == ASSOCIATIVE)
		{
			register Assoc_t*	ap;

			f->assocdisc.key = offsetof(Assoc_t, name);
			f->assocdisc.size = -1;
			if (!(f->assoc = dtopen(&f->assocdisc, Dtset)))
				error(3, "out of space [assoc]");
			p = INDIRECT(f, 0);
			q = f->field->last;
			while (p < q)
			{
				if (!(ap = (Assoc_t*)dtmatch(f->assoc, p->f_string)))
				{
					if (!(ap = newof(0, Assoc_t, 1, 0)))
						error(ERROR_SYSTEM|3, "out of space [assoc]");
					ap->name = p->f_string;
					dtinsert(f->assoc, ap);
				}
				ap->field = p;
				p += n;
			}
		}
		else if (!f->access)
			f->access = NONE;
	}
	message((-3, "ini: record=%s file=%s delimiter='%c' access=%c offset=%d records=%d scanlimit=%d", sym->name, filename(f), f->delimiter, -f->access, f->offset, f->records, f->scanlimit));
}

/*
 * attach reference symbol sym of record type r to file
 */

File_t*
attach(register Exid_t* sym, register Record_t* r, int n)
{
	register Local_t*	p;
	register File_t*	f;

	if (!(p = (Local_t*)sym->local.pointer))
	{
		if (!(p = newof(0, Local_t, 1, 0)))
			error(3, "out of space [local]");
		sym->local.pointer = (char*)p;
		message((-6, "lcl: sym=%s local=%p [%s:%d]", sym->name, p, __FILE__, __LINE__));
	}
	if (!(f = n ? p->index : p->file))
	{
		if (!(f = newof(0, File_t, 1, 0)))
			error(3, "out of space [file]");
		f->delimiter = f->terminator = DELDEF;
		f->maxfield = -1;
		if (n)
			p->index = f;
		else
			p->file = f;
	}
	if (r)
	{
		if (f->record && f->record != r)
			error(1, "%s: file schema %s clashes with %s", sym->name, f->record->symbol->name, r->symbol->name);
		if (p->record && p->record != r && p->record->symbol != sym)
			error(1, "%s: schema type %s clashes with %s", sym->name, p->record->symbol->name, r->symbol->name);
		f->record = p->record = r;
		if (sym != state.schema && ((Local_t*)state.schema->local.pointer)->file && ((Local_t*)state.schema->local.pointer)->file->name && (f->name && streq(((Local_t*)state.schema->local.pointer)->file->name, f->name) || !f->name && ((Local_t*)state.schema->local.pointer)->file->record == r))
			state.generate = 1;
	}
	message((-7, "att: sym=%s fil=%s rec=%s%s", sym->name, filename(f), f->record ? f->record->symbol->name : (char*)0, f->hix ? " OPEN" : ""));
	return f;
}

/*
 * update field index with value
 */

int
update(File_t* f, Field_t* field, int index, char* value)
{
	register Update_t*	p;
	register char***	u;
	char*			o;

	if (!(o = (field + index)->f_string) || !streq(o, value))
	{
		if (!(p = search(&f->update, f->hix ? f->hix->offset : 0, f->record->subfields)))
			return -1;
		u = p->value + index;
		if (!*u && !(*u = newof(0, char*, 2, 0)) || o && !(*(*u + 1) = strdup(o)))
			return -1;
		if (!(**u = newof(**u, char, strlen(value), 1)))
			return -1;
		(field + index)->f_string = strcpy(**u, value);
	}
	return 0;
}

/*
 * generate updated records to layer on main db
 */

int
layer(register Update_t* p, register File_t* f, Sfio_t* sp)
{
	register int		n;
	register int		m;
	register int		d;
	register Field_t*	field;

	do
	{
		if (p->left && (n = layer(p->left, f, sp)))
			return n;
		if (hixseek(f->hix, p->offset))
		{
			error(2, "%s: update hixseek(%lu) error", filename(f), p->offset);
			return -1;
		}
		if (!(field = record(f, NiL, 0, 0, 0, -1)))
		{
			error(2, "%s: update record(%lu) error", filename(f), p->offset);
			return -1;
		}
		m = f->record->subfields - 1;
		for (n = 0; n <= m; n++)
			if (p->value[n] && strcmp(p->value[n][0], p->value[n][1]))
			{
				d = f->delimiter;
				for (n = 0; n <= m; n++)
				{
					if (n == m ) d = f->terminator;
					if (sfputr(sp, p->value[n] ? p->value[n][0] : (field + n)->f_string, d) <= 0)
					{
						error(2, "%s: update sfputr(%lu) error", filename(f), p->offset);
						return -1;
					}
				}
				break;
			}
	} while (p = p->right);
	return 0;
}

/*
 * close f and commit any updates to top view
 */

void
commit(register File_t* f)
{

	if (f->hix)
	{
		if (f->update && !state.again)
		{
			message((-2, ":::: commit updates to %s", filename(f)));
			if (hixseek(f->hix, 0))
				error(2, "%s: cannot commit updates", filename(f));
			else
			{
				Sfio_t*	sp;

				if (!state.replace)
					sp = sfstdout;
				else
				{
					error(1, "%s: update replacement not implemented yet", filename(f));
					sp = sfstdout;
				}
				if (state.update)
				{
					Update_t*	u = f->update;

					f->update = 0;
					layer(u, f, sp);
					f->update = u;
				}
				else
				{
					register Field_t*	field;
					register int		n;
					register int		m;
					register int		d;
					register char*		s;

					m = f->record->subfields - 1;
					while (field = record(f, NiL, 0, 0, 0, -1))
					{
						d = f->delimiter;
						for (n = 0; n <= m; n++)
						{
							if (n == m ) d = f->terminator;
							if (!(s = (field + n)->f_string))
								s = "";
							if (sfputr(sp, s, d) <= 0)
							{
								error(2, "%s: update sfputr(%lu) error", filename(f), f->hix->offset);
								break;
							}
						}
					}
				}
			}
		}
		if (f->index && *f->index != hixend)
			message((-2, ":::: generate hash indices for %s", filename(f)));
		if (hixclose(f->hix) && f->index && *f->index != hixend)
			error(ERROR_SYSTEM|2, "%s: %s", filename(f), hixerror(f->hix));
		f->hix = 0;
	}
}
