/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1991-2000 AT&T Corp.              *
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
/*
 * David Korn (algorithm)
 * Glenn Fowler (interface, virtual directory, file partitions)
 * AT&T Research
 *
 * hash index file implementation
 */

static const char ID[] = "\n@(#)hix (AT&T Research) 1998-11-01\0\n";

#include <sfio_t.h>
#include <ast.h>
#include <cdt.h>
#include <ls.h>
#include <swap.h>
#include <ctype.h>
#include <error.h>
#include <debug.h>

#define HIX_ALIGN	sizeof(Number_t)

#define HIX_SCAN	(HIX_USER>>1)

#define HIX_ERROR_DATA_FILE	1
#define HIX_ERROR_DATA_READ	2
#define HIX_ERROR_DATA_SEEK	3
#define HIX_ERROR_EVAL		4
#define HIX_ERROR_INDEX_WRITE	5
#define HIX_ERROR_MALLOC	6
#define HIX_ERROR_TMP		7
#define HIX_ERROR_VERSION	8

#define ERROR(p,n)		((n)==HIX_ERROR_INDEX_WRITE?(error(1,"AHA#%d HIX_ERROR_INDEX_WRITE",__LINE__),0):0,(p)->flags|=HIX_ERROR,state.error=(p)->error=(n))

#define IDBASE		64
#define HASHCHUNK	256
#define HASHMARK	0x80000000
#define MAGIC		0x08091800
#define MASKSHIFT	3
#define MAXOFFSET	0x7fffffff
#define NOFFSET		(2<<MASKSHIFT)

#define AND		((Instruction_t*)0)
#define OR		((Instruction_t*)1)

#define ISREFERENCE(i)	((i)&(1<<14))
#define INDEX(i)	(ISREFERENCE(i)?(((i)>>7)&((1<<7)-1)):(i))
#define REFERENCE(i)	((i)&((1<<7)-1))

#define bucket(m,h)	((m)&(h))
#define ioflush(x)	((x)->flushed++,(state.size=(char*)(x)->cur-(x)->buf)>0&&(sfwrite((x)->vio->sp,(x)->buf,state.size)!=state.size)?-1:0)
#define ioget(p)	((p)->data<(p)->last?*(p)->data++:iogetnum(p))
#define ioput(x,l)	((x)->cur<(x)->end?(*(x)->cur++=(l),0):ioputnum(x,l))

#define VIR_primary	(1<<0)		/* primary data+info		*/
#define VIR_secondary	(1<<1)		/* secondary data+info		*/
#define VIR_info	(1<<2)		/* info only			*/
#define VIR_last	(1<<3)		/* primary only			*/

#define VIR_directory	(1<<4)		/* data is dir			*/

#define Number_t	int_4

typedef struct
{
	Number_t	magic;		/* hix file magic		*/
	Number_t	records;	/* number of hashed records	*/
	Number_t	recsize;	/* max record size		*/
	Number_t	stamp;		/* data file time stamp		*/
	Number_t	mask;		/* bin offset mask		*/
	Number_t	binsize;	/* bin size			*/
	Number_t	future[2];	/* just in case			*/
	char		version[32];	/* user defined version magic	*/
} Header_t;

typedef struct
{
	Sfio_t*		sp; 		/* data file stream		*/
	Sfoff_t		offset;		/* virtual file offset		*/
	Sfoff_t		size;		/* virtual file size		*/
	unsigned char	empty;		/* no records			*/
	unsigned char	sequential;	/* strict sequential access	*/
} Vio_t;

typedef struct
{
	Header_t	header;		/* index file header		*/
	Vio_t*		vio;		/* virtual index file stream	*/
	struct Hix_s*	generate;	/* generating this index	*/
	char*		buf;		/* peek buffer base		*/
	Number_t*	cur;		/* current position in buf	*/
	Number_t*	end;		/* buf end + 1			*/
	int		flushed;	/* some data flushed to file	*/
	int		id;		/* user defined index id	*/
	int		swap;		/* data byte order swap op	*/
} Index_t;

typedef struct Virfile_s
{
	struct Virfile_s*next;		/* next file in directory	*/
	Vio_t		vio;		/* physical info		*/
	Index_t*	index;		/* generate this index		*/
	char*		tag;		/* union tag			*/
	int		id;		/* index in directory		*/
	int		delimiter;	/* field delimiter		*/
	char		name[1];	/* virtual file name		*/
} Virfile_t;

typedef struct Virdir_s
{
	struct Virdir_s*next;		/* next directory		*/
	Sfio_t*		sp;		/* directory read stream	*/
	Virfile_t*	file;		/* first file in directory	*/
	int		delimiter;	/* field delimiter		*/
	long		generate;	/* index bitmap to be generated	*/
	char		name[1];	/* virtual dir name		*/
} Virdir_t;

typedef struct Part_s
{
	struct Part_s*	next;		/* next file part		*/
	Vio_t		vio; 		/* virtual data file stream	*/
	Virdir_t*	dir;		/* index virtual directory	*/
	char*		name;		/* data file name		*/
	char*		tag;		/* data union name		*/
	Number_t	stamp;		/* data file stamp		*/
	int		level;		/* index file level		*/
	int		restrict;	/* restrict to this part	*/
	int		suffix;		/* index file suffix		*/
	Sfoff_t		base;		/* file partition offset	*/
	Index_t		index[1];	/* index file state		*/
} Part_t;

typedef struct Instruction_s
{
	struct Instruction_s*next;	/* next on active/free list	*/
	int		id;		/* index offset for instruction	*/
	Part_t*		part;		/* current file partition	*/
	Number_t*	data;		/* current position in base	*/
	Number_t*	last;		/* base end + 1			*/
	Number_t	hash;		/* target hash			*/
	Number_t	offset;		/* next offset match lo-to-hi	*/
	Number_t	position;	/* offset for next data chunk	*/
	Number_t	base[HASHCHUNK/sizeof(Number_t)];/* data buffer	*/
} Instruction_t;

#define _HIX_PRIVATE_ \
	Hixdisc_t*	disc; 		/* user discipline		*/ \
	Part_t*		parts; 		/* data file parts		*/ \
	Part_t*		part; 		/* current data file part	*/ \
	Instruction_t**	pb;		/* program base			*/ \
	Instruction_t**	pc;		/* program counter		*/ \
	Instruction_t**	pe;		/* program end			*/ \
	Instruction_t*	active;		/* active instruction list	*/ \
	Instruction_t*	end;		/* end of active		*/ \
	Instruction_t*	free;		/* free instruction list	*/ \
	int		error;		/* last HIX_ERROR_* error	*/ \
	int		indices;	/* number of index files	*/ \
	int*		gen;		/* generate map			*/ \
	int		map[1];		/* index id to ordinal map	*/

#include <hix.h>
#include <vdb.h>

typedef struct				/* shared index read stream	*/
{
	Dtlink_t	link;		/* table link			*/
	Vio_t*		vio;		/* virtual read stream		*/
	char		name[1];	/* stream name			*/
} Io_t;

static struct
{
	int		announce;	/* id message done		*/
	int		error;		/* last HIX_ERROR_* error	*/
	int		mask;		/* global state for compare()	*/
	int		size;		/* temporary for ioflush()	*/
	Dt_t*		io;		/* shared index read streams	*/
	Dtdisc_t	iodisc;		/* io cdt discipline		*/
	Virdir_t*	vir;		/* virtual directories		*/
} state;

/*
 * return text for most recent error
 */

char*
hixerror(Hix_t* hix)
{
	int		n;

	static char	buf[32];

	switch (n = hix ? hix->error : state.error)
	{
	case 0:
		return "internal error";
	case HIX_ERROR_DATA_FILE:
		return "data file not found";
	case HIX_ERROR_DATA_READ:
		return "cannot read record";
	case HIX_ERROR_DATA_SEEK:
		return "cannot seek to record";
	case HIX_ERROR_EVAL:
		return "expression syntax error";
	case HIX_ERROR_INDEX_WRITE:
		return "cannot generate indices";
	case HIX_ERROR_MALLOC:
		return "out of memory";
	case HIX_ERROR_TMP:
		return "tmp data IO error";
	}
	sfsprintf(buf, sizeof(buf), "unknown error %d", n);
	return buf;
}

/*
 * virtual file lookup match of s with virtual dir entry t
 * op:
 *
 *	0 exact match
 *	1 case insensitive match
 *	2 case insensitive match after last VDB_IGNORE
 */

static int
virmatch(register const char* s, register const char* t, int op)
{
	register int	a;
	register int	b;
	const char*	e;

	if ((e = strrchr(t, '/')) || (e = strrchr(t, '\\')))
		t = e + 1;
	if (op > 1)
	{
		if (!(t = strrchr(t, VDB_IGNORE)))
			return 0;
		t++;
	}
	while (a = *s++)
	{
		if (!(b = *t++))
			return 0;
		if (a != b && (!op || isupper(a) || toupper(a) != b))
			return 0;
	}
	return *t == 0 || *t == '.';
}

/*
 * return virtual dir pointer if dir is virtual dir
 * rp open for read on dir
 */

static Virdir_t*
virdir(Sfio_t* rp, const char* dir)
{
	register char*		s;
	register Virdir_t*	dp;
	register Virfile_t*	fp;
	register Sfio_t*	sp;
	char*			t;
	int			n;
	int			delimiter;
	int			variant;
	int			index;
	Sfoff_t			eof;
	Sfoff_t			off;

	for (dp = state.vir; dp; dp = dp->next)
		if (streq(dir, dp->name))
			return dp;
	if (!(sp = rp) && !(sp = sfopen(NiL, dir, "r")))
		return 0;
	if (sfseek(sp, (Sfoff_t)(-(VDB_LENGTH + 1)), 2) <= 0)
		goto bad;
	if (!(s = sfgetr(sp, '\n', 0)))
		goto bad;
	if (variant = *s != '\n')
		s++;
	else if (!(s = sfgetr(sp, '\n', 0)))
		goto bad;
	if (sfvalue(sp) != (VDB_LENGTH + variant))
		goto bad;
	if (!strneq(s, VDB_DIRECTORY, sizeof(VDB_DIRECTORY) - 1))
		goto bad;
	delimiter = s[VDB_OFFSET - 1];
	off = strtol(s + VDB_OFFSET, NiL, 10) - sizeof(VDB_DIRECTORY);
	eof = strtol(s + VDB_SIZE, NiL, 10);
	if (sfseek(sp, off, 0) != off)
		goto bad;
	if (!(s = sfgetr(sp, '\n', 0)))
		goto bad;
	if (sfvalue(sp) != sizeof(VDB_DIRECTORY))
		goto bad;
	if (!strneq(s, VDB_DIRECTORY, sizeof(VDB_DIRECTORY) - 1))
		goto bad;
	if (!(dp = newof(0, Virdir_t, 1, strlen(dir))))
		goto bad;
	strcpy(dp->name, dir);
	dp->next = state.vir;
	state.vir = dp;
	off = 0;
	index = 0;
	while (off < eof && (s = sfgetr(sp, '\n', 1)))
	{
		off += sfvalue(sp);
		delimiter = variant ? *s++ : delimiter;
		if (!(t = strchr(s, delimiter)))
			break;
		if (strneq(s, VDB_DIRECTORY, t - s))
			continue;
		if (!(fp = newof(0, Virfile_t, 1, t - s)))
			break;
		strncpy(fp->name, s, t - s);
		fp->vio.sp = sp;
		fp->vio.offset = strtol(t + 1, &t, 10);
		fp->vio.size = strtol(t + 1, &t, 10);
		fp->delimiter = delimiter;
		if (*t++) do
		{
			if (s = strchr(t, delimiter))
				*s++ = 0;
			if (strneq(t, VDB_UNION, sizeof(VDB_UNION) - 1))
			{
				t += sizeof(VDB_UNION);
				n = strlen(t);
				if (fp->tag = newof(0, char, n, 2))
				{
					strcpy(fp->tag, t);
					fp->tag[n] = delimiter;
				}
				break;
			}
		} while (t = s);
		fp->id = index++;
		fp->next = dp->file;
		message((-4, "vir: dir=%s name=%s offset=%llu size=%llu tag=%s delimiter=%c", dir, fp->name, (Sflong_t)fp->vio.offset, (Sflong_t)fp->vio.size, fp->tag, fp->delimiter));
		dp->file = fp;
	}
	dp->sp = sp;
	dp->delimiter = delimiter;
	return dp;
 bad:
	if (!rp)
		sfclose(sp);
	else
		sfseek(sp, (Sfoff_t)0, 0);
	return 0;
}

/*
 * virtual hixopen
 * data may be modified on success or failure
 */

static int
viropen(register Hix_t* hix, char* data, const char* primary, const char* secondary, const char* info, int flags)
{
	register Virfile_t*	fp;
	int			i;
	Virdir_t*		dp;
	struct stat		st;
	struct stat		sst;
	char			buf[PATH_MAX];

	if (streq(data, "-") || streq(data, "/dev/fd/0") || streq(data, "/dev/stdin"))
	{
		if (!(flags & VIR_primary))
			return -1;
		data = "/dev/stdin";
		hix->part->vio.sp = sfstdin;
		if (fstat(sffileno(hix->part->vio.sp), &st))
		{
			st.st_mtime = 0;
			st.st_size = 0;
		}
	}
	else
	{
		if (stat(data, &st))
		{
			if (!(data = pathfind(data, hix->disc->lib, NiL, buf, sizeof(buf))))
				return -1;
			if (stat(data, &st))
				return -1;
		}
		if (S_ISDIR(st.st_mode))
		{
			if (!info)
				return -1;
			sfsprintf(buf, sizeof(buf), "%s/%s.db", data, info);
			data = buf;
			flags |= VIR_directory;
			if (stat(data, &st))
				return -1;
		}
		if (!(hix->part->vio.sp = sfopen(NiL, data, "r")))
			return -1;
		if ((hix->flags & HIX_TEST1) && st.st_size >= 8 * 1024 * 1024)
			sfsetbuf(hix->part->vio.sp, (hix->flags & HIX_TEST2) ? (void*)0 : (void*)hix->part->vio.sp, 1024 * 1024);
		pathcanon(data, 0);
	}
	if (!(hix->part->name = newof(0, char, strlen(data), 3)))
	{
		if (hix->part->vio.sp != sfstdin)
			sfclose(hix->part->vio.sp);
		return -1;
	}
	strcpy(hix->part->name, data);
	hix->part->stamp = st.st_mtime;
	if (hix->part->vio.sp == sfstdin || !fstat(0, &sst) && st.st_ino == sst.st_ino && st.st_dev == sst.st_dev || sfseek(hix->part->vio.sp, (Sfoff_t)1, 0) != 1L || sfseek(hix->part->vio.sp, (Sfoff_t)0, 0) != 0L)
	{
		if (!S_ISREG(st.st_mode))
		{
			hix->part->vio.sequential = 1;
			st.st_size = 0;
		}
	}
	else if (!(flags & VIR_directory) && (dp = virdir(hix->part->vio.sp, data)))
	{
		for (i = 0; i <= 2; i++)
			for (fp = dp->file; fp; fp = fp->next)
				if (virmatch(info, fp->name, i))
				{
					if (sfseek(hix->part->vio.sp, fp->vio.offset, 0) != fp->vio.offset)
						return -1;
					hix->delimiter = fp->delimiter;
					hix->part->vio.offset = fp->vio.offset;
					if (!(hix->part->vio.size = fp->vio.size))
						hix->part->vio.empty = 1;
					hix->part->tag = fp->tag;
					if (fp->id < 8)
						hix->part->suffix = 'I' + fp->id + 1;
					else
						hix->part->suffix = *fmtbase(fp->id, IDBASE, 0);
					return 0;
				}
		return -1;
	}
	else if (flags & (VIR_primary|VIR_secondary))
	{
		if (hix->part->vio.sp != sfstdin)
			sfclose(hix->part->vio.sp);
		return -1;
	}
	if (!(hix->part->vio.size = st.st_size))
		hix->part->vio.empty = 1;
	hix->part->suffix = 'I';
	return 0;
}

/*
 * open (shared) virtual index file for read
 */

static Vio_t*
vioopen(Virdir_t* dp, const char* name)
{
	register Io_t*	io;
	Virfile_t*	fp;
	char*		s;
	int		n;
	char		buf[PATH_MAX];

	if (!state.io)
	{
		state.iodisc.key = offsetof(Io_t, name);
		if (!(state.io = dtopen(&state.iodisc, Dtset)))
			return 0;
	}
	sfsprintf(buf, sizeof(buf), "%s/%s", dp->name, name);
	s = pathcanon(buf, 0);
	if (!(io = (Io_t*)dtmatch(state.io, buf)))
	{
		n = s - buf;
		if (!(io = newof(0, Io_t, 1, n)))
			return 0;
		memcpy(io->name, buf, n);
		dtinsert(state.io, io);
	}
	if (!io->vio)
	{
		if (dp)
		{
			for (fp = dp->file; fp; fp = fp->next)
				if (virmatch(name, fp->name, 0))
					break;
			if (!fp)
				return 0;
			io->vio = &fp->vio;
		}
		else if (!(io->vio = newof(0, Vio_t, 1, 0)) || !(io->vio->sp = sfopen(NiL, name, "r")))
			return 0;
		else
			sfsetbuf(io->vio->sp, NiL, 0);
	}
	if (sfseek(io->vio->sp, io->vio->offset, 0) != io->vio->offset)
		return 0;
	message((-5, "vio: %s: offset=%lld size=%lld fd=%d", buf, (Sflong_t)io->vio->offset, (Sflong_t)io->vio->size, sffileno(io->vio->sp)));
	return io->vio;
}

/*
 * get the next index chunk of numbers and swap accordingly
 *
 * NOTE: no sfreserve() here since there may be more than one index reader
 */

static Number_t
iogetnum(register Instruction_t* p)
{
	register Sfoff_t	n;
	register ssize_t	m;
	register int		i;
	register Vio_t*		v;

	if (p->part)
	{
		v = p->part->index[p->id].vio;
		n = p->position + v->offset;
		if (sfseek(v->sp, n, 0) == n && (m = sfread(v->sp, (char*)p->base, sizeof(p->base))) >= sizeof(Number_t))
		{
			p->position += m;
			p->data = p->base;
			p->last = p->base + m / sizeof(Number_t);
			if (i = p->part->index[p->id].swap)
				swapmem(i, p->base, p->base, m);
			return *p->data++;
		}
	}
	p->offset = MAXOFFSET;
	return 0;
}

/*
 * empty the buffer if necessary and put <l> into the next buffer
 */

static int
ioputnum(register Index_t* x, Number_t l)
{
	register long	n;
	int		i;

	if (ioflush(x))
	{
		error(1, "ioputnum#%d", __LINE__);
		return -1;
	}
	for (i = 0;;)
	{
		if (!(x->buf = sfreserve(x->vio->sp, SF_UNBOUND, 1)))
		{
			error(1, "ioputnum#%d io=%p", __LINE__, x->vio->sp);
			return -1;
		}
		if ((n = sfvalue(x->vio->sp)) <= 0)
		{
			error(1, "ioputnum#%d", __LINE__);
			return -1;
		}
		if (!(((unsigned long)x->buf) & (HIX_ALIGN - 1)))
			break;
		if (i++)
		{ 
			error(1, "ioputnum#%d", __LINE__);
			return -1;
		}
		message((-3, "hix: alignment sync: fd=%d offset=%lu buf=%p", sffileno(x->vio->sp), sftell(x->vio->sp), x->buf));
		sfsync(x->vio->sp);
	}
	x->cur = x->end = (Number_t*)x->buf;
	if (n < sizeof(Number_t))
	{
		return sfwrite(x->vio->sp, (char*)&l, sizeof(Number_t)) == sizeof(Number_t) ? 0 : (error(1,"#%d",__LINE__),-1);
	}
	x->end += n / sizeof(Number_t);
	*x->cur++ = l;
	return 0;
}

/*
 * put <z> 0's into the output buffer
 */

static int
iozero(register Index_t* x, register int z)
{
	register long	n;
	register long	nleft;

	for (;;)
	{
		nleft = x->end - x->cur;
		switch (n = z < nleft ? z : nleft)
		{
		case 2:
			*x->cur++ = 0;
			/*FALLTHROUGH*/
		case 1:
			*x->cur++ = 0;
			/*FALLTHROUGH*/
		case 0:
			break;
		default:
			memzero(x->cur, n * sizeof(Number_t));
			x->cur += n;
			break;
		}
		if ((z -= n) > 0)
		{
			if (ioputnum(x, 0))
				return -1;
			if (--z > 0)
				continue;
		}
		return 0;
	}
}

/*
 * qsort comparison
 * compare last mask bits first, and then compare rest of hash
 * finally compare offsets
 */

static int
compare(const void* a, const void* b)
{
	register Number_t*		left = (Number_t*)a;
	register Number_t*		right = (Number_t*)b;
	register unsigned Number_t	l;
	register unsigned Number_t	r;

	if ((l = bucket(state.mask, *left)) > (r = bucket(state.mask, *right)))
		return 1;
	else if (l < r)
		return -1;
	else if ((l = *left) == (r = *right))
		return left[1] - right[1];
	return l - r;
}

/*
 * build the hash index file for x in sp
 * x->base[i],x->base[i+1] contains hash,offset pairs for i>0
 */

static int
build(Hix_t* hix, register Index_t* x, Sfio_t* sp)
{
	register unsigned Number_t	hash;
	register unsigned Number_t	c;
	register Number_t		offset;
	register Number_t*		p;
	register Number_t*		m;
	Sfio_t*				op;
	char*				b = 0;

	/*
	 * finish up from hixput()
	 * the last even x->base[] element is a 0 sentinel
	 */

	op = x->vio->sp;
	if (x->flushed <= 0)
		goto bad;
	if (x->flushed <= 1)
		offset = (char*)x->cur - x->buf;
	else if (ioflush(x) < 0)
		goto bad;
	else
	{
		sfclrlock(x->vio->sp);
		if ((offset = sftell(x->vio->sp)) < 0 || sfseek(x->vio->sp, (Sfoff_t)0, 0))
			goto bad;
	}
	if (!(b = newof(0, char, offset, sizeof(Number_t))))
		goto bad;
	sfset(x->vio->sp, SF_READ, 1);
	sfset(x->vio->sp, SF_WRITE, 0);
	if (x->flushed <= 1)
		memcpy(b, x->buf, offset);
	else if (sfseek(x->vio->sp, (Sfoff_t)0, 0) || sfread(x->vio->sp, b, offset) != offset || sfseek(x->vio->sp, (Sfoff_t)0, 0))
		goto bad;
	p = (Number_t*)b;
	*(p + offset / sizeof(Number_t)) = 0;
	x->header.records = offset / (2 * sizeof(Number_t));

	/*
	 * find next power of 1.125*n
	 */

	hash = x->header.records + (x->header.records>>3);
	for (c = (8<<MASKSHIFT); c < hash; c <<= 1);
	x->header.mask = (c>>MASKSHIFT) - 1;
	state.mask = x->header.mask;
	qsort((char*)p, (unsigned)x->header.records, 2 * sizeof(Number_t), compare);

	/*
	 * write the header
	 */

	x->header.magic = MAGIC;
	x->header.binsize = NOFFSET * sizeof(Number_t);
	x->header.recsize = hix->maxsize;
	x->cur = x->end = (Number_t*)x->buf;
	x->vio->sp = sp;
	sfwrite(sp, (char*)&x->header, sizeof(x->header));

	/*
	 * write the data
	 */

	m = p + 2 * x->header.records;
	for (offset = 0; p < m;)
	{
		hash = *p;
		if ((c = bucket(x->header.mask, hash) * NOFFSET) > offset)
		{
			iozero(x, c - offset);
			offset = c;
		}
		offset++;
		ioput(x, hash);
		do
		{
			offset++;
			ioput(x, *++p);
		} while (*++p == hash);
	}
	free(b);
	ioflush(x);
	x->vio->sp = op;
	return sferror(sp) ? -1 : 0;
 bad:
	if (b)
		free(b);
	x->vio->sp = op;
	return -1;
}

/*
 * evaluate the RPN index expression given data offset
 * return -1:error 0:false 1:true
 */

static int
eval(register Hix_t* hix, Number_t offset)
{
	char				stack[256];
	register char*			sp = &stack[0];
	register char*			se = &stack[elementsof(stack)];
	register Instruction_t**	pc = hix->pb;
	register Instruction_t**	pe = hix->pc;
	register Instruction_t*		p;

	for (;;)
	{
		if (pc >= pe)
		{
			if (sp != &stack[1])
				break;
			return *(sp - 1);
		}
		else if ((p = *pc++) == AND)
		{
			if (sp-- < &stack[2])
				break;
			*(sp - 1) = *sp && *(sp - 1);
		}
		else if (p == OR)
		{
			if (sp-- < &stack[2])
				break;
			*(sp - 1) = *sp || *(sp - 1);
		}
		else if (sp >= se)
			break;
		else
			*sp++ = p->offset == offset;
	}
	return -1;
}

/*
 * default hix.splitf
 */

static void*
split_default(Hix_t* hix, Sfio_t* ip, char* path, register char* tag, Hixdisc_t* disc)
{
	register char*	s;
	int		n;

	NoP(disc);
	if ((hix->size = (s = sfgetr(ip, '\n', 1)) ? sfvalue(ip) : 0) && tag)
	{
		do
		{
			if (*tag != *s++)
				return 0;
		} while (*++tag);
		n = tag - hix->part->tag;
		hix->size -= n;
	}
	return s;
}

/*
 * open a hix stream
 *
 *	primary		primary data file(s)
 *	secondary	secondary data file(s)
 *	info		data info tag
 *	id		optional -1 terminated index id array
 *	disc		hix discipline
 *
 * file found in this order
 *
 *	primary/info		if primary is vir
 *	primary/info.db		if primary is dir
 *	secondary/info		if secondary is vir
 *	secondary/info.db	if secondary is dir
 *	info.db
 *	primary			if primary is not vir or dir
 *
 * on return id is modified to be the -1 terminated array of out of
 * date indices that must be generated by hixput()
 */

Hix_t*
hixopen(const char* primary, const char* secondary, const char* info, int* id, Hixdisc_t* disc)
{
	register Hix_t*		hix;
	register Index_t*	x;
	register int		i;
	register int		m;
	int			n;
	int			k;
	Number_t		magic = MAGIC;
	char*			b;
	char*			s;
	Part_t*			p;
	Virdir_t*		dp;
	Virfile_t*		fp;
	char			buf[PATH_MAX];

	if (!state.announce)
	{
		state.announce = 1;
		message((-1, ":::: %s", ID + 5));
	}

	/*
	 * allocate and initialize
	 */

	if (disc->version < HIX_VERSION)
	{
		state.error = HIX_ERROR_VERSION;
		return 0;
	}
	if (!disc->splitf)
		disc->splitf = split_default;
	m = n = 0;
	if (id)
		for (; (i = id[n]) != hixend; n++)
		{
			if (ISREFERENCE(i))
				i = INDEX(i);
			if (i >= m)
				m = i + 1;
		}
	if (!(hix = newof(0, Hix_t, 1, (m - 1) * sizeof(int) + (n + 1) * sizeof(int*))) || !(hix->part = hix->parts = newof(0, Part_t, 1, (n - 1) * sizeof(Index_t))))
		return 0;
	hix->disc = disc;
	hix->flags = disc->flags & (HIX_LOCAL|HIX_READONLY|HIX_TEST1|HIX_TEST2|(~(HIX_USER-1)));
	hix->indices = n;
	hix->gen = (int*)&hix->map[m];
	for (k = 0, i = VIR_primary; i <= VIR_last; i <<= 1)
	{
		switch (i)
		{
		case VIR_primary:
		case VIR_last:
			if (!(s = (char*)primary))
				continue;
			break;
		case VIR_secondary:
			if (!(s = (char*)secondary))
				continue;
			break;
		case VIR_info:
			s = ".";
			break;
		}
		strncpy(buf, s, sizeof(buf));
		b = buf;
		for (;;)
		{
			if (s = strchr(b, ':'))
			{
				*s++ = 0;
				if (*s == ':')
					hix->restrict = hix->part->restrict = 1;
			}
			if (viropen(hix, b, primary, secondary, info, i))
			{
				if (k > 0)
				{
					ERROR(hix, HIX_ERROR_DATA_FILE);
					goto bad;
				}
				break;
			}
			hix->part->level = ++k;
			message((-3, "hix: part=%d name=%s info=%s tag=%s delimiter=%c base=%llu offset=%llu size=%llu fd=%d stamp=%s%s%s%s", k, hix->part->name, info, hix->part->tag, hix->delimiter ? hix->delimiter : ';', (Sflong_t)hix->part->base, (Sflong_t)hix->part->vio.offset, (Sflong_t)hix->part->vio.size, sffileno(hix->part->vio.sp), fmttime("%K", hix->part->stamp), hix->part->vio.empty ? " EMPTY" : "", hix->part->vio.sequential ? " SEQUENTIAL" : "", hix->part->restrict ? " RESTRICT" : ""));
			if (!(b = s))
				break;
			if (!(p = newof(0, Part_t, 1, (n - 1) * sizeof(Index_t))))
			{
				ERROR(hix, HIX_ERROR_MALLOC);
				goto bad;
			}
			p->base = hix->part->base + hix->part->vio.size;
			hix->part = hix->part->next = p;
			if (*b == ':')
			{
				b++;
				hix->restrict = p->restrict = 1;
			}
		}
		if (k > 0)
			break;
	}
	if (!k)
	{
		ERROR(hix, HIX_ERROR_DATA_FILE);
		goto bad;
	}
	hix->partitions = k;
	if (disc->eventf)
	{
		hix->name = hix->parts->name;
		sfseek(hix->parts->vio.sp, hix->parts->base + hix->parts->vio.offset, SEEK_SET);
		if ((*disc->eventf)(hix, HIX_OPEN, hix->parts->vio.sp, disc))
		{
			ERROR(hix, HIX_ERROR_DATA_FILE);
			goto bad;
		}
		hix->parts->vio.offset += sftell(hix->parts->vio.sp) - (hix->parts->base + hix->parts->vio.offset);
		if (hix->parts->vio.sp->disc)
			hix->part->vio.sequential = 1;
	}
	for (i = 0; i < n; i++)
		hix->map[INDEX(id[i])] = i;
	for (hix->part = hix->parts; hix->part; hix->part = hix->part->next)
		if (hix->part->vio.empty || hix->part->vio.sequential)
		{
			hix->flags |= HIX_SCAN;
			break;
		}
		else
		{
			strncpy(buf, hix->part->name, sizeof(buf) - 4);
			if (b = strrchr(buf, '/'))
				b++;
			else
				b = buf;
			if (!(s = strrchr(b, '.')))
				s = b + strlen(b);
			strcpy(s, ".hix");
			if (!(dp = virdir(NiL, buf)) && (!(b = s = strrchr(buf, '/')) || !(disc->flags & HIX_LOCAL) && (b = buf) || !(dp = virdir(NiL, ++b))) && !(disc->flags & HIX_READONLY))
			{
				if (!b)
					b = buf;
				else if (b == buf)
				{
					*s = 0;
					if (access(buf, W_OK))
					{
						ERROR(hix, HIX_ERROR_INDEX_WRITE);
						goto bad;
					}
					*s = '/';
				}
				if (!(dp = newof(0, Virdir_t, 1, strlen(b))))
				{
					ERROR(hix, HIX_ERROR_MALLOC);
					goto bad;
				}
				strcpy(dp->name, b);
				dp->next = state.vir;
				state.vir = dp;
			}
			hix->part->dir = dp;

			/*
			 * open and validate each index file
			 */

			for (i = 0; i < n; i++)
			{
				x = &hix->part->index[i];
				if (ISREFERENCE(id[i]))
				{
					*x = hix->part->index[REFERENCE(id[i])];
					x->generate = 0;
					hix->map[INDEX(id[i])] = REFERENCE(id[i]);
				}
				else
				{
					x->id = id[i];
					sfsprintf(buf, sizeof(buf), "%s.%d", info ? info : "FIELD", x->id);
					if (!dp || !dp->sp || !(x->vio = vioopen(dp, buf)))
					{
						message((-5, "hix: %s: update: vio not found", buf));
						goto update;
					}
					if (sfread(x->vio->sp, (char*)&x->header, sizeof(x->header)) != sizeof(x->header))
					{
						message((-5, "hix: %s: update: vio header read", buf));
						goto update;
					}
					if (disc->stamp && strncmp(disc->stamp, x->header.version, sizeof(x->header.version)))
					{
						message((-5, "hix: %s: update: vio header version %-.*s mismatch: expected %-.*s", buf, sizeof(x->header.version), x->header.version, sizeof(x->header.version), disc->stamp));
						goto update;
					}
					if (x->swap = swapop(&magic, &x->header.magic, sizeof(Number_t)))
					{
						if (x->swap < 0)
						{
							message((-5, "hix: %s: update: vio header swap invalid", buf));
							goto update;
						}
						swapmem(x->swap, &x->header, &x->header, sizeof(x->header));
					}
					if (x->header.stamp && x->header.stamp != hix->part->stamp)
					{
						message((-5, "hix: %s: update: vio header stamp %s mismatch: expected %s", buf, fmttime("%K", x->header.stamp), fmttime("%K", hix->part->stamp)));
						goto update;
					}
				}
				if (x->header.recsize > hix->maxsize)
					hix->maxsize = x->header.recsize;
				if (!i)
					hix->records += x->header.records;
				continue;
			update:
				if ((disc->flags & HIX_READONLY) || hix->part->level > 1)
				{
					errno = EACCES;
					ERROR(hix, HIX_ERROR_INDEX_WRITE);
					goto bad;
				}
				for (fp = dp->file; fp; fp = fp->next)
					if (streq(fp->name, buf))
						break;
				if (!fp)
				{
					fp = newof(0, Virfile_t, 1, strlen(buf));
					strcpy(fp->name, buf);
					fp->next = dp->file;
					dp->file = fp;
				}
				fp->index = x;
				x->vio = &fp->vio;
				if (!(x->vio->sp = sftmp(SF_BUFSIZE)))
				{
					ERROR(hix, HIX_ERROR_TMP);
					goto bad;
				}
				sfset(x->vio->sp, SF_READ, 0);
				x->generate = hix;
				memzero(&x->header, sizeof(x->header));
				x->header.stamp = hix->part->stamp;
				if (disc->stamp)
					strncpy(x->header.version, disc->stamp, sizeof(x->header.version) - 1);
				hix->gen[hix->map[x->id]] = 1;

				/*
				 * NOTE: if >32 indices then hixclose()
				 *	 may do duplicate build()s
				 */

				dp->generate |= (1<<hix->map[x->id]);
				hix->flags |= HIX_SCAN;
			}
		}
	if (id)
	{
		for (i = k = 0; i < n; i++)
			if (hix->gen[hix->map[m = INDEX(id[i])]])
			{
				id[k] = m;
				hix->gen[k] = hix->map[m];
				k++;
			}
		hix->gen[k] = id[k] = hixend;
	}
	if (hix->flags & HIX_SCAN)
		hix->maxsize = hix->records = 0;
	hix->part = hix->parts;
	hix->name = hix->part->name;
	return hix;
 bad:
	i = errno;
	hixclose(hix);
	errno = i;
	return 0;
}

/*
 * seek to record offset
 */

int
hixseek(Hix_t* hix, off_t offset)
{
	hix->part = hix->parts;
	hix->offset = offset;
	hix->size = 0;
	while (hix->part && offset > hix->part->vio.size)
	{
		offset -= hix->part->vio.size;
		hix->part = hix->part->next;
	}
	if (!hix->part)
	{
		hix->part = hix->parts;
		hix->offset = offset = 0;
	}
	offset += hix->part->vio.offset - hix->part->base;
	return sfseek(hix->part->vio.sp, offset, 0) == offset ? 0 : -1;
}

/*
 * close an open hix stream
 */

int
hixclose(register Hix_t* hix)
{
	if (hix)
	{
		register Instruction_t*	p;
		register Virfile_t*	fp;
		register Virdir_t*	dp;
		register Sfio_t*	sp;
		long			n;
		Sfoff_t			offset;

		while (hix->part = hix->parts)
		{
			if (hix->part->vio.sp)
			{
				if (hix->part->vio.sp != sfstdin)
					sfclose(hix->part->vio.sp);
				hix->part->vio.sp = 0;
			}
			if ((dp = hix->part->dir) && dp->generate)
			{
				remove(dp->name);
				if (!(sp = sfopen(NiL, dp->name, "w+")))
					ERROR(hix, HIX_ERROR_INDEX_WRITE);
				else
				{
					message((-5, "hix: generate %s [0x%08x]", dp->name, dp->generate));
					sfset(sp, SF_READ, 0);
					sfprintf(sp, "%c%s%c%s hash index\n", VDB_DELIMITER, VDB_MAGIC, VDB_DELIMITER, hix->part->index[0].header.version);
					for (fp = dp->file; fp; fp = fp->next)
					{
						if (!(hix->flags & HIX_ERROR) && (!fp->index || fp->index->flushed > 0 && fp->index->generate == hix))
						{
							sfputr(sp, fp->name, '\n');
							offset = sftell(sp);
							while (offset & (HIX_ALIGN - 1))
							{
								offset++;
								sfputc(sp, '\n');
							}
							if (fp->index)
							{
								message((-5, "hix: build %s/%s", dp->name, fp->name));
								if (build(hix, fp->index, sp))
									ERROR(hix, HIX_ERROR_INDEX_WRITE);
								sfclose(fp->index->vio->sp);
								dp->generate &= ~(1<<hix->map[fp->index->id]);
								fp->index = 0;
							}
							else
							{
								message((-5, "hix: copy %s/%s", dp->name, fp->name));
								if (sfseek(dp->sp, fp->vio.offset, 0) != fp->vio.offset || sfmove(dp->sp, sp, fp->vio.size, -1) != fp->vio.size)
									ERROR(hix, HIX_ERROR_INDEX_WRITE);
							}
							fp->vio.sp = sp;
							fp->vio.offset = offset;
							fp->vio.size = sftell(sp) - offset;
						}
					}
					sfputr(sp, VDB_DIRECTORY, '\n');
					offset = sftell(sp);
					for (fp = dp->file; fp; fp = fp->next)
						if (!fp->index || fp->index->generate == hix)
							sfprintf(sp, ";%s;%llu;%llu;%s=0444;%s=%lu\n", fp->name, (Sflong_t)fp->vio.offset, (Sflong_t)fp->vio.size, VDB_MODE, VDB_DATE, hix->part->stamp);
					n = sftell(sp) - offset + VDB_LENGTH;
					sfprintf(sp, ";%s;%0*llu;%0*lu\n", VDB_DIRECTORY, VDB_FIXED, (Sflong_t)offset, VDB_FIXED, n);
					if (sfsync(sp))
						ERROR(hix, HIX_ERROR_INDEX_WRITE);
					sfset(sp, SF_READ, 1);
					sfset(sp, SF_WRITE, 0);
					if (dp->sp)
						sfclose(dp->sp);
					dp->sp = sp;
				}
			}
			hix->parts = hix->part->next;
			if (hix->part->name)
				free(hix->part->name);
			free(hix->part);
		}
		hixset(hix, 0);
		while (p = hix->free)
		{
			hix->free = p->next;
			free(p);
		}
		if (hix->pb)
			free(hix->pb);
		if (hix->disc->eventf && (*hix->disc->eventf)(hix, HIX_CLOSE, NiL, hix->disc) < 0)
			hix->flags |= HIX_ERROR;
		n = (hix->flags & HIX_ERROR) ? -1 : 0;
		free(hix);
		return n;
	}
	else
	{
		register Virdir_t*	dp;
		register Virfile_t*	fp;

		while (dp = state.vir)
		{
			state.vir = dp->next;
			while (fp = dp->file)
			{
				dp->file = fp->next;
				if (fp->tag)
					free(fp->tag);
				free(fp);
			}
			free(dp);
		}
	}
	return 0;
}

/*
 * return the current expression position
 */

int
hixpos(Hix_t* hix)
{
	return hix->pc - hix->pb;
}

/*
 * set the expression position to pos
 * pos==0 clears the expression
 * otherwise pos must be a value returned by hixpos()
 */

int
hixset(register Hix_t* hix, int pos)
{
	register Instruction_t**	pc;
	register Instruction_t**	pn;
	register Instruction_t*		p;

	pc = hix->pc;
	pn = hix->pc = hix->pb + pos;
	while (pc > pn)
		if ((p = *--pc) != AND && p != OR)
		{
			hix->active = p->next;
			p->next = hix->free;
			hix->free = p;
		}
	return pos;
}

/*
 * push (op,arg) onto the index expression stack
 * if op={HIX_and,HIX_or} then hash ignored
 * otherwise op is the index id
 */

int
hixop(register Hix_t* hix, int op, long hash)
{
	register int		n;
	register Instruction_t*	p;
	register Number_t	v;
	register Number_t	h = hash;

	if (hix->flags & (HIX_ERROR|HIX_SCAN))
		return -1;
	if (hix->indices <= 0)
		return 0;
	if (hix->pc >= hix->pe)
	{
		n = hix->pc - hix->pb;
		if (!(hix->pb = newof(hix->pb, Instruction_t*, 2 * n + 2, 0)))
		{
			hix->pc = hix->pe = 0;
			ERROR(hix, HIX_ERROR_MALLOC);
			return -1;
		}
		hix->pc = hix->pb + n;
		hix->pe = hix->pc + n + 2;
	}
	if (op == HIX_and)
		p = AND;
	else if (op == HIX_or)
		p = OR;
	else
	{
		if (p = hix->free)
			hix->free = p->next;
		else if (!(p = newof(0, Instruction_t, 1, 0)))
		{
			ERROR(hix, HIX_ERROR_MALLOC);
			return -1;
		}
		p->next = hix->active;
		hix->active = p;
		p->id = hix->map[op];
		p->part = hix->parts;
		p->data = p->last = p->base;
		p->offset = MAXOFFSET;
		p->hash = h |= HASHMARK;
		do
		{
			p->position = bucket(p->part->index[p->id].header.mask, h) * p->part->index[p->id].header.binsize + sizeof(p->part->index[p->id].header);
			while (v = ioget(p))
				if (v == h)
				{
					p->offset = ioget(p) + p->part->base - 1;
					goto found;
				}
			p->data = p->last;
		} while (p->part = p->part->next);
	}
 found:
	*hix->pc++ = p;
	return 0;
}

/*
 * return the next record matching the index expression
 * 0 returned when no more records match
 */

static int
show(Sfio_t* fp)
{
	message((-11, "HIXGET %d %lld", sffileno(fp), sftell(fp)));
	return 0;
}

void*
hixget(register Hix_t* hix, int partition)
{
	register Instruction_t*	p;
	register Instruction_t*	b;
	register Number_t	offset;
	register Number_t	n;
	int			v;
	void*			r;

	if ((hix->flags & (HIX_ERROR|HIX_SCAN)) || hix->pc <= hix->pb)
	{
		if (!hix->part)
			goto notfound;
	again:
		hix->offset += hix->size;
		for (;;)
		{
			if (hix->offset >= (hix->part->base + hix->part->vio.size) && !hix->part->vio.sequential || !(hix->flags & HIX_SCAN) && ((hix->part->restrict && partition > hix->part->level || sfseek(hix->part->vio.sp, hix->offset + hix->part->vio.offset - hix->part->base, 0) != (hix->offset + hix->part->vio.offset - hix->part->base))))
			{
				hix->size = 0;
				if (!(hix->part = hix->part->next))
					goto notfound;
			}
			else if (show(hix->part->vio.sp), r = (*hix->disc->splitf)(hix, hix->part->vio.sp, hix->part->name, hix->part->tag, hix->disc))
				break;
			else
			{
				if (hix->size)
					goto again;
				if (!(hix->part = hix->part->next))
					goto notfound;
			}
		}
	}
	else for (;;)
	{
		/*
		 * find the lowest index offset and evaluate
		 */

		for (b = p = hix->active; p; p = p->next)
			if (p->offset < b->offset)
				b = p;
		if (b->offset == MAXOFFSET)
			goto notfound;
		offset = b->offset;
		hix->part = b->part;
		if ((v = eval(hix, offset)) < 0)
		{
			ERROR(hix, HIX_ERROR_EVAL);
			goto notfound;
		}

		/*
		 * bump all indices matching the lowest offset
		 */

		for (p = hix->active; p; p = p->next)
			if (p->offset == offset)
			{
				if ((p->offset = ioget(p)) && !(p->offset & HASHMARK))
					p->offset += p->part->base - 1;
				else for (;;)
				{
					if (!(p->part = p->part->next))
					{
						p->offset = MAXOFFSET;
						break;
					}
					p->position = bucket(p->part->index[p->id].header.mask, p->hash) * p->part->index[p->id].header.binsize + sizeof(p->part->index[p->id].header);
					p->data = p->last;
					while (n = ioget(p))
						if (n == p->hash)
						{
							p->offset = ioget(p) + p->part->base - 1;
							goto hit;
						}
				}
			hit:	;
			}
		if (partition > 0 && hix->part->restrict)
		{
			if (partition < hix->part->level)
				goto notfound;
			if (partition > hix->part->level)
				continue;
		}
		if (v)
		{
			if ((hix->offset = offset) >= hix->part->base + hix->part->vio.size)
				goto notfound;
			offset += hix->part->vio.offset - hix->part->base;
			if (sfseek(hix->part->vio.sp, offset, 0) != offset)
			{
				ERROR(hix, HIX_ERROR_DATA_SEEK);
				goto notfound;
			}
			if (!(r = (*hix->disc->splitf)(hix, hix->part->vio.sp, hix->part->name, NiL, hix->disc)))
			{
				ERROR(hix, HIX_ERROR_DATA_READ);
				goto notfound;
			}
			break;
		}
	}
	if (hix->size > hix->maxsize)
		hix->maxsize = hix->size;
	hix->partition = hix->part->level;
	hix->restrict = hix->part->restrict ? hix->partition : 0;
	return r;
 notfound:
	hix->size = 0;
	return 0;
}

/*
 * update the hash values for the current hixget() record
 */

int
hixput(register Hix_t* hix, register long* hash)
{
	register int*	g;
	register int	m;
	register long	n;

	g = hix->gen;
	if ((m = *g++) >= 0)
	{
		while (hix->part && (n = hix->offset - hix->part->base + 1) > hix->part->vio.size)
			hix->part = hix->part->next;
		if (hix->part)
		{
			if (!hix->part->dir->generate)
				*(g - 1) = hixend;
			else do
			{
				if (hix->part->index[m].generate == hix && (ioput(&hix->part->index[m], ((Number_t)*hash)|HASHMARK) || ioput(&hix->part->index[m], n)))
				{
					ERROR(hix, HIX_ERROR_INDEX_WRITE);
					return -1;
				}
				hash++;
			} while ((m = *g++) > 0);
		}
	}
	return 0;
}
