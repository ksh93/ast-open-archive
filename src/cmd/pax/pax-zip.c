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
 * pax zip format
 */

#include "format.h"

#include <tm.h>

#define ZIP_HEADER	46		/* largest header size		*/
#define ZIP_COPY	0		/* no need to unzip		*/
#define ZIP_SUM_INIT	0xffffffffL	/* zipsum() initial value	*/

#define ZIP_CEN_HEADER	46		/* central header size		*/
#define ZIP_CEN_MAGIC	0x504b0102L	/* central header magic		*/
#define ZIP_CEN_VEM	4		/* version made by		*/
#define ZIP_CEN_VER	6		/* version needed to extract	*/
#define ZIP_CEN_FLG	8		/* encrypt, deflate flags	*/
#define ZIP_CEN_HOW	10		/* compression method		*/
#define ZIP_CEN_TIM	12		/* DOS format modify time	*/
#define ZIP_CEN_DAT	14		/* DOS format modify date	*/
#define ZIP_CEN_CRC	16		/* uncompressed data crc-32	*/
#define ZIP_CEN_SIZ	20		/* compressed data size		*/
#define ZIP_CEN_LEN	24		/* uncompressed data size	*/
#define ZIP_CEN_NAM	28		/* length of filename		*/
#define ZIP_CEN_EXT	30		/* length of extra field	*/
#define ZIP_CEN_COM	32		/* file comment length		*/
#define ZIP_CEN_DSK	34		/* disk number start		*/
#define ZIP_CEN_ATT	36		/* internal file attributes	*/
#define ZIP_CEN_ATX	38		/* external file attributes	*/
#define ZIP_CEN_OFF	42		/* local header relative offset	*/

#define ZIP_LOC_HEADER	30		/* local header size		*/
#define ZIP_LOC_MAGIC	0x504b0304L	/* local header magic		*/
#define ZIP_LOC_VER	4		/* version needed to extract	*/
#define ZIP_LOC_FLG	6		/* encrypt, deflate flags	*/
#define ZIP_LOC_HOW	8		/* compression method		*/
#define ZIP_LOC_TIM	10		/* DOS format modify time	*/
#define ZIP_LOC_DAT	12		/* DOS format modify date	*/
#define ZIP_LOC_CRC	14		/* uncompressed data crc-32	*/
#define ZIP_LOC_SIZ	18		/* compressed data size		*/
#define ZIP_LOC_LEN	22		/* uncompressed data size	*/
#define ZIP_LOC_NAM	26		/* length of filename		*/
#define ZIP_LOC_EXT	28		/* length of extra field	*/

#define ZIP_END_HEADER	22		/* end header size		*/
#define ZIP_END_MAGIC	0x504b0506L	/* end header magic		*/
#define ZIP_END_DSK	4		/* number of this disk		*/
#define ZIP_END_BEG	6		/* number of the starting disk	*/
#define ZIP_END_SUB	8		/* entries on this disk		*/
#define ZIP_END_TOT	10		/* total number of entries	*/
#define ZIP_END_SIZ	12		/* central directory total size	*/
#define ZIP_END_OFF	16		/* central offset starting disk	*/
#define ZIP_END_COM	20		/* length of zip file comment	*/

#define ZIP_EXT_HEADER	16		/* ext header size		*/
#define ZIP_EXT_MAGIC	0x504b0708L	/* ext header magic		*/
#define ZIP_EXT_SIZ	8		/* compressed data size		*/
#define ZIP_EXT_LEN	12		/* uncompressed data size	*/

static int
zip_getheader(Pax_t* pax, Archive_t* ap, register File_t* f)
{
	char*	s;
	int	i;
	long	n;
	size_t	num;
	Tm_t	tm;
	char	hdr[ZIP_HEADER];

	for (;;)
	{
		num = ZIP_LOC_HEADER;
		if (bread(ap, hdr, (off_t)num, (off_t)0, 0) <= 0)
			goto done;
		switch ((long)swapget(0, hdr, 4))
		{
		case ZIP_CEN_MAGIC:
			bunread(ap, hdr, num);
			if (bread(ap, hdr, (off_t)ZIP_CEN_HEADER, (off_t)0, 0) <= 0)
			{
				error(2, "%s: invalid %s format verification header", ap->name, ap->format->name);
				return 0;
			}
			n = swapget(3, &hdr[ZIP_CEN_NAM], 2);
			s = stash(&ap->stash.zip, NiL, n);
			if (bread(ap, s, (off_t)n, (off_t)0, 0) <= 0)
			{
				error(2, "%s: invalid %s format verification header name [size=%I*u]", ap->name, ap->format->name, sizeof(n), n);
				return -1;
			}
			if (s[n - 1] == '/')
				n--;
			s[n] = 0;
			if ((n = swapget(3, &hdr[ZIP_CEN_EXT], 2)) && bread(ap, NiL, (off_t)n, (off_t)0, 0) <= 0)
			{
				error(2, "%s: %s: invalid %s format verification header extended data [size=%I*u]", ap->name, s, ap->format->name, sizeof(n), n);
				return -1;
			}
			if ((n = swapget(3, &hdr[ZIP_CEN_COM], 2)) && bread(ap, NiL, (off_t)n, (off_t)0, 0) <= 0)
			{
				error(2, "%s: %s: invalid %s format verification header comment data [size=%I*u]", ap->name, s, ap->format->name, sizeof(n), n);
				return -1;
			}
			ap->verified++;
			if (ap->tab)
			{
				if (!hashget(ap->tab, s))
				{
					error(1, "%s: %s: file data not found", ap->name, s);
					continue;
				}
				n = ((unsigned long)swapget(3, &hdr[ZIP_CEN_ATX], 4) >> 16) & 0xffff;
				switch ((int)X_ITYPE(n))
				{
				case 0:
				case X_IFREG:
					break;
				case X_IFDIR:
					break;
				case X_IFLNK:
					error(1, "%s: %s: symbolic link copied as regular file", ap->name, s);
					break;
				default:
					error(1, "%s: %s: unknown file type %07o -- regular file assumed (0x%08x)", ap->name, s, X_ITYPE(n), n);
					break;
				}
			}
			continue;
		case ZIP_END_MAGIC:
			bunread(ap, hdr, num);
			if (bread(ap, hdr, (off_t)ZIP_END_HEADER, (off_t)0, 0) <= 0)
			{
				error(2, "%s: invalid %s format verification header", ap->name, ap->format->name);
				return 0;
			}
			n = swapget(3, &hdr[ZIP_END_COM], 2);
			if (bread(ap, NiL, (off_t)n, (off_t)0, 0) <= 0)
			{
				error(2, "%s: invalid %s format trailer data", ap->name, ap->format->name);
				return 0;
			}
			goto done;
		case ZIP_LOC_MAGIC:
			n = swapget(3, &hdr[ZIP_LOC_NAM], 2);
			f->name = stash(&ap->stash.zip, NiL, n);
			if (bread(ap, f->name, (off_t)n, (off_t)0, 0) <= 0)
				return 0;
			num += n;
			f->st->st_mode = ((n > 0 && f->name[n - 1] == '/') ? (X_IFDIR|X_IRUSR|X_IWUSR|X_IXUSR|X_IRGRP|X_IWGRP|X_IXGRP|X_IROTH|X_IWOTH|X_IXOTH) : (X_IFREG|X_IRUSR|X_IWUSR|X_IRGRP|X_IWGRP|X_IROTH|X_IWOTH)) & state.modemask;
			f->name[n] = 0;
			if ((n = swapget(3, &hdr[ZIP_LOC_EXT], 2)) > 0)
			{
				if (bread(ap, NiL, (off_t)n, (off_t)0, 0) <= 0)
					return 0;
				num += n;
			}
			f->st->st_dev = 0;
			f->st->st_ino = 0;
			f->st->st_uid = state.uid;
			f->st->st_gid = state.gid;
			f->st->st_nlink = 1;
			IDEVICE(f->st, 0);
			n = swapget(3, &hdr[ZIP_LOC_TIM], 4);
			memset(&tm, 0, sizeof(tm));
			tm.tm_year = ((n>>25)&0377) + 80;
			tm.tm_mon = ((n>>21)&017) - 1;
			tm.tm_mday = ((n>>16)&037);
			tm.tm_hour = ((n>>11)&037);
			tm.tm_min = ((n>>5)&077);
			tm.tm_sec = ((n<<1)&037);
			f->st->st_mtime = tmtime(&tm, TM_LOCALZONE);
			f->linktype = NOLINK;
			f->linkpath = 0;
			f->uidname = 0;
			f->gidname = 0;
			ap->checkdelta = 0;
			f->st->st_size = swapget(3, &hdr[ZIP_LOC_SIZ], 4);
			f->uncompressed = swapget(3, &hdr[ZIP_LOC_LEN], 4);
			n = swapget(3, &hdr[ZIP_LOC_FLG], 2);
			if (n & 8)
			{
				s = state.tmp.buffer;
				i = ZIP_EXT_HEADER;
				if (!f->st->st_size)
				{
					i--;
					for (;;)
					{
						while (bread(ap, s, (off_t)1, (off_t)0, 1) > 0 && *s != 0x50)
							num++;
						if (bread(ap, s + 1, (off_t)i, (off_t)0, 1) <= 0)
							error(3, "%s: invalid %s local extension header", ap->name, ap->format->name);
						if (swapget(0, s, 4) == ZIP_EXT_MAGIC)
							break;
					}
				}
				else if (bread(ap, NiL, (off_t)f->st->st_size, (off_t)0, 1) <= 0 || bread(ap, s, (off_t)i, (off_t)0, 1) <= 0)
					error(3, "%s: invalid %s local extension header", ap->name, ap->format->name);
				f->datasize = num + ZIP_EXT_HEADER;
				if (X_ITYPE(f->st->st_mode) == X_IFREG)
				{
					f->st->st_size = f->datasize;
					f->uncompressed = swapget(3, &s[ZIP_EXT_LEN], 4);
				}
				else
				{
					f->st->st_size = 0;
					f->uncompressed = 0;
				}
				if (bseek(ap, -(off_t)f->datasize, SEEK_CUR, 0) < 0)
					error(3, "%s: %s local extension header seek error", ap->name, ap->format->name);
				f->delta.op = f->uncompressed ? DELTA_zip : DELTA_pass;
			}
			else if (!f->st->st_size || swapget(3, &hdr[ZIP_LOC_HOW], 2) == ZIP_COPY)
			{
				f->delta.op = DELTA_pass;
				f->uncompressed = 0;
			}
			else
			{
				f->delta.op = DELTA_zip;
				f->st->st_size += num;
				bunread(ap, hdr, num);
			}
			if (!ap->tab)
			{
				if (!(ap->tab = hashalloc(NiL, HASH_set, HASH_ALLOCATE, HASH_name, "entries", 0)))
					error(1, "%s: cannot allocate verification hash table", ap->name);
				undoable(ap, getformat(FMT_ZIP, 1));
			}
			return 1;
		default:
			bunread(ap, hdr, num);
			return 0;
		}
	}
 done:
	if (ap->entries == ap->verified)
		return 0;
	error(2, "%s: %d out of %d verification entries omitted", ap->name, ap->entries - ap->verified, ap->entries);
	return -1;
}

static int
zip_getprologue(Pax_t* pax, Format_t* fp, register Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	int	n;

	ap->format = fp;
	if ((n = zip_getheader(pax, ap, f)) > 0)
		ap->peek = 1;
	else
		ap->format = 0;
	return n;
}

Format_t	pax_zip_format =
{
	"zip",
	0,
	"zip 2.1 / PKZIP 2.04g archive",
	0,
	ARCHIVE|COMPRESSED|DOS|KEEPSIZE|IN,
	DEFBUFFER,
	DEFBLOCKS,
	0,
	pax_zip_next,
	0,
	0,
	zip_getprologue,
	zip_getheader,
};
