/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1987-2003 AT&T Corp.                *
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
 * AT&T Bell Laboratories
 *
 * pax file copy support
 */

#include "pax.h"

/*
 * copy files in from archive
 */

void
copyin(register Archive_t* ap)
{
	register File_t*	f = &ap->file;

	deltabase(ap);
	while (getprologue(ap))
	{
		while (getheader(ap, f))
		{
			if (selectfile(ap, f)) filein(ap, f);
			else fileskip(ap, f);
			gettrailer(ap, f);
		}
		getepilogue(ap);
	}
	deltaverify(ap);
}

/*
 * copy a single file out to the archive
 * called by ftwalk()
 */

int
copyout(register Ftw_t* ftw)
{
	register Archive_t*	ap = state.out;
	register File_t*	f = &ap->file;

	if (getfile(ap, f, ftw))
	{
		if (selectfile(ap, f) && (!state.verify || verify(ap, f, NiL)))
		{
			f->fd = openin(ap, f);
			deltaout(NiL, ap, f);
		}
		else
			ftw->status = FTW_SKIP;
	}
	return 0;
}

/*
 * fileout() record support
 */

static void
recordout(Archive_t* ap, File_t* f, Sfio_t* fp)
{
	register int	c;
	register char*	p;
	register char*	recdat;
	register char*	blkdat;
	char*		rec;
	char*		blk;
	int		span;

	int		count = 0;
	int		partial = 0;
	int		truncated = 0;

	static char	span_out[] = "0132";

	if (!fp) error(3, "cannot handle record output from buffer");
	ap->record = f;
	f->record.blocks = 0;
	span = 0;
	blk = state.tmp.buffer;

	/*
	 * file loop
	 */

	for (;;)
	{
		p = blk;
		switch (state.record.format)
		{
		case 'V':
			p += 4;
			break;
		}
		blkdat = p;

		/*
		 * block loop
		 */

		for (;;)
		{
			rec = p;
			switch (state.record.format)
			{
			case 'D':
			case 'V':
				p += 4;
				break;
			case 'S':
				p += 5;
				break;
			}
			recdat = p;

			/*
			 * check for partial record from previous block
			 */

			if (partial)
			{
				memcpy(recdat, f->record.partial, partial);
				p += partial;
				partial = 0;
			}

			/*
			 * record loop
			 */

			span &= 01;
			span <<= 1;
			for (;;)
			{
				if (p >= &rec[state.record.size] && state.record.size)
				{
					if (state.record.line)
					{
						truncated++;
						while ((c = sfgetc(fp)) != EOF && c != '\n');
					}
					break;
				}
				else if (p >= &blk[state.blocksize])
				{
					if (state.record.format == 'S' || state.record.format == 'V')
					{
						if (p > recdat)
						{
							span |= 01;
							break;
						}
					}
					else if (partial = p - recdat)
					{
						/*
						 * save partial record for next block
						 */

						if (!f->record.partial && !(f->record.partial = newof(0, char, state.blocksize, 0)))
							nospace();
						memcpy(f->record.partial, recdat, partial);
					}
					p = rec;
					goto eob;
				}
				else if ((c = sfgetc(fp)) == EOF)
				{
					if (p == recdat)
					{
						if (rec == blkdat) goto eof;
						p = rec;
						goto eob;
					}
					break;
				}
				else if (c == '\n' && state.record.line) break;
				else *p++ = c;
			}
			switch (state.record.format)
			{
			case 'D':
				c = recdat[0];
				sfsprintf(rec, 4, "%04d", p - rec);
				recdat[0] = c;
				break;
			case 'F':
				if (c != EOF || state.record.pad)
				{
					memset(p, ' ', state.record.size - (p - rec));
					p = rec + state.record.size;
				}
				break;
			case 'S':
				c = recdat[0];
				sfsprintf(rec, 4, "%c%04d", span_out[span], p - rec);
				recdat[0] = c;
				break;
			case 'U':
				if (p == recdat) *p++ = ' ';
				break;
			case 'V':
				rec[0] = ((p - rec) >> 8) & 0xff;
				rec[1] = (p - rec) & 0xff;
				rec[2] = span;
				rec[3] = 0;
				break;
			}
			if (state.record.charset && ap->format == IBMAR)
				ccmapstr(state.map.n2e, recdat, p - recdat);
			count++;
			if (p >= &blk[state.blocksize] || state.record.format == 'U')
				break;
		}
	eob:
		switch (state.record.format)
		{
		case 'D':
		case 'S':
			if (state.record.pad)
			{
				memset(p, '^', state.blocksize - (p - blk));
				p = blk + state.blocksize;
			}
			break;
		case 'V':
			blk[0] = ((p - blk) >> 8) & 0xff;
			blk[1] = (p - blk) & 0xff;
			blk[2] = 0;
			blk[3] = 0;
			break;
		}
		bwrite(ap, blk, p - blk);
		f->record.blocks++;
	}
 eof:
	ap->record = 0;
	if (truncated)
		error(1, "%s: %d out of %d record%s truncated", f->name, truncated, count, count == 1 ? "" : "s");
}

/*
 * low level for copyout()
 * if rfd<0 && st_size>0 then input from bread()
 */

void
fileout(register Archive_t* ap, register File_t* f)
{
	register size_t		m;
	register ssize_t	n;
	register off_t		c;
	int			err;
	Buffer_t*		bp;
	Sfio_t*			rfp;

	if (f->delta.op == DELTA_verify)
	{
		ap->selected--;
		if (f->fd >= 0)
			close(f->fd);
	}
	else
	{
		putheader(ap, f);
		switch (ap->format)
		{
		case ALAR:
		case IBMAR:
			if (ap->io->blocked)
			{
				if (f->st->st_size > 0)
				{
					if (state.record.format == 'F' && !state.record.line)
					{
						/*
						 * this is faster than recordout()
						 */

						ap->record = f;
						err = 0;
						c = f->st->st_size;
						while (c > 0)
						{
							n = m = c > state.record.size ? state.record.size : c;

							/*
							 * NOTE: we expect that all but the last
							 *	 read returns state.record.size
							 *	 if not the the intermediate short
							 *	 reads are filled with 0's
							 */

							if (!err)
							{
								if (f->fd >= 0)
									n = read(f->fd, state.tmp.buffer, m);
								else if (bp = getbuffer(f->fd))
								{
									memcpy(ap->io->next, bp->next, m);
									bp->next += m;
								}
								else if (bread(f->ap, state.tmp.buffer, (off_t)0, (off_t)m, 1) <= 0)
									n = -1;
							}
							if (n <= 0)
							{
								if (n)
									error(ERROR_SYSTEM|2, "%s: read error", f->path);
								else
									error(2, "%s: file size changed", f->path);
								memzero(state.tmp.buffer, state.record.size);
								err = 1;
							}
							else
							{
								c -= n;
								if (n < state.record.size && (c > 0 || state.record.pad))
								{
									memzero(state.tmp.buffer + n, state.record.size - n);
									n = state.record.size;
								}
								bwrite(ap, state.tmp.buffer, n);
							}
						}
						ap->record = 0;
						if (f->fd >= 0)
							close(f->fd);
					}
					else if (f->fd < 0)
						recordout(ap, f, NiL);
					else if (!(rfp = sfnew(NiL, NiL, SF_UNBOUND, f->fd, SF_READ)))
					{
						error(1, "%s: cannot read", f->path);
						close(f->fd);
					}
					else
					{
						recordout(ap, f, rfp);
						sfclose(rfp);
					}
				}
				break;
			}
			/*FALLTHROUGH*/
		default:
			err = 0;
			c = f->st->st_size;
			while (c > 0)
			{
				n = m = c > state.buffersize ? state.buffersize : c;
				if (!err)
				{
					if (f->fd >= 0)
					{
						if ((n = read(f->fd, ap->io->next, m)) < 0 && errno == EIO)
						{
							static char*	buf;

							if (!buf)
							{
								n = 1024 * 8;
								error(1, "EIO read error -- falling back to aligned reads");
								if (!(buf = malloc(state.buffersize + n)))
									nospace();
								buf += n - (((ssize_t)buf) & (n - 1));
							}
							if ((n = read(f->fd, buf, m)) > 0)
								memcpy(ap->io->next, buf, n);
						}
					}
					else if (bp = getbuffer(f->fd))
					{
						memcpy(ap->io->next, bp->next, m);
						bp->next += m;
					}
					else if (bread(f->ap, ap->io->next, (off_t)0, (off_t)n, 1) <= 0)
						n = -1;
				}
				if (n <= 0)
				{
					if (n)
						error(ERROR_SYSTEM|2, "%s: read error", f->path);
					else
						error(2, "%s: file size changed", f->path);
					memzero(ap->io->next, state.buffersize);
					err = 1;
				}
				else
				{
					c -= n;
					bput(ap, n);
				}
			}
			if (f->fd >= 0)
				close(f->fd);
			break;
		}
		puttrailer(ap, f);
	}
	if (state.acctime && f->type != X_IFLNK && !f->skip && !f->extended)
	{
		Tv_t	av;
		Tv_t	mv;

		tvgetstat(f->st, &av, &mv, NiL);
		settime(f->name, &av, &mv, NiL);
	}
}

/*
 * filein() record support
 */

static void
recordin(register Archive_t* ap, register File_t* f, int wfd)
{
	register off_t		n;
	register off_t		size;
	int			c;
	int			i;
	int			j;
	int			k;
	int			nl;
	off_t			m;
	Sfio_t*			wfp;

	if (wfd < 0) wfp = 0;
	else if (!(wfp = sfnew(NiL, NiL, SF_UNBOUND, wfd, SF_WRITE)))
		error(1, "%s: cannot write", f->name);
	ap->io->empty = 0;
	nl = state.record.line;
	size = 0;
	for (;;)
	{
		if (ap->io->blocked) n = bread(ap, state.tmp.buffer, (off_t)0, (off_t)state.buffersize, 0);
		else if ((m = f->st->st_size - size) <= 0) n = 0;
		else if (wfp) 
		{
			if (m > state.buffersize) m = state.buffersize;
			n = bread(ap, state.tmp.buffer, (off_t)0, m, 1);
		}
		else n = bread(ap, NiL, (off_t)0, m, 1);
		if (n < 0) break;
		if (n == 0)
		{
			k = 1;
			ap->sum--;
			while (getlabel(ap, f))
			{
				if (strneq(alar_header, "EOV1", 4)) k = 0;
				else if (!strneq(alar_header, "EOF", 3) && !strneq(alar_header, "EOV", 3) && !strneq(alar_header, "UTL", 3) && ++n >= 16 && !state.keepgoing)
					error(3, "%s: %s: %d invalid %s end of file/volume labels detected", ap->name, f->name, n, format[ap->format].name);
			}
			if (n) error(1, "%s: %s: %d invalid %s end of file/volume labels detected", ap->name, f->name, n, format[ap->format].name);
			if (k)
			{
				ap->sum++;
				break;
			}
			f->record.section++;
			f->id = strcpy(state.tmp.buffer, f->id);
			f->name = strcpy(state.tmp.buffer + ALAR_NAMESIZE + 1, f->name);
			for (;;)
			{
				newio(ap, 0, 0);
				if (getprologue(ap))
				{
					File_t		v;
					struct stat	st;

					v.st = &st;
					if (getheader(ap, &v))
					{
						if (streq(f->id, v.id) && streq(f->name, v.name) && f->record.section == v.record.section)
						{
							f->id = v.id;
							f->name = v.name;
							break;
						}
						error(1, "volume containing %s id %s section %d required", f->name, f->id, f->record.section);
					}
					ap->volume--;
				}
				ap->part--;
			}
			ap->sum++;
			continue;
		}
		if (f->record.format == 'V')
		{
			if ((k = ((unsigned char*)state.tmp.buffer)[0] << 8 | ((unsigned char*)state.tmp.buffer)[1]) != n)
				error(3, "%s: invalid %s V format block descriptor [%d!=%d]", f->name, format[ap->format].name, k, n);
			i = 4;
		}
		else i = 0;
		while (i < n)
		{
			i += state.record.offset;
			if (state.tmp.buffer[i] == '^') switch (f->record.format)
			{
			case 'F':
				if (ap->format == IBMAR) break;
				for (j = i; j < n && state.tmp.buffer[j] == '^'; j++);
				if (j < n) break;
				/*FALLTHROUGH*/
			case 'D':
			case 'S':
				i = n;
				continue;
			}

			/*
			 * get record size
			 */

			switch (f->record.format)
			{
			case 'D':
				if (sfsscanf(&state.tmp.buffer[i], "%4d", &k) != 1) k = -1;
				j = i + 4;
				break;
			case 'F':
				if (i + state.record.size > n) k = n - i;
				else if (state.record.line || state.record.offset) k = state.record.size;
				else k = n;
				j = i;
				break;
			case 'S':
				switch (state.tmp.buffer[i])
				{
				case '0':
				case '3':
					nl = 1;
					break;
				default:
					nl = 0;
					break;
				}
				if (sfsscanf(&state.tmp.buffer[i + 1], "%4d", &k) != 1) k = -1;
				j = i + 5;
				break;
			case 'U':
				k = n;
				j = i;
				break;
			case 'V':
				nl = !(state.tmp.buffer[i + 2] & 01);
				k = ((unsigned char*)state.tmp.buffer)[i] << 8 | ((unsigned char*)state.tmp.buffer)[i + 1];
				j = i + 4;
				break;
			}
			if (k < 0)
			{
				error(2, "invalid %s %c record size", format[ap->format].name, f->record.format);
				break;
			}
			m = i += k;
			if (state.record.charset && ap->format == IBMAR)
				ccmapstr(state.map.e2n, &state.tmp.buffer[j], m - j);
			if (state.record.line) switch (f->record.format)
			{
			case 'F':
			case 'U':
				while (--m >= j && state.tmp.buffer[m] == ' ');
				m++;
				break;
			}
			k = m - j + nl;
			size += k;
			if (wfp)
			{
				if (nl)
				{
					c = state.tmp.buffer[m];
					state.tmp.buffer[m] = '\n';
				}
				if (sfwrite(wfp, &state.tmp.buffer[j], k) != k)
				{
					error(ERROR_SYSTEM|1, "%s: write error", f->name);
					break;
				}
				if (nl) state.tmp.buffer[m] = c;
			}
		}
	}
	if (f->st->st_size && f->st->st_size != size)
		error(1, "%s: header size %I*d does not match data size %I*d", f->name, sizeof(f->st->st_size), f->st->st_size, sizeof(size), size);
	f->st->st_size = size;
	if (wfp)
	{
		sfclose(wfp);
		setfile(ap, f);
	}
	if (n < 0) error(ERROR_SYSTEM|3, "%s: %s: archive read error", ap->name, f->name);
}

#if SAVESET

/*
 * filein() saveset support
 */

static void
savesetin(register Archive_t* ap, register File_t* f, int wfd)
{
	register off_t		c;
	int			i;
	int			j;
	int			k;
	Sfio_t*			wfp;

	if (wfd < 0) wfp = 0;
	else if (!(wfp = sfnew(NiL, NiL, SF_UNBOUND, wfd, SF_WRITE)))
		error(1, "%s: cannot write", f->name);
	j = 0;
	k = 0;
	c = 0;
	while (getsaveset(ap, f, 0))
	{
		/*
		 * this part transcribed from vmsbackup
		 */

		i = 0;
		if (wfp) while ((c + i) < f->st->st_size && i < state.saveset.lastsize) switch (state.saveset.recfmt)
		{
		case 1:	/* fixed length		*/
			if (j <= 0) j = state.saveset.reclen;
			sfputc(wfp, state.saveset.bp[i]);
			i++;
			j--;
			break;
		case 2:	/* variable length	*/
		case 3:	/* with fixed control	*/
			if (j <= 0)
			{
				j = k = swapget(1, &state.saveset.bp[i], 2);
				i += 2;
				if (state.saveset.recfmt == 3)
				{
					i += state.saveset.recvfc;
					j -= state.saveset.recvfc;
				}
			}
			else
			{
				if (j == k && state.saveset.recatt == 1)
				{
					if (state.saveset.bp[i] == '0') state.saveset.bp[i] = '\n';
					else if (state.saveset.bp[i] == '1') state.saveset.bp[i] = '\f';
				}
				sfputc(wfp, state.saveset.bp[i]);
				i++;
				j--;
			}
			if (j <= 0)
			{
				sfputc(wfp, '\n');
				if (i & 1) i++;
			}
			break;
		case 4:	/* seq stream		*/
		case 5:	/* seq LF stream	*/
			if (j <= 0) j = 512;
			if (state.saveset.bp[i] == '\n') j = 0;
			else j--;
			sfputc(wfp, state.saveset.bp[i]);
			i++;
			break;
		case 6:	/* seq CR stream	*/
			if (state.saveset.bp[i] == '\r') state.saveset.bp[i] = '\n';
			sfputc(wfp, state.saveset.bp[i]);
			i++;
			break;
		default:
			error(state.keepgoing ? 1 : 3, "%s: invalid %s format data record format=%d", f->name, format[ap->format].name, state.saveset.recfmt);
			goto next;
		}
	next:
		c += i;
	}
	if (wfp)
	{
		sfclose(wfp);
		setfile(ap, f);
	}
}

#endif

/*
 * low level for copyin()
 */

void
filein(register Archive_t* ap, register File_t* f)
{
	register off_t	c;
	register int	n;
	register char*	s;
	int		dfd;
	int		wfd;
	long		checksum;
	struct stat	st;

	if (f->skip) goto skip;
	else if (state.list)
	{
		listentry(f);
		goto skip;
	}
	else switch (f->delta.op)
	{
	case DELTA_create:
		if (f->delta.base)
			error(3, "%s: base archive mismatch [%s#%d]", f->name, __FILE__, __LINE__);
		if (ap->delta->format != COMPRESS && ap->delta->format != DELTA) goto regular;
		if ((wfd = openout(ap, f)) < 0) goto skip;
		else paxdelta(NiL, ap, f, DELTA_TAR|DELTA_FD|DELTA_FREE|DELTA_OUTPUT|DELTA_COUNT, wfd, DELTA_DEL|DELTA_BIO|DELTA_SIZE, ap, f->st->st_size, 0);
		break;
	case DELTA_update:
		if (!f->delta.base || (unsigned long)f->delta.base->mtime.tv_sec >= (unsigned long)f->st->st_mtime)
			error(3, "%s: base archive mismatch [%s#%d]", f->name, __FILE__, __LINE__);
		c = f->st->st_size;
		if ((wfd = openout(ap, f)) < 0)
			goto skip;
		if (state.ordered)
		{
			if (f->delta.base->expand < 0)
				paxdelta(NiL, ap, f, DELTA_SRC|DELTA_BIO|DELTA_SIZE, ap->delta->base, f->delta.base->size, DELTA_TAR|DELTA_FD|DELTA_FREE|DELTA_OUTPUT|DELTA_COUNT, wfd, DELTA_DEL|DELTA_BIO|DELTA_SIZE, ap, c, 0);
			else if (!paxdelta(NiL, ap, f, DELTA_DEL|DELTA_BIO|DELTA_SIZE, ap->delta->base, f->delta.base->size, DELTA_TAR|DELTA_TEMP|DELTA_OUTPUT, &dfd, 0))
				paxdelta(NiL, ap, f, DELTA_SRC|DELTA_FD|DELTA_SIZE|DELTA_FREE, dfd, f->delta.base->expand, DELTA_TAR|DELTA_FD|DELTA_FREE|DELTA_OUTPUT|DELTA_COUNT, wfd, DELTA_DEL|DELTA_BIO|DELTA_SIZE, ap, c, 0);
		}
		else if (f->delta.base->expand < 0)
			paxdelta(NiL, ap, f, DELTA_SRC|DELTA_FD|DELTA_OFFSET|DELTA_SIZE, ap->delta->base->io->fd, f->delta.base->offset, f->delta.base->size, DELTA_TAR|DELTA_FD|DELTA_FREE|DELTA_OUTPUT|DELTA_COUNT, wfd, DELTA_DEL|DELTA_BIO|DELTA_SIZE, ap, c, 0);
		else if (!paxdelta(NiL, ap, f, DELTA_DEL|DELTA_FD|DELTA_OFFSET|DELTA_SIZE, ap->delta->base->io->fd, f->delta.base->offset, f->delta.base->size, DELTA_TAR|DELTA_TEMP|DELTA_OUTPUT, &dfd, 0))
			paxdelta(NiL, ap, f, DELTA_SRC|DELTA_FD|DELTA_SIZE|DELTA_FREE, dfd, f->delta.base->expand, DELTA_TAR|DELTA_FD|DELTA_FREE|DELTA_OUTPUT|DELTA_COUNT, wfd, DELTA_DEL|DELTA_BIO|DELTA_SIZE, ap, c, 0);
		break;
	case DELTA_verify:
		if (!f->delta.base || f->delta.base->mtime.tv_sec != f->st->st_mtime)
			error(3, "%s: base archive mismatch [%s#%d]", f->name, __FILE__, __LINE__);
		if ((*state.statf)(f->name, &st))
			error(2, "%s: not copied from base archive", f->name);
		else if (st.st_size != f->delta.base->size || state.modtime && st.st_mtime != f->st->st_mtime)
			error(1, "%s: changed from base archive", f->name);
		break;
	case DELTA_zip:
		if ((wfd = openout(ap, f)) >= 0)
		{
			Proc_t*		proc;
			long		ops[2];
			char*		cmd[3];

			cmd[0] = format[GZIP].undo[0];
			cmd[1] = format[GZIP].undo[1];
			cmd[2] = 0;
			ops[0] = PROC_FD_DUP(wfd, 1, PROC_FD_PARENT|PROC_FD_CHILD);
			ops[1] = 0;
			if (!(proc = procopen(*cmd, cmd, NiL, ops, PROC_WRITE)))
				error(3, "%s: cannot execute %s filter", f->name, format[GZIP].undo[0]);
			for (c = f->st->st_size; c > 0; c -= n)
			{
				n = (c > state.buffersize) ? state.buffersize : c;
				if (!(s = bget(ap, n, NiL)))
				{
					error(ERROR_SYSTEM|2, "%s: read error", f->name);
					break;
				}
				if (write(proc->wfd, s, n) != n)
				{
					error(ERROR_SYSTEM|2, "%s: write error", f->name);
					break;
				}
			}

			/*
			 * some versions of gunzip look past EOF and bomb
			 * if nothing's there; the extra bytes should be
			 * benign for other implementations
			 */

			if (write(proc->wfd, "\0\0\0", 4) != 4)
				error(ERROR_SYSTEM|2, "%s: pad write error", f->name);
			if (n = procclose(proc))
				error(2, "%s: %s filter exit status %d", f->name, format[GZIP].undo[0], n);
			setfile(ap, f);
		}
		else goto skip;
		break;
	case DELTA_delete:
		if (!f->delta.base)
			error(3, "%s: base archive mismatch [%s#%d]", f->name, __FILE__, __LINE__);
		/*FALLTHROUGH*/
	default:
	regular:
		wfd = openout(ap, f);
		switch (ap->format)
		{
		case ALAR:
		case IBMAR:
			recordin(ap, f, wfd);
			break;
		case AR:
			if (ap->ardirent->offset < 0)
				error(3, "%s: read not supported for %s format", f->name, format[ap->format].name);
			if (ardircopy(ap->ardir, ap->ardirent, wfd) < 0)
				error(ERROR_SYSTEM|2, "%s: copy error", f->name);
			closeout(ap, f, wfd);
			setfile(ap, f);
			break;
		case CAB:
			{
				register Cab_t*		cab = (Cab_t*)ap->data;
				register Cabfile_t*	file;
				register Cabchunk_t*	chunk;

				file = cab->file + cab->index - 1;
				chunk = cab->chunk + file->entry.chunk;
				switch (CAB_TYPE(chunk->compress))
				{
				case CAB_TYPE_NONE:
					error(3, "%s: %s: %s format NONE read not implemented", ap->name, f->name, format[ap->format].name);
				case CAB_TYPE_MSZIP:
					error(3, "%s: %s: %s format MSZIP read not implemented", ap->name, f->name, format[ap->format].name);
				case CAB_TYPE_QUANTUM:
					error(3, "%s: %s: %s format QUANTUM read not implemented", ap->name, f->name, format[ap->format].name);
				case CAB_TYPE_LZX:
					error(3, "%s: %s: %s format LZX read not implemented", ap->name, f->name, format[ap->format].name);
				default:
					error(3, "%s: %s: %s format COMPRESS=%d read not implemented", ap->name, f->name, CAB_TYPE(chunk->compress), format[ap->format].name);
				}
			}
			break;
#if SAVESET
		case SAVESET:
			savesetin(ap, f, wfd);
			break;
#endif
		default:
			if (wfd >= 0)
			{
				checksum = 0;
				holeinit(wfd);
				for (c = f->st->st_size; c > 0; c -= n)
				{
					n = (c > state.buffersize) ? state.buffersize : c;
					if (!(s = bget(ap, n, NiL)))
					{
						error(ERROR_SYSTEM|2, "%s: read error", f->name);
						break;
					}
					if (holewrite(wfd, s, n) != n)
					{
						error(ERROR_SYSTEM|2, "%s: write error", f->name);
						break;
					}
					if (ap->format == ASCHK) checksum = asc_checksum(s, n, checksum);
				}
				holedone(wfd);
				closeout(ap, f, wfd);
				setfile(ap, f);
				if (ap->format == ASCHK && checksum != f->checksum)
					error(1, "%s: %s checksum error (0x%08x != 0x%08x)", f->name, format[ap->format].name, checksum, f->checksum);
			}
			else goto skip;
			break;
		}
	}
	listentry(f);
	return;
 skip:
	fileskip(ap, f);
}

/*
 * skip over archive member f file data
 */

void
fileskip(register Archive_t* ap, register File_t* f)
{
	Member_t*	d;
	off_t		n;

	if (ap->delta && (d = (Member_t*)hashget(ap->delta->tab, f->name)))
		d->info->delta.op = DELTA_delete;
	switch (ap->format)
	{
	case ALAR:
	case IBMAR:
		recordin(ap, f, -1);
		break;
	case CAB:
		break;
#if SAVESET
	case SAVESET:
		savesetin(ap, f, -1);
		break;
#endif
	default:
		if ((n = f->st->st_size) > 0 && (f->type == X_IFREG || (n = f->datasize)) && bread(ap, NiL, (off_t)0, n, 1) < 0)
			error(ERROR_SYSTEM|2, "%s: skip error", f->name);
		break;
	}
}

/*
 * single file copyin() and copyout() smashed together
 * called by ftwalk()
 */

int
copyinout(Ftw_t* ftw)
{
	register File_t*	f = &state.out->file;
	register char*		s;
	register off_t		c;
	register ssize_t	n;
	register int		rfd;
	register int		wfd;

	if (getfile(state.out, f, ftw) && selectfile(state.out, f))
	{
		s = f->name;
		f->name = stash(&state.out->path.copy, NiL, state.pwdlen + f->namesize);
		strcpy(strcopy(f->name, state.pwd), s + (*s == '/'));
		if ((wfd = openout(state.out, f)) >= 0)
		{
			if ((rfd = openin(state.out, f)) >= 0)
			{
				holeinit(wfd);
				for (c = f->st->st_size; c > 0; c -= n)
				{
					if ((n = read(rfd, state.tmp.buffer, (size_t)((c > state.buffersize) ? state.buffersize : c))) <= 0)
					{
						error(ERROR_SYSTEM|2, "%s: read error", f->name);
						break;
					}
					if (holewrite(wfd, state.tmp.buffer, n) != n)
					{
						error(ERROR_SYSTEM|2, "%s: write error", f->name);
						break;
					}
					state.out->io->count += n;
				}
				holedone(wfd);
				closeout(state.out, f, wfd);
				close(rfd);
				setfile(state.out, f);
				listentry(f);
			}
			else
				closeout(state.out, f, wfd);
		}
		else if (wfd != -1)
			listentry(f);
	}
	return 0;
}

/*
 * compare ft1 and ft2 for ftwalk() sort
 */

int
cmpftw(Ftw_t* ft1, Ftw_t* ft2)
{
	return strcoll(ft1->name, ft2->name);
}

/*
 * skip to the next unquoted occurrence of d in s
 */

static char*
skip(register char* s, register int d)
{
	register int	c;
	register int	q;

	q = 0;
	while (c = *s++)
		if (c == q)
			q = 0;
		else if (c == '\\')
		{
			if (*s)
				s++;
		}
		else if (!q)
		{
			if (c == d)
				return s - 1;
			else if (c == '"' || c == '\'')
				q = c;
		}
	return 0;
}

/*
 * copy files out using copyfile
 */

typedef int (*Ftw_cmp_t)(Ftw_t*, Ftw_t*);

void
copy(register Archive_t* ap, register int (*copyfile)(Ftw_t*))
{
	register char*	s;
	register char*	t;
	register char*	v;
	register int	c;
	unsigned long	flags;

	if (ap)
	{
		deltabase(ap);
		putprologue(ap);
	}
	if (state.files)
		ftwalk((char*)state.files, copyfile, state.ftwflags|FTW_MULTIPLE, state.exact ? (Ftw_cmp_t)0 : cmpftw);
	else
	{
		sfopen(sfstdin, NiL, "rt");
		sfset(sfstdin, SF_SHARE, 0);
		for (;;)
		{
			if (s = state.peekfile)
				state.peekfile = 0;
			else if (!(s = sfgetr(sfstdin, '\n', 1)))
				break;
			sfprintf(state.tmp.lst, "%s", s);
			s = sfstruse(state.tmp.lst);
			flags = state.ftwflags;
			if (state.filter.line)
			{
				if (!(c = *s++))
					continue;
				state.filter.options = s;
				if (!(s = skip(s, c)))
					continue;
				*s++ = 0;
				state.filter.command = s;
				if (!(s = skip(s, c)))
					continue;
				*s++ = 0;
				state.filter.path = s;
				if (!(s = skip(s, c)))
					state.filter.name = state.filter.path;
				else
				{
					*s++ = 0;
					state.filter.name = s;
					if (s = skip(s, c))
						*s = 0;
				}
				s = state.filter.options;
				for (;;)
				{
					if (t = strchr(s, ','))
						*t = 0;
					if (v = strchr(s, '='))
					{
						*v = 0;
						c = strtol(v + 1, NiL, 0);
					}
					else
						c = 1;
					if (s[0] == 'n' && s[1] == 'o')
					{
						s += 2;
						c = !c;
					}
					if (streq(s, "logical") || streq(s, "physical"))
					{
						if (s[0] == 'p')
							c = !c;
						if (c)
							flags &= ~(FTW_META|FTW_PHYSICAL);
						else
						{
							flags &= ~(FTW_META);
							flags |= FTW_PHYSICAL;
						}
					}
					if (v)
						*v = '=';
					if (!t)
						break;
					*t++ = ',';
					s = t;
				}
				s = state.filter.path;
				state.filter.line = *state.filter.name ? 2 : 1;
			}
			if (*s && ftwalk(s, copyfile, flags, NiL))
			{
				error(2, "%s: not completely copied", s);
				break;
			}
		}
	}
	if (ap)
	{
		deltadelete(ap);
		putepilogue(ap);
	}
}

/*
 * position archive for appending
 */

void
append(register Archive_t* ap)
{
	if (state.update) initdelta(ap);
	ap->format = IN_DEFAULT;
	copyin(ap);
	state.append = 0;
}
