/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1987-2000 AT&T Corp.              *
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
 * Glenn Fowler
 * AT&T Bell Laboratories
 *
 * pax archive format support
 */

#include "pax.h"
#include "options.h"

#include <tm.h>

#if __hppa__ || __hppa || hppa

/*
 * the inline macros are apparently too much for the hp optimizer
 * (the calls in putprologue() make the difference)
 */

static int
delta_lo(long x)
{
	return DELTA_LO(x);
}

#undef	DELTA_LO
#define DELTA_LO	delta_lo

static int
delta_hi(long x)
{
	return DELTA_HI(x);
}

#undef	DELTA_HI
#define DELTA_HI	delta_hi

#endif

/*
 * check for ASCII or EBCDIC ALAR prologue in alar_header
 */

static int
isalar(Archive_t* ap, register char* hdr)
{
	char		buf[4];

	if (ap->expected >= 0 && ap->expected != ALAR && ap->expected != IBMAR)
		return 0;
	memcpy(buf, hdr, 4);
	ccmaps(buf, 4, CC_ASCII, CC_NATIVE);
	if ((ap->expected < 0 || ap->expected == ALAR) && strneq(hdr, "VOL1", 4))
	{
		ap->format = ALAR;
		ccmaps(hdr, ALAR_HEADER, CC_ASCII, CC_NATIVE);
		convert(SECTION_CONTROL, CC_NATIVE, CC_ASCII);
		if (!state.convert[0].on)
			convert(SECTION_DATA, CC_NATIVE, CC_ASCII);
	}
	else if (ap->expected < 0 || ap->expected == IBMAR)
	{
		memcpy(buf, hdr, 4);
		ccmaps(buf, 4, CC_EBCDIC1, CC_NATIVE);
		if (!strneq(buf, "VOL1", 4))
			return 0;
		ccmaps(hdr, ALAR_HEADER, CC_EBCDIC1, CC_NATIVE);
		ap->format = IBMAR;
		convert(SECTION_CONTROL, CC_NATIVE, CC_EBCDIC1);
		if (!state.convert[0].on)
			convert(SECTION_DATA, CC_NATIVE, CC_EBCDIC1);
	}
	getlabstr(hdr, 5, 6, state.id.volume);
	getlabstr(hdr, 25, 6, state.id.format);
	getlabstr(hdr, 31, 7, state.id.implementation);
	getlabstr(hdr, 38, 14, state.id.owner);
	ap->io.blocked = !bcount(ap);
	return 1;
}

/*
 * verify that compress undo command exists
 * alternate undotoo checked if undo not found
 */

static void
undoable(Archive_t* ap, Format_t* fp)
{
	if (!pathpath(tar_block, fp->undo[0], NiL, PATH_EXECUTE))
	{
		if (!fp->undotoo[0] || !pathpath(tar_block, fp->undotoo[0], NiL, PATH_EXECUTE))
			error(3, "%s: %s: command required to read compressed archive", ap->name, fp->undo[0]);
		fp->undo[0] = fp->undotoo[0];
		fp->undotoo[0] = 0;
		fp->undo[1] = fp->undotoo[1];
	}
}

/*
 * read archive prologue before files are copied
 */

int
getprologue(register Archive_t* ap)
{
	int		n;
	off_t		size;

	if (ap->volume && ap->io.mode != O_RDONLY)
		return 0;
	state.id.volume[0] = 0;
	ap->format = -1;
	ap->io.offset += ap->io.count;
	ap->io.count = 0;
	ap->section = SECTION_CONTROL;
	convert(SECTION_CONTROL, CC_NATIVE, CC_NATIVE);
	if (bread(ap, alar_header, (off_t)ALAR_HEADER, (off_t)ALAR_HEADER, 0) <= 0)
	{
		if (!bcount(ap))
			return 0;
	}
	else if (!isalar(ap, alar_header))
	{
		if ((ap->expected < 0 || ap->expected == PORTAR || ap->expected == RANDAR) && strneq(alar_header, PORTAR_MAG, PORTAR_MAGSIZ))
		{
			bunread(ap, alar_header, ALAR_HEADER - 8);
			if (bread(ap, &portar_header, (off_t)0, (off_t)PORTAR_HEADER, 0) > 0)
			{
				if (!strneq(portar_header.ar_fmag, PORTAR_END, PORTAR_ENDSIZ) || sfsscanf(portar_header.ar_size, "%I*d", sizeof(size), &size) != 1)
				{
					bunread(ap, &portar_header, PORTAR_HEADER);
					bunread(ap, PORTAR_MAG, PORTAR_MAGSIZ);
				}
				else
				{
					size += (size & 01);
					if (strmatch(portar_header.ar_name, PORTAR_SYM) && (ap->format = PORTAR) || strmatch(portar_header.ar_name, RANDAR_SYM) && (ap->format = RANDAR))
					{
						if (ap->format == PORTAR && portar_header.ar_uid[0] == ' ' && portar_header.ar_gid[0] == ' ')
							ap->separator = '\\';
						if (bread(ap, NiL, (off_t)0, size, 0) <= 0)
							error(3, "%s: invalid %s format symbol table", ap->name, format[ap->format].name);
						n = bread(ap, &portar_header, (off_t)0, (off_t)PORTAR_HEADER, 0);
					}
					else
					{
						n = portar_header.ar_date[0];
						portar_header.ar_date[0] = 0;
						ap->format = strchr(portar_header.ar_name, PORTAR_TERM) ? PORTAR : RANDAR;
						portar_header.ar_date[0] = n;
						n = 1;
					}
					if (ap->expected >= 0)
						ap->expected = ap->format;
					if (n > 0)
					{
						while (portar_header.ar_name[0] == PORTAR_TERM && sfsscanf(portar_header.ar_size, "%I*d", sizeof(size), &size) == 1)
						{
							size += (size & 01);
							if (!ap->names && portar_header.ar_name[1] == PORTAR_TERM && portar_header.ar_name[2] == ' ')
							{
								if (!(ap->names = newof(0, char, size, 0)))
									error(3, "%s: cannot allocate %s format long name table", ap->name, format[ap->format].name);
								if (bread(ap, ap->names, (off_t)0, size, 0) <= 0)
									error(3, "%s: invalid %s format long name table", ap->name, format[ap->format].name);
							}
							else if (isdigit(portar_header.ar_name[1]))
								break;
							else if (bread(ap, NiL, (off_t)0, size, 0) <= 0)
								error(3, "%s: invalid %s format meta data", ap->name, format[ap->format].name);
							if ((n = bread(ap, &portar_header, (off_t)0, (off_t)PORTAR_HEADER, 0)) <= 0)
								break;
						}
						if (n > 0)
							bunread(ap, &portar_header, PORTAR_HEADER);
					}
				}
			}
		}
		else if (strneq(alar_header, "<ar>", 4) ||	/* s5r1	 */
			swapget(0, alar_header, 2) == 0177545 ||/* pdp11 */
			swapget(1, alar_header, 2) == 0177545)	/* pdp11 */
				error(3, "%s: use ar(1) for library archives", ap->name);
		else if (!ap->compress)
		{
			unsigned long	m;

			m = ((*((unsigned char*)alar_header+0))<<24)|
			    ((*((unsigned char*)alar_header+1))<<16)|
			    ((*((unsigned char*)alar_header+2))<<8)|
			     (*((unsigned char*)alar_header+3));
			for (n = COMPRESS; n < DELTA; n++)
				if (format[n].special == (m & format[n].regular))
				{
					ap->compress = n;
					break;
				}
			bunread(ap, alar_header, ALAR_HEADER);
			if (ap->compress)
			{
				Proc_t*	proc;
				long	ops[3];
				char*	cmd[3];
				off_t	pos;
				char	buf[4];

				if (bseek(ap, (off_t)0, SEEK_SET, 1))
					error(3, "%s: %s input must be seekable", ap->name, format[ap->compress].name);
				undoable(ap, &format[ap->compress]);
				if (state.meter.on && ap == state.in)
					switch (ap->compress)
					{
					case COMPRESS:
						state.meter.size *= 3;
						break;
					case GZIP:
						if ((pos = lseek(ap->io.fd, (off_t)0, SEEK_CUR)) < 0 || lseek(ap->io.fd, (off_t)-4, SEEK_END) <= 0 || read(ap->io.fd, buf, 4) != 4)
							state.meter.size *= 6;
						else
							state.meter.size = ((unsigned char)buf[0]) |
					      				   ((unsigned char)buf[1] << 8) |
					        			   ((unsigned char)buf[2] << 16) |
					        			   ((unsigned char)buf[3] << 24);
						lseek(ap->io.fd, pos, SEEK_SET);
						break;
					case BZIP:
						state.meter.size *= 7;
						break;
					}
				cmd[0] = format[ap->compress].undo[0];
				cmd[1] = format[ap->compress].undo[1];
				cmd[2] = 0;
				ops[0] = PROC_FD_DUP(ap->io.fd, 0, PROC_FD_PARENT|PROC_FD_CHILD);
				if (ap->parent && !state.ordered)
				{
					if ((n = open(state.tmp.file, O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, S_IRUSR)) < 0)
						error(ERROR_SYSTEM|3, "%s: cannot create %s base temporary file %s", ap->name, format[ap->compress].undo[0], state.tmp.file);
					ops[1] = PROC_FD_DUP(n, 1, PROC_FD_PARENT|PROC_FD_CHILD);
					ops[2] = 0;
					proc = procopen(*cmd, cmd, NiL, ops, 0);
				}
				else
				{
					ops[1] = 0;
					proc = procopen(*cmd, cmd, NiL, ops, PROC_READ);
				}
				if (!proc)
					error(3, "%s: cannot execute %s filter", ap->name, format[ap->compress].undo[0]);
				if (ap->parent && !state.ordered)
				{
					if (n = procclose(proc))
						error(3, "%s: %s filter exit code %d", ap->name, format[ap->compress].undo[0], n);
					if ((ap->io.fd = open(state.tmp.file, O_RDONLY|O_BINARY)) < 0)
						error(ERROR_SYSTEM|3, "%s: cannot read %s base temporary file %s", ap->name, format[ap->compress].undo[0], state.tmp.file);
					if (remove(state.tmp.file))
						error(ERROR_SYSTEM|1, "%s: cannot remove %s base temporary file %s", ap->name, format[ap->compress].undo[0], state.tmp.file);
				}
				else
				{
					List_t*	p;

					ap->io.fd = proc->rfd;
					if (!(p = newof(0, List_t, 1, 0)))
						error(3, "out of space [state.proc]");
					p->item = (void*)proc;
					p->next = state.proc;
					state.proc = p;
				}
			}
		}
	}
	if (ap->volume++)
	{
		if (ap->delta)
		{
			if (state.operation == (IN|OUT) || ap->delta->format != DELTA)
				error(3, "%s: %s archive cannot be multi-volume", ap->name, ap->parent ? "base" : "delta");
			ap->delta = 0;
		}

		/*
		 * no hard links between volumes
		 */

		hashfree(state.linktab);
		if (!(state.linktab = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_namesize, sizeof(Fileid_t), HASH_name, "links", 0)))
			error(3, "cannot re-allocate hard link table");
	}
	ap->entry = 0;
	if (ap->format < 0)
	{
		ap->format = ap->expected >= 0 ? ap->expected : IN_DEFAULT;
		convert(SECTION_CONTROL, CC_NATIVE, CC_ASCII);
	}
	return 1;
}

/*
 * set pseudo file header+trailer info
 */

static void
setinfo(register Archive_t* ap, register File_t* f)
{
	long	n;

	if (ap->delta)
	{
		if (ap->delta->format != DELTA_IGNORE && ap->entry > 1 && f->st->st_mtime)
		{
			if ((n = f->st->st_mtime - ap->delta->index) < 0)
				error(3, "%s: corrupt archive: %d extra file%s", ap->name, -n, n == -1 ? "" : "s");
			else if (n > 0)
				error(3, "%s: corrupt archive: %d missing file%s", ap->name, n, n == 1 ? "" : "s");
		}
		ap->delta->epilogue = 1;
	}
}

/*
 * output pseudo file header+trailer
 */

static void
putinfo(register Archive_t* ap, char* file, unsigned long mtime, unsigned long checksum)
{
	register File_t*	f = &ap->file;
	Sfio_t*			np = 0;

	if (!file)
	{
		np = sfstropen();
		if (!ap->delta || ap->delta->version == DELTA_88) sfprintf(np, "DELTA");
		else
		{
			sfprintf(np, "%c%s%c%c%c%s", INFO_SEP, ID, INFO_SEP, ap->delta->format == DELTA ? TYPE_DELTA : TYPE_COMPRESS, INFO_SEP, format[ap->delta->version ? ap->delta->version : DELTA].algorithm);
			if (state.ordered)
				sfprintf(np, "%c%c", INFO_SEP, INFO_ORDERED);
		}
		sfprintf(np, "%c%c%c", INFO_SEP, INFO_SEP, INFO_SEP);
		file = sfstruse(np);
	}
	initfile(ap, f, file, X_IFREG);
	f->skip = 1;
	f->st->st_mtime = mtime;
	f->st->st_uid = DELTA_LO(checksum);
	f->st->st_gid = DELTA_HI(checksum);
	putheader(ap, f);
	puttrailer(ap, f);
	if (np) sfstrclose(np);
}

/*
 * write archive prologue before files are copied
 */

void
putprologue(register Archive_t* ap)
{
	ap->section = SECTION_CONTROL;
	if (ap->delta && ap->delta->version == DELTA_88)
		ap->checksum = ap->old.checksum;
	switch (ap->format)
	{
	case IBMAR:
		convert(SECTION_CONTROL, CC_NATIVE, CC_EBCDIC1);
		if (!state.convert[0].on)
			convert(SECTION_DATA, CC_NATIVE, CC_EBCDIC1);
		break;
	default:
		convert(SECTION_CONTROL, CC_NATIVE, CC_ASCII);
		if (!state.convert[0].on)
			convert(SECTION_DATA, CC_NATIVE, CC_NATIVE);
		break;
	}
	switch (ap->format)
	{
	case ALAR:
	case IBMAR:
#if DEBUG
		if (ap->io.blok) ap->io.blocked = 1;
		else
#endif
		ap->io.blocked = !ap->io.unblocked;
		if (!state.id.owner[0])
		{
			strncpy(state.id.owner, fmtuid(getuid()), sizeof(state.id.owner) - 1);
			state.id.owner[sizeof(state.id.owner) - 1] = 0;
		}
		strupper(state.id.owner);
		if (!state.id.volume[0])
		{
			strncpy(state.id.volume, state.id.owner, sizeof(state.id.volume) - 1);
			state.id.volume[sizeof(state.id.volume) - 1] = 0;
		}
		strupper(state.id.volume);
		strncpy(state.id.format, ALAR_ID, sizeof(state.id.format) - 1);
		strncpy(state.id.implementation, IMPLEMENTATION, sizeof(state.id.implementation) - 1);
		if (ap->format == IBMAR) sfsprintf(state.id.standards, sizeof(state.id.standards), "%-5.5s%-5.5s%-5.5s%-4.4s", "ATT", "1", "EBCDIC", "1979");
		else sfsprintf(state.id.standards, sizeof(state.id.standards), "%-5.5s%-5.5s%-5.5s%-4.4s", "ISO", "646", "IRV", "1990");
		sfsprintf(alar_header, sizeof(alar_header), "VOL1%-6.6s              %-6.6s%-7.7s%-14.14s                            4", state.id.volume, state.id.format, state.id.implementation, state.id.owner);
		bwrite(ap, alar_header, ALAR_HEADER);
		sfsprintf(alar_header, sizeof(alar_header), "VOL2%-19.19s                                                         ", state.id.standards);
		bwrite(ap, alar_header, ALAR_HEADER);
		if (ap->delta && (ap->delta->format == COMPRESS || ap->delta->format == DELTA))
		{
			sfsprintf(alar_header, sizeof(alar_header), "UVL1 %-6.6s%c%-6.6s%010ld%010ld                                         ", ID, ap->delta->format == DELTA ? TYPE_DELTA : TYPE_COMPRESS, format[ap->delta->version ? ap->delta->version : DELTA].algorithm, state.operation == OUT ? (long)ap->size : (long)0, state.operation == OUT ? ap->checksum : 0L);
			bwrite(ap, alar_header, ALAR_HEADER);
		}
		break;
	case VDB:
		state.vdb.directory = sfstropen();
		sfprintf(state.vdb.directory, "%c%s%c%s\n", VDB_DELIMITER, VDB_MAGIC, VDB_DELIMITER, state.id.volume);
		bwrite(ap, sfstrbase(state.vdb.directory), sfstrtell(state.vdb.directory));
		sfstrset(state.vdb.directory, 0);
		sfprintf(state.vdb.directory, "%s\n", VDB_DIRECTORY);
		break;
	default:
		if (ap->delta && (ap->delta->format == COMPRESS || ap->delta->format == DELTA))
		{
			if (ap->delta->base) putinfo(ap, NiL, ap->delta->base->size, ap->delta->base->checksum);
			else putinfo(ap, NiL, 0, 0);
		}
		break;
	}
}

/*
 * read archive epilogue after all files have been copied
 */

void
getepilogue(register Archive_t* ap)
{
	register char*	s;
	register off_t	n;
	register off_t	i;

	ap->section = SECTION_CONTROL;
	if (ap->delta && ap->delta->epilogue < 0)
		error(3, "%s: corrupt archive: missing epilogue", ap->name);
	if (ap->io.mode != O_RDONLY) backup(ap);
	else
	{
		if (ap->names)
		{
			free(ap->names);
			ap->names = 0;
		}
		switch (ap->format)
		{
		case ALAR:
		case IBMAR:
		case PORTAR:
		case RANDAR:
#if SAVESET
		case SAVESET:
#endif
			break;
		case VDB:
			if (state.vdb.header.base)
			{
				free(state.vdb.header.base);
				state.vdb.header.base = 0;
			}
			break;
		default:
			/*
			 * check for more volumes
			 * volumes begin on BLOCKSIZE boundaries
			 * separated by up to MAXBLOCKS null byte filler
			 */

			if (ap->io.keep)
			{
				bskip(ap);
				if (ap->io.eof) ap->io.keep = 0;
				else if (ap->io.keep > 0) ap->io.keep--;
				ap->format = IN_DEFAULT;
				message((-2, "go for next tape volume"));
				return;
			}
			i = MAXBLOCKS;
			if (!(n = roundof(ap->io.count, BLOCKSIZE) - ap->io.count) || bread(ap, state.tmp.buffer, (off_t)0, (off_t)n, 0) > 0) do
			{
				if (*(s = state.tmp.buffer) && n == BLOCKSIZE)
				{
					bunread(ap, state.tmp.buffer, BLOCKSIZE);
					ap->format = IN_DEFAULT;
					message((-2, "go for next volume %-.32s...", state.tmp.buffer));
					return;
				}
				while (s < state.tmp.buffer + n && !*s++);
				if (s < state.tmp.buffer + n)
				{
					if (ap->volume > 1) error(1, "junk data after volume %d", ap->volume);
					break;
				}
				n = BLOCKSIZE;
			} while (i-- > 0 && bread(ap, state.tmp.buffer, (off_t)0, n, 0) > 0);
			bflushin(ap);
			break;
		}
		ap->format = IN_DEFAULT;
	}
}

/*
 * write archive epilogue after files have been copied
 */

void
putepilogue(register Archive_t* ap)
{
	register ssize_t	n;
	register off_t		boundary;

	static int		selected;

	if (state.install.path)
	{
		if (sfclose(state.install.sp))
			error(ERROR_SYSTEM|2, "%s: install temporary write error", state.checksum.path);
		state.filter.line = 2;
		state.filter.name = state.install.name;
		ftwalk(state.install.path, copyout, state.ftwflags, NiL);
		state.filter.line = 0;
	}
	if (state.checksum.path)
	{
		if (sfclose(state.checksum.sp))
			error(ERROR_SYSTEM|2, "%s: checksum temporary write error", state.checksum.path);
		sumclose(state.checksum.sum);
		state.checksum.sum = 0;
		state.filter.line = 2;
		state.filter.name = state.checksum.name;
		ftwalk(state.checksum.path, copyout, state.ftwflags, NiL);
		state.filter.line = 0;
	}
	ap->section = SECTION_CONTROL;
	if (ap->selected > selected)
	{
		if (ap->delta && (ap->delta->format == COMPRESS || ap->delta->format == DELTA))
			switch (ap->format)
			{
			case BINARY:
			case CPIO:
			case ASC:
			case ASCHK:
				break;
			default:
				putinfo(ap, NiL, ap->delta->index + 1, 0);
				break;
			}
		selected = ap->selected;
		boundary = ap->io.count;
		switch (ap->format)
		{
		case ALAR:
		case IBMAR:
			bwrite(ap, alar_header, 0);
			bwrite(ap, alar_header, 0);
			break;
		case BINARY:
		case CPIO:
		case ASC:
		case ASCHK:
			putinfo(ap, CPIO_TRAILER, ap->delta && (ap->delta->format == COMPRESS || ap->delta->format == DELTA) ? ap->delta->index + 1 : 0, 0);
			boundary = ap->io.unblocked ? BLOCKSIZE : state.blocksize;
			break;
		case PAX:
		case TAR:
		case USTAR:
			memzero(tar_block, TAR_HEADER);
			bwrite(ap, tar_block, TAR_HEADER);
			bwrite(ap, tar_block, TAR_HEADER);
			boundary = ap->io.unblocked ? BLOCKSIZE : state.blocksize;
			break;
		case VDB:
			if (state.record.header)
				bwrite(ap, state.record.header, state.record.headerlen);
			sfprintf(state.vdb.directory, "%c%s%c%0*I*d%c%0*I*d\n", VDB_DELIMITER, VDB_DIRECTORY, VDB_DELIMITER, VDB_FIXED, sizeof(ap->io.offset), ap->io.offset + ap->io.count + sizeof(VDB_DIRECTORY), VDB_DELIMITER, VDB_FIXED, sizeof(Sfoff_t), sftell(state.vdb.directory) - sizeof(VDB_DIRECTORY) + VDB_LENGTH + 1);
			bwrite(ap, sfstrbase(state.vdb.directory), sfstrtell(state.vdb.directory));
			sfstrclose(state.vdb.directory);
			boundary = ap->io.count;
			break;
		}
		if (n = ((ap->io.count > boundary) ? roundof(ap->io.count, boundary) : boundary) - ap->io.count)
		{
			memzero(state.tmp.buffer, n);
			bwrite(ap, state.tmp.buffer, n);
		}
		bflushout(ap);
		ap->volume++;
	}
	else
	{
		ap->io.count = ap->io.offset = 0;
		ap->io.next = ap->io.buffer;
	}
}

#if CPIO_EXTENDED

static char	opsbuf[PATH_MAX];	/* extended ops buffer		*/

static char*	ops = opsbuf;		/* opsbuf output pointer	*/

/*
 * get and execute extended ops from input
 */

static void
getxops(register Archive_t* ap, register File_t* f)
{
	register char*	p;
	register char*	s;
	register int	c;

	if (f->namesize > (c = strlen(f->name) + 1)) for (p = f->name + c; c = *p++;)
	{
		for (s = p; *p; p++);
		p++;
		message((-2, "%s: %s: entry %d.%d op = %c%s", ap->name, f->name, ap->volume, ap->entry, c, s));
		switch (c)
		{
		case 'd':
			IDEVICE(f->st, strtol(s, NiL, 16));
			break;
		case 'g':
			f->st->st_gid = strtol(s, NiL, 16);
			break;
		case 's':
			f->st->st_size = strtoll(s, NiL, 16);
			break;
		case 'u':
			f->st->st_uid = strtol(s, NiL, 16);
			break;
		case 'G':
			f->gidname = s;
			break;
		case 'U':
			f->uidname = s;
			break;

			/*
			 * NOTE: ignore unknown ops for future extensions
			 */
		}
	}
}

/*
 * set end of extended ops
 */

static void
setxops(Archive_t* ap, register File_t* f)
{
	register int	n;

	NoP(ap);
	if (n = ops - opsbuf)
	{
		n++;
		*ops++ = 0;
		if ((f->namesize += n) > CPIO_NAMESIZE) error(1, "%s: extended ops may crash older cpio programs", f->name);
	}
}

/*
 * output filename and extended ops
 */

static void
putxops(Archive_t* ap, register File_t* f)
{
	register int	n;

	n = ops - opsbuf;
	bwrite(ap, f->name, f->namesize -= n);
	if (n) bwrite(ap, ops = opsbuf, n);
}


/*
 * add extended op string
 */

static void
addxopstr(Archive_t* ap, int op, register char* s)
{
	register char*	p = ops;
	register char*	e = opsbuf + sizeof(opsbuf) - 3;

	NoP(ap);
	if (p < e)
	{
		*p++ = op;
		while (*s && p < e) *p++ = *s++;
		*p++ = 0;
		ops = p;
	}
#if DEBUG
	if (*s) error(PANIC, "addxopstr('%c',\"%s\") overflow", op, s);
#endif
}

/*
 * add extended op number
 */

static void
addxopnum(Archive_t* ap, int op, Sflong_t n)
{
	char	buf[33];

	sfsprintf(buf, sizeof(buf), "%I*x", sizeof(n), n);
	addxopstr(ap, op, buf);
}

#endif

/*
 * get key name value
 */

static void
getkeyname(Archive_t* ap, File_t* f, int index, char** nm)
{
	register Option_t*	op;

	static int		uinit;
	static int		ginit;
	static uid_t		euid;
	static gid_t		egid;

	op = &options[index];
	if (op->level >= 6) switch (index)
	{
	case OPT_gname:
		*nm = 0;
		if (!uinit)
		{
			uinit = 1;
			euid = geteuid();
		}
		f->st->st_uid = euid;
		break;
	case OPT_uname:
		*nm = 0;
		if (!ginit)
		{
			ginit = 1;
			egid = getegid();
		}
		f->st->st_gid = egid;
		break;
	}
	else if (op->level < 5)
	{
		if (op->entry == ap->entry || op->level > 3)
			*nm = op->temp.string;
		else if (op->level >= 1 && op->level < 3)
			*nm = op->perm.string;
	}
}

/*
 * get key size value
 */

static void
getkeysize(Archive_t* ap, File_t* f, int index, off_t* size)
{
	register Option_t*	op;

	NoP(f);
	op = &options[index];
	if (op->level < 5)
	{
		if (op->entry == ap->entry || op->level >= 3)
			*size = strtoll(op->temp.string, NiL, 10);
		else if (op->level >= 1)
			*size = strtoll(op->perm.string, NiL, 10);
	}
}

/*
 * get key time value
 */

static void
getkeytime(Archive_t* ap, File_t* f, int index, time_t* tm)
{
	register Option_t*	op;

	NoP(f);
	op = &options[index];
	if (op->level >= 6)
		*tm = time((time_t*)0);
	else if (op->level < 5)
	{
		if (op->entry == ap->entry || op->level >= 3)
			*tm = op->temp.number;
		else if (op->level >= 1)
			*tm = op->perm.number;
	}
}

#if 0
/*
 * CAB data close function
 */

static void
cabclose(Archive_t* ap)
{
	register Cab_t*	cab;
	register int	i;

	if (cab = (Cab_t*)ap->data)
	{
		ap->data = 0;
		if (cab->optional.prev.name)
			free(cab->optional.prev.name);
		if (cab->optional.prev.disk)
			free(cab->optional.prev.disk);
		if (cab->optional.next.name)
			free(cab->optional.next.name);
		if (cab->optional.next.disk)
			free(cab->optional.next.disk);
		for (i = 0; i < cab->files; i++)
			if (cab->file[i].name)
				free(cab->file[i].name);
		free(cab);
	}
}
#endif

/*
 * read next archive entry header
 */

int
getheader(register Archive_t* ap, register File_t* f)
{
	register char*	s;
	register int	i;
	register off_t	n;
	char*		t;
	struct tm	tm;
	long		num;
	long		sum;
	int		warned;
	int		checkdelta;
	int		lab;
	int		type;
	int		ordered;
	int_2		magic;

	struct
	{
		long	dev;
		long	ino;
		long	mode;
		long	uid;
		long	gid;
		long	nlink;
		long	rdev;
		long	mtime;
		long	size;
		long	dev_major;
		long	dev_minor;
		long	rdev_major;
		long	rdev_minor;
		long	checksum;
	}		lst;

	static char*	nam;
	static int	namlen;
	static char	idbuffer[ALAR_NAMESIZE + 1];

	static struct tar_header_info	tar_last[16];
	static int			tar_last_index;

	ap->section = SECTION_CONTROL;
	ap->sum++;
 volume:
	warned = 0;
	checkdelta = !ap->entry++ && (!ap->delta || ap->delta->format != DELTA_IGNORE && ap->delta->format != DELTA_PATCH);
	type = 0;
	if (ap->io.mode != O_RDONLY) bsave(ap);
 again:
	ap->memsum = 0;
	for (;;)
	{
		f->record.format = 0;
		f->skip = 0;
		message((-2, "%s:", format[ap->format].name));
		switch (ap->format)
		{
		case ALAR:
		case IBMAR:
			if (!(lab = getlabel(ap, f))) return 0;
			f->name = ap->path.header;
			f->st->st_dev = 0;
			f->st->st_ino = 0;
			f->st->st_mode = X_IFREG|X_IRUSR|X_IWUSR|X_IRGRP|X_IROTH;
			f->st->st_uid = state.uid;
			f->st->st_gid = state.gid;
			f->st->st_nlink = 1;
			IDEVICE(f->st, 0);
			f->st->st_size = 0;
			f->linktype = NOLINK;
			f->linkname = 0;
			f->uidname = 0;
			f->gidname = 0;
			type = 0;
			do
			{
				if (checkdelta && strneq(alar_header, "UVL1", 4) && strneq(alar_header + 5, ID, IDLEN))
				{
					checkdelta = 0;
					s = alar_header + 10;
					f->st->st_mtime = getlabnum(alar_header, 14, 10, 10);
					n = getlabnum(alar_header, 24, 10, 10);
					f->st->st_uid = DELTA_LO(n);
					f->st->st_gid = DELTA_HI(n);
					if (t = strchr(s, ' ')) *t = 0;
					goto deltaverify;
				}
				else if (strneq(alar_header, "HDR", 3))
				{
					if (getlabnum(alar_header, 4, 1, 10) != ++type) error(3, "%s format HDR label out of sequence", format[ap->format].name);
					if (type == 1)
					{
						s = ap->path.header;
						for (i = 4; i <= ALAR_NAMESIZE + 3; i++)
						{
							if (alar_header[i] == ' ')
							{
								if (i >= ALAR_NAMESIZE + 3 || alar_header[i + 1] == ' ') break;
								*s++ = '.';
							}
							else *s++ = isupper(alar_header[i]) ? tolower(alar_header[i]) : alar_header[i];
						}
						if ((n = getlabnum(alar_header, 40, 2, 10)) > 0 && n < 99) sfsprintf(s, 3, ".%02d", n);
						else *s = 0;
						f->record.section = getlabnum(alar_header, 28, 4, 10);
						getlabstr(alar_header, 5, ALAR_NAMESIZE, f->id = idbuffer);
						getlabstr(alar_header, 61, 6, state.id.format);
						getlabstr(alar_header, 67, 7, state.id.implementation);
#if SAVESET
						if (streq(state.id.format, SAVESET_ID) && streq(state.id.implementation, SAVESET_IMPL))
							ap->format = SAVESET;
#endif
						f->st->st_mtime = 0;
						if (n = getlabnum(alar_header, 43, 2, 10))
						{
							if (alar_header[41] == '0') n += 100;
							if ((i = getlabnum(alar_header, 45, 3, 10)) >= 0 && i <= 365)
							{
								f->st->st_mtime = i;
								while (n-- > 70) f->st->st_mtime += ((n % 4) || n == 100) ? 365 : 366;
								f->st->st_mtime *= 24L * 60L * 60L;
								f->st->st_mtime += 12L * 60L * 60L;
							}
						}
						if (!f->st->st_mtime)
							f->st->st_mtime = time((time_t*)0);
					}
					else if (type == 2)
					{
						switch (f->record.format = alar_header[4])
						{
						case 'D': /* decimal variable	*/
						case 'F': /* fixed length	*/
						case 'S': /* spanned		*/
						case 'U': /* input block size	*/
						case 'V': /* binary variable	*/
							break;
						default:
							error(2, "%s record format %c not supported", format[ap->format].name, f->record.format);
							f->skip = 1;
						}
						state.blocksize = getlabnum(alar_header, 6, 5, 10);
						state.record.size = getlabnum(alar_header, 11, 5, 10);
						if (!ap->io.blocked) f->st->st_size = getlabnum(alar_header, 16, 10, 10);
						state.record.offset = getlabnum(alar_header, 51, 2, 10);
					}
				}
				else if (!ap->io.blocked && strneq(alar_header, "VOL1", 4))
				{
					bunread(ap, alar_header, lab);
					if (!(getprologue(ap))) return 0;
					goto volume;
				}
			} while ((lab = getlabel(ap, f)));
#if SAVESET
			if (ap->format != SAVESET) goto found;
			state.saveset.time = f->st->st_mtime;
			if (state.blocksize > state.saveset.blocksize)
			{
				state.saveset.blocksize = state.blocksize;
				if (state.saveset.block) free(state.saveset.block);
				if (!(state.saveset.block = newof(0, char, state.saveset.blocksize, 0)))
					error(3, "cannot allocate %s format buffer", format[ap->format].name);
			}
			state.saveset.bp = state.saveset.block + state.blocksize;
			/*FALLTHROUGH*/
		case SAVESET:
			f->name = ap->path.header;
			if (!getsaveset(ap, f, 1)) goto again;
#endif
			goto found;
		case BINARY:
			if (bread(ap, &binary_header, (off_t)BINARY_HEADER, (off_t)BINARY_HEADER, 0) <= 0) break;
			if (ap->swap)
			{
				memcpy(state.tmp.buffer, &binary_header, BINARY_HEADER);
				swapmem(ap->swap, &binary_header, &binary_header, BINARY_HEADER);
			}
			f->magic = binary_header.magic;
			if (f->magic == CPIO_MAGIC)
			{
				f->namesize = binary_header.namesize;
				f->st->st_dev = binary_header.dev;
				f->st->st_ino = binary_header.ino;
				f->st->st_mode = binary_header.mode;
				f->st->st_uid = binary_header.uid;
				f->st->st_gid = binary_header.gid;
				f->st->st_nlink = binary_header.links;
				IDEVICE(f->st, binary_header.rdev);
				f->st->st_mtime = cpio_long(binary_header.mtime);
				f->st->st_size = cpio_long(binary_header.size);
			cpio_common:
				f->linktype = NOLINK;
				f->linkname = 0;
				f->uidname = 0;
				f->gidname = 0;
				f->name = ap->path.header;
				switch (ap->format)
				{
				case BINARY:
					i = BINARY_ALIGN;
					n = BINARY_HEADER;
					break;
				case ASC:
				case ASCHK:
					i = ASC_ALIGN;
					n = ASC_HEADER;
					break;
				default:
					i = 0;
					break;
				}
				if (i)
				{
					if (n = (n + f->namesize) % i) i -= n;
					else i = 0;
				}
				if (f->namesize >= sizeof(ap->path.header))
				{
					error(2, "%s: entry %d.%d file name too long", ap->name, ap->volume, ap->entry);
					for (n = f->namesize + i; n > 0; n -= sizeof(ap->path.header))
						bread(ap, ap->path.header, (off_t)0, n > sizeof(ap->path.header) ? (off_t)sizeof(ap->path.header) : n, 1);
					f->skip = 1;
				}
				else
				{
					bread(ap, ap->path.header, (off_t)0, (off_t)(f->namesize + i), 1);
					if (ap->path.header[f->namesize - 1])
					{
						bunread(ap, &ap->path.header[f->namesize - 1], 1);
						ap->path.header[f->namesize - 1] = 0;
						error(state.keepgoing ? 1 : 3, "%s: entry %d.%d file name terminating null missing", ap->name, ap->volume, ap->entry);
					}
#if CPIO_EXTENDED
					getxops(ap, f);
#endif
				}
				if (streq(f->name, CPIO_TRAILER))
				{
					getdeltaheader(ap, f);
					if (ap->delta)
						setinfo(ap, f);
					return 0;
				}
				switch (f->type = X_ITYPE(f->st->st_mode))
				{
				case X_IFBLK:
				case X_IFCHR:
				case X_IFDIR:
				case X_IFIFO:
				case X_IFLNK:
				case X_IFREG:
				case X_IFSOCK:
					break;
				default:
					error(1, "%s: unknown file type %07o -- regular file assumed", f->name, f->type);
					f->type = X_IFREG;
					break;
				}
				f->st->st_mode &= X_IPERM;
				f->st->st_mode |= f->type;
				switch (f->type)
				{
				case X_IFLNK:
					if (f->st->st_size > sizeof(ap->path.link) - 1)
					{
						error(2, "%s: entry %d.%d symbolic link text too long", ap->name, ap->volume, ap->entry);
						f->skip = 1;
					}
					else
					{
						f->linktype = SOFTLINK;
						s = f->linkname = ap->path.link;
						while (bread(ap, s, (off_t)1, (off_t)1, 1) > 0)
						{
							f->st->st_size--;
							if (!*s++) break;
							if (!f->st->st_size)
							{
								*s = 0;
								break;
							}
						}
					}
					break;
				default:
					f->linktype = NOLINK;
					break;
				}
				goto found;
			}
			bunread(ap, ap->swap ? state.tmp.buffer : (char*)&binary_header, BINARY_HEADER);
			break;
		case CPIO:
			if (bread(ap, state.tmp.buffer, (off_t)0, (off_t)CPIO_HEADER, 0) <= 0) break;
			state.tmp.buffer[CPIO_HEADER] = 0;
			if (state.tmp.buffer[0] == '0' && sfsscanf(state.tmp.buffer, "%6o%6lo%6lo%6lo%6lo%6lo%6lo%6lo%11lo%6o%11lo",
				&f->magic,
				&lst.dev,
				&lst.ino,
				&lst.mode,
				&lst.uid,
				&lst.gid,
				&lst.nlink,
				&lst.rdev,
				&lst.mtime,
				&f->namesize,
				&lst.size) == 11 && f->magic == CPIO_MAGIC)
			{
				f->st->st_dev = lst.dev;
				f->st->st_ino = lst.ino;
				f->st->st_mode = lst.mode;
				f->st->st_uid = lst.uid;
				f->st->st_gid = lst.gid;
				f->st->st_nlink = lst.nlink;
				IDEVICE(f->st, lst.rdev);
				f->st->st_mtime = lst.mtime;
				f->st->st_size = lst.size;
				goto cpio_common;
			}
			bunread(ap, state.tmp.buffer, CPIO_HEADER);
			break;
		case PAX:
		case TAR:
		case USTAR:
			if (bread(ap, tar_block, (off_t)0, (off_t)TAR_HEADER, 0) <= 0) break;
			if (!*tar_header.name)
			{
				if (ap->entry == 1) goto notar;
				return 0;
			}
			if (sfsscanf(tar_header.mode, "%7lo", &num) != 1) goto notar;
			f->st->st_mode = num;
			if (sfsscanf(tar_header.uid, "%7lo", &num) != 1) goto notar;
			f->st->st_uid = num;
			if (sfsscanf(tar_header.gid, "%7lo", &num) != 1) goto notar;
			f->st->st_gid = num;
			if (sfsscanf(tar_header.size, "%11lo", &num) != 1) goto notar;
			f->st->st_size = num;
			if (sfsscanf(tar_header.mtime, "%11lo", &num) != 1) goto notar;
			f->st->st_mtime = num;
			if (sfsscanf(tar_header.chksum, "%7lo", &num) != 1) goto notar;
			if ((num &= TAR_SUMASK) != (sum = tar_checksum()))
			{
				if (ap->entry == 1) goto notar;
				error(state.keepgoing ? 1 : 3, "%s format checksum error (%ld != %ld)", format[ap->format].name, num, sum);
			}
			if (ap->format != TAR)
			{
				if (!streq(tar_header.magic, TMAGIC))
				{
					if (strneq(tar_header.magic, TMAGIC, TMAGLEN - 1))
						error(1, "%s: %s format botched -- %s format assumed", ap->name, format[ap->format].name, format[TAR].name);
					else if (ap->entry > 1) goto notar;
					ap->format = TAR;
				}
				else if (!strneq(tar_header.version, TVERSION, sizeof(tar_header.version)))
				{
					error(1, "%s: %s format version %-.*s incompatible with implementation version %-.*s -- assuming %s", ap->name, format[ap->format].name, sizeof(tar_header.version), tar_header.version, sizeof(tar_header.version), TVERSION, format[TAR].name);
					ap->format = TAR;
				}
			}
			*(tar_header.name + sizeof(tar_header.name)) = 0;
			if (ap->format != TAR && *tar_header.prefix)
				sfsprintf(f->name = ap->path.header, sizeof(ap->path.header), "%-.*s/%s", sizeof(tar_header.prefix), tar_header.prefix, tar_header.name);
			else f->name = tar_header.name;
			*(tar_header.linkname + sizeof(tar_header.name)) = 0;
			f->linktype = NOLINK;
			f->linkname = 0;
			f->st->st_nlink = 1;
			switch (tar_header.typeflag)
			{
			case LNKTYPE:
				f->linktype = HARDLINK;
				f->st->st_mode |= X_IFREG;
				f->st->st_nlink = 2;
				if (!ap->delta)
					f->st->st_size = 0;
				f->linkname = strcpy(ap->path.link, tar_header.linkname);
				break;
			case SYMTYPE:
				f->linktype = SOFTLINK;
				f->st->st_mode |= X_IFLNK;
				f->linkname = strcpy(ap->path.link, tar_header.linkname);
				break;
			case CHRTYPE:
				f->st->st_mode |= X_IFCHR;
			device:
				if (sfsscanf(tar_header.devmajor, "%7lo", &num) != 1) goto notar;
				i = num;
				if (sfsscanf(tar_header.devminor, "%7lo", &num) != 1) goto notar;
				IDEVICE(f->st, makedev(i, num));
				break;
			case BLKTYPE:
				f->st->st_mode |= X_IFBLK;
				goto device;
			case DIRTYPE:
				f->st->st_mode |= X_IFDIR;
				break;
			case FIFOTYPE:
				f->st->st_mode |= X_IFIFO;
				break;
#ifdef SOKTYPE
			case SOKTYPE:
				f->st->st_mode |= X_IFSOCK;
				break;
#endif
			case EXTTYPE:
				ap->format = PAX;
				if (f->st->st_size > 0)
				{
					if (s = bget(ap, f->st->st_size))
					{
						s[f->st->st_size - 1] = 0;
						setoptions(s, NiL, state.usage, ap);
					}
					else error(3, "invalid %s format extended header", format[ap->format].name);
				}
				gettrailer(ap, f);
				goto again;
			default:
				error(1, "unknown file type `%c' -- regular file assumed", tar_header.typeflag);
				/*FALLTHROUGH*/
			case REGTYPE:
			case AREGTYPE:
				f->namesize = strlen(f->name) + 1;
				if (f->name[f->namesize - 2] == '/')
				{
					f->st->st_mode |= X_IFDIR;
					if (f->namesize > 2) f->name[--f->namesize - 1] = 0;
				}
				else f->st->st_mode |= X_IFREG;
				break;
			}
			f->uidname = 0;
			f->gidname = 0;
			if (ap->format != TAR)
			{
				if (*tar_header.uname) f->uidname = tar_header.uname;
				if (*tar_header.gname) f->gidname = tar_header.gname;
			}
			goto found;
		notar:
			bunread(ap, tar_block, TAR_HEADER);
			break;
		case ASC:
		case ASCHK:
			if (bread(ap, state.tmp.buffer, (off_t)0, (off_t)ASC_HEADER, 0) <= 0) break;
			state.tmp.buffer[ASC_HEADER] = 0;
			if (state.tmp.buffer[0] == '0' && sfsscanf(state.tmp.buffer, "%6o%8lx%8lx%8lx%8lx%8lx%8lx%8lx%8lx%8lx%8lx%8lx%8x%8lx",
				&f->magic,
				&lst.ino,
				&lst.mode,
				&lst.uid,
				&lst.gid,
				&lst.nlink,
				&lst.mtime,
				&lst.size,
				&lst.dev_major,
				&lst.dev_minor,
				&lst.rdev_major,
				&lst.rdev_minor,
				&f->namesize,
				&lst.checksum) == 14 && (f->magic == ASC_MAGIC || f->magic == ASCHK_MAGIC))
			{
				if (f->magic == ASCHK_MAGIC) ap->format = ASCHK;
				f->checksum = lst.checksum;
				f->st->st_dev = makedev(lst.dev_major, lst.dev_minor);
				f->st->st_ino = lst.ino;
				f->st->st_mode = lst.mode;
				f->st->st_uid = lst.uid;
				f->st->st_gid = lst.gid;
				f->st->st_nlink = lst.nlink;
				IDEVICE(f->st, makedev(lst.rdev_major, lst.rdev_minor));
				f->st->st_mtime = lst.mtime;
				f->st->st_size = lst.size;
				goto cpio_common;
			}
			bunread(ap, state.tmp.buffer, ASC_HEADER);
			break;
		case PORTAR:
		case RANDAR:
			if (bread(ap, &portar_header, (off_t)0, (off_t)PORTAR_HEADER, 0) <= 0)
				break;
			if (strneq(portar_header.ar_fmag, PORTAR_END, PORTAR_ENDSIZ) && isdigit(portar_header.ar_date[0]) && isdigit(portar_header.ar_size[0]))
			{
				if (!isdigit(portar_header.ar_uid[0]))
				{
					n = sfsprintf(portar_header.ar_uid, sizeof(portar_header.ar_uid), "%d", state.uid);
					portar_header.ar_uid[n] = ' ';
				}
				if (!isdigit(portar_header.ar_gid[0]))
				{
					n = sfsprintf(portar_header.ar_gid, sizeof(portar_header.ar_gid), "%d", state.gid);
					portar_header.ar_gid[n] = ' ';
				}
				if (!isdigit(portar_header.ar_mode[0]))
				{
					n = sfsprintf(portar_header.ar_mode, sizeof(portar_header.ar_mode), "0%o", X_IRUSR|X_IWUSR|X_IRGRP|X_IWGRP|X_IROTH|X_IWOTH);
					portar_header.ar_mode[n] = ' ';
				}
				if (sfsscanf(portar_header.ar_date, "%12ld%6ld%6ld%8lo%10ld",
					&lst.mtime,
					&lst.uid,
					&lst.gid,
					&lst.mode,
					&lst.size) == 5)
				{
					f->name = (ap->names && portar_header.ar_name[0] == PORTAR_TERM) ? (ap->names + strtol(portar_header.ar_name + 1, NiL, 10)) : portar_header.ar_name;
					portar_header.ar_date[0] = 0;
					if ((s = strchr(f->name, PORTAR_TERM)) || (s = strchr(f->name, RANDAR_TERM)))
						*s = 0;
					if (ap->separator)
					{
						s = f->name;
						while (s = strchr(f->name, ap->separator))
							*s++ = '/';
					}
					f->st->st_dev = 0;
					f->st->st_ino = 0;
					f->st->st_mode = X_IFREG|(lst.mode&X_IPERM);
					f->st->st_uid = lst.uid;
					f->st->st_gid = lst.gid;
					f->st->st_nlink = 1;
					IDEVICE(f->st, 0);
					f->st->st_mtime = lst.mtime;
					f->st->st_size = lst.size;
					f->linktype = NOLINK;
					f->linkname = 0;
					f->uidname = 0;
					f->gidname = 0;
					goto found;
				}
			}
			bunread(ap, &portar_header, PORTAR_HEADER);
			return 0;
		case VDB:
			if (!state.vdb.header.base)
			{
				if (fstat(ap->io.fd, &state.vdb.st)) break;
				state.vdb.st.st_mode = modex(state.vdb.st.st_mode);
				s = tar_block;
				n = sizeof(VDB_MAGIC) + sizeof(state.id.volume) + 1;
				if (bread(ap, s, (off_t)0, n, 0) <= 0) break;
				bunread(ap, s, n);
				if (s[0] != VDB_DELIMITER || !strneq(s + 1, VDB_MAGIC, sizeof(VDB_MAGIC) - 1) || s[sizeof(VDB_MAGIC)] != VDB_DELIMITER) break;
				if (s[sizeof(VDB_MAGIC) + 1] != '\n')
				{
					s[n] = 0;
					if (t = strchr(s, '\n')) *t = 0;
					strncpy(state.id.volume, s + sizeof(VDB_MAGIC) + 1, sizeof(state.id.volume) - 2);
				}
				if (lseek(ap->io.fd, (off_t)(-(VDB_LENGTH + 1)), SEEK_END) <= 0) break;
				if (read(ap->io.fd, s, VDB_LENGTH + 1) != (VDB_LENGTH + 1)) break;
				state.vdb.variant = *s++ != '\n';
				if (!strneq(s, VDB_DIRECTORY, sizeof(VDB_DIRECTORY) - 1)) break;
				state.vdb.delimiter = s[VDB_OFFSET - 1];
				n = strtol(s + VDB_OFFSET, NiL, 10) - sizeof(VDB_DIRECTORY);
				i = lseek(ap->io.fd, (off_t)0, SEEK_CUR) - n - VDB_LENGTH - state.vdb.variant;
				if (!(state.vdb.header.base = newof(0, char, i, 0))) break;
				if (lseek(ap->io.fd, n, SEEK_SET) != n) break;
				if (read(ap->io.fd, state.vdb.header.base, i) != i) break;
				*(state.vdb.header.base + i) = 0;
				if (!strneq(state.vdb.header.base, VDB_DIRECTORY, sizeof(VDB_DIRECTORY) - 1)) break;
				if (!(state.vdb.header.next = strchr(state.vdb.header.base, '\n'))) break;
				state.vdb.header.next++;
			}
			t = state.vdb.header.next;
			if (!(state.vdb.header.next = strchr(t, '\n'))) goto vdb_eof;
			*state.vdb.header.next++ = 0;
			message((-1, "VDB: next=`%s'", t));
			if (state.vdb.variant) state.vdb.delimiter = *t++;
			f->name = t;
			if (!(t = strchr(t, state.vdb.delimiter))) goto vdb_eof;
			*t++ = 0;
			n = strtol(t, &t, 10);
			if (*t++ != state.vdb.delimiter) goto vdb_eof;
			if (lseek(ap->io.fd, n, SEEK_SET) != n) goto vdb_eof;
			*f->st = state.vdb.st;
			f->st->st_size = strtol(t, &t, 10);
			if (*t++ == state.vdb.delimiter) do
			{
				if (s = strchr(t, state.vdb.delimiter))
					*s++ = 0;
				if (strneq(t, VDB_DATE, sizeof(VDB_DATE) - 1))
					f->st->st_mtime = strtol(t + sizeof(VDB_DATE), NiL, 10);
				else if (strneq(t, VDB_MODE, sizeof(VDB_MODE) - 1))
					f->st->st_mode = (strtol(t + sizeof(VDB_MODE), NiL, 8) & X_IPERM) | X_IFREG;
			} while (t = s);
			f->st->st_dev = 0;
			f->st->st_ino = 0;
			f->st->st_nlink = 1;
			IDEVICE(f->st, 0);
			f->linktype = NOLINK;
			f->linkname = 0;
			f->uidname = 0;
			f->gidname = 0;
			ap->io.eof = 1;
			bflushin(ap);
			ap->io.eof = 0;
			goto found;
		case RPM:
		{
			Rpm_magic_t	magic;
			Rpm_magic_t	verify;
			Rpm_lead_t	lead;
			Rpm_lead_old_t	lead_old;
			Rpm_head_t	head;

			if (bread(ap, &magic, (off_t)0, (off_t)sizeof(magic), 0) <= 0)
				break;
			if (magic.magic == RPM_MAGIC)
				ap->swap = 0;
			else if (magic.magic == RPM_CIGAM)
				ap->swap = 3;
			else
			{
				bunread(ap, &magic, sizeof(magic));
				break;
			}
			switch (magic.major)
			{
			case 1:
				if (bread(ap, &lead_old, (off_t)sizeof(lead_old), (off_t)sizeof(lead_old), 0) <= 0)
					break;
				if (ap->swap)
					swapmem(ap->swap, &lead_old, &lead_old, sizeof(lead_old));
				if (bseek(ap, (off_t)lead_old.archoff, SEEK_SET, 0) != (off_t)lead_old.archoff)
					error(3, "%s: %s imbedded archive seek error", ap->name, format[ap->compress].name);
				break;
			case 2:
			case 3:
				if (bread(ap, &lead, (off_t)sizeof(lead), (off_t)sizeof(lead), 0) <= 0)
					break;
				memcpy(state.id.volume, lead.name, sizeof(state.id.volume) - 1);
				if (s = strrchr(ap->name, '/'))
					s++;
				else
					s = ap->name;
				if (!memcmp(s, state.id.volume, strlen(state.id.volume)))
					state.id.volume[0] = 0;
				if (ap->swap & 1)
					swapmem(ap->swap & 1, &lead, &lead, sizeof(lead));
				switch (lead.sigtype)
				{
				case 0:
					num = 0;
					break;
				case 1:
					num = 256;
					break;
				case 5:
					if (bread(ap, &verify, (off_t)sizeof(verify), (off_t)sizeof(verify), 0) <= 0)
					{
						error(2, "%s: %s format header magic expected at offset %ld", ap->name, format[ap->format].name, ap->io.offset + ap->io.count);
						return 0;
					}
					if (ap->swap)
						swapmem(ap->swap, &verify, &verify, sizeof(verify));
					if (verify.magic != RPM_HEAD_MAGIC)
					{
						error(2, "%s: invalid %s format signature header magic", ap->name, format[ap->format].name);
						return 0;
					}
					if (bread(ap, &head, (off_t)sizeof(head), (off_t)sizeof(head), 0) <= 0)
					{
						error(2, "%s: %s format signature header expected", ap->name, format[ap->format].name);
						return 0;
					}
					if (ap->swap)
						swapmem(ap->swap, &head, &head, sizeof(head));
					num = head.entries * sizeof(Rpm_entry_t) + head.datalen;
					num += (8 - (num % 8)) % 8;
					break;
				default:
					error(2, "%s: %s format version %d.%d signature type %d not supported", ap->name, format[ap->format].name, magic.major, magic.minor, lead.sigtype);
					return 0;
				}
				if (num && bread(ap, NiL, (off_t)num, (off_t)num, 0) <= 0)
				{
					error(2, "%s: %s format header %ld byte data block expected", ap->name, format[ap->format].name, num);
					return 0;
				}
				if (magic.major >= 3)
				{
					if (bread(ap, &verify, (off_t)sizeof(verify), (off_t)sizeof(verify), 0) <= 0)
					{
						error(2, "%s: %s format header magic expected", ap->name, format[ap->format].name);
						return 0;
					}
					if (ap->swap)
						swapmem(ap->swap, &verify, &verify, sizeof(verify));
					if (verify.magic != RPM_HEAD_MAGIC)
					{
						error(2, "%s: invalid %s format header magic", ap->name, format[ap->format].name);
						return 0;
					}
				}
				if (bread(ap, &head, (off_t)sizeof(head), (off_t)sizeof(head), 0) <= 0)
				{
					error(2, "%s: %s format header expected", ap->name, format[ap->format].name);
					return 0;
				}
				if (ap->swap)
					swapmem(ap->swap, &head, &head, sizeof(head));
				num = head.entries * sizeof(Rpm_entry_t) + head.datalen;
				if (num && bread(ap, NiL, (off_t)num, (off_t)num, 0) <= 0)
				{
					error(2, "%s: %s format header %ld byte entry+data block expected", ap->name, format[ap->format].name, num);
					return 0;
				}
				break;
			default:
				error(2, "%s: %s format version %d.%d not supported", ap->name, format[ap->format].name, magic.major, magic.minor);
				return 0;
			}
			ap->entry = 0;
			ap->format = -1;
			ap->swap = 0;
			ap->volume--;
			i = state.id.volume[0];
			if (!getprologue(ap))
			{
				error(2, "%s: %s format embedded archive expected", ap->name, format[RPM].name);
				return 0;
			}
			state.id.volume[0] = i;
			sfsprintf(idbuffer, sizeof(idbuffer), "%s %d.%d", format[RPM].name, magic.major, magic.minor);
			ap->package = idbuffer;
			goto volume;
		}
		case ZIP:
			num = ZIP_LOC_HEADER;
			if (bread(ap, zip_header, (off_t)num, (off_t)num, 0) <= 0)
			{
			zip_done:
				if (ap->entries == ap->verified)
					return 0;
				error(2, "%s: %d out of %d verification entries omitted", ap->entries - ap->verified, ap->entries);
				break;
			}
			n = swapget(0, zip_header, 4);
			if (n == ZIP_CEN_MAGIC)
			{
				bunread(ap, zip_header, num);
				if (bread(ap, zip_header, (off_t)ZIP_CEN_HEADER, (off_t)ZIP_CEN_HEADER, 0) <= 0)
				{
					error(2, "%s: invalid %s format verification header", ap->name, format[ap->format].name);
					return 0;
				}
				n = swapget(3, &zip_header[ZIP_CEN_NAM], 2);
				if (n >= namlen)
				{
					namlen = roundof(n + 1, PATH_MAX);
					if (!(nam = newof(nam, char, namlen, 0)))
						error(ERROR_SYSTEM|3, "out of space [name]");
				}
				if (bread(ap, nam, n, n, 0) <= 0)
				{
					error(2, "%s: invalid %s format verification header name [size=%ld]", ap->name, format[ap->format].name, n);
					return 0;
				}
				if (nam[n - 1] == '/')
					n--;
				nam[n] = 0;
				if ((n = swapget(3, &zip_header[ZIP_CEN_EXT], 2)) && bread(ap, NiL, n, n, 0) <= 0)
				{
					error(2, "%s: %s: invalid %s format verification header extended data [size=%ld]", ap->name, nam, format[ap->format].name, n);
					return 0;
				}
				if ((n = swapget(3, &zip_header[ZIP_CEN_COM], 2)) && bread(ap, NiL, n, n, 0) <= 0)
				{
					error(2, "%s: %s: invalid %s format verification header comment data [size=%ld]", ap->name, nam, format[ap->format].name, n);
					return 0;
				}
				ap->verified++;
				if (ap->tab)
				{
					if (!hashget(ap->tab, nam))
					{
						error(1, "%s: %s: file data not found", ap->name, nam);
						goto again;
					}
					n = ((unsigned long)swapget(3, &zip_header[ZIP_CEN_ATX], 4) >> 16) & 0xffff;
					switch ((int)X_ITYPE(n))
					{
					case 0:
					case X_IFREG:
						break;
					case X_IFDIR:
						break;
					case X_IFLNK:
						error(1, "%s: %s: symbolic link copied as regular file", ap->name, nam);
						break;
					default:
						error(1, "%s: %s: unknown file type %07o -- regular file assumed (0x%08x)", ap->name, nam, X_ITYPE(n), n);
						break;
					}
				}
				goto again;
			}
			if (n == ZIP_END_MAGIC)
			{
				bunread(ap, zip_header, num);
				if (bread(ap, zip_header, (off_t)ZIP_END_HEADER, (off_t)ZIP_END_HEADER, 0) <= 0)
				{
					error(2, "%s: invalid %s format trailer", ap->name, format[ap->format].name);
					return 0;
				}
				n = swapget(3, &zip_header[ZIP_END_COM], 2);
				if (bread(ap, NiL, n, n, 0) <= 0)
				{
					error(2, "%s: invalid %s format trailer data", ap->name, format[ap->format].name);
					return 0;
				}
				goto zip_done;
			}
			if (n != ZIP_LOC_MAGIC)
			{
				bunread(ap, zip_header, num);
				break;
			}
			n = swapget(3, &zip_header[ZIP_LOC_NAM], 2);
			if (n >= namlen)
			{
				namlen = roundof(n + 1, PATH_MAX);
				if (!(nam = newof(nam, char, namlen, 0)))
					error(ERROR_SYSTEM|3, "out of space [name]");
			}
			if (bread(ap, nam, n, n, 0) <= 0)
				break;
			num += n;
			f->st->st_mode = (n > 0 && nam[n - 1] == '/') ? (X_IFDIR|X_IRUSR|X_IWUSR|X_IXUSR|X_IRGRP|X_IXGRP|X_IROTH|X_IXOTH) : (X_IFREG|X_IRUSR|X_IWUSR|X_IRGRP|X_IROTH);
			nam[n] = 0;
			f->name = nam;
			if ((n = swapget(3, &zip_header[ZIP_LOC_EXT], 2)) > 0)
			{
				if (bread(ap, NiL, n, n, 0) <= 0)
					break;
				num += n;
			}
			f->st->st_dev = 0;
			f->st->st_ino = 0;
			f->st->st_uid = state.uid;
			f->st->st_gid = state.gid;
			f->st->st_nlink = 1;
			IDEVICE(f->st, 0);
			n = swapget(3, &zip_header[ZIP_LOC_TIM], 4);
			memset(&tm, 0, sizeof(tm));
			tm.tm_year = ((n>>25)&0377) + 80;
			tm.tm_mon = ((n>>21)&017) - 1;
			tm.tm_mday = ((n>>16)&037);
			tm.tm_hour = ((n>>11)&037);
			tm.tm_min = ((n>>5)&077);
			tm.tm_sec = ((n<<1)&037);
			f->st->st_mtime = tmtime(&tm, TM_LOCALZONE);
			f->linktype = NOLINK;
			f->linkname = 0;
			f->uidname = 0;
			f->gidname = 0;
			checkdelta = 0;
			f->st->st_size = swapget(3, &zip_header[ZIP_LOC_SIZ], 4);
			f->delta.size = swapget(3, &zip_header[ZIP_LOC_LEN], 4);
			n = swapget(3, &zip_header[ZIP_LOC_FLG], 2);
			if (n & 8)
			{
				n = bseek(ap, (off_t)0, SEEK_CUR, 0) - num;
				s = state.tmp.buffer;
				i = ZIP_EXT_HEADER;
				if (!f->st->st_size)
				{
					i--;
					for (;;)
					{
						while (bread(ap, s, 1, 1, 1) > 0 && *s != 0x50);
						if (bread(ap, s + 1, i, i, 1) <= 0)
							error(3, "%s: invalid %s local extension header", ap->name, format[ap->format].name);
						if (swapget(0, s, 4) == ZIP_EXT_MAGIC)
							break;
						bunread(ap, s + 1, i);
					}
				}
				else if (bread(ap, NiL, f->st->st_size, f->st->st_size, 1) <= 0 || bread(ap, s, i, i, 1) <= 0)
					error(3, "%s: invalid %s local extension header", ap->name, format[ap->format].name);
				num += ZIP_EXT_HEADER;
				f->delta.size = swapget(3, &s[ZIP_EXT_LEN], 4);
				f->st->st_size = swapget(3, &s[ZIP_EXT_SIZ], 4) + num;
				if ((n = bseek(ap, -(off_t)f->st->st_size, SEEK_CUR, 0)) < 0)
					error(3, "%s: %s local extension header seek error", ap->name, format[ap->format].name);
				f->delta.op = f->delta.size ? DELTA_zip : DELTA_pass;
			}
			else if (!f->st->st_size || swapget(3, &zip_header[ZIP_LOC_HOW], 2) == ZIP_COPY)
			{
				f->delta.op = DELTA_pass;
				f->delta.size = -1;
			}
			else
			{
				f->delta.op = DELTA_zip;
				f->st->st_size += num;
				bunread(ap, zip_header, num);
			}
			if (!ap->entries)
			{
				if (!ap->tab && !(ap->tab = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_name, "entries", 0)))
					error(1, "%s: cannot allocate verification hash table", ap->name);
				undoable(ap, &format[GZIP]);
			}
			goto found;
		case CAB:
		{
			register Cab_t*	cab;

			if (!(cab = (Cab_t*)ap->data))
			{
				Cabheader_t	hdr;
				int		k;
				int_2		i2;
				Sfio_t*		tp;

				if (bread(ap, &hdr, (off_t)sizeof(hdr), (off_t)sizeof(hdr), 0) <= 0)
					break;
				if (memcmp(hdr.magic, CAB_MAGIC, sizeof(hdr.magic)))
				{
					bunread(ap, &hdr, sizeof(hdr));
					break;
				}
				swapmem(SWAPOP(3), &hdr.hdrsum, &hdr.hdrsum, (char*)&hdr.version - (char*)&hdr.hdrsum);
				swapmem(SWAPOP(1), &hdr.version, &hdr.version, (char*)(&hdr + 1) - (char*)&hdr.version);
				if (hdr.version != CAB_VERSION)
					error(3, "%s: %s format version %04x not supported", ap->name, format[CAB].name, hdr.version);
				if (hdr.fileoff < sizeof(hdr))
				{
				cab_error:
					error(3, "%s: %s format version %04x header corrupted", ap->name, format[CAB].name, hdr.version);
					break;
				}
				if (!(cab = newof(0, Cab_t, 1, hdr.chunks * sizeof(Cabchunk_t) + hdr.files * sizeof(Cabfile_t))))
					error(ERROR_SYSTEM|3, "out of space [cab header]");
				cab->chunk = (Cabchunk_t*)(cab + 1);
				cab->file = (Cabfile_t*)(cab->chunk + hdr.chunks);
				ap->data = (void*)cab;
				cab->header = hdr;
				if (hdr.flags & CAB_FLAG_RESERVE)
				{
					s = state.tmp.buffer;
					if (bread(ap, s, 2, 2, 2) < 0)
						goto cab_error;
					swapmem(SWAPOP(1), s, &i2, sizeof(i2));
					cab->reserved.header = i2;
					if (bread(ap, s, 1, 1, 1) < 0)
						goto cab_error;
					cab->reserved.chunk = *(unsigned char*)s;
					if (bread(ap, s, 1, 1, 1) < 0)
						goto cab_error;
					cab->reserved.block = *(unsigned char*)s;
					if (cab->reserved.header)
						bread(ap, NiL, (off_t)cab->reserved.header, cab->reserved.header, 0);
					message((-1, "cab reserved.header=%u reserved.chunk=%u reserved.block=%u", cab->reserved.header, cab->reserved.chunk, cab->reserved.block));
				}
				if (hdr.flags & CAB_FLAG_HASPREV)
				{
					s = state.tmp.buffer;
					while (bread(ap, s, 1, 1, 1) > 0 && *s++);
					*s = 0;
					if (!(cab->optional.prev.name = strdup(state.tmp.buffer)))
						error(ERROR_SYSTEM|3, "out of space [cab optional header]");
					for (s = state.tmp.buffer; bread(ap, s, 1, 1, 1) > 0 && *s; s++)
						if (*s == '\\')
							*s = '/';
					*s = 0;
					if (!(cab->optional.prev.disk = strdup(state.tmp.buffer)))
						error(ERROR_SYSTEM|3, "out of space [cab optional header]");
					message((-1, "cab prev name=%s disk=%s", cab->optional.prev.name, cab->optional.prev.disk));
				}
				if (hdr.flags & CAB_FLAG_HASNEXT)
				{
					s = state.tmp.buffer;
					while (bread(ap, s, 1, 1, 1) > 0 && *s++);
					*s = 0;
					if (!(cab->optional.next.name = strdup(state.tmp.buffer)))
						error(ERROR_SYSTEM|3, "out of space [cab optional header]");
					for (s = state.tmp.buffer; bread(ap, s, 1, 1, 1) > 0 && *s; s++)
						if (*s == '\\')
							*s = '/';
					*s = 0;
					if (!(cab->optional.next.disk = strdup(state.tmp.buffer)))
						error(ERROR_SYSTEM|3, "out of space [cab optional header]");
					message((-1, "cab next name=%s disk=%s", cab->optional.next.name, cab->optional.next.disk));
				}
				if (bread(ap, cab->chunk, (off_t)(hdr.chunks * sizeof(Cabchunk_t)), (off_t)(hdr.chunks * sizeof(Cabchunk_t)), 0) <= 0)
					return 0;
				message((-1, "cab %s header info:\n\thdrsum=%u\n\tsize=%u\n\tchunksum=%u\n\tfileoff=%u\n\tdatasum=%u\n\tversion=%04x\n\tchunks=%u\n\tfiles=%u\n\tflags=%06o\n\tid=%u\n\tnumber=%u", ap->name, hdr.hdrsum, hdr.size, hdr.chunksum, hdr.fileoff, hdr.datasum, hdr.version, hdr.chunks, hdr.files, hdr.flags, hdr.id, hdr.number));
				if (!(tp = sfstropen()))
					error(ERROR_SYSTEM|3, "out of space [cab type]");
				k = 1 << CAB_TYPE_NONE;
				for (i = 0; i < hdr.chunks; i++)
				{
					swapmem(SWAPOP(3), &cab->chunk[i].offset, &cab->chunk[i].offset, sizeof(cab->chunk[i].offset));
					swapmem(SWAPOP(1), &cab->chunk[i].blocks, &cab->chunk[i].blocks, sizeof(cab->chunk[i].blocks));
					swapmem(SWAPOP(1), &cab->chunk[i].compress, &cab->chunk[i].compress, sizeof(cab->chunk[i].compress));
					if (!(k & (1 << (i2 = CAB_TYPE(cab->chunk[i].compress)))))
					{
						k |= (1 << i2);
						if (sfstrtell(tp))
							sfputc(tp, ' ');
						switch (i2)
						{
						case CAB_TYPE_MSZIP:
							sfputr(tp, "mszip", -1);
							break;
						case CAB_TYPE_QUANTUM:
							sfputr(tp, "quantum", -1);
							break;
						case CAB_TYPE_LZX:
							sfputr(tp, "lzx", -1);
							break;
						default:
							sfprintf(tp, "COMPRESS=%d", i2);
							break;
						}
						if (i2 = CAB_TYPE_LEVEL(cab->chunk[i].compress))
							sfprintf(tp, "-%d", i2);
						if (i2 = CAB_TYPE_WINDOW(cab->chunk[i].compress))
							sfprintf(tp, ":%d", i2);
					}
					message((-1, "cab chunk %02d offset=%011u blocks=%02u compress=<%d,%d,%d>", i, cab->chunk[i].offset, cab->chunk[i].blocks, CAB_TYPE(cab->chunk[i].compress), CAB_TYPE_LEVEL(cab->chunk[i].compress), CAB_TYPE_WINDOW(cab->chunk[i].compress)));
				}
				s = sfstruse(tp);
				if (*s && !(cab->format = strdup(s)))
					error(ERROR_SYSTEM|3, "out of space [cab type]");
				sfstrclose(tp);
				for (i = 0; i < hdr.files; i++)
				{
					if (bread(ap, &cab->file[i], (off_t)sizeof(Cabentry_t), (off_t)sizeof(Cabentry_t), 1) <= 0)
						goto cab_error;
					swapmem(SWAPOP(3), &cab->file[i].entry.size, &cab->file[i].entry.size, (char*)&cab->file[i].entry.chunk - (char*)&cab->file[i].entry.size);
					swapmem(SWAPOP(1), &cab->file[i].entry.chunk, &cab->file[i].entry.chunk, (char*)(&cab->file[i].entry + 1) - (char*)&cab->file[i].entry.chunk);
					for (s = state.tmp.buffer; bread(ap, s, 1, 1, 1) > 0 && *s; s++)
						if (*s == '\\')
							*s = '/';
					if (!(cab->file[i].name = strdup(state.tmp.buffer)))
						error(ERROR_SYSTEM|3, "out of space [cab entry]");
					num = (cab->file[i].entry.date << 16) | cab->file[i].entry.time;
					memset(&tm, 0, sizeof(tm));
					tm.tm_year = ((num>>25)&0377) + 80;
					tm.tm_mon = ((num>>21)&017) - 1;
					tm.tm_mday = ((num>>16)&037);
					tm.tm_hour = ((num>>11)&037);
					tm.tm_min = ((num>>5)&077);
					tm.tm_sec = ((num<<1)&037);
					cab->file[i].date = tmtime(&tm, TM_LOCALZONE);
					switch (cab->file[i].entry.chunk)
					{
					case CAB_FILE_CONT:
					case CAB_FILE_BOTH:
						cab->file[i].entry.chunk = 0;
						cab->file[i].append = 1;
						break;
					case CAB_FILE_SPAN:
						cab->file[i].entry.chunk = cab->header.chunks - 1;
						break;
					}
					message((-1, "cab [%04d] size=%08u chunk=%02d offset=%08u %s %s%s", i, cab->file[i].entry.size, cab->file[i].entry.chunk, cab->file[i].entry.offset, fmttime("%l", cab->file[i].date), cab->file[i].name, cab->file[i].append ? " APPEND" : ""));
				}
				checkdelta = 0;
			}
			if ((i = cab->index++) >= cab->header.files)
				return 0;
			f->linkname = 0;
			f->name = cab->file[i].name;
			f->st->st_dev = 0;
			f->st->st_ino = 0;
			f->st->st_mode = X_IFREG|X_IRUSR|X_IRGRP|X_IROTH;
			if (!(cab->file[i].entry.attr & CAB_ATTR_READONLY))
				f->st->st_mode |= X_IWUSR;
			if ((cab->file[i].entry.attr & CAB_ATTR_EXECUTE) || strmatch(cab->file[i].name, "*.([Cc][Oo][Mm]|[Bb][Aa][Tt]|[Ee][Xx][Ee]|[BbCcKk][Ss][Hh]|[Pp][Ll]|[Ss][Hh])"))
				f->st->st_mode |= X_IXUSR|X_IXGRP|X_IXOTH;
			f->st->st_uid = state.uid;
			f->st->st_gid = state.gid;
			f->st->st_nlink = 1;
			IDEVICE(f->st, 0);
			f->st->st_mtime = f->st->st_ctime = f->st->st_atime = cab->file[i].date;
			f->st->st_size = cab->file[i].entry.size;
			goto found;
		}
		vdb_eof:
			ap->io.eof = 0;
			lseek(ap->io.fd, (off_t)0, SEEK_END);
			bflushin(ap);
			return 0;
		default:
			error(PANIC, "%s: incomplete input format implementation", format[ap->format].name);
			break;
		}
		if (ap->entry == 1)
		{
			if (ap->expected >= 0)
			{
				if (!state.keepgoing)
					error(3, "%s: unknown input format -- %s expected", ap->name, format[ap->expected].name);
				goto skip;
			}
			for (;;) switch (ap->format)
			{
			case BINARY:
				ap->format = USTAR;
				goto again;
			case CPIO:
				ap->format = BINARY;
				ap->swap = 0;
				if (bread(ap, &binary_header.magic, (off_t)0, (off_t)sizeof(binary_header.magic), 0) <= 0) break;
				bunread(ap, &binary_header.magic, sizeof(binary_header.magic));
				if (binary_header.magic == CPIO_MAGIC) goto again;
				ap->swap++;
				magic = binary_header.magic;
				swapmem(ap->swap, &magic, &magic, sizeof(magic));
				message((-1, "binary_header.magic=0%05o swap(BYTE)=0%05o", (unsigned short)binary_header.magic, (unsigned short)magic));
				if (magic == CPIO_MAGIC)
				{
					message((-1, "swapping %s header bytes", format[ap->format].name));
					goto again;
				}
				break;
			case PAX:
			case TAR:
			case USTAR:
				ap->format = ASC;
				goto again;
			case ASC:
			case ASCHK:
				ap->format = VDB;
				goto again;
			case VDB:
				ap->format = RPM;
				goto again;
			case RPM:
				ap->format = ZIP;
				goto again;
			case ZIP:
				ap->format = CAB;
				goto again;
			case CAB:
				if (ap->entries > 0 && ap->volume > 1)
				{
					if (--ap->volume > 1) error(1, "junk data after volume %d", ap->volume);
					finish(0);
				}
				if (!state.keepgoing)
					error(3, "%s: unknown input format", ap->name);
				ap->format = IN_DEFAULT;
				goto skip;
			}
		}
	skip:
		if (ap->io.eof) return 0;
		i = 3;
		if (state.keepgoing && bread(ap, ap->path.header, (off_t)0, (off_t)1, 0) > 0)
		{
			if (warned) continue;
			warned = 1;
			i = 1;
		}
		error(i, "%s: entry %d.%d is out of phase", ap->name, ap->volume, ap->entry);
		if (ap->entry > 1) ap->entry++;
	}
 found:
	if (checkdelta || ap->delta)
	{
		if (!f->st->st_size && !f->st->st_dev && !f->st->st_ino && !(f->st->st_mode & (X_IRWXU|X_IRWXG|X_IRWXO)) && strmatch(f->name, INFO_MATCH))
		{
			s = f->name;
			if (checkdelta)
			{
				Archive_t*	bp;

				checkdelta = 0;
				ordered = 0;
				i = 0;
				n = 0;
				if (*s++ == INFO_SEP)
				{
					if (strneq(s, ID, IDLEN) && (s = strchr(s, INFO_SEP)))
					{
					deltaverify:
						ordered = 0;
						switch (*++s)
						{
						case TYPE_COMPRESS:
							n = COMPRESS;
							break;
						case TYPE_DELTA:
							n = DELTA;
							break;
						default:
							error(3, "type %c encoding not supported", *s);
						}
						if (*++s == INFO_SEP)
						{
							if (t = strchr(++s, INFO_SEP)) *t++ = 0;
							for (i = DELTA; format[i].algorithm; i++)
								if (streq(s, format[i].algorithm))
									break;
							if (!format[i].algorithm)
								error(3, "delta version %s not supported", s);

							/*
							 * [<INFO_SEP>[<OP>]<VAL>]* may appear here
							 */

							while ((s = t) && *s != INFO_SEP)
							{
								if (t = strchr(s, INFO_SEP)) *t++ = 0;
								switch (*s++)
								{
								case INFO_ORDERED:
									ordered = 1;
									break;
								}
							}
						}
					}
					else
					{
						if (s = strchr(f->name + 3, INFO_SEP)) *s = 0;
						error(1, "unknown %s header ignored", f->name + 3);
						goto again;
					}
				}
				else if (streq(f->name, "DELTA!!!"))
				{
					n = DELTA;
					i = DELTA_88;
				}
				if (n)
				{
					if (n == DELTA && ap->parent)
						error(3, "%s: %s: base archive cannot be a delta", ap->parent->name, ap->name);
					initdelta(ap);
					ap->delta->format = n;
					ap->delta->version = i;
					if (bp = ap->delta->base)
					{
						if (i == DELTA_88)
							bp->checksum = bp->old.checksum;
						message((-5, "checkdelta: %s size=%ld:%ld LO=%d:%d HI=%d:%d", format[ap->delta->version].name, f->st->st_mtime, bp->size, DELTA_LO(f->st->st_uid), DELTA_LO(bp->checksum), DELTA_LO(f->st->st_gid), DELTA_HI(bp->checksum)));
						if (n == DELTA)
						{
							if (!ordered)
							{
								if (state.ordered)
									error(3, "%s: delta archive not ordered", ap->name);
								if (f->st->st_mtime != bp->size)
									error(3, "%s: %s: base archive size mismatch", ap->name, bp->name);
							}
							if (DELTA_LO(f->st->st_uid) != DELTA_LO(bp->checksum) || DELTA_LO(f->st->st_gid) != DELTA_HI(bp->checksum))
								error(1, "%s: %s: base archive checksum mismatch", ap->name, bp->name);
						}
					}
					else if (n == DELTA)
					{
						error(state.list ? 1 : 3, "%s: base archive must be specified", ap->name);
						deltabase(ap);
						ap->delta->format = COMPRESS;
					}
					if (ap->sum <= 0)
						ap->sum++;
					goto again;
				}
				else error(1, "%s: %s: unknown control header treated as regular file", ap->name, f->name);
			}
			else if (*s++ == INFO_SEP && strneq(s, ID, IDLEN) && *(s + IDLEN) == INFO_SEP)
			{
				getdeltaheader(ap, f);
				setinfo(ap, f);
				goto again;
			}
		}
		if (checkdelta && ap->delta)
			ap->delta->format = DELTA_PATCH;
	}
	ap->entries++;
	getkeysize(ap, f, OPT_size, &f->st->st_size);
	getkeytime(ap, f, OPT_atime, &f->st->st_atime);
	getkeytime(ap, f, OPT_ctime, &f->st->st_ctime);
	getkeytime(ap, f, OPT_mtime, &f->st->st_mtime);
	getkeyname(ap, f, OPT_gname, &f->gidname);
	getkeyname(ap, f, OPT_path, &f->name);
	getkeyname(ap, f, OPT_linkpath, &f->linkname);
	getkeyname(ap, f, OPT_uname, &f->uidname);
	if (f->name != ap->path.header)
		f->name = strcpy(ap->path.header, f->name);
	for (i = strlen(f->name); i > 2 && *(f->name + i - 1) == '/'; i--);
	*(f->name + i) = 0;
	f->path = strcpy(ap->path.name, f->name);
	pathcanon(f->name, 0);
	f->name = map(f->name);
	f->namesize = strlen(f->name) + 1;
	if (f->linkname)
	{
		pathcanon(f->linkname, 0);
		if (!(state.ftwflags & FTW_PHYSICAL))
			f->linkname = map(f->linkname);
		f->linknamesize = strlen(f->linkname) + 1;
	}
	else f->linknamesize = 0;
	if (f->uidname || f->gidname) setidnames(f);
	f->type = X_ITYPE(f->st->st_mode);
	f->perm = modei(f->st->st_mode);
	f->ro = ropath(f->name);
	getdeltaheader(ap, f);
#if DEBUG
	if (error_info.trace)
	{
		s = &state.tmp.buffer[0];
		if (f->record.format) sfsprintf(s, state.tmp.buffersize, " [%c,%d,%d]", f->record.format, state.blocksize, state.record.size);
		else *s = 0;
		message((-1, "archive=%s path=%s name=%s entry=%d.%d size=%I*d delta=%c%s", ap->name, f->path, f->name, ap->volume, ap->entry, sizeof(f->st->st_size), f->st->st_size, f->delta.op ? f->delta.op : DELTA_nop, s));
	}
#endif
	if (ap->entry == 1)
	{
		/*
		 * some tar implementations write garbage after
		 * the 2 null blocks that sometimes contain
		 * valid headers that were previously output
		 * but are still in the output buffer
		 */

		if ((ap->format == PAX || ap->format == TAR || ap->format == USTAR) && ap->volume > 1)
			for (i = 0; i < elementsof(tar_last); i++)
				if (!memcmp((char*)&tar_last[i], (char*)&tar_header, sizeof(tar_header)))
				{
					if (--ap->volume == 1) error(1, "junk data after volume %d", ap->volume);
					return 0;
				}
		if (ap->parent)
		{
			if (ap->io.blocked)
				error(3, "%s: blocked base archives are not supported", ap->delta->base);
			switch (ap->format)
			{
			case ALAR:
			case IBMAR:
#if SAVESET
			case SAVESET:
#endif
				error(3, "%s: %s format base archives not supported", ap->delta->base, format[ap->format].name);
			}
		}
		if (state.summary && state.verbose)
		{
			if (ap->io.blok) sfprintf(sfstderr, "BLOK ");
			if (ap->parent) sfprintf(sfstderr, "%s base %s", ap->parent->name, ap->name);
			else sfprintf(sfstderr, "%s volume %d", ap->name, ap->volume);
			if (state.id.volume[0])
				sfprintf(sfstderr, " label %s", state.id.volume);
			sfprintf(sfstderr, " in");
			if (ap->data && (s = *(char**)ap->data))
				sfprintf(sfstderr, " %s", s);
			if (ap->package)
				sfprintf(sfstderr, " %s", ap->package);
			if (ap->compress)
				sfprintf(sfstderr, " %s", format[ap->compress].name);
			if (ap->delta && ap->delta->version)
				sfprintf(sfstderr, " %s", format[ap->delta->version].name);
			sfprintf(sfstderr, " %s format", format[ap->format].name);
			if (error_info.trace)
			{
				if (*state.id.format) sfprintf(sfstderr, " %s", state.id.format);
				if (*state.id.implementation) sfprintf(sfstderr, " implementation %s", state.id.implementation);
			}
			sfprintf(sfstderr, "\n");
		}
	}
	if (ap->format == PAX || ap->format == TAR || ap->format == USTAR)
	{
		if (++tar_last_index >= elementsof(tar_last)) tar_last_index = 0;
		tar_last[tar_last_index] = tar_header;
	}
	if (!ap->delta || !ap->delta->trailer)
		ap->memsum = 0;
	ap->old.memsum = 0;
	ap->section = SECTION_DATA;
	return 1;
}

/*
 * output extended header keyword assignment
 */

static void
putkey(Sfio_t* sp, Sfio_t* tp, const char* name, int op, const char* value)
{
	register const char*	s;
	register int		c;
	register int		n;

	sfprintf(tp, " %s", name);
	if (op) sfputc(tp, op);
	sfputc(tp, '=');
	n = 0;
	s = value;
	for (;;)
	{
		switch (*s++)
		{
		case 0:
			break;
		case '"':
			n |= 04;
			continue;
		case '\'':
			n |= 02;
			continue;
		case '\\':
		case ' ':
		case '\t':
		case '\n':
			n |= 01;
			continue;
		default:
			continue;
		}
		break;
	}
	s = value;
	if (n)
	{
		n = (n & 06) == 04 ? '\'' : '"';
		sfputc(tp, n);
		while (c = *s++)
		{
			if (c == n || c == '\\')
				sfputc(tp, '\\');
			sfputc(tp, c);
		}
		sfputc(tp, n);
		sfputc(tp, '\n');
	}
	else sfputr(tp, s, '\n');
	n = sfstrtell(tp) + 1;
	if (n >= 10000)
		n += 4;
	else if (n >= 1000)
		n += 3;
	else if (n >= 100)
		n += 2;
	else if (n >= 10)
		n += 1;
	sfprintf(sp, "%d%s", n, sfstruse(tp));
}

/*
 * write next archive entry header
 */

void
putheader(register Archive_t* ap, register File_t* f)
{
	register char*	s;
	register off_t	n;
	int		c;
	int		q;

	ap->section = SECTION_CONTROL;
	setdeltaheader(ap, f);
	if (state.install.sp)
	{
		c = 0;
		if (f->st->st_gid != state.gid && ((f->st->st_mode & S_ISGID) || (f->st->st_mode & S_IRGRP) && !(f->st->st_mode & S_IROTH) || (f->st->st_mode & S_IXGRP) && !(f->st->st_mode & S_IXOTH)))
		{
			sfprintf(state.install.sp, "chgrp %s %s\n", fmtgid(f->st->st_gid), f->name);
			c = 1;
		}
		if (f->st->st_uid != state.uid && ((f->st->st_mode & S_ISUID) || (f->st->st_mode & S_IRUSR) && !(f->st->st_mode & (S_IRGRP|S_IROTH)) || (f->st->st_mode & S_IXUSR) && !(f->st->st_mode & (S_IXGRP|S_IXOTH))))
		{
			sfprintf(state.install.sp, "chown %s %s\n", fmtuid(f->st->st_uid), f->name);
			c = 1;
		}
		if (c || (f->st->st_mode & (S_ISUID|S_ISGID)))
			sfprintf(state.install.sp, "chmod %04o %s\n", modex(f->st->st_mode & S_IPERM), f->name);
	}
	if (!f->extended)
	{
		ap->entry++;
		if (ap->format == PAX)
		{
			unsigned long		t;
			char*			base;
			char*			next;
			Buffer_t*		bp;
			File_t			h;
			Hash_position_t*	pos;
			Option_t*		op;
			Sfio_t*			hp;
			Sfio_t*			tp;
			Value_t*		vp;
			char			num[64];

			if (!(hp = sfstropen()) || !(tp = sfstropen()))
				error(3, "out of space [extended]");
			if (pos = hashscan(state.options, 0))
			{
				while (hashnext(pos))
				{
					op = (Option_t*)pos->bucket->value;
					if ((op->flags & (OPT_HEADER|OPT_READONLY)) == OPT_HEADER && op->name == pos->bucket->name && (!op->level || op->level >= (ap->entry == 1 ? 1 : 3) && op->level < 5))
					{
						vp = (op->level >= 3) ? &op->temp : &op->perm;
						s = vp->string;
						switch (op->index)
						{
						case OPT_atime:
						case OPT_ctime:
						case OPT_mtime:
							if (op->flags & OPT_SET) t = vp->number;
							else switch (op->index)
							{
							case OPT_atime:
								t = f->st->st_atime;
								break;
							case OPT_ctime:
								t = f->st->st_ctime;
								break;
							case OPT_mtime:
								/* already in tar_header */
								continue;
							}
							sfsprintf(s = num, sizeof(num), "%lu.0", t);
							break;
						case OPT_gname:
							if (op->flags & OPT_SET)
							{
								f->gidname = s;
								s = 0;
							}
							getidnames(f);
							if (strlen(f->gidname) >= sizeof(tar_header.gname) || !portable(f->gidname))
								s = f->gidname;
							break;
						case OPT_size:
							if (f->st->st_size > 0x7fffffff)
								sfsprintf(s = num, sizeof(num), "%I*u", sizeof(f->st->st_size), f->st->st_size);
							else s = 0;
							break;
						case OPT_uname:
							if (op->flags & OPT_SET)
							{
								f->uidname = s;
								s = 0;
							}
							getidnames(f);
							if (strlen(f->uidname) >= sizeof(tar_header.uname) || !portable(f->uidname))
								s = f->uidname;
							break;
						default:
							if (!s && (op->flags & OPT_SET))
								sfsprintf(s = num, sizeof(num), "%ld", vp->number);
							break;
						}
						if (s) putkey(hp, tp, op->name, !op->level || op->level >= 3 ? ':' : 0, s);
					}
				}
				hashdone(pos);
			}
			s = f->name;
			if (f->namesize > sizeof(tar_header.name) - (f->type == X_IFDIR))
				for (; s = strchr(s, '/'); s++)
				{
					if ((n = s - f->name) >= sizeof(tar_header.prefix))
					{
						s = 0;
						break;
					}
					if (f->namesize <= sizeof(tar_header.name) + n)
						break;
				}
			if (!s || !portable(f->name))
				putkey(hp, tp, options[OPT_path].name, ':', f->name);
			if (f->linktype != NOLINK && (f->linknamesize > sizeof(tar_header.linkname) || !portable(f->linkname)))
				putkey(hp, tp, options[OPT_linkpath].name, ':', f->linkname);
			if (n = sfstrtell(hp))
			{
				s = sfstruse(hp);
				listprintf(tp, ap, f, state.header.name);
				initfile(ap, &h, sfstruse(tp), X_IFREG|X_IRUSR|X_IWUSR|X_IRGRP|X_IROTH);
				h.extended = 1;
				h.st->st_gid = f->st->st_gid;
				h.st->st_uid = f->st->st_uid;
				h.st->st_mtime = time((time_t*)0);
				h.st->st_size = n;
				h.fd = setbuffer(0);
				bp = getbuffer(h.fd);
				base = bp->base;
				next = bp->next;
				bp->base = bp->next = s;
				fileout(ap, &h);
				bp->base = base;
				bp->next = next;
			}
			sfstrclose(hp);
			sfstrclose(tp);
		}
	}
	if (state.complete)
	{
		off_t	c = f->st->st_size;

		switch (ap->format)
		{
		case ALAR:
		case IBMAR:
			c += 4 * ALAR_HEADER;
			break;
		case ASC:
		case ASCHK:
			c += ASC_HEADER + f->namesize;
			break;
		case BINARY:
			c += BINARY_HEADER + f->namesize;
			c = roundof(c, 2);
			break;
		case CPIO:
			c += CPIO_HEADER + f->namesize;
			break;
		case PAX:
		case TAR:
		case USTAR:
			c += TAR_HEADER;
			c = roundof(c, BLOCKSIZE);
			break;
		case VDB:
			c += state.record.header ? state.record.headerlen : f->namesize;
			break;
		}
		if (ap->io.count + c > state.maxout)
		{
			if (c > state.maxout) error(1, "%s: too large to fit in one volume", f->name);
			else
			{
				state.complete = 0;
				putepilogue(ap);
				newio(ap, 0, 0);
				putprologue(ap);
				state.complete = 1;
			}
		}
	}
	switch (ap->format)
	{
	case ALAR:
	case IBMAR:
		putlabels(ap, f, "HDR");
		break;
	case BINARY:
		binary_header.magic = CPIO_MAGIC;
		binary_header.namesize = f->namesize;
		cpio_short(binary_header.size, f->st->st_size + (f->type == X_IFLNK ? f->linknamesize : 0));
		binary_header.dev = f->st->st_dev;
		binary_header.ino = f->st->st_ino;
		binary_header.mode = f->st->st_mode;
		binary_header.uid = f->st->st_uid;
		binary_header.gid = f->st->st_gid;
		binary_header.links = f->st->st_nlink;
		binary_header.rdev = idevice(f->st);
		if (binary_header.rdev != idevice(f->st)) error(1, "%s: special device numbers truncated", f->name);
		cpio_short(binary_header.mtime, (long)f->st->st_mtime);
		bwrite(ap, &binary_header, BINARY_HEADER);
		bwrite(ap, f->name, f->namesize);
		if (n = (BINARY_HEADER + f->namesize) % BINARY_ALIGN)
			while (n++ < BINARY_ALIGN) bwrite(ap, "", 1);
		if (f->type == X_IFLNK)
		{
		cpio_link:
			if (streq(f->name, f->linkname))
				error(1, "%s: symbolic link loops to self", f->name);
			bwrite(ap, f->linkname, f->linknamesize);
			putdeltaheader(ap, f);
		}
		break;
	case CPIO:
#if CPIO_EXTENDED
		if (!f->skip)
		{
			getidnames(f);
			addxopstr(ap, 'U', f->uidname);
			addxopstr(ap, 'G', f->gidname);
			if (CPIO_TRUNCATE(idevice(f->st)) != idevice(f->st))
				addxopnum(ap, 'd', (Sflong_t)idevice(f->st));
#if NUMBER_EVEN_THOUGH_NAME
			if (CPIO_TRUNCATE(f->st->st_gid) != f->st->st_gid)
				addxopnum(ap, 'g', (Sflong_t)f->st->st_gid);
			if (CPIO_TRUNCATE(f->st->st_uid) != f->st->st_uid)
				addxopnum(ap, 'u', (Sflong_t)f->st->st_uid);
#endif
			if (f->st->st_size > 0x7fffffff)
				addxopnum(ap, 's', (Sflong_t)f->st->st_size);
			setxops(ap, f);
		}
#else
		if (CPIO_TRUNCATE(idevice(f->st)) != idevice(f->st)) error(1, "%s: special device number truncated", f->name);
		if (CPIO_TRUNCATE(f->st->st_gid) != f->st->st_gid) error(1, "%s: gid number truncated", f->name);
		if (CPIO_TRUNCATE(f->st->st_uid) != f->st->st_uid) error(1, "%s: uid number truncated", f->name);
#endif
		sfsprintf(state.tmp.buffer, state.tmp.buffersize, "%0.6lo%0.6lo%0.6lo%0.6lo%0.6lo%0.6lo%0.6lo%0.6lo%0.11lo%0.6o%0.11lo",
			(long)CPIO_TRUNCATE(CPIO_MAGIC),
			(long)CPIO_TRUNCATE(f->st->st_dev),
			(long)CPIO_TRUNCATE(f->st->st_ino),
			(long)CPIO_TRUNCATE(f->st->st_mode),
			(long)CPIO_TRUNCATE(f->st->st_uid),
			(long)CPIO_TRUNCATE(f->st->st_gid),
			(long)CPIO_TRUNCATE(f->st->st_nlink),
			(long)CPIO_TRUNCATE(idevice(f->st)),
			(long)f->st->st_mtime,
			(long)f->namesize,
			(long)(f->st->st_size + (f->type == X_IFLNK ? f->linknamesize : 0)));
		bwrite(ap, state.tmp.buffer, CPIO_HEADER);
#if CPIO_EXTENDED
		putxops(ap, f);
#else
		bwrite(ap, f->name, f->namesize);
#endif
		if (f->type == X_IFLNK) goto cpio_link;
		break;
	case ASC:
		f->checksum = 0;
		/*FALLTHROUGH*/
	case ASCHK:
		sfsprintf(state.tmp.buffer, state.tmp.buffersize, "%0.6lo%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx%0.8lx",
			(long)(ap->format == ASC ? ASC_MAGIC : ASCHK_MAGIC),
			(long)f->st->st_ino,
			(long)f->st->st_mode,
			(long)f->st->st_uid,
			(long)f->st->st_gid,
			(long)f->st->st_nlink,
			(long)f->st->st_mtime,
			(long)f->st->st_size + (long)f->linknamesize,
			(long)major(f->st->st_dev),
			(long)minor(f->st->st_dev),
			(long)major(idevice(f->st)),
			(long)minor(idevice(f->st)),
			(long)f->namesize,
			f->checksum);
		bwrite(ap, state.tmp.buffer, ASC_HEADER);
		bwrite(ap, f->name, f->namesize);
		if (n = (ASC_HEADER + f->namesize) % ASC_ALIGN)
			while (n++ < ASC_ALIGN) bwrite(ap, "", 1);
		if (f->type == X_IFLNK) goto cpio_link;
		break;
	case PAX:
	case TAR:
	case USTAR:
		memzero(tar_block, TAR_HEADER);
		s = f->name;
		if (f->namesize > sizeof(tar_header.name) - (f->type == X_IFDIR))
			for (;; s++)
			{
				if (!(s = strchr(s, '/')))
				{
					s = f->name;
					break;
				}
				if ((f->namesize - (s - f->name)) <= sizeof(tar_header.name))
				{
					*s = 0;
					sfsprintf(tar_header.prefix, sizeof(tar_header.prefix), "%-.*s", sizeof(tar_header.prefix), f->name);
					*s++ = '/';
					break;
				}
			}
		sfsprintf(tar_header.name, sizeof(tar_header.name), "%s%s", s, f->type == X_IFDIR ? "/" : "");
		if (f->extended) tar_header.typeflag = EXTTYPE;
		else switch (f->linktype)
		{
		case HARDLINK:
			tar_header.typeflag = LNKTYPE;
		linked:
			sfsprintf(tar_header.linkname, sizeof(tar_header.linkname), "%s", f->linkname);
			break;
		case SOFTLINK:
			tar_header.typeflag = SYMTYPE;
			goto linked;
		default:
			switch (ap->format == TAR ? X_IFREG : f->type)
			{
			case X_IFCHR:
				tar_header.typeflag = CHRTYPE;
			special:
				sfsprintf(tar_header.devmajor, sizeof(tar_header.devmajor), "%*o ", sizeof(tar_header.devmajor) - 2, major(idevice(f->st)));
				sfsprintf(tar_header.devminor, sizeof(tar_header.devminor), "%*o ", sizeof(tar_header.devminor) - 2, minor(idevice(f->st)));
				break;
			case X_IFBLK:
				tar_header.typeflag = BLKTYPE;
				goto special;
			case X_IFDIR:
				tar_header.typeflag = DIRTYPE;
				break;
			case X_IFIFO:
				tar_header.typeflag = FIFOTYPE;
				break;
			case X_IFSOCK:
				tar_header.typeflag = SOKTYPE;
				goto special;
			default:
				if (!f->skip)
					error(1, "%s: unknown file type 0%03o -- assuming regular file", f->name, f->type >> 12);
				/*FALLTHROUGH*/
			case X_IFREG:
				tar_header.typeflag = REGTYPE;
				break;
			}
			break;
		}
		sfsprintf(tar_header.mode, sizeof(tar_header.mode), "%*o ", sizeof(tar_header.mode) - 2, f->st->st_mode & X_IPERM);
		sfsprintf(tar_header.uid, sizeof(tar_header.uid), "%*o ", sizeof(tar_header.uid) - 2, f->st->st_uid);
		sfsprintf(tar_header.gid, sizeof(tar_header.gid), "%*o ", sizeof(tar_header.gid) - 2, f->st->st_gid);
		sfsprintf(tar_header.size, sizeof(tar_header.size), "%*lo ", sizeof(tar_header.size) - 1, (long)f->st->st_size);
		sfsprintf(tar_header.mtime, sizeof(tar_header.mtime), "%*lo ", sizeof(tar_header.mtime) - 2, f->st->st_mtime);
		if (ap->format != TAR)
		{
			strncpy(tar_header.magic, TMAGIC, sizeof(tar_header.magic));
			strncpy(tar_header.version, TVERSION, sizeof(tar_header.version));
			getidnames(f);
			strcpy(tar_header.uname, f->uidname);
			strcpy(tar_header.gname, f->gidname);
		}
		sfsprintf(tar_header.chksum, sizeof(tar_header.chksum), "%*lo ", sizeof(tar_header.chksum) - 1, tar_checksum());
		bwrite(ap, tar_block, TAR_HEADER);
		break;
	case VDB:
		if (state.record.header) bwrite(ap, state.record.header, state.record.headerlen);
		else
		{
			f->name[f->namesize - 1] = '\n';
			bwrite(ap, f->name, f->namesize);
			f->name[f->namesize - 1] = 0;
		}
		if (!(c = state.record.delimiter))
		{
			c = VDB_DELIMITER;
			if (f->fd >= 0)
			{
				n = read(f->fd, tar_block, TAR_HEADER);
				if (n > 0) lseek(f->fd, -n, SEEK_CUR);
			}
#if 0
			else if ((n = bread(ap, tar_block, (off_t)0, (off_t)TAR_HEADER, 1)) > 0)
				bunread(ap, tar_block, n);
#else
			else
				n = 0;
#endif
			if (n > 0)
			{
				int		dp;
				int		ds;
				int		mp = 0;
				int		ms = 0;
				unsigned char	hit[UCHAR_MAX + 1];

				/*
				 * to determine the delimiter, if any
				 */

				memzero(hit, sizeof(hit));
				while (--n > 0)
					hit[*((unsigned char*)tar_block + n)]++;
				for (n = 0; n <= UCHAR_MAX; n++)
					if (n == '_' || n == '/' || n == '.' || n == '\n' || n == '\\')
						/* nop */;
					else if (isspace(n))
					{
						if ((int)hit[n] > ms)
						{
							ms = hit[n];
							ds = n;
						}
					}
					else if ((int)hit[n] > mp && isprint(n) && !isalnum(n))
					{
						mp = hit[n];
						dp = n;
					}
				if (mp) c = dp;
				else if (ms) c = ds;
			}
		}
		q = (c == '=') ? ':' : '=';
		sfprintf(state.vdb.directory, "%c%s%c%I*u%c%I*u%c%s%c%I*u%c%s%c0%o\n", c, f->name, c, sizeof(ap->io.offset), ap->io.offset + ap->io.count, c, sizeof(f->st->st_size), f->st->st_size, c, VDB_DATE, q, sizeof(f->st->st_mtime), f->st->st_mtime, c, VDB_MODE, q, modex(f->st->st_mode & X_IPERM));
		break;
	}
	putdeltaheader(ap, f);
	if (state.checksum.sum)
		suminit(state.checksum.sum);
	ap->section = SECTION_DATA;
}

/*
 * read entry trailer
 */

void
gettrailer(register Archive_t* ap, File_t* f)
{
	register off_t	n;

	NoP(f);
	ap->section = SECTION_CONTROL;
	if (ap->sum-- > 0)
	{
		ap->checksum ^= ap->memsum;
		ap->old.checksum ^= ap->old.memsum;
	}
	getdeltatrailer(ap, f);
	if ((n = format[ap->format].align) && (n = roundof(ap->io.count, n) - ap->io.count))
		bread(ap, state.tmp.buffer, (off_t)0, n, 1);
	ap->memsum = 0;
	ap->old.memsum = 0;
}

/*
 * write entry trailer
 */

void
puttrailer(register Archive_t* ap, register File_t* f)
{
	register int	n;

	if (state.checksum.sum)
	{
		sumdone(state.checksum.sum);
		if (f->link)
		{
			if (!f->link->checksum)
			{
				sumprint(state.checksum.sum, state.tmp.str, 0);
				if (!(f->link->checksum = strdup(sfstruse(state.tmp.str))))
					error(ERROR_SYSTEM|3, "out of space [link checksum]");
			}
			sfputr(state.checksum.sp, f->link->checksum, -1);
		}
		else
			sumprint(state.checksum.sum, state.checksum.sp, 0);
		sfprintf(state.checksum.sp, " %04o %s %s %s\n"
			, modex(f->st->st_mode & S_IPERM)
			, (f->st->st_uid != state.uid && ((f->st->st_mode & S_ISUID) || (f->st->st_mode & S_IRUSR) && !(f->st->st_mode & (S_IRGRP|S_IROTH)) || (f->st->st_mode & S_IXUSR) && !(f->st->st_mode & (S_IXGRP|S_IXOTH)))) ? fmtuid(f->st->st_uid) : "-"
			, (f->st->st_gid != state.gid && ((f->st->st_mode & S_ISGID) || (f->st->st_mode & S_IRGRP) && !(f->st->st_mode & S_IROTH) || (f->st->st_mode & S_IXGRP) && !(f->st->st_mode & S_IXOTH))) ? fmtgid(f->st->st_gid) : "-"
			, f->name
			);
	}
	ap->section = SECTION_CONTROL;
	putdeltatrailer(ap, f);
	switch (ap->format)
	{
	case ALAR:
	case IBMAR:
		putlabels(ap, f, "EOF");
		break;
	case VDB:
		if (state.record.trailer)
			bwrite(ap, state.record.trailer, state.record.trailerlen);
		break;
	}
	if ((n = format[ap->format].align) && (n = roundof(ap->io.count, n) - ap->io.count))
	{
		memzero(state.tmp.buffer, n);
		bwrite(ap, state.tmp.buffer, n);
	}
	listentry(f);
}

/*
 * return length of next label
 * variable length labels have label number > 3 and Vnnnn at position 5
 * where nnnn is the decimal length of the entire label
 * nnnn may be < ALAR_HEADER but label block must be >= ALAR_HEADER
 * 0 returned at end of label group
 */

int
getlabel(register Archive_t* ap, register File_t* f)
{
	register int	c;
	register int	n;

	static char	last[5];
	static int	done;

	if (done || (c = bread(ap, alar_header, (off_t)ALAR_HEADER, (off_t)ALAR_LABEL, 0)) < ALAR_HEADER) return *last = done = c = 0;
	if (alar_header[4] == 'V' && ((n = getlabnum(alar_header, 4, 1, 10)) < 1 || n > 3) && (n = getlabnum(alar_header, 6, 4, 10)) != c)
	{
		if ((c = n - c) > 0)
		{
			if (ap->io.blocked || bread(ap, alar_header + ALAR_HEADER, (off_t)0, (off_t)c, 1) != c)
			{
				c = ALAR_HEADER;
				error(2, "%s: %-*.*s: variable length label record too short", f->name, c, c, alar_header);
			}
			else c = n;
		}
		else if (n <= ALAR_VARHDR) c = ALAR_VARHDR;
		else c = n;
	}
	if (!ap->io.blocked && !*last && alar_header[3] == '2' && (strneq(alar_header, "HDR", 3) || strneq(alar_header, "EOF", 3) || strneq(alar_header, "EOV", 3)))
		getlabstr(alar_header, 26, 4, last);
	if (*last && strneq(alar_header, last, 4)) done = 1;
	message((-4, "label: %-*.*s", c, c, alar_header));
	return c;
}

/*
 * output file HDR/EOF/EOV labels
 */

void
putlabels(register Archive_t* ap, register File_t* f, char* type)
{
	struct tm*	tm;

	static int	section = 1;
	static int	sequence;

	switch (*type)
	{
	case 'E':
		bwrite(ap, alar_header, 0);
		break;
	case 'H':
		sequence++;
		break;
	}
	tm = gmtime(&f->st->st_mtime);
	sfsprintf(alar_header, sizeof(alar_header), "%s1%-17.17s000001%04d%04d000100 %02d%03d 00000 %06d%-6.6sD%-7.7s       ", type, f->id, section, sequence, tm->tm_year, tm->tm_yday, f->record.blocks, state.id.format, state.id.implementation);
	bwrite(ap, alar_header, ALAR_HEADER);
	sfsprintf(alar_header, sizeof(alar_header), "%s2%c%05d%05d%010d%s%c                     00                            ", type, state.record.format, state.blocksize, state.record.size, f->st->st_size, type, '2');
	bwrite(ap, alar_header, ALAR_HEADER);
	bwrite(ap, alar_header, 0);
	if (streq(type, "EOV"))
	{
		section++;
		sequence = 0;
	}
	else section = 1;
}

#ifdef SAVESET

/*
 * get next saveset record
 * if header!=0 then all records skipped until REC_file found
 * otherwise REC_vbn cause non-zero return
 * 0 returned for no record match
 */

int
getsaveset(register Archive_t* ap, register File_t* f, int header)
{
	register char*	p;
	register char*	s;
	char*		t;
	int		i;
	long		n;

	for (;;)
	{
		state.saveset.bp += state.saveset.lastsize;
		while (state.saveset.bp >= state.saveset.block + state.blocksize)
		{
			state.saveset.bp = state.saveset.block;
			state.saveset.lastsize = 0;
			if (bread(ap, state.saveset.bp, (off_t)0, (off_t)state.blocksize, 0) <= 0)
			{
				ap->format = ALAR;
				if (header) gettrailer(ap, f);
				return 0;
			}
			if (swapget(1, state.saveset.bp + BLKHDR_hdrsiz, 2) != BLKHDR_SIZE)
				error(3, "invalid %s format block header", format[ap->format].name);
			if (!(n = swapget(3, state.saveset.bp + BLKHDR_blksiz, 4)))
				state.saveset.bp += state.blocksize;
			else if (n != state.blocksize)
				error(3, "invalid %s format blocksize=%ld", format[ap->format].name, n);
			state.saveset.bp += BLKHDR_SIZE;
		}
		state.saveset.lastsize = swapget(1, state.saveset.bp + RECHDR_size, 2);
		i = swapget(1, state.saveset.bp + RECHDR_type, 2);
		state.saveset.bp += RECHDR_SIZE;
		message((-2, "record: type=%d size=%d", i, state.saveset.lastsize));
		if (i == REC_file)
		{
			if (header)
			{
				p = state.saveset.bp;
				if (swapget(1, p, 2) != FILHDR_MAGIC)
					error(3, "invalid %s format file header", format[ap->format].name);
				p += 2;
				i = swapget(1, p + FILHDR_size, 2);
				if (p + FILHDR_data + i > state.saveset.block + state.blocksize)
					error(3, "invalid %s format file attribute", format[ap->format].name);
				t = f->name;
				n = 0;
				for (s = p + FILHDR_data + 1; s < p + FILHDR_data + i; s++)
				{
					if (isupper(*s)) *t++ = tolower(*s);
					else if (n)
					{
						if (*s == ';') break;
						*t++ = *s;
					}
					else if (*s == ']')
					{
						n = 1;
						*t++ = '/';
					}
					else if (*s == '.') *t++ = '/';
					else *t++ = *s;
				}
				*t = 0;
				for (i = 0; i < 5; i++)
				{
					s = p + FILHDR_size;
					if ((p += FILHDR_SIZE + (long)swapget(1, s, 2)) > state.saveset.block + state.blocksize)
						error(3, "invalid %s format file attribute", format[ap->format].name);
				}
				state.saveset.recatt = *(p + FILHDR_data + FILATT_recfmt);
				state.saveset.recfmt = *(p + FILHDR_data + FILATT_recfmt);
				state.saveset.reclen = swapget(1, p + FILHDR_data + FILATT_reclen, 2);
				state.saveset.recvfc = swapget(1, p + FILHDR_data + FILATT_recvfc, 2);
				f->st->st_size = (long)(swapget(1, p + FILHDR_data + FILATT_blocks, 2) - 1) * BLOCKSIZE + (long)swapget(1, p + FILHDR_data + FILATT_frag, 2);
				for (; i < 15; i++)
				{
					s = p + FILHDR_size;
					if ((p += FILHDR_SIZE + (long)swapget(1, s, 2)) > state.saveset.block + state.blocksize)
						error(3, "invalid %s format file attribute", format[ap->format].name);
				}

				/*
				 * pure guesswork based on 100 nsec epoch
				 * 17-NOV-1858 00:00:00 GMT
				 */

				if ((n = swapget(3, p + FILHDR_data + 4, 4) - 7355000) < 0) n = 1;
				else n = (n << 8) + *(p + FILHDR_data + 3);
				f->st->st_mtime = (time_t)n;
				return 1;
			}
			state.saveset.bp -= RECHDR_SIZE;
			state.saveset.lastsize = 0;
			return 0;
		}
		else if (i == REC_vbn && !header) return 1;
	}
}

#endif
