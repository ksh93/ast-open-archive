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
 * pax calib format
 *
 * test registry
 *
 *	0001000	disable auto-sequence
 *	0002000 disable latest version selection
 *	0004000 trace directory type { 0 1 }
 *	0010000 disable ibm => standard cobol conversions
 *	0020000 trace all directory types
 *	0040000 trace partial output record buffers
 *	0100000 trace record headers in decimal
 *	0200000 trace record headers in base 4
 *	0400000 trace record headers in base 4 (alternate format)
 */

#include "format.h"

#define MAGIC		"\301\304\331\100\323\311\302\331\306"

typedef struct Cadir_s
{
	unsigned short	offset;
	unsigned short	blocks;
	size_t		size;
} Cadir_t;

typedef struct Calib_s
{
	char*		format;
	char*		suffix;
	unsigned char*	map;
	unsigned char*	imap;
	unsigned char*	buffer;
	unsigned char*	next;
	int		count;
	int		digits;
	int		flags;
	int		increment;
	int		linesize;
	int		position;
	int		sequence;
	int		version;
	size_t		bufsize;
	size_t		blocksize;
	size_t		nblocks;
	size_t	 	block;
	size_t		headsize;
	size_t		line;
	size_t		buf;
	size_t		left;
	Cadir_t*	dir;
} Calib_t;

#define CALIB_LINE	256

#define cabcd5(x)	((((unsigned char*)x)[0]>>4)*10000+(((unsigned char*)x)[0]&0xf)*1000+(((unsigned char*)x)[1]>>4)*100+(((unsigned char*)x)[1]&0xf)*10+(((unsigned char*)x)[2]>>4))
#define casize2(x)	((((unsigned char*)x)[0]<<8)|(((unsigned char*)x)[1]))
#define casize3(x)	((((unsigned char*)x)[0]<<16)|(((unsigned char*)x)[1]<<8)|(((unsigned char*)x)[2]))
#define casize4(x)	((((unsigned char*)x)[0]<<24)|(((unsigned char*)x)[1]<<16)|(((unsigned char*)x)[2]<<8)|(((unsigned char*)x)[3]))

/*
 * ca cobol keyword map
 */

static const char*	cakey[] =
{
	"FILLER ",
	"PICTURE ",
	"USAGE ",
	"VALUE ",
	"PERFORM ",
	"SUBTRACT ",
	"COMPUTE ",
	"COMPUTATIONAL",
	"COMP",
	"REDEFINES ",
	"RENAMES ",
	"JUSTIFIED ",
	"GIVING ",
	"USING ",
	"CALL ",
	"ALTER ",
	"EQUAL ",
	"GREATER ",
	"POSITIVE ",
	"NEGATIVE ",
	"ELSE ",
	"OTHERWISE ",
	"{22}",
	"VARYING ",
	"FROM ",
	"UNTIL ",
	"THRU ",
	"ROUNDED ",
	"GO TO ",
	"MOVE ",
	"ZERO ",
	"ZEROS ",
	"DEPENDING ",
	"PIC ",
	"BLANK ",
	"OCCURS ",
	"{36}",
	"{37}",
	"{38}",
	"{39}",
	"{40}",
	"{41}",
	"{42}",
	"{43}",
	"{44}",
	"{45}",
	"{46}",
	"{47}",
	"{48}",
	"{49}",
	"{50}",
	"{51}",
	"{52}",
	"{53}",
	"{54}",
	"{55}",
	"{56}",
	"{57}",
	"{58}",
	"{59}",
	"{60}",
	"{61}",
	"{62}",
	"{63}",
	"{64}",
	"{65}",
	"{66}",
	"{67}",
	"{68}",
	"{69}",
	"{70}",
	"{71}",
	"{72}",
	"{73}",
	"{74}",
	"{75}",
	"{76}",
	"{77}",
	"{78}",
	"{79}",
	"{80}",
	"{81}",
	"{82}",
	"{83}",
	"{84}",
	"{85}",
	"{86}",
	"{87}",
	"{88}",
	"{89}",
	"{90}",
	"{91}",
	"{92}",
	"{93}",
	"{94}",
	"{95}",
	"{96}",
	"{97}",
	"{98}",
	"{99}",
	"{100}",
	"{101}",
	"{102}",
	"{103}",
	"{104}",
	"{105}",
	"{106}",
	"{107}",
	"{108}",
	"{109}",
	"{110}",
	"{111}",
	"{112}",
	"{113}",
	"{114}",
	"{115}",
	"{116}",
	"{117}",
	"{118}",
	"{119}",
	"{120}",
	"{121}",
	"{122}",
	"{123}",
	"{124}",
	"{125}",
	"{126}",
	"{127}",
};

/*
 * return next nbits nibble from the input line
 */

#define cagetbits(ca,n)  ((n)<=(ca)->left? \
				((((ca)->buf)>>((ca)->left-=(n)))&((1L<<(n))-1)):\
				_cagetbits((ca), (n)))

static int
_cagetbits(Calib_t* ca, int nbits)
{
	int	c;

	while (ca->left <= 8 * (sizeof(ca->buf) - 1))
	{
		if (ca->count-- > 0)
		{
			c = *ca->next++;
			ca->buf <<= 8;
			ca->buf |= c;
			ca->left += 8;
		}
		else if (ca->left < nbits)
			return 0;
		else
			break;
	}
	return cagetbits(ca, nbits);
}

static int
calib_getprologue(Pax_t* pax, Format_t* fp, register Archive_t* ap, File_t* f, unsigned char* buf, size_t size)
{
	register Calib_t*	ca;
	register Cadir_t*	dp;
	register Cadir_t*	de;
	register Cadir_t*	db;
	unsigned char		hdr[30];
	unsigned char		dir[22];
	unsigned char		blk[16];
	off_t			n;
	int			j;
	int			k;

	if (sizeof(hdr) <= size)
		memcpy(hdr, buf, sizeof(hdr));
	else if (bread(ap, hdr, (off_t)sizeof(hdr), (off_t)sizeof(hdr), 0) <= 0)
		return 0;
	else
		bunread(ap, hdr, sizeof(hdr));
	if (memcmp(hdr, MAGIC, sizeof(MAGIC) - 1))
		return 0;
	if (!(ca = newof(0, Calib_t, 1, 0)))
		nospace();
	ca->imap = (ca->map = ccmap(CC_EBCDIC_O, CC_NATIVE)) ? ca->map : ccmap(0, 0);
	ca->bufsize = casize2(&hdr[24]);
	ca->linesize = 80;
	if (state.buffersize < ca->bufsize)
	{
		state.buffersize = ca->bufsize;
		binit(ap);
	}
	db = dp = de = 0;
	j = 3;
	do
	{
		n = j * ca->bufsize;
		if (bseek(ap, n, SEEK_SET, 1) != n)
			error(3, "%s: %s format block header seek error", ap->name, ap->format->name);
		if (bread(ap, blk, (off_t)sizeof(blk), (off_t)sizeof(blk), 0) <= 0)
			error(3, "%s: %s format block header read error", ap->name, ap->format->name);
		j = casize2(&blk[10]);
		k = casize2(&blk[12]);
		if (state.test & 0400000)
			error(1, "blk %c%c%c%c%c%c%c%c %02x %02x %02x %02x %02x %02x %02x %02x", ccmapchr(ca->map, blk[0]), ccmapchr(ca->map, blk[1]), ccmapchr(ca->map, blk[2]), ccmapchr(ca->map, blk[3]), ccmapchr(ca->map, blk[4]), ccmapchr(ca->map, blk[5]), ccmapchr(ca->map, blk[6]), ccmapchr(ca->map, blk[7]), blk[8], blk[9], blk[10], blk[11], blk[12], blk[13], blk[14], blk[15]);
		while (k-- > 0)
		{
			if (bread(ap, dir, (off_t)sizeof(dir), (off_t)sizeof(dir), 0) <= 0)
				error(3, "%s: %s format header read error", ap->name, ap->format->name);
			if (state.test & 0400000)
				error(1, "dir %c%c%c%c%c%c%c%c %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", ccmapchr(ca->map, dir[0]), ccmapchr(ca->map, dir[1]), ccmapchr(ca->map, dir[2]), ccmapchr(ca->map, dir[3]), ccmapchr(ca->map, dir[4]), ccmapchr(ca->map, dir[5]), ccmapchr(ca->map, dir[6]), ccmapchr(ca->map, dir[7]), dir[8], dir[9], dir[10], dir[11], dir[12], dir[13], dir[14], dir[15], dir[16], dir[17], dir[18], dir[19], dir[20], dir[21]);
			if (dp >= de)
			{
				n = dp - db + 64;
				if (!(db = newof(db, Cadir_t, n, 0)))
					nospace();
				dp = db + n - 64;
				de = db + n - 1;
			}
			dp->offset = casize2(&dir[9]);
			dp->blocks = dir[14];
			dp->size = cabcd5(&dir[15]) * ca->linesize;
			ccmapstr(ca->map, dir, sizeof(dir));
			dp++;
		}
	} while (j);
	dp->offset = 0;
	dp->size = 0;
	ca->dir = db;
	ap->data = ca;
	return 1;
}

static int
calib_done(Pax_t* pax, register Archive_t* ap)
{
	if (ap->data)
	{
		free(ap->data);
		ap->data = 0;
	}
	return 0;
}

static int
calib_getdata(Pax_t* pax, register Archive_t* ap, register File_t* f, int wfd)
{
	register Calib_t*	ca = (Calib_t*)ap->data;
	const char*		s;
	unsigned char*		out;
	long			sequence;
	int			c;
	int			bits;
	int			flags;
	int			generate;
	int			noted;
	int			oline;
	int			version;
	unsigned char*		hdr;
	unsigned char*		b;
	unsigned char*		m;
	char*			ofile;
	char*			suffix;
	size_t			block;
	size_t			index;
	off_t			n;
	Sfio_t*			wfp;
	char			from[CALIB_LINE + 1];
	char			to[CALIB_LINE + 1];
	char			comment[CALIB_LINE + 1];
	unsigned char		outbuf[5 * CALIB_LINE + 1];

				       /*          10        20        30        40        50        60   */
				       /*01234567890123456789012345678901234567890123456789012345678901234*/
	static char		map[] = "? ~.<(+|&!$*);^-/,%_>?:#@'=\"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

	if (ca->suffix)
	{
		wfp = 0;
		suffix = 0;
	}
	else if (wfd < 0)
		return 1;
	else if (!(wfp = sfnew(NiL, NiL, SF_UNBOUND, wfd, SF_WRITE)))
	{
		error(2, "%s: cannot write", f->name);
		return -1;
	}
	comment[0] = from[0] = to[0] = 0;
	sequence = ca->digits && ca->increment && (ca->position + ca->digits) <= ca->linesize ? ca->sequence : -1;
	noted = !wfp || !state.warn;
	if (state.test & 0001000)
		sequence = -1;
	generate = ca->flags != 0xf1;
	block = 0;
	oline = error_info.line;
	error_info.line = 0;
	ofile = error_info.file;
	error_info.file = f->name;
	version = (state.test & 0002000) ? -1 : ca->version;
	for (;;)
	{
		block++;
		while (ca->line <= ca->blocksize - ca->headsize + 1)
		{
			ca->next = &ca->buffer[index = ca->line];
			hdr = ca->next;
			flags = ca->next[1];
			if (flags & 0x0c)
				break;
			ca->line += ca->next[0];
			ca->next += ca->headsize;
			if (flags & 0x80)
			{
				ca->next += 2;
				if (!(flags & 0x01))
					ca->next += 1;
			}
			if (version >= 0 && hdr[ca->headsize - 3] != version)
				continue;
			while ((ca->count = ca->next[-1]) == 1 && ca->next[0] == 0)
				ca->next += 2;
			error_info.line++;
			if (state.test & 0100000)
				error(0, "%03d %3u %3u %5u %3u %03o %4u %4u %4u %4u %4u %4o %4o %4o %4u %4u", error_info.line, ca->count, ca->next - hdr, ca->next - ca->buffer, hdr[0], hdr[1], hdr[2], hdr[3], hdr[4], hdr[5], hdr[6], hdr[7], hdr[8], hdr[9], hdr[10], hdr[11]);
			if (state.test & 0200000)
				error(0, "%03d %3u %3u %3u %03o %04..4u %04..4u %04..4u %04..4u %04..4u %04..4u %04..4u %04..4u %04..4u %04..4u", error_info.line, ca->count, ca->next - hdr, hdr[0], hdr[1], hdr[2], hdr[3], hdr[4], hdr[5], hdr[6], hdr[7], hdr[8], hdr[9], hdr[10], hdr[11]);
			if (state.test & 0400000)
				error(0, "%03d %3d %03d:%02x:%04..4u:%03u | %03d:%02x:%04..4u:%03u %03d:%02x:%04..4u:%03u %03d:%02x:%04..4u:%03u %03d:%02x:%04..4u:%03u %03d:%02x:%04..4u:%03u %03d:%02x:%04..4u:%03u",
					error_info.line,
					ca->count,
					ca->next[-1], ca->next[-1], ca->next[-1], ca->next[-1] & 0x7f,
					ca->next[0], ca->next[0], ca->next[0], ca->next[0] & 0x7f,
					ca->next[1], ca->next[1], ca->next[1], ca->next[1] & 0x7f,
					ca->next[2], ca->next[2], ca->next[2], ca->next[2] & 0x7f,
					ca->next[3], ca->next[3], ca->next[3], ca->next[3] & 0x7f,
					ca->next[4], ca->next[4], ca->next[4], ca->next[4] & 0x7f,
					ca->next[5], ca->next[5], ca->next[5], ca->next[5] & 0x7f);
			if (flags & 0x40)
			{
				bits = 8;
				m = ca->imap;
			}
			else
			{
				bits = 6;
				m = (unsigned char*)map;
			}
			out = outbuf;
			if (state.warn)
				noted = 0;
			while (ca->count && (ca->next - ca->buffer) < ca->line)
			{
				if (ca->count < 64)
				{
					ca->left = 0;
					while ((c = cagetbits(ca, bits)) || bits == 8 && ca->count > 0)
						if (out < &outbuf[ca->linesize])
							*out++ = m[c];
						else if (!noted)
						{
							noted = 1;
							error(2, "%s: overbyte (bits=%d offset=%I*u+%I*u block=%I*u)", ap->name, bits, sizeof(off_t), bseek(ap, (off_t)0, SEEK_CUR, 0) - ca->bufsize, sizeof(index), index, sizeof(block), block);
						}
					if ((ca->next - ca->buffer) >= ca->line)
						break;
					if ((c = (*ca->next ^ 64)) & 0x80)
						c = 0;
					else
						ca->next++;
				}
				else if (ca->count & 0x80)
				{
					if (state.test & 0040000)
						error(1, "part c=%d n=%d k=%d \"%-.*s\"", c, out - outbuf, ca->count & 0x7f, out - outbuf, outbuf);
					goto key;
				}
				else if ((c = ca->count - 64) & 0x80)
					c = (c + 64) & 0x7f;
				if (state.test & 0040000)
					error(1, "part c=%d:%d:%d r=%d:%d bits=%d n=%d x=%u \"%-.*s\"", c, &outbuf[ca->linesize] - out, c ^ 64, ca->next - ca->buffer, ca->line, bits, out - outbuf, *ca->next, out - outbuf, outbuf);
				if (c > (&outbuf[ca->linesize] - out))
				{
					ca->count = c ^ 64;
					if (bits == 8 && ca->count <= (&outbuf[ca->linesize] - out))
						continue;
					goto key;
				}
				else if (c > 0)
				{
					memset(out, ' ', c);
					out += c;
				}
				while ((ca->next - ca->buffer) < ca->line && (ca->count = *ca->next++) & 0x80)
				{
				key:
					s = cakey[ca->count & 0x7f];
					if (state.warn && *s == '{')
						error(1, "%s: keyword %s (bits=%d offset=%I*u+%I*u block=%I*u)", ap->name, s, bits, sizeof(off_t), bseek(ap, (off_t)0, SEEK_CUR, 0) - ca->bufsize, sizeof(index), index, sizeof(block), block);
					while (c = *s++)
						if (out < &outbuf[ca->linesize])
							*out++ = c;
						else
						{
							if (!noted)
							{
								noted = 1;
								error(2, "%s: key overbyte (bits=%d offset=%I*u+%I*u block=%I*u)", ap->name, bits, sizeof(off_t), bseek(ap, (off_t)0, SEEK_CUR, 0) - ca->bufsize, sizeof(index), index, sizeof(block), block);
							}
							break;
						}
				}
			}
			if (!wfp)
			{
				if (flags & 0x01)
				{
					for (b = outbuf; b < out && *b == ' ';  b++);
					for (; b < out && isdigit(*b); b++);
					for (; b < out && *b == ' '; b++);
					for (m = b; b < out && *b != ' '; b++);
					if (m == outbuf)
					{
						if ((b - m) == 7 && strneq((char*)m, "NEWPAGE", 7) || (b - m) == 4 && strneq((char*)m, "FILE", 4))
						{
							suffix = ".EZT";
							break;
						}
						else if ((out - outbuf) > 2 && m[0] == 'C' && (m[1] == ' ' || m[1] == '*'))
						{
							suffix = ".F";
							break;
						}
					}
					else if ((b - m) == 7 && strneq((char*)m, "INCLUDE", 7) || (b - m) == 4 && strneq((char*)m, "NAME", 4))
					{
						suffix = ".LNK";
						break;
					}
					else if ((b - m) == 5 && (strneq((char*)m, "TITLE", 5) || strneq((char*)m, "SPACE", 5)))
					{
						suffix = ".ASM";
						break;
					}
					else if ((b - m) == 3 && strneq((char*)m, "DCL", 3))
					{
						suffix = ".PLI";
						break;
					}
					else if ((b - m) == 10 && strneq((char*)m, "SUBROUTINE", 10) || (b - m) == 9 && strneq((char*)m, "DIMENSION", 9) || (b - m) == 6 && strneq((char*)m, "COMMON", 6) || b < out && (b - m) >= 6 && strneq((char*)m, "FORMAT", 6) && (m[6] == '(' || m[6] == ' '))
					{
						suffix = ".F";
						break;
					}
					if (b < out && *m++ == '.' && ((b - m) == 7 && strneq((char*)m, "REPLACE", 7) || (b - m) == 5 && strneq((char*)m, "QUOTE", 5) || (b - m) > 4 && strneq((char*)m, "END-", 4)))
					{
						suffix = ".QBL";
						break;
					}
					if ((b - outbuf) > 6)
					{
						if (outbuf[6] == ' ')
						{
							for (b = outbuf + 7; b < out && *b == ' ';  b++);
							for (m = b; b < out && *b != ' '; b++);
							if (b < out && (b - m) == 2 && isdigit(m[0]) && isdigit(m[1]))
							{
								for (; b < out && *b != '.'; b++);
								if (b < out && *b == '.')
								{
									suffix = ".CPY";
									break;
								}
							}
							else
							{
								for (; b < out && *b == ' '; b++);
								for (m = b; b < out && *b != '.'; b++);
								if (b < out && (b - m) == 8 && strneq((char*)m, "DIVISION", 8))
								{
									suffix = ".COB";
									break;
								}
							}
						}
						else if (outbuf[6] == '*')
							suffix = ".CPY";
					}
				}
			}
			else if (flags & 0x01)
			{
				if (sequence >= 0)
				{
					if ((b = outbuf + ca->position) >= out || *b == ' ' || isdigit(*b) || (out - outbuf) == ca->linesize && (b = out - ca->digits))
					{
						if ((c = ca->position - (out - outbuf)) > 0)
						{
							memset(out, ' ', c);
							out += c;
						}
						b += sfsprintf((char*)b, ca->digits, "%0.*lu", ca->digits, generate ? sequence : casize3(&hdr[2]));
						if (out < b)
							out = b;
					}
					sequence += ca->increment;
				}
				for (; out > outbuf && *(out - 1) == ' '; out--);
				*out++ = '\n';
				if ((out - outbuf) > 10 && outbuf[6] == '*' && outbuf[7] == '/')
				{
					if ((out - outbuf) == 11 && outbuf[8] == '/' && outbuf[9] == '*')
						from[0] = to[0] = 0;
					else
					{
						m = 0;
						for (b = outbuf + 8; b < out; b++)
							if (*b == '/')
							{
								if (!m)
									m = b;
								else
								{
									if (*++b == '*')
									{
										c = m - outbuf - 8;
										memcpy(from, outbuf + 8, c);
										from[c] = 0;
										c = b - m - 2;
										memcpy(to, m + 1, c);
										to[c] = 0;
									}
									break;
								}
							}
					}
				}
				else if (outbuf[0] == '-' && outbuf[1] == 'I' && outbuf[2] == 'N' && outbuf[3] == 'C' && outbuf[4] == ' ' && !(state.test & 0010000))
				{
					for (b = outbuf + 5; *b == ' '; b++);
					for (m = b; m < (out - 1) && *m != ' '; m++);
					if (from[0])
						c = sfsprintf((char*)outbuf, sizeof(outbuf), "       COPY %-.*s REPLACING =%s= BY =%s=.\n", m - b, b, from, to);
					else
						c = sfsprintf((char*)outbuf, sizeof(outbuf), "       COPY %-.*s.\n", m - b, b);
					out = outbuf + c;
				}
				sfwrite(wfp, outbuf, out - outbuf);
			}
			else
			{
				error_info.line--;
				if (state.warn)
				{
					for (b = outbuf; b < out && *b == ' '; b++);
					for (; out > outbuf && *(out - 1) == ' '; out--);
					*out = 0;
					if (!streq((char*)b, comment))
						error(0, "comment %s \"%s\"", error_info.file, strcpy(comment, (char*)b));
				}
			}
		}
		if (!wfp)
			break;
		if (--ca->nblocks <= 0)
			break;
		if (ca->buffer[2] == 0xff)
		{
			n = casize2(ca->buffer + ca->blocksize + (block != 1)) * ca->bufsize;
			if (bseek(ap, n, SEEK_SET, 1) != n)
				error(3, "%s: %s format data seek error", ap->name, ap->format->name);
		}
		if (!(ca->buffer = (unsigned char*)bget(ap, (off_t)ca->bufsize, NiL)))
			error(3, "%s: format member read error (offset=%I*u block=%I*u)", ap->name, sizeof(off_t), bseek(ap, (off_t)0, SEEK_CUR, 0), sizeof(block), block);
		ca->blocksize = casize2(ca->buffer);
		ca->next = ca->buffer;
		ca->line = 4;
	}
	error_info.line = oline;
	error_info.file = ofile;
	if (wfp)
	{
		sfclose(wfp);
		setfile(ap, f);
	}
	else if (suffix)
		strcpy(ca->suffix, suffix);
	return 1;
}

static int
calib_getheader(Pax_t* pax, register Archive_t* ap, register File_t* f)
{
	register Calib_t*	ca = (Calib_t*)ap->data;
	register unsigned char*	h;
	register off_t		n;
	char*			s;
	char*			t;
	char*			v;
	int			i;
	int			j;
	int			k;
	int			x;
	char			suf[4];

	if (!ca->dir->offset)
		return 0;
	n = ca->bufsize * ca->dir->offset;
	if (bseek(ap, n, SEEK_SET, 1) != n)
		error(3, "%s: %s format member seek error", ap->name, ap->format->name);
	ca->nblocks = ca->dir->blocks;
	if (!(ca->buffer = (unsigned char*)bget(ap, (off_t)ca->bufsize, NiL)))
		error(3, "%s: %s format member read error", ap->name, ap->format->name);
	ca->blocksize = casize2(&ca->buffer[0]) + 1;
	ca->headsize = 6;
	ca->version = -1;
	ca->next = ca->buffer;
	s = 0;
	k = 0;
	for (j = 1, x = 0, h = ca->buffer + 4; !x && h < ca->buffer + ca->bufsize; j++, h += i)
	{
		if ((i = *h) == 78)
		{
			j = h[2];
			if (j == 5 && !h[1] && !h[3])
				x = 1;
			i -= 6;
			h += 6;
		}
		else
		{
			if (j == 5)
				x = 1;
			i -= 2;
			h += 2;
		}
		if (!(k & (1<<j)))
		{
			k |= (1<<j);
			if (state.test & 0020000)
			{
				int	y;

				sfprintf(sfstderr, "head %-8s %d", f->name, j);
				for (y = 0; y < i; y++)
					sfprintf(sfstderr, " %02x", h[y]);
				sfprintf(sfstderr, "\n");
			}
			switch (j)
			{
			case 0:
				switch (ca->flags = h[32])
				{
				case 0x40:
					ca->headsize = 5;
					break;
				case 0xf1:
					ca->headsize = 8;
					break;
				}
				ca->version = h[-3];
				ca->position = h[33];
				ca->digits = h[34] + 1;
				ca->increment = casize2(&h[35]);
				if ((ca->sequence = casize2(&h[37])) > 0x7fff)
					ca->sequence = ca->sequence - 0x10000;
				ca->sequence += ca->increment;
				ccmapstr(ca->map, h, 24);
				f->name = stash(&ap->stash.head, (char*)h, 13);
				for (s = f->name + 8; *(s - 1) == ' '; s--);
				*(ca->suffix = s) = 0;
				f->st->st_mtime = f->st->st_ctime = f->st->st_atime = tmscan(sfprints("%-.6s%02u%02u%02u", memcmp(h + 18, "000000", 6) ? (h + 18) : (h + 12), h[24], h[25], h[26]), NiL, "%y%m%d%H%M%S", NiL, NiL, 0);
				if (state.test & 0004000)
					error(0, "head %-8s %d %03o %03o %03o %03o %03o %03o %03o", f->name, j, h[32], h[33], h[34], h[35], h[36], h[37], h[38]);
				break;
			case 1:
				switch (ca->flags = h[32])
				{
				case 0x40:
					ca->headsize = 3;
					break;
				}
				if (h[31] == 0x48)
				{
					ca->digits = 0;
					ca->linesize = 133;
				}
				else
				{
					ca->position = h[33];
					ca->digits = h[34] + 1;
					ca->increment = casize2(&h[35]);
					if ((ca->sequence = casize2(&h[37])) > 0x7fff)
						ca->sequence = ca->sequence - 0x10000;
					ca->sequence += ca->increment;
				}
				ccmapstr(ca->map, h, 32);
				f->name = stash(&ap->stash.head, (char*)h, 13);
				for (s = f->name + 8; *(s - 1) == ' '; s--);
				*(ca->suffix = s) = 0;
				f->st->st_mtime = f->st->st_ctime = f->st->st_atime = tmscan(sfprints("%-.6s%-.4s00", memcmp(h + 18, "000000", 6) ? (h + 18) : (h + 12), h + 24), NiL, "%m%d%y%H%M%S", NiL, NiL, 0);
				if (state.test & 0004000)
					error(0, "head %-8s %d %03o %03o %03o %03o %03o %03o %03o", f->name, j, h[32], h[33], h[34], h[35], h[36], h[37], h[38]);
				break;
			case 5:
				if (s)
				{
					if (state.test & 4)
					{
						memcpy(suf, h + 34, 3);
						ccmapstr(ca->map, suf, 4);
						sfprintf(sfstdout, "%02x %02x %-.3s %s\n", h[38], h[39], suf, f->name);
					}
					switch (h[38])
					{
					case 0x31:
						v = "PLI";
						break;
					case 0x32:
					case 0x42:
					case 0x4c:
						v = "COB";
						break;
					case 0x43:
						v = "EST";
						break;
					case 0x4d:
						v = "ASM";
						break;
					default:
						t = (char*)h + 34;
						ccmapstr(ca->map, t, 3);
						v = suf;
						while (t < ((char*)h + 37) && *t != ' ')
							*v++ = *t++;
						*v = 0;
						if (!*(v = suf))
						{
							if (ca->linesize == 133)
							{
								v = "LST";
								ca->suffix = 0;
							}
							else
								switch (h[38])
								{
								case 0x00:
								case 0x02:
									v = "COB";
									break;
								case 0x01:
								case 0x0a:
									v = 0;
									break;
								default:
									v = 0;
									if (state.warn)
										error(1, "%s: no suffix for type=%02x", f->name, h[38]);
									break;
								}
						}
						else if (streq(v, "CB2") || streq(v, "CBL"))
							v = "COB";
						else if (streq(v, "PL1"))
							v = "PLI";
						break;
					}
					if (v)
					{
						*s++ = '.';
						strcpy(s, v);
						if (streq(v, "EZT") || streq(v, "PLI"))
							ca->suffix = 0;
					}
				}
				break;
			}
		}
	}
	for (s = f->name; *s; s++)
		if (!isalnum(*s) && *s != '.')
			*s = '_';
	ca->line = h - ca->buffer;
	if (ca->suffix)
	{
		calib_getdata(pax, ap, f, -1);
		ca->line = h - ca->buffer;
		ca->suffix = 0;
	}
	for (i = 0, s = f->name; *s && *s != '.'; s++)
		if (islower(*s))
			i = 1;
	if (i && *s)
		while (*++s)
			if (isupper(*s))
				*s = tolower(*s);
	f->linkpath = 0;
	f->st->st_dev = 0;
	f->st->st_ino = 0;
	f->st->st_mode = X_IFREG|X_IRUSR|X_IWUSR|X_IRGRP|X_IROTH;
	f->st->st_uid = state.uid;
	f->st->st_gid = state.gid;
	f->st->st_nlink = 1;
	IDEVICE(f->st, 0);
	f->st->st_size = ca->dir->size;
	ca->dir++;
	return 1;
}

Format_t	pax_calib_format =
{
	"ca-librarian",
	"calib|librarian",
	"mvs CA-librarian file",
	0,
	ARCHIVE|NOHARDLINKS|IN,
	DEFBUFFER,
	DEFBLOCKS,
	0,
	pax_calib_next,
	0,
	calib_done,
	calib_getprologue,
	calib_getheader,
	calib_getdata,
};
