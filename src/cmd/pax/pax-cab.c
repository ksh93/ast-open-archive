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
 * pax cabinet format
 */

#include "format.h"

#include <int.h>
#include <tm.h>
#include <vmalloc.h>

#define CAB_MAGIC		"MSCF"
#define CAB_VERSION		0x0103

#define CAB_SELF_MSZIP		0x00014db8
#define CAB_SELF_QUANTUM	0x00019d48

#define CAB_FLAG_HASPREV	0x0001
#define CAB_FLAG_HASNEXT	0x0002
#define CAB_FLAG_RESERVE	0x0004

#define CAB_ATTR_READONLY	0x0001
#define CAB_ATTR_HIDDEN		0x0002
#define CAB_ATTR_SYSTEM		0x0004
#define CAB_ATTR_VOLUME		0x0008
#define CAB_ATTR_DIRECTORY	0x0010
#define CAB_ATTR_ARCHIVE	0x0020
#define CAB_ATTR_EXECUTE	0x0040
#define CAB_ATTR_UTF		0x0080

#define CAB_TYPE(f)		((f)&0x000f)
#define CAB_TYPE_NONE		0x0000
#define CAB_TYPE_MSZIP		0x0001
#define CAB_TYPE_QUANTUM	0x0002
#define CAB_TYPE_LZX		0x0003

#define CAB_TYPE_LEVEL(f)	(((f)>>4)&0x0007)
#define CAB_TYPE_WINDOW(f)	(((f)>>8)&0x001f)

#define CAB_FILE_CONT		0xfffd
#define CAB_FILE_SPAN		0xfffe
#define CAB_FILE_BOTH		0xffff

typedef struct Cabheader_s		/* cab file header		*/
{
	char		magic[4];	/* magic string			*/
	unsigned int_4	hdrsum;		/* header checksum		*/
	unsigned int_4	size;		/* total file size		*/
	unsigned int_4	chunksum;	/* chunk checksum		*/
	unsigned int_4	fileoff;	/* first file entry offset	*/
	unsigned int_4	datasum;	/* file data checksum		*/
	unsigned int_2	version;	/* CAB_VERSION			*/
	unsigned int_2	chunks;		/* # chunks			*/
	unsigned int_2	files;		/* # files			*/
	unsigned int_2	flags;		/* CAB_FLAG_*			*/
	unsigned int_2	id;		/* cabinet set id		*/
	unsigned int_2	number;		/* cabinet number		*/
} Cabheader_t;

typedef struct Cabopt_s			/* cab optional header info	*/
{
	struct
	{
	char*		name;		/* cab file name		*/
	char*		disk;		/* disk id			*/
	}		prev,next;
} Cabopt_t;

typedef struct Cabchunk_s		/* cab chunk header		*/
{
	unsigned int_4	offset;		/* data offset			*/
	unsigned int_2	blocks;		/* block count			*/
	unsigned int_2	compress;	/* compression type		*/
} Cabchunk_t;

typedef struct Cabblock_s		/* chunk block header		*/
{
	unsigned int_4	checksum;	/* decompressed checksum	*/
	unsigned int_2	compressed;	/* compressed size		*/
	unsigned int_2	decompressed;	/* decompressed size		*/
} Cabblock_t;

typedef struct Cabentry_s		/* file entry header		*/
{
	unsigned int_4	size;		/* uncompressed data size	*/
	unsigned int_4	offset;		/* decompressed chunk offset	*/
	unsigned int_2	chunk;		/* chunk index			*/
	unsigned int_2	date;		/* dos date			*/
	unsigned int_2	time;		/* dos time			*/
	unsigned int_2	attr;		/* CAB_ATTR_*			*/
} Cabentry_t;

typedef struct Cabfile_s		/* file entry			*/
{
	Cabentry_t	entry;		/* entry header			*/
	char*		name;		/* file name			*/
	time_t		date;		/* mtime			*/
	int		append;		/* append this part to existing	*/
} Cabfile_t;

typedef struct Cab_s
{
	Cabheader_t	header;
	Cabopt_t	optional;
	Cabchunk_t*	chunk;
	Cabfile_t*	file;
	Vmalloc_t*	vm;
	char*		format;
	struct
	{
	unsigned int	header;
	unsigned int	chunk;
	unsigned int	block;
	}		reserved;
	int		index;
} Cab_t;

static int
cab_getprologue(Pax_t* pax, Format_t* fp, register Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	register Cab_t*		cab;
	register char*		s;
	Cabheader_t*		hdr;
	int			i;
	int			k;
	int_2			i2;
	long			num;
	Sfio_t*			tp;
	Vmalloc_t*		vm;
	Tm_t			tm;

	if (size < sizeof(*hdr))
		return 0;
	hdr = (Cabheader_t*)buf;
	if (memcmp(hdr->magic, CAB_MAGIC, sizeof(hdr->magic)))
		return 0;
	swapmem(SWAPOP(3), &hdr->hdrsum, &hdr->hdrsum, (char*)&hdr->version - (char*)&hdr->hdrsum);
	swapmem(SWAPOP(1), &hdr->version, &hdr->version, (char*)(hdr + 1) - (char*)&hdr->version);
	if (hdr->version != CAB_VERSION)
	{
		error(3, "%s: %s format version %04x not supported", ap->name, fp->name, hdr->version);
		return -1;
	}
	if (!(vm = vmopen(Vmdcheap, Vmbest, 0)))
		nospace();
	if (!(cab = vmnewof(vm, 0, Cab_t, 1, hdr->chunks * sizeof(Cabchunk_t) + hdr->files * sizeof(Cabfile_t))))
		nospace();
	cab->header = *hdr;
	cab->vm = vm;
	cab->chunk = (Cabchunk_t*)(cab + 1);
	cab->file = (Cabfile_t*)(cab->chunk + cab->header.chunks);
	ap->data = (void*)cab;
	if (cab->header.fileoff < sizeof(cab->header))
		goto bad;
	bread(ap, NiL, (off_t)sizeof(cab->header), (off_t)sizeof(cab->header), 1);
	if (cab->header.flags & CAB_FLAG_RESERVE)
	{
		s = state.tmp.buffer;
		if (bread(ap, s, 2, 2, 2) < 0)
			goto bad;
		swapmem(SWAPOP(1), s, &i2, sizeof(i2));
		cab->reserved.header = i2;
		if (bread(ap, s, 1, 1, 1) < 0)
			goto bad;
		cab->reserved.chunk = *(unsigned char*)s;
		if (bread(ap, s, 1, 1, 1) < 0)
			goto bad;
		cab->reserved.block = *(unsigned char*)s;
		if (cab->reserved.header)
			bread(ap, NiL, (off_t)cab->reserved.header, cab->reserved.header, 0);
		message((-1, "cab reserved.header=%u reserved.chunk=%u reserved.block=%u", cab->reserved.header, cab->reserved.chunk, cab->reserved.block));
	}
	if (cab->header.flags & CAB_FLAG_HASPREV)
	{
		s = state.tmp.buffer;
		while (bread(ap, s, 1, 1, 1) > 0 && *s++);
		*s = 0;
		if (!(cab->optional.prev.name = vmstrdup(vm, state.tmp.buffer)))
			nospace();
		for (s = state.tmp.buffer; bread(ap, s, 1, 1, 1) > 0 && *s; s++);
		*s = 0;
		if (!(cab->optional.prev.disk = vmstrdup(vm, state.tmp.buffer)))
			nospace();
		message((-1, "cab prev name=%s disk=%s", cab->optional.prev.name, cab->optional.prev.disk));
	}
	if (cab->header.flags & CAB_FLAG_HASNEXT)
	{
		s = state.tmp.buffer;
		while (bread(ap, s, 1, 1, 1) > 0 && *s++);
		*s = 0;
		if (!(cab->optional.next.name = vmstrdup(vm, state.tmp.buffer)))
			nospace();
		for (s = state.tmp.buffer; bread(ap, s, 1, 1, 1) > 0 && *s; s++);
		*s = 0;
		if (!(cab->optional.next.disk = vmstrdup(vm, state.tmp.buffer)))
			nospace();
		message((-1, "cab next name=%s disk=%s", cab->optional.next.name, cab->optional.next.disk));
	}
	if (bread(ap, cab->chunk, (off_t)(cab->header.chunks * sizeof(Cabchunk_t)), (off_t)(cab->header.chunks * sizeof(Cabchunk_t)), 0) <= 0)
		return 0;
	message((-1, "cab %s header info:\n\thdrsum=%u\n\tsize=%u\n\tchunksum=%u\n\tfileoff=%u\n\tdatasum=%u\n\tversion=%04x\n\tchunks=%u\n\tfiles=%u\n\tflags=%06o\n\tid=%u\n\tnumber=%u", ap->name, cab->header.hdrsum, cab->header.size, cab->header.chunksum, cab->header.fileoff, cab->header.datasum, cab->header.version, cab->header.chunks, cab->header.files, cab->header.flags, cab->header.id, cab->header.number));
	if (!(tp = sfstropen()))
		nospace();
	k = 1 << CAB_TYPE_NONE;
	for (i = 0; i < cab->header.chunks; i++)
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
	if (*s && !(cab->format = vmstrdup(vm, s)))
		nospace();
	sfstrclose(tp);
	for (i = 0; i < cab->header.files; i++)
	{
		if (bread(ap, &cab->file[i], (off_t)sizeof(Cabentry_t), (off_t)sizeof(Cabentry_t), 1) <= 0)
			goto bad;
		swapmem(SWAPOP(3), &cab->file[i].entry.size, &cab->file[i].entry.size, (char*)&cab->file[i].entry.chunk - (char*)&cab->file[i].entry.size);
		swapmem(SWAPOP(1), &cab->file[i].entry.chunk, &cab->file[i].entry.chunk, (char*)(&cab->file[i].entry + 1) - (char*)&cab->file[i].entry.chunk);
		for (s = state.tmp.buffer; bread(ap, s, 1, 1, 1) > 0 && *s; s++);
		if (!(cab->file[i].name = vmstrdup(vm, state.tmp.buffer)))
			nospace();
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
	return 1;
 bad:
	vmclose(vm);
	error(3, "%s: %s format version %04x header corrupted", ap->name, fp->name, cab->header.version);
	return -1;
}

static int
cab_done(Pax_t* pax, register Archive_t* ap)
{
	Cab_t*	cab = (Cab_t*)ap->data;

	if (cab)
	{
		if (cab->vm)
			vmclose(cab->vm);
		ap->data = 0;
	}
	return 0;
}

static int
cab_getheader(Pax_t* pax, register Archive_t* ap, register File_t* f)
{
	register Cab_t*		cab = (Cab_t*)ap->data;
	int			i;

	if ((i = cab->index++) >= cab->header.files)
		return 0;
	f->linkpath = 0;
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
	return 1;
}

static int
cab_getdata(Pax_t* pax, register Archive_t* ap, register File_t* f, int wfd)
{
	register Cab_t*		cab = (Cab_t*)ap->data;
	register Cabfile_t*	file;
	register Cabchunk_t*	chunk;

	if (wfd < 0)
		return 1;
	file = cab->file + cab->index - 1;
	chunk = cab->chunk + file->entry.chunk;
	switch (CAB_TYPE(chunk->compress))
	{
	case CAB_TYPE_NONE:
		error(3, "%s: %s: %s format NONE read not implemented", ap->name, f->name, ap->format->name);
		break;
	case CAB_TYPE_MSZIP:
		error(3, "%s: %s: %s format MSZIP read not implemented", ap->name, f->name, ap->format->name);
		break;
	case CAB_TYPE_QUANTUM:
		error(3, "%s: %s: %s format QUANTUM read not implemented", ap->name, f->name, ap->format->name);
		break;
	case CAB_TYPE_LZX:
		error(3, "%s: %s: %s format LZX read not implemented", ap->name, f->name, ap->format->name);
		break;
	default:
		error(3, "%s: %s: %s format COMPRESS=%d read not implemented", ap->name, f->name, CAB_TYPE(chunk->compress), ap->format->name);
		break;
	}
	return -1;
}

/*
 * pax cab format
 */

Format_t	pax_cab_format =
{
	"cab",
	"cabinet",
	"MS cabinet file",
	0,
	ARCHIVE|DOS|NOHARDLINKS,
	DEFBUFFER,
	DEFBLOCKS,
	0,
	pax_cab_next,
	0,
	cab_done,
	cab_getprologue,
	cab_getheader,
	cab_getdata,
};
