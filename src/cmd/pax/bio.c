/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1987-2000 AT&T Corp.                *
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
 * AT&T Bell Laboratories
 *
 * pax block io support
 */

#include "pax.h"

#include "FEATURE/mtio"
#if _sys_mtio
#include <ast_tty.h>
#include <sys/mtio.h>
#endif
#ifdef MTIOCTOP
#if defined(MTIOBSF) && !defined(MTBSF)
#define MTBSF	MTIOBSF
#endif
#if defined(MTIOBSR) && !defined(MTBSR)
#define MTBSR	MTIOBSR
#endif
#if defined(MTIOEOM) && !defined(MTEOM)
#define MTEOM	MTIOEOM
#endif
#if defined(MTIOFSF) && !defined(MTFSF)
#define MTFSF	MTIOFSF
#endif
#if defined(MTIOWEOF) && !defined(MTWEOF)
#define MTWEOF	MTIOWEOF
#endif
#if !defined(MTBSF) || !defined(MTBSR) || !defined(MTWEOF) || defined(__hppa)/*hppa-compiler-signal-10*/
#undef	MTIOCTOP
#endif
#endif

#if 0 && DEBUG

/*
 * --blok=i	input is BLOK file
 * --blok=o	output file is BLOK file
 */

static int
blokread(register Archive_t* ap, char* buf, int n)
{
	register int		i;
	register int		j;
	char			c;

	static int		eof;

	if (!ap->io->blokflag)
	{
		ap->io->blokflag = 1;
		eof = 0;
		if ((i = read(ap->io->fd, buf, ap->io->blok ? 4 : n)) < 4 || !strneq(buf, "\002\014\017\013", 4))
		{
			if (ap->io->blok) error(3, "%s: input archive is not a BLOK file", ap->name);
			return i;
		}
		if (i > 4 && lseek(ap->io->fd, (off_t)4, SEEK_SET) != 4)
			error(3, "%s: cannot seek on input archive BLOK file -- use --blok=i", ap->name);
		ap->io->blok = 1;
	}
	if (ap->io->blok)
	{
		j = 0;
		do
		{
			if ((i = read(ap->io->fd, &c, 1)) < 1)
				return i < 0 && ++eof == 1 ? 0 : -1;
			j <<= 7;
			j |= c & 0177;
		} while (c & 0200);
		if (j > 0)
		{
			if (j > n)
				error(2, "%s: blokread buffer overflow (%d>%d)", ap->name, j, n);
			if ((i = read(ap->io->fd, buf, j)) != j)
				error(2, "%s: blokread blocking error", ap->name);
		}
		else i = 0;
	}
	else i = read(ap->io->fd, buf, n);
	return i;
}

static int
blokwrite(register Archive_t* ap, char* buf, int n)
{
	register char*	s;
	register int	i;
	register int	j;
	char		blk[9];

	if (ap->io->blok)
	{
		s = blk;
		if (!ap->io->blokflag)
		{
			ap->io->blokflag = 1;
			*s++ = 002;
			*s++ = 014;
			*s++ = 017;
			*s++ = 013;
		}
		i = 0;
		if (j = (n >> 21) & 0177)
		{
			*s++ = j | 0200;
			i = 1;
		}
		if ((j = (n >> 14) & 0177) || i)
		{
			*s++ = j | 0200;
			i = 1;
		}
		if ((j = (n >> 7) & 0177) || i)
		{
			*s++ = j | 0200;
			i = 1;
		}
		*s++ = n & 0177;
		j = s - blk;
		if ((i = write(ap->io->fd, blk, j)) != j)
			error(ERROR_SYSTEM|3, "%s: blokwrite count write error (%d!=%d)", ap->name, i, j);
		if (n <= 0) i = n;
		else if ((i = write(ap->io->fd, buf, n)) != n)
			error(ERROR_SYSTEM|3, "%s: blokwrite data write error (%d!=%d", ap->name, i, n);
	}
	else i = write(ap->io->fd, buf, n);
	return i;
}

#define read(f,b,n)	blokread(f,b,n)
#define write(f,b,n)	blokwrite(f,b,n)

#endif

/*
 * initialize buffered io
 */

void
binit(register Archive_t* ap)
{
	unsigned long	n;

	if (ap->delta)
		ap->delta->hdr = ap->delta->hdrbuf;
	n = 2 * state.buffersize;
	if (!(ap->io->mode & O_WRONLY))
		n += MAXUNREAD;
	else if (!(format[ap->format].flags & OUT))
		error(3, "%s: archive format not supported on output" , format[ap->format].name);
	if (!(ap->io->buffer = newof(0, char, n, 0)))
		error(3, "%s: cannot allocate buffer", ap->name);
	ap->io->next = ap->io->last = ap->io->buffer;
}

/*
 * skip files on tape fd
 */

int
bskip(register Archive_t* ap)
{
	long		c;
	int		skip = ap->io->skip;
#ifdef MTIOCTOP
	struct mtop	mt;
#ifdef MTEOM
	int		mteom = 1;
#endif
#ifdef MTFSF
	int		mtfsf = 1;
#endif
#endif

	if (ap->io->mode != O_RDONLY)
	{
		ap->io->next = ap->io->last = ap->io->buffer + MAXUNREAD;
		ap->io->eof = 0;
	}
	while (skip)
	{
#ifdef MTIOCTOP
#ifdef MTEOM
		if (skip < 0 && mteom)
		{
			mt.mt_op = MTEOM;
			mt.mt_count = 1;
			if (ioctl(ap->io->fd, MTIOCTOP, &mt) >= 0)
			{
				if (ap->io->mode != O_RDONLY) ap->io->eof = 1;
				break;
			}
			mteom = 0;
		}
#endif
#ifdef MTFSF
		if (mtfsf)
		{
			mt.mt_op = MTFSF;
			mt.mt_count = 1;
			if (ioctl(ap->io->fd, MTIOCTOP, &mt) >= 0)
			{
				skip--;
				continue;
			}
			if (errno != ENOTTY)
			{
				if (ap->io->mode != O_RDONLY) ap->io->eof = 1;
				break;
			}
			mtfsf = 0;
		}
#endif
#endif
		while ((c = read(ap->io->fd, state.tmp.buffer, state.tmp.buffersize)) > 0);
		if (c < 0)
		{
			if (ap->io->mode != O_RDONLY) ap->io->eof = 1;
			break;
		}
		skip--;
	}
	return 0;
}

/*
 * fill input buffer at ap->io->last
 * if must!=0 then EOF causes query for next input volume file
 */

static int
bfill(register Archive_t* ap, int must)
{
	register int	c;

	if (ap->io->eof)
		return -1;
	if (ap->io->skip)
		ap->io->skip = bskip(ap);
	while ((c = read(ap->io->fd, ap->io->last, state.buffersize)) <= 0)
	{
		if (must)
			newio(ap, c, 0);
		else
		{
			ap->io->eof = 1;
			return -1;
		}
	}
	message((-8, "read(%s,%d): %-.*s%s", ap->name, c, c > 32 ? 32 : c, ap->io->last, c > 32 ? "..." : ""));
	ap->io->eof = 0;
	ap->io->last += c;
	return 0;
}

/*
 * buffered char input
 * at least n chars required, m chars max
 * if b is 0 then n chars are skipped
 * if must!=0 then EOF causes query for next input volume file
 */

off_t
bread(register Archive_t* ap, void* ab, register off_t n, off_t m, int must)
{
	register char*		b = (char*)ab;
	register ssize_t	c;
	register char*		ob;
	register off_t		k;

	if (ap->io->eof) return -1;
	if (m <= 0) return 0;
	ob = b;
	if (ap->io->blocked)
	{
		if (!b) b = state.tmp.buffer;
		while ((c = read(ap->io->fd, b, m)) <= 0)
		{
			if (must) newio(ap, c, 0);
			else if (ap->io->empty)
			{
				ap->io->eof = 1;
				return -1;
			}
			else
			{
				if (c < 0) ap->io->eof = 1;
				else
				{
					ap->io->empty = 1;
#if DEBUG
					if (ob) message((-7, "bread(%s,%d@%I*d):", ap->name, c, sizeof(ap->io->count), ap->io->count));
#endif
				}
				return c;
			}
		}
		ap->io->empty = 0;
		if (must && c < n) return -1;
		ap->io->count += c;
		if (ap->sum > 0)
		{
			ap->memsum = memsum(b, c, ap->memsum);
			ap->old.memsum = omemsum(b, c, ap->old.memsum);
		}
		if (state.convert[SECTION(ap)].on)
			ccmaps(b, c, state.convert[SECTION(ap)].external, state.convert[SECTION(ap)].internal);
#if DEBUG
		if (ob) message((-7, "bread(%s,%d@%I*d): %-.*s%s", ap->name, c, sizeof(ap->io->count), ap->io->count, c > 32 ? 32 : c, ob, c > 32 ? "..." : ""));
#endif
		return c;
	}
	else
	{
		if (n <= 0) n = m;
		else m = n;
		for (;;)
		{
			if (n > (c = ap->io->last - ap->io->next))
			{
				if (c > 0)
				{
					if (ap->sum > 0)
					{
						ap->memsum = memsum(ap->io->next, c, ap->memsum);
						ap->old.memsum = omemsum(ap->io->next, c, ap->old.memsum);
					}
					if (ob)
					{
						memcpy(b, ap->io->next, c);
						if (state.convert[SECTION(ap)].on)
							ccmaps(b, c, state.convert[SECTION(ap)].external, state.convert[SECTION(ap)].internal);
					}
					b += c;
					n -= c;
					ap->io->count += c;
				}
				ap->io->next = ap->io->last = ap->io->buffer + MAXUNREAD;
				if (!ob && ap->sum <= 0 && ap->io->seekable && (k = n / BUFFERSIZE) && lseek(ap->io->fd, k *= BUFFERSIZE, SEEK_CUR) >= 0)
				{
					b += k;
					n -= k;
					ap->io->count += k;
					message((-7, "bread(%s) skip(%I*d@%I*d)", ap->name, sizeof(k), k, sizeof(ap->io->count), ap->io->count));
				}
				if (bfill(ap, must) < 0)
				{
					if (ob && (c = (b - ob)))
					{
						bunread(ap, ob, c);
						return 0;
					}
					return -1;
				}
			}
			else
			{
				if (ap->sum > 0)
				{
					ap->memsum = memsum(ap->io->next, n, ap->memsum);
					ap->old.memsum = omemsum(ap->io->next, n, ap->old.memsum);
				}
				if (ob)
				{
					memcpy(b, ap->io->next, n);
					if (state.convert[SECTION(ap)].on)
						ccmaps(b, n, state.convert[SECTION(ap)].external, state.convert[SECTION(ap)].internal);
				}
				ap->io->next += n;
				ap->io->count += n;
#if DEBUG
				if (ob)
				{
					n += b - ob;
					message((-7, "bread(%s,%I*d@%I*d): %-.*s%s", ap->name, sizeof(n), n, sizeof(ap->io->count), ap->io->count, n > 32 ? 32 : (int)n, ob, n > 32 ? "..." : ""));
				}
				else
					message((-7, "bread(%s) skip(%I*d@%I*d)", ap->name, sizeof(n), n, sizeof(ap->io->count), ap->io->count));
#endif
				return m;
			}
		}
	}
}

/*
 * pushback for next bread()
 */

void
bunread(register Archive_t* ap, void* ab, register int n)
{
	register char*	b = (char*)ab;

	ap->io->eof = 0;
	ap->io->count -= n;
	if ((ap->io->next -= n) < ap->io->buffer + MAXUNREAD)
	{
		if (ap->io->next < ap->io->buffer)
			error(PANIC, "bunread(%s,%d): too much pushback", ap->name, n);
		memcpy(ap->io->next, b, n);
		if (state.convert[SECTION(ap)].on)
			ccmaps(ap->io->next, n, state.convert[SECTION(ap)].internal, state.convert[SECTION(ap)].external);
	}
	message((-7, "bunread(%s,%d@%I*d): %-.*s%s", ap->name, n, sizeof(ap->io->count), ap->io->count, n > 32 ? 32 : n, ap->io->next, n > 32 ? "..." : ""));
}

/*
 * bread() n chars and return a pointer to the char buffer
 */

char*
bget(register Archive_t* ap, register off_t n, off_t* p)
{
	register char*	s;

	if (n == -1)
	{
		if (ap->io->eof || !ap->io->seekable)
			return 0;
		if (ap->io->last > ap->io->next)
			n = ap->io->last - ap->io->next;
		else if ((n = ap->io->size - (ap->io->offset + ap->io->count)) < 0)
			return 0;
		else if (n > state.buffersize)
			n = state.buffersize;
	}
	if (p)
		*p = n;
	ap->io->count += n;
	s = ap->io->next;
	ap->io->next += n;
	while (ap->io->next > ap->io->last)
	{
		if (ap->io->last > ap->io->buffer + MAXUNREAD + state.buffersize)
		{
                        register char*  b;
                        register int    k;
                        register int    m;

                        k = ap->io->last - s;
			m = roundof(k, IOALIGN) - k;
#if DEBUG
			if (m)
				message((-8, "bget(%s) buffer alignment offset=%d", ap->name, m));
#endif
                        b = ap->io->next = ap->io->buffer + MAXUNREAD + m;
                        ap->io->last = b + k;
                        m = s - b;
                        while (k > m)
                        {
                                message((-8, "bget(%s) overlapping memcpy n=%I*d k=%d m=%d", ap->name, sizeof(n), n, k, m));
                                memcpy(b, s, m);
                                b += m;
                                s += m;
                                k -= m;
                        }
                        memcpy(b, s, k);
			s = ap->io->next;
			ap->io->next += n;
		}
		if (bfill(ap, 1) < 0)
			return 0;
	}
	if (ap->sum > 0)
	{
		ap->memsum = memsum(s, n, ap->memsum);
		ap->old.memsum = omemsum(s, n, ap->old.memsum);
	}
	if (state.convert[SECTION(ap)].on)
		ccmaps(s, n, state.convert[SECTION(ap)].external, state.convert[SECTION(ap)].internal);
	message((-7, "bget(%s,%I*d@%I*d): %-.*s%s", ap->name, sizeof(n), n, sizeof(ap->io->count), ap->io->count, n > 32 ? 32 : (int)n, s, n > 32 ? "..." : ""));
	return s;
}

/*
 * restore input to bsave()'d position
 */

void
brestore(Archive_t* ap)
{
	*ap->io = state.backup;
	bseek(ap, ap->io->offset + ap->io->count, SEEK_SET, 1);
	bflushin(ap, 0);
}

/*
 * back up input to bsave()'d position and prime output buffer
 */

void
backup(register Archive_t* ap)
{
	register off_t	n;
	register off_t	m;
#ifdef MTIOCTOP
	struct mtop	mt;
#endif

	switch (ap->format)
	{
	case ALAR:
	case IBMAR:
#ifdef MTIOCTOP
		mt.mt_op = MTBSF;
		mt.mt_count = 1;
		if (!(ioctl(ap->io->fd, MTIOCTOP, &mt))) return;
#endif
		break;
	default:
		m = ap->io->next - (ap->io->buffer + MAXUNREAD);
		if ((n = ap->io->count - m) > state.backup.count)
		{
			message((-1, "backup(%s): reread %ld", ap->name, n + m));
			m = state.backup.last - (state.backup.buffer + MAXUNREAD);
			if (lseek(ap->io->fd, (off_t)(-(n + m)), SEEK_CUR) == -1)
			{
#ifdef MTIOCTOP
				mt.mt_op = MTBSR;
				mt.mt_count = 2;
				if (ioctl(ap->io->fd, MTIOCTOP, &mt)) break;
#else
				break;
#endif
			}
			if (read(ap->io->fd, ap->io->buffer + MAXUNREAD, m) != m) break;
		}
		else m = ap->io->last - (ap->io->buffer + MAXUNREAD);
		message((-1, "backup(%s): %ld", ap->name, m));
		if ((m = lseek(ap->io->fd, -m, SEEK_CUR)) == -1)
		{
#ifdef MTIOCTOP
			mt.mt_op = MTBSR;
			mt.mt_count = 1;
			if (ioctl(ap->io->fd, MTIOCTOP, &mt)) break;
#else
			break;
#endif
		}
		if (state.backup.next < state.backup.last)
			bwrite(ap, ap->io->buffer + MAXUNREAD, state.backup.next - (state.backup.buffer + MAXUNREAD));
		return;
	}
	error(3, "%s: cannot position %s archive for append", ap->name, format[ap->format].name);
}

/*
 * flush buffered input
 */

void
bflushin(register Archive_t* ap, int hard)
{
	ap->io->count += ap->io->last - ap->io->next;
	ap->io->next = ap->io->last = ap->io->buffer + MAXUNREAD;
	if (hard && !ap->io->eof)
	{
		while (read(ap->io->fd, state.tmp.buffer, state.buffersize) > 0);
		ap->io->eof = 1;
	}
}

/*
 * buffered seek
 */

off_t
bseek(register Archive_t* ap, off_t pos, int op, int hard)
{
	off_t	l;
	off_t	u;

	if (hard)
	{
		if (op != SEEK_SET)
			return -1;
	}
	else if (op == SEEK_CUR)
	{
		l = ap->io->buffer - ap->io->next + MAXUNREAD;
		u = ap->io->last - ap->io->next;
		if (pos >= l && pos <= u)
		{
			ap->io->next += pos;
			return ap->io->count += pos;
		}
		pos += ap->io->count;
		op = SEEK_SET;
	}
	else if (op != SEEK_SET)
		return -1;
	else if ((pos - ap->io->count) >= (ap->io->buffer - ap->io->next) && (pos - ap->io->count) <= (ap->io->last - ap->io->buffer))
	{
		ap->io->next += (pos - ap->io->count);
		return ap->io->count = pos;
	}
	ap->io->next = ap->io->last = ap->io->buffer + MAXUNREAD;
	if ((pos = lseek(ap->io->fd, ap->io->offset + pos, op)) < 0)
		return -1;
	ap->io->empty = 0;
	ap->io->eof = 0;
	return ap->io->count = pos - ap->io->offset;
}

/*
 * flush buffered output
 */

void
bflushout(register Archive_t* ap)
{
	register int	n;
	register int	c;

	if (n = ap->io->next - ap->io->buffer)
	{
		ap->io->next = ap->io->buffer;
		while ((c = write(ap->io->fd, ap->io->next, n)) != n)
		{
			if (c <= 0) newio(ap, c, n);
			else
			{
				ap->io->next += c;
				n -= c;
			}
		}
		ap->io->next = ap->io->buffer;
	}
}

/*
 * buffered output
 */

void
bwrite(register Archive_t* ap, void* ab, register int n)
{
	register char*	b = (char*)ab;
	register int	c;
	int		an;

	if (state.convert[SECTION(ap)].on)
	{
		ccmaps(b, n, state.convert[SECTION(ap)].internal, state.convert[SECTION(ap)].external);
		an = n;
	}
	else an = 0;
	if (ap->sum > 0)
		ap->memsum = memsum(b, n, ap->memsum);
	if (state.checksum.sum && SECTION(ap) == SECTION_DATA)
		sumblock(state.checksum.sum, b, n);
	if (ap->io->skip)
		ap->io->skip = bskip(ap);
	if (state.maxout && ap->io->count >= state.maxout)
	{
		bflushout(ap);
		newio(ap, 0, 0);
	}
	ap->io->count += n;
	if (ap->io->blocked)
	{
#if DEBUG
		if (n > 0) message((-7, "bwrite(%s,%d@%ld): %-.*s...", ap->name, n, ap->io->count + n, n > 32 ? 32 : n, b));
		else message((-7, "bwrite(%s,%d@%ld):", ap->name, n, ap->io->count + n));
#endif
		while ((c = write(ap->io->fd, b, n)) != n)
		{
			if (n <= 0)
			{
#ifdef MTIOCTOP
				{
					struct mtop	mt;

					mt.mt_op = MTWEOF;
					mt.mt_count = 1;
					if (ioctl(ap->io->fd, MTIOCTOP, &mt) >= 0) break;
				}
#endif
				error(3, "%s: cannot write tape EOF marks", ap->name);
			}
			if (c <= 0) newio(ap, c, n);
			else if ((n -= c) > 0) b += c;
			else break;
		}
	}
	else
	{
#if DEBUG
		if (n > 0) message((-7, "bwrite(%s,%d@%ld): %-.*s...", ap->name, n, ap->io->count + n, n > 32 ? 32 : n, b));
		else message((-7, "bwrite(%s,%d@%ld):", ap->name, n, ap->io->count + n));
#endif
		for (;;)
		{
			if ((c = ap->io->buffer + state.blocksize - ap->io->next) <= n)
			{
				if (c)
				{
					memcpy(ap->io->next, b, c);
					n -= c;
					b += c;
				}
				ap->io->next = ap->io->buffer;
				while ((c = write(ap->io->fd, ap->io->next, state.blocksize)) != state.blocksize)
				{
					if (c <= 0) newio(ap, c, n);
					else
					{
						memcpy(state.tmp.buffer, ap->io->buffer + c, state.blocksize - c);
						memcpy(ap->io->buffer, state.tmp.buffer, state.blocksize - c);
						ap->io->next = ap->io->buffer + state.blocksize - c;
						break;
					}
				}
				message((-8, "write(%s,%d): %-.32s...", ap->name, c, ap->io->buffer));
			}
			else
			{
				memcpy(ap->io->next, b, n);
				ap->io->next += n;
				break;
			}
		}
	}
	if (an)
		ccmaps(ab, an, state.convert[SECTION(ap)].external, state.convert[SECTION(ap)].internal);
}

/*
 * bwrite() n chars that have been placed in ap->io->next
 */

void
bput(register Archive_t* ap, register int n)
{
	ap->io->count += n;
	message((-7, "bput(%s,%d@%ld): %-.*s%s", ap->name, n, ap->io->count, n > 32 ? 32 : n, ap->io->next, n > 32 ? "..." : ""));
	if (state.convert[SECTION(ap)].on)
		ccmaps(ap->io->next, n, state.convert[SECTION(ap)].internal, state.convert[SECTION(ap)].external);
	if (ap->sum > 0)
		ap->memsum = memsum(ap->io->next, n, ap->memsum);
	if (state.checksum.sum && SECTION(ap) == SECTION_DATA)
		sumblock(state.checksum.sum, ap->io->next, n);
	if ((ap->io->next += n) > ap->io->buffer + state.blocksize)
	{
		n = (ap->io->next - ap->io->buffer) - state.blocksize;
		ap->io->count -= n;

		/*
		 * flush out the buffer and slide over the remains
		 */

		SECTION_SKIP(ap);
		bwrite(ap, ap->io->next = ap->io->buffer + state.blocksize, n);
		SECTION_KEEP(ap);
	}
}

/*
 * return recommended io block size for fd
 * 0 returned for no recommendation
 */

long
bblock(int fd)
{
#ifdef MTIOCGETBLKINFO
	struct mtblkinfo	mt;

	return ioctl(fd, MTIOCGETBLKINFO, &mt) < 0 ? 0 : mt.recblksz;
#else
	return 0;
#endif
}

static struct
{
	char*		path;
	struct stat*	st;
} dev;

/*
 * find path name in /dev for <dev.st->st_dev,dev.st->st_ino>
 * called by ftwalk()
 */

static int
devpath(register Ftw_t* ftw)
{
	if (ftw->info == FTW_F && ftw->statb.st_rdev == dev.st->st_rdev && S_ISCHR(ftw->statb.st_mode) == S_ISCHR(dev.st->st_mode))
	{
		message((-1, "device name is %s", ftw->path));
		dev.path = strdup(ftw->path);
		return 1;
	}
	return 0;
}

/*
 * initilize tty file pointers for interactive prompting
 */

void
interactive(void)
{
	int	fd;

	if (!state.rtty)
	{
		fd = dup(2);
		if (!(state.rtty = sfopen(NiL, "/dev/tty", "r")) || !(state.wtty = sfopen(NiL, "/dev/tty", "w")))
			error(ERROR_SYSTEM|3, "cannot prompt for interactive input");
		sfsetbuf(state.rtty, NiL, 0);
		sfsetbuf(state.wtty, NiL, 0);
		if (fd >= 0) close(fd);
	}
}

/*
 * check for new input or output stream
 * c is the io count causing the newio()
 * n is the pending buffered io count
 */

void
newio(register Archive_t* ap, int c, int n)
{
	register char*	s;
	register char*	rw;
	char*		file;
	char*		io;
	char*		t;
	int		vol;
	off_t		z;
	struct stat	st;

	Sfio_t*		cp = 0;
	Sfio_t*		pp = 0;
	int		oerrno = errno;

	static int	locked;
	static off_t	total;

	if (!ap->part)
		ap->part++;
	vol = 0;
	if (ap->io->mode != O_RDONLY)
	{
		rw = "write";
		io = "output";
		ap->io->offset += ap->io->count - n;
		ap->io->count = n;
		z = ap->io->offset + ap->io->count;
		if (ap->io->blocked && state.record.file)
			switch (ap->format)
			{
			case ALAR:
			case IBMAR:
				if (locked)
					return;
				locked = 1;
				putlabels(ap, state.record.file, "EOV");
				locked = 0;
				vol = 1;
				break;
			}
	}
	else
	{
		rw = "read";
		io = "input";
		z = ap->io->offset + ap->io->count;
	}
	if (fstat(ap->io->fd, &st) < 0)
		error(ERROR_SYSTEM|3, "%s: cannot stat %s", ap->name, io);
	errno = oerrno;
	switch (X_ITYPE(modex(st.st_mode)))
	{
	case X_IFBLK:
	case X_IFCHR:
		file = 0;
		break;
	default:
		if (ap->io->mode != O_RDONLY)
			switch (c < 0 ? errno : 0)
			{
			case 0:
#ifdef EFBIG
			case EFBIG:
#endif
#ifdef EDQUOT
			case EDQUOT:
#endif
				file = "file";
				break;
			default:
				error(ERROR_SYSTEM|3, "%s: %s %s error -- cannot recover", ap->name, io, rw);
				break;
			}
		else
			file = "file";
		break;
	}
	switch (c < 0 ? errno : 0)
	{
	case 0:
	case ENOSPC:
	case ENXIO:
		error(1, "%s: end of %s medium", ap->name, io);
		break;
	default:
		error(ERROR_SYSTEM|1, "%s: %s %s error", ap->name, io, rw);
		break;
	}
	if (total == z)
		error(1, "%s: no %s on part %d", ap->name, io, ap->part--);
	else
		total = z;
	if (!file && (ap->name == definput || ap->name == defoutput))
	{
		dev.path = 0;
		dev.st = &st;
		ftwalk("/dev", devpath, 0, NiL);
		ap->name = dev.path;
	}
	close(ap->io->fd);
	if (eomprompt && *eomprompt == '!')
		file = 0;
	if (file && ap->name != definput && ap->name != defoutput && strmatch(ap->name, "*.+([0-9])") && (s = strrchr(ap->name, '.')) && (int)strtol(++s, NiL, 10) == ap->part)
	{
		/*
		 * the parts will be ap->name in sequence
		 * the first part realloc the name with
		 * enough sequence space
		 */

		if (ap->part == 1)
		{
			c = s - ap->name;
			if (!(t = newof(0, char, s - ap->name, 16)))
				error(ERROR_SYSTEM|3, "out of space");
			strcpy(t, ap->name);
			s = (ap->name = t) + c;
		}
		sfsprintf(s, 16, "%d", ap->part + 1);
		if ((ap->io->fd = open(ap->name, ap->io->mode|O_BINARY, st.st_mode & (S_IRWXU|S_IRWXG|S_IRWXO))) >= 0)
			goto nextpart;
		error(ERROR_SYSTEM|1, "%s: cannot %s", ap->name, rw);
	}
	if (state.test & 040)
		file = 0;
	else if (file || ap->name == definput || ap->name == defoutput)
	{
		for (;;)
		{
			interactive();
			sfputc(state.wtty, CC_bel);
			sfprintf(state.wtty, "Enter part %d %s %s name: ", ap->part + 1, io, file ? file : "device");
			if (!(s = sfgetr(state.rtty, '\n', 1)))
			{
				sfputc(state.wtty, '\n');
				finish(2);
			}
			if (*s)
			{
				if (!file)
					break;
				if ((ap->io->fd = open(s, ap->io->mode|O_BINARY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) >= 0)
					break;
				error(ERROR_SYSTEM|1, "%s: cannot open", s);
			}
		}
		ap->name = strdup(s);
	}
	if (!file)
	{
		for (;;)
		{
			if (eomprompt && *eomprompt == '!')
			{

				if (!cp && !(cp = sfstropen()))
					error(ERROR_SYSTEM|3, "out of space [eom]");
				sfprintf(cp, "%s %s %d", eomprompt + 1, rw, ap->part + 1);
				if (ap->name)
					sfprintf(cp, " %s", ap->name);
				s = (pp = sfpopen(pp, sfstruse(cp), "r")) ? sfgetr(pp, '\n', 1) : (char*)0;
			}
			else
			{
				interactive();
				sfputc(state.wtty, CC_bel);
				if (eomprompt)
					sfprintf(state.wtty, eomprompt, ap->part + 1);
				if (!(s = sfgetr(state.rtty, '\n', 1)))
					sfputc(state.wtty, '\n');
			}
			if (!s)
				finish(2);
			if (*s == '!')
			{
				static char*	last;

				if (*++s)
				{
					if (last)
						free(last);
					last = strdup(s);
				}
				else
					s = last;
				if (!s)
					error(1, "no previous command");
				else if (n = system(s))
					error(1, "exit status %d", n);
			}
			else if (file = *s ? s : ap->name)
			{
				if ((ap->io->fd = open(file, ap->io->mode|O_BINARY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) >= 0)
					break;
				if (!strchr(file, '/'))
				{
					oerrno = errno;
					file = strtape(file, &t);
					if (!*t && (ap->io->fd = open(file, ap->io->mode|O_BINARY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) >= 0)
						break;
					errno = oerrno;
				}
				error(ERROR_SYSTEM|1, "cannot %s %s", rw, *s ? s : ap->name);
			}
			else
				error(1, "pathname required");
		}
		if (ap->name != file)
			ap->name = strdup(file);
		if (cp)
			sfclose(cp);
		if (pp)
			sfclose(pp);
	nextpart:
		ap->part++;
		error(1, "continuing %s %d %s on %s", ap->part == ap->volume + 1 ? "volume" : "part", ap->part, io, ap->name);
	}
	else
		ap->part++;
	if (vol && !locked)
	{
		locked = 1;
		putprologue(ap);
		putlabels(ap, state.record.file, "HDR");
		locked = 0;
	}
}
