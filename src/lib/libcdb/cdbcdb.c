/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1997-2000 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
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
 * cdb format method
 *
 * header:
 *
 *	S   0-terminated CC_ASCII string
 *	T   <type><op>[<data>], <type> and <op> != 0
 *	    <HDR_BUFFER><op><sfputu()-size><data>
 *	    <HDR_NUMBER><op><sfputu()-value>
 *	    <HDR_STRING><op><0-terminated-CC_ASCII-string>
 *	R   record
 *
 *   size   description
 *	4   magic
 *	1   major version
 *	1   minor version
 *      S   comment
 *      S   schema
 *	T   optional header fields
 *	1   0 end of header
 *      R   records
 *
 * major number 1 is an obsolete format used for testing
 * CDB_MAJOR should only change if the record format changes
 * all other changes should be hidden in the optional header fields
 */

#include "cdblib.h"

#define CDB_MAGIC	"\003\004\002\000"	/* data magic number	*/

#define CDB_MAJOR	2		/* data major version		*/
#define CDB_MINOR	0		/* data minor version		*/

#define HDR_BUFFER	1		/* <op><sfputu()-size><data>	*/
#define HDR_NUMBER	2		/* <op><sfputu()-value>		*/
#define HDR_STRING	3		/* <op><0-terminated-string>	*/

typedef struct				/* read fill discipline		*/
{
	Sfdisc_t	disc;		/* sfio discipline		*/
	Cdb_t*		cdb;		/* cdb handle			*/
} Fill_t;

/*
 * return -1 if cdb->io is not a cdb file
 * otherwise return magic size
 * io position is not advanced
 */

static int
cdbrecognize(register Cdb_t* cdb)
{
	register char*		s;
	register int		n;

	n = sizeof(CDB_MAGIC) - 1;
	if (!(s = (char*)sfreserve(cdb->io, n, 1)))
		return -1;
	if (memcmp(CDB_MAGIC, s, n))
		n = -1;
	sfread(cdb->io, s, 0);
	return n;
}

/*
 * called with SF_READ just before the input buffer is refilled
 * the current record string data must be cached before the refill
 * this allows most records to point directly to the buffer string data
 */

static int
cdbfill(Sfio_t* sp, int op, Void_t* value, Sfdisc_t* disc)
{
	register Cdb_t*		cdb = ((Fill_t*)disc)->cdb;
	register Cdbrecord_t*	rp;
	register Cdbdata_t*	dp;
	register Cdbdata_t*	ep;
	register Vmalloc_t*	vp;

	NoP(value);
	if (op == SF_READ && (rp = cdb->record))
	{
		/*
		 * only clear on the first refill for this record
		 * needed for records that span more than 2 input buffers
		 */

		vp = rp->vm;
		if (cdb->clear)
		{
			cdb->clear = 0;
			vmclear(vp);
		}

		/*
		 * cache the referenced string data in vp
		 */

		do
		{
			for (ep = (dp = rp->data) + cdb->strings; dp < ep; dp++)
				if ((dp->flags & (CDB_STRING|CDB_CACHED)) == CDB_STRING && dp->string.base >= (char*)sp->data && dp->string.base < (char*)sp->endb)
				{
					if (!(dp->string.base = vmstrdup(vp, dp->string.base)))
						return cdbnospace(cdb);
					dp->flags |= CDB_CACHED|CDB_TERMINATED;
				}
		} while (rp = rp->next);
	}
	return 0;
}

/*
 * read and return the next record from the cdb file
 */

static Cdbrecord_t*
cdbrecread(Cdb_t* cdb, Cdbkey_t* key)
{
	register Cdbrecord_t*	rp;
	register Cdbformat_t*	fp;
	register Cdbdata_t*	dp;
	register Cdbdata_t*	ep;
	register Cdbdata_t*	xp;
	register Cdbschema_t*	sp;
	register int		n;
	register unsigned long	u;
	unsigned long		end;
	Cdbrecord_t*		pp;
	Vmalloc_t*		vp;

	rp = cdb->record;
	rp->next = 0;
	vp = rp->vm;
	pp = 0;
	cdb->clear = 1;
	do
	{
		rp->offset = sftell(cdb->io);
		if (cdb->partitions > 1)
		{
			n = sfgetu(cdb->io);
			if (sfeof(cdb->io))
				return 0;
			if ((n & 1) && pp)
			{
				/*XXX*/sfungetc(cdb->io, n);/*XXX*/
				break;
			}
			sp = cdb->table[n >> 1];
			if (pp)
			{
				if (cdb->clear)
				{
					cdb->clear = 0;
					vmclear(vp);
				}
				if (!(rp = vmnewof(vp, 0, Cdbrecord_t, 1, sp->fields * sizeof(Cdbdata_t))))
					goto nospace;
				rp->vm = vp;
				rp->data = (Cdbdata_t*)(rp + 1);
				pp->next = rp;
			}
			pp = rp;
			rp->schema = sp;
		}
		else
			rp->schema = sp = cdb->table[0];
		fp = sp->format;
		dp = rp->data;
		if (cdb->sized)
		{
			u = sfgetu(cdb->io);
			if (sfeof(cdb->io))
				goto eof;
			end = u + sftell(cdb->io);
			xp = dp + sp->referenced;
			dp->flags = CDB_INTEGER;
			dp->number.integer = end;
			if (!(n = (sp->referenced < sp->permanent) ? sp->referenced : sp->permanent))
				n = 1;
		}
		else
		{
			end = 0;
			n = sp->permanent;
			xp = dp + sp->fields;
		}
		for (;;)
		{
			for (ep = dp + n; dp < ep; dp++, fp++)
				switch (fp->ptype)
				{
				case CDB_FLOATING:
					dp->number.floating = sfgetd(cdb->io);
					if (sfeof(cdb->io))
						goto eof;
					dp->flags = CDB_FLOATING;
					break;
				case CDB_INTEGER:
					dp->number.integer = (cdb->major == 1) ? sfgetu(cdb->io) : sfgetl(cdb->io);
					if (sfeof(cdb->io))
						goto eof;
					dp->flags = CDB_INTEGER;
					break;
				case CDB_LONG|CDB_INTEGER:
					dp->number.linteger = sfgetl(cdb->io);
					if (sfeof(cdb->io))
						goto eof;
					dp->flags = CDB_LONG|CDB_INTEGER;
					break;
				case CDB_UNSIGNED|CDB_INTEGER:
					dp->number.uinteger = sfgetu(cdb->io);
					if (sfeof(cdb->io))
						goto eof;
					dp->flags = CDB_UNSIGNED|CDB_INTEGER;
					break;
				case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
					dp->number.winteger = sfgetu(cdb->io);
					if (sfeof(cdb->io))
						goto eof;
					dp->flags = CDB_UNSIGNED|CDB_LONG|CDB_INTEGER;
					break;
				case CDB_STRING:
					u = sfgetu(cdb->io);
					if (sfeof(cdb->io))
						goto eof;
					if (fp->flags & CDB_STRING)
					{
						if (dp->string.length = u)
						{
							dp->string.length--;
							if (!(dp->string.base = (char*)sfreserve(cdb->io, u, 0)))
								goto truncated;
							dp->flags = CDB_STRING|CDB_TERMINATED;
						}
						else
						{
							dp->string.base = "";
							dp->flags = CDB_STRING|CDB_CACHED|CDB_TERMINATED;
						}
					}
					else
						sfseek(cdb->io, (Sfoff_t)u, SEEK_CUR);
					break;
				default:
					if (fp->virtuals && (fp+1)->virtuals)
					{
						dp++;
						fp++;
					}
					break;
				}
			if (ep >= xp)
			{
				if (cdb->major == 1 && (cdb->minor == 0 || n != sp->permanent) && sfgetu(cdb->io))
					goto truncated;
				if (end && sfseek(cdb->io, end, SEEK_SET) < 0)
					goto truncated;
				goto bypass;
			}
			if (!(n = sfgetu(cdb->io)))
				break;
			u = sfgetu(cdb->io);
			if (sfeof(cdb->io))
				goto eof;
			for (ep = dp + u; dp < ep; dp++, fp++)
				if (fp->flags & (CDB_FLOATING|CDB_INTEGER|CDB_STRING))
				{
					dp->string.base = "";
					dp->string.length = 0;
					dp->number.floating = 0;
					dp->flags = CDB_CACHED|CDB_TERMINATED;
				}
		}
		for (ep = rp->data + sp->referenced; dp < ep; dp++, fp++)
			if (fp->flags & (CDB_FLOATING|CDB_INTEGER|CDB_STRING))
			{
				dp->string.base = "";
				dp->string.length = 0;
				dp->number.floating = 0;
				dp->flags = CDB_CACHED|CDB_TERMINATED;
			}
 bypass:
		sp->count++;
		cdb->count++;
	} while (cdb->partitions > 1 && !(cdb->flags & CDB_RAW));
 done:
	if (cdb->flags & CDB_DUMP)
		cdbdump(cdb, sfstderr, "read", cdb->record);
	return cdb->record;
 eof:
	if (pp)
	{
		pp->next = 0;
		goto done;
	}
	if (dp == rp->data)
		return 0;
 truncated:
	if (cdb->disc->errorf)
		(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: input truncated", cdb->path);
	return 0;
 nospace:
	cdbnospace(cdb);
	return 0;
}

/*
 * write a record to the cdb file
 */

static int
cdbrecwrite(register Cdb_t* cdb, Cdbkey_t* key, Cdbrecord_t* rp)
{
	register Cdbformat_t*	fp;
	register Cdbdata_t*	dp;
	register Cdbdata_t*	ep;
	register Cdbschema_t*	sp;
	register int		n;
	register int		skip;
	register int		keep;
	Sfio_t*			io;
	char*			s;
	long			x;
	int			first;

	rp->offset = sftell(cdb->io);
	if (cdb->flags & CDB_DUMP)
		cdbdump(cdb, sfstderr, "write", cdb->record);
	first = 1;
	do
	{
		sp = rp->schema;
		if (cdb->partitions > 1)
		{
			sfputu(cdb->io, (sp->index << 1) | first);
			first = 0;
		}
		io = cdb->sized ? cdb->cvt : cdb->io;
		ep = (dp = rp->data) + sp->permanent;
		for (fp = sp->format; dp < ep; dp++, fp++)
			switch (fp->ptype)
			{
			case CDB_FLOATING:
				sfputd(io, dp->number.floating);
				break;
			case CDB_INTEGER:
				sfputl(io, dp->number.integer);
				break;
			case CDB_UNSIGNED|CDB_INTEGER:
				sfputu(io, dp->number.uinteger);
				break;
			case CDB_LONG|CDB_INTEGER:
				sfputl(io, dp->number.linteger);
				break;
			case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
				sfputu(io, dp->number.winteger);
				break;
			case CDB_STRING:
				if (n = dp->string.length)
				{
					sfputu(io, n + 1);
					if (CC_NATIVE != CC_ASCII)
					{
						n++;
						if (!(s = (char*)sfreserve(io, n, 0)))
							goto bad;
						ccmapcpy(s, dp->string.base, n, CC_NATIVE, CC_ASCII);
					}
					else
						sfwrite(io, dp->string.base, n);
					sfputc(io, 0);
				}
				else
					sfputu(io, 0);
				break;
			default:
				if (fp->virtuals && (fp+1)->virtuals)
				{
					dp++;
					fp++;
				}
				break;
			}
		if (dp < (ep = rp->data + sp->fields))
		{
			for (skip = keep = 0; dp < ep; dp++, fp++)
			{
				switch (fp->ptype)
				{
				case CDB_FLOATING:
					if (dp->flags & CDB_FLOATING)
					{
						keep++;
						sfputd(cdb->tmp, dp->number.floating);
						continue;
					}
					break;
				case CDB_INTEGER:
					if (dp->flags & CDB_INTEGER)
					{
						keep++;
						sfputl(cdb->tmp, dp->number.integer);
						continue;
					}
					break;
				case CDB_UNSIGNED|CDB_INTEGER:
					if (dp->flags & CDB_INTEGER)
					{
						keep++;
						sfputu(cdb->tmp, dp->number.uinteger);
						continue;
					}
					break;
				case CDB_LONG|CDB_INTEGER:
					if (dp->flags & CDB_INTEGER)
					{
						keep++;
						sfputl(cdb->tmp, dp->number.linteger);
						continue;
					}
					break;
				case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
					if (dp->flags & CDB_INTEGER)
					{
						keep++;
						sfputu(cdb->tmp, dp->number.winteger);
						continue;
					}
					break;
				case CDB_STRING:
					if ((dp->flags & CDB_STRING) && (n = dp->string.length))
					{
						keep++;
						if (n)
						{
							sfputu(cdb->tmp, n + 1);
							if (CC_NATIVE != CC_ASCII)
								x = sfstrtell(cdb->tmp);
							sfwrite(cdb->tmp, dp->string.base, n);
							if (CC_NATIVE != CC_ASCII)
								CCMAPS(sfstrbase(cdb->tmp) + x, n, CC_NATIVE, CC_ASCII);
							sfputc(cdb->tmp, 0);
						}
						else
							sfputu(cdb->tmp, 0);
						continue;
					}
					break;
				default:
					dp += fp->virtuals;
					fp += fp->virtuals;
					continue;
				}
				if (keep)
				{
					sfputu(io, keep);
					keep = 0;
					sfputu(io, skip);
					skip = 1;
					sfwrite(io, sfstrbase(cdb->tmp), sfstrtell(cdb->tmp));
					sfstrset(cdb->tmp, 0);
				}
				else
					skip++;
			}
			if (keep)
			{
				sfputu(io, keep);
				sfputu(io, skip);
				sfwrite(io, sfstrbase(cdb->tmp), sfstrtell(cdb->tmp));
				sfstrset(cdb->tmp, 0);
			}
			else
				sfputu(io, 0);
		}
		if (cdb->sized)
		{
			x = sfstrtell(io);
			sfputu(cdb->io, x);
			sfwrite(cdb->io, sfstrbase(io), x);
			sfstrset(io, 0);
		}
		if (sferror(cdb->io))
			goto bad;
		sp->count++;
		cdb->count++;
	} while (rp = rp->next);
	return 1;
 bad:
	if (cdb->disc->errorf)
		(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: write error", cdb->path);
	return -1;
}

/*
 * cdb record seek
 */

static Sfoff_t
cdbrecseek(Cdb_t* cdb, Sfoff_t off)
{
	if (cdb->meth.flags & CDB_STACK)
	{
		if (cdb->disc->errorf)
			(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %s seek not implemented", cdb->path, cdb->meth.name);
		return -1;
	}
	return (off < 0) ? cdb->record->offset : sfseek(cdb->io, off, SEEK_SET);
}

/*
 * read CC_ASCII string from cdb->io
 * save!=0 saves string in cdb region
 */

static char*
getstr(Cdb_t* cdb, int save)
{
	register char*	s;

	if (!(s = sfgetr(cdb->io, 0, 0)))
		return 0;
	if (save || CC_NATIVE != CC_ASCII)
	{
		if (!(s = vmstrdup(cdb->vm, s)))
			return 0;
		if (CC_NATIVE != CC_ASCII)
			CCMAPS(s, sfvalue(cdb->io), CC_ASCII, CC_NATIVE);
	}
	return s;
}

/*
 * write CC_ASCII string to cdb->io
 */

static int
putstr(Cdb_t* cdb, register const char* s)
{
	register char*	t;
	register int	n;

	if (CC_NATIVE != CC_ASCII)
	{
		n = strlen(s) + 1;
		if (!(t = (char*)sfreserve(cdb->io, n, 0)))
			return -1;
		ccmapcpy(t, s, n, CC_NATIVE, CC_ASCII);
	}
	else
		sfputr(cdb->io, s, 0);
	return 0;
}

/*
 * read the cdb header
 */

static int
cdbhdrread(register Cdb_t* cdb)
{
	register char*		s;
	register int		c;
	register int		n;
	register unsigned long	u;

	/*
	 * verify the magic number
	 * file offset not advanced on error
	 */

	if ((n = cdbrecognize(cdb)) < 0)
	{
		if (cdb->disc->errorf)
			(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: bad magic", cdb->path);
		return -1;
	}
	sfseek(cdb->io, (Sfoff_t)n, SEEK_CUR);

	/*
	 * the rest depends on major.minor
	 */

	cdb->major = sfgetc(cdb->io);
	cdb->minor = sfgetc(cdb->io);
	switch (cdb->major)
	{
	case 2:
		/*
		 * grab the comment and schema strings
		 */

		if (!(cdb->comment = getstr(cdb, 1)))
			goto badheader;
		if (!(s = getstr(cdb, 0)))
			goto badheader;
		if (cdbparse(cdb, s) < 0)
			return -1;

		/*
		 * loop on the optional headers
		 */

		for (;;)
		{
			switch ((int)sfgetu(cdb->io))
			{
			case 0:
				break;
			case HDR_BUFFER:
				c = sfgetu(cdb->io);
				u = sfgetu(cdb->io);
				if (!(s = (char*)sfreserve(cdb->io, u, 0)))
					goto badheader;
				/* no buffer fields yet */
				continue;
			case HDR_NUMBER:
			case 'N': /* obosolete -- ASCII-centric */
				c = sfgetu(cdb->io);
				u = sfgetu(cdb->io);
				if (sfeof(cdb->io))
					goto badheader;
				/* no number fields yet */
				continue;
			case HDR_STRING:
				c = sfgetu(cdb->io);
				if (!(s = getstr(cdb, 0)))
					goto badheader;
				/* no string fields yet */
				continue;
			default:
				goto badheader;
			}
			break;
		}
		break;
	default:
		if (cdb->disc->errorf)
			(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: %s version %d.%d data not supported by implementation version %d.%d", cdb->path, cdb->meth.name, cdb->major, cdb->minor, CDB_MAJOR, CDB_MINOR);
		return -1;
	}
	return 0;
 badheader:
	if (cdb->disc->errorf)
		(*cdb->disc->errorf)(cdb, cdb->disc, 2, "%s: bad header", cdb->path);
	return -1;
}

/*
 * write the cdb header
 */

static int
cdbhdrwrite(register Cdb_t* cdb)
{
	sfwrite(cdb->io, CDB_MAGIC, sizeof(CDB_MAGIC) - 1);
	sfputc(cdb->io, CDB_MAJOR);
	sfputc(cdb->io, CDB_MINOR);
	if (putstr(cdb, cdb->comment) < 0)
		return -1;
	if (putstr(cdb, cdb->schema) < 0)
		return -1;

	/*
	 * optional headers here
	 *
	 *	sfputu(cdb->io, HDR_STRING);
	 *	sfputu(cdb->io, HDR_method);
	 *	if (putstr(cdb, cdb->meth.name) < 0)
	 *		return -1;
	 */

	sfputu(cdb->io, 0);
	return 0;
}

/*
 * handle method events
 */

static int
cdbevent(register Cdb_t* cdb, int op)
{
	switch (op)
	{
	case CDB_OPEN:
		/*
		 * if any strings are referenced then push the fill discipline
		 */

		if (cdb->strings)
		{
			register Fill_t*	fp;

			if (!(fp = vmnewof(cdb->vm, 0, Fill_t, 1, 0)))
				return cdbnospace(cdb);
			fp->disc.exceptf = cdbfill;
			fp->cdb = cdb;
			sfdisc(cdb->io, &fp->disc);
			sfset(cdb->io, SF_IOCHECK, 1);
		}
		break;
	}
	return 0;
}

Cdbmeth_t	_Cdb =
{
	"cdb",
	"cql database",
	".cdb",
	0,
	cdbevent,
	cdbrecognize,
	cdbhdrread,
	cdbhdrwrite,
	cdbrecread,
	cdbrecwrite,
	cdbrecseek,
	0,
	&_Cdbflat
};

__DEFINE__(Cdbmeth_t*, Cdb, &_Cdb);
