/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1984-2000 AT&T Corp.              *
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
 * AT&T Research
 *
 * make archive access routines
 */

#include "make.h"

#include <swap.h>
#include <tm.h>
#include <int.h>

/*
 * these routines simultaneously support all archive formats
 */

#define arpointer(x)	(((x)>=0&&(x)<elementsof(artab)&&artab[x].fp)?&artab[x]:0)

struct ar				/* open archive table entry	*/
{
	Sfio_t*		fp;		/* archive file pointer		*/
	unsigned long	time;		/* archive modify time		*/
	int		type;		/* current archive type		*/
	int		swap;		/* swapget() op if necessary	*/
	long		first;		/* first member header offset	*/
	long		last;		/* past last member header off	*/
	long		sym_offset;	/* symbol directory offset	*/
	char*		names;		/* long name table		*/
};

static struct ar	artab[2];	/* open archive table		*/

static int		ntouched;	/* count of touched members	*/

/*
 * archive format definitions (NAME and TYPE entries are required)
 *
 *	NAME		description
 *	TYPE		type index
 *	MAGIC		magic number (string)
 *	MSIZE		MAGIC size in chars
 *	TERM		member name termination char
 *	ENDHDR		end of header magic number (string)
 *	ESIZE		ENDHDR size in chars
 *	SCAN		TYPE_local scan action
 *	SYMDIR		symbol directory name pattern
 *	UPDATE		SYMDIR update command(s)
 *	header		header struct
 *	member		member struct
 *	symbol		symbol table entry
 *	directory	symbol directory header
 */

#define NAME_local	"local"
#define TYPE_local	0
#define SCAN_local	"$(ARSCAN|AR) $(ARSCANFLAGS|\"tv\") $(>)"
#define UPDATE_local	"($(RANLIB|\":\") $(<)) >/dev/null 2>&1 || true"
#define SYMDIR_local	"(._|_.|__.|___|*/)*"

#define NAME_pdp	"pdp 11"
#define TYPE_pdp	1
#define MAGIC_pdp	0177545
#define MSIZE_pdp	2
#define TERM_pdp	0

struct header_pdp
{
	char		ar_pad[2];
	char		ar_name[14];
	unsigned int_4	ar_date;		/* native representation*/
	char		ar_uid;			/* 	"		*/
	char		ar_gid;			/* 	"		*/
	int_2		ar_mode;		/* 	"		*/
	unsigned int_4	ar_size;		/* 	"		*/
};

#define NAME_V0		"System V Release 0"
#define TYPE_V0		2
#define MAGIC_V0	"<ar>"
#define MSIZE_V0	4
#define TERM_V0		0

struct header_V0
{
	char	ar_magic[MSIZE_V0];	/* MAGIC			*/
	char	ar_name[16];
	char	ar_date[4];		/* swapget() accessed		*/
	char	ar_syms[4];		/*	"			*/
};

struct member_V0
{
	char	arf_name[16];
	char	arf_date[4];		/* swapget() accessed		*/
	char	arf_uid[4];		/*	"			*/
	char	arf_gid[4];		/*	"			*/
	char	arf_mode[4];		/*	"			*/
	char	arf_size[4];		/*	"			*/
};

struct symbol_V0
{
	char	sym_name[8];		/* ' ' terminated		*/
	char	sym_ptr[4];		/* swapget() accessed		*/
};

#define NAME_port	"portable"
#define TYPE_port	3
#define MAGIC_port	"!<arch>\n"
#define MSIZE_port	8
#define TERM_port	'/'
#define ENDHDR_port	"`\n"
#define SENDHDR_port	2
#define SYMDIR_port	"(/ |/*/|_______[0-9_][0-9_][0-9_]E[BL]E[BL]_)*"
#define SYMDIR_other	"(._|_.|__.|___)*"
#define SYMDIR_age	5

struct header_port
{
	char	ar_name[16];
	char	ar_date[12];	/* left-adj; decimal char*; blank fill	*/
	char	ar_uid[6];	/*	"				*/
	char	ar_gid[6];	/*	"				*/
	char	ar_mode[8];	/* left-adj; octal char*; blank fill	*/
	char	ar_size[10];	/* left-adj; decimal char*; blank fill	*/
	char	ar_fmag[2];	/* ENDHDR				*/
};

#define NAME_rand	"portable with ranlib"	/* almost TYPE_port	*/
#define TYPE_rand	4
#define TERM_rand	' '
#define SYMDIR_rand	"(__.SYMDEF|__________E???X)*"
#define SYMDIR_strict	"__.SYMDEF SORTED*"
#define UPDATE_rand	"$(RANLIB) $(<)"

#define NAME_aix	"aix"
#define TYPE_aix	5
#define MAGIC_aix	"<aiaff>\n"
#define MSIZE_aix	8

struct header_aix
{
	char	fl_magic[MSIZE_aix];
	char	fl_memoff[12];	
	char	fl_gstoff[12];
	char	fl_fstmoff[12];
	char	fl_lstmoff[12];
	char	fl_freeoff[12];
};

struct member_aix
{
	char	ar_size[12];
	char	ar_nxtmem[12];
	char	ar_prvmem[12];
	char	ar_date[12];
	char	ar_uid[12];
	char	ar_gid[12];
	char	ar_mode[12];
	char	ar_namlen[4];
	union
	{
		char	ar_name[2];
		char	ar_fmag[2];
	}	_ar_name;
};

union header_data
{
	struct header_aix	aix;
	struct header_pdp	pdp;
	struct header_port	port;
	struct header_V0	V0;
};

union header
{
	union header_data	data;
	char			term[sizeof(union header_data) + 1];
};

union magic_data
{
	char			aix[MSIZE_aix];
	unsigned int_2		pdp;
	char			port[MSIZE_port];
	char			V0[MSIZE_V0];
};

union magic
{
	union magic_data	data;
	char			term[sizeof(union magic_data) + 1];
};

union member_data
{
	struct member_aix	aix;
	struct member_V0	V0;
};

union member
{
	union member_data	data;
	char			term[sizeof(union member_data) + 1];
};

/*
 * open an archive and determine its type
 */

int
openar(register char* name, char* mode)
{
	int			arfd;
	register struct ar*	ar;
	long			size;
	unsigned long		uptodate;
	struct stat		st;
	union header		header;
	union magic		magic;

	internal.arupdate = 0;
	for (arfd = 0; arfd < elementsof(artab); arfd++)
		if (!artab[arfd].fp)
			break;
	if (arfd >= elementsof(artab))
		goto bad;
	ar = &artab[arfd];
	if (!(ar->fp = sfopen(NiL, name, mode)))
		goto bad;
	if (fstat(sffileno(ar->fp), &st))
		goto bad;
	ar->time = st.st_mtime;
#if DEBUG
	if (state.test & 0x00000020)
		goto local;
#endif
	if (sfread(ar->fp, (char*)&magic.data.pdp, sizeof(magic.data.pdp)) != sizeof(magic.data.pdp))
		goto local;
	if (magic.data.pdp == (unsigned int_2)MAGIC_pdp || swapget(1, (char*)&magic.data.pdp, 2) == (unsigned int_2)MAGIC_pdp && (ar->swap = 3))
	{
		ar->type = TYPE_pdp;
		ar->first = sftell(ar->fp);
		ar->sym_offset = 0;
		return arfd;
	}
	if (sfseek(ar->fp, (Sfoff_t)0, 0) < 0 || sfread(ar->fp, magic.data.V0, sizeof(magic.data.V0)) != sizeof(magic.data.V0))
		goto local;
	magic.term[sizeof(magic.data.V0)] = 0;
	if (streq(magic.data.V0, MAGIC_V0))
	{
		ar->type = TYPE_V0;
		if (sfseek(ar->fp, (Sfoff_t)0, 0) < 0 ||
		    sfread(ar->fp, (char*)&header.data.V0, sizeof(header.data.V0)) != sizeof(header.data.V0))
			goto local;
#if __pdp11__ || pdp11
		error(3, "%s: %s archive format not supported", name, NAME_V0);
		goto local;
#else
		ar->sym_offset = sftell(ar->fp);
		if (sfseek(ar->fp, (Sfoff_t)(swapget(0, header.data.V0.ar_syms, sizeof(header.data.V0.ar_syms)) * sizeof(struct symbol_V0)), 1) < 0)
			goto local;
		ar->first = sftell(ar->fp);
		return arfd;
#endif
	}
	if (sfseek(ar->fp, (Sfoff_t)0, 0) < 0 || sfread(ar->fp, magic.data.port, sizeof(magic.data.port)) != sizeof(magic.data.port))
		goto local;
	magic.term[sizeof(magic.data.port)] = 0;
	if (streq(magic.data.port, MAGIC_port))
	{
		/*
		 * check for a symbol directory
		 */

		header.term[sizeof(header.data.port)] = 0;
		if (sfread(ar->fp, (char*)&header.data.port, sizeof(header.data.port)) != sizeof(header.data.port) ||
		    strncmp(ENDHDR_port, header.data.port.ar_fmag, sizeof(header.data.port.ar_fmag)))
			goto local;
#if __pdp11__ || pdp11
		error(3, "%s: %s archive format not supported", name, NAME_port);
		goto local;
#else
		uptodate = 0;
		if (strmatch(header.data.port.ar_name, SYMDIR_port) && (ar->type = TYPE_port) || strmatch(header.data.port.ar_name, SYMDIR_rand) && (ar->type = TYPE_rand))
		{
			if (sfsscanf(header.data.port.ar_size, "%ld", &size) != 1)
				goto local;
			size += (size & 01);
			ar->sym_offset = sftell(ar->fp);
			if (ar->type == TYPE_rand && (sfsscanf(header.data.port.ar_date, "%lu", &uptodate) != 1 || (unsigned long)ar->time > uptodate + (strmatch(header.data.port.ar_name, SYMDIR_strict) ? 0 : SYMDIR_age)))
				uptodate = 0;
		}
		else
		{
			/*
			 * there is no symbol directory
			 */

			header.data.port.ar_date[0] = 0;
			ar->type = strchr(header.data.port.ar_name, TERM_port) && strmatch(header.data.port.ar_name, SYMDIR_other) ? TYPE_port : TYPE_rand;
			ar->sym_offset = 0;
			size = -sizeof(header.data.port);
		}
		if (sfseek(ar->fp, (Sfoff_t)size, 1) < 0)
			goto local;
		ar->first = sftell(ar->fp);
		while (sfread(ar->fp, (char*)&header.data.port, sizeof(header.data.port)) == sizeof(header.data.port) && header.data.port.ar_name[0] == TERM_port)
		{
			if (sfsscanf(header.data.port.ar_size, "%ld", &size) != 1)
				goto local;
			size += (size & 01);
			if (!ar->names && header.data.port.ar_name[1] == TERM_port && header.data.port.ar_name[2] == ' ')
			{
				/*
				 * long name string table
				 */

				if (!(ar->names = newof(0, char, size, 0)) || sfread(ar->fp, ar->names, size) != size)
					goto local;
			}
			else if (isdigit(header.data.port.ar_name[1]))
				break; 
			else if (sfseek(ar->fp, (Sfoff_t)size, SEEK_CUR) < 0)
				goto local;
			ar->first = sftell(ar->fp);
		}
		if (sfseek(ar->fp, (Sfoff_t)ar->first, 0) != ar->first)
			goto local;
		if (ar->type == TYPE_rand && !uptodate)
			internal.arupdate = UPDATE_rand;
		return arfd;
#endif
	}
	if (sfseek(ar->fp, (Sfoff_t)0, 0) < 0 || sfread(ar->fp, magic.data.aix, sizeof(magic.data.aix)) != sizeof(magic.data.aix))
		goto local;
	magic.term[sizeof(magic.data.aix)] = 0;
	if (streq(magic.data.aix, MAGIC_aix))
	{
		ar->type = TYPE_aix;
		if (sfseek(ar->fp, (Sfoff_t)0, 0) < 0 || sfread(ar->fp, (char*)&header.data.aix, sizeof(header.data.aix)) != sizeof(header.data.aix))
			goto local;
		header.term[sizeof(header.data.aix)] = 0;
		if (sfsscanf(header.data.aix.fl_gstoff, "%ld", &ar->sym_offset) != 1)
			goto local;
		if (sfsscanf(header.data.aix.fl_fstmoff, "%ld", &ar->first) != 1)
			goto local;
		if (sfsscanf(header.data.aix.fl_memoff, "%ld", &ar->last) != 1)
			goto local;
		return arfd;
	}
 local:
	sfclose(ar->fp);
	if (ar->fp = fapply(internal.scan, null, name, SCAN_local, CO_ALWAYS|CO_IGNORE|CO_LOCAL|CO_SILENT|CO_URGENT))
	{
		ar->type = TYPE_local;
		ar->first = 0;
		internal.arupdate = UPDATE_local;
		return arfd;
	}
 bad:
	closear(arfd);
	return -1;
}

/*
 * close an archive opened by openar()
 */

void
closear(int arfd)
{
	register struct ar*	ar;

	if (ar = arpointer(arfd))
	{
		sfclose(ar->fp);
		ar->fp = 0;
		if (ar->names)
		{
			free(ar->names);
			ar->names = 0;
		}
	}
}

/*
 * walk through an archive
 * d==0 updates the modify time of preselected members (see touchar())
 * else each member is recorded using addfile()
 */

int
walkar(struct dir* d, int arfd, char* name)
{
	register struct ar*	ar;
	register char*		s;
	register struct rule*	r;
	char*			op;
	char*			mem;
	char			buf[512];
	long			offset;
	long			patch;
	unsigned long		date;
	union header		header;
	union member		member;
	int			len;
	int			c;

	if (!(ar = arpointer(arfd)))
		return 0;
	if (d)
		putar(d->name, d);
	op = "seek";
	mem = null;
	sfseek(ar->fp, (Sfoff_t)ar->first, 0);
	offset = 0;
	patch = -1;
	switch (ar->type)
	{
	case TYPE_pdp:
		if (d)
			d->truncate = 14;
		mem = buf;
		header.term[sizeof(header.data.pdp)] = 0;
		while (sfseek(ar->fp, (Sfoff_t)offset, 1) >= 0)
		{
			if (sfread(ar->fp, header.data.pdp.ar_name, sizeof(header.data.pdp) - sizeof(header.data.pdp.ar_pad)) != (sizeof(header.data.pdp) - sizeof(header.data.pdp.ar_pad)))
			{
				if (!sferror(ar->fp))
					return 1;
				else
				{
					op = "read";
					break;
				}
			}
			strncpy(mem, header.data.pdp.ar_name, 14);
			mem[14] = 0;
			if (d)
			{
				if ((date = swapget(ar->swap, (char*)&header.data.pdp.ar_date, 4)) > ar->time)
				{
					error(1, "member %s is newer than archive %s", mem, name);
					date = ar->time;
				}
				addfile(d, mem, date);
			}
			else if ((r = getrule(mem)) && r->status == TOUCH)
			{
				r->status = EXISTS;
				staterule(RULE, r, NiL, 1)->time = header.data.pdp.ar_date = r->time = CURTIME;
				if (ar->swap)
					header.data.pdp.ar_date = swapget(ar->swap, (char*)&header.data.pdp.ar_date, 4);
				state.savestate = 1;
				if (sfseek(ar->fp, -(Sfoff_t)(sizeof(header.data.pdp) - sizeof(header.data.pdp.ar_pad)), 1) < 0)
					break;
				if (sfwrite(ar->fp, header.data.pdp.ar_name, sizeof(header.data.pdp) - sizeof(header.data.pdp.ar_pad)) != (sizeof(header.data.pdp) - sizeof(header.data.pdp.ar_pad)))
				{
					op = "write";
					break;
				}
				if (!state.silent)
					error(0, "touch %s/%s", name, mem);
				ntouched--;
			}
			offset = swapget(ar->swap, (char*)&header.data.pdp.ar_size, 4);
			offset += (offset & 01);
		}
		break;
#if !_pdp11__ && !pdp11
	case TYPE_V0:
		if (d)
			d->truncate = 15;
		mem = member.data.V0.arf_name;
		header.term[sizeof(header.data.V0)] = 0;
		member.term[sizeof(member.data.V0)] = 0;
		while (sfseek(ar->fp, (Sfoff_t)offset, 1) >= 0)
		{
			if (sfread(ar->fp, (char*)&member.data.V0, sizeof(member.data.V0)) != sizeof(member.data.V0))
			{
				if (!sferror(ar->fp))
					return 1;
				else
				{
					op = "read";
					break;
				}
			}
			if (d)
			{
				if ((date = swapget(0, member.data.V0.arf_date, sizeof(member.data.V0.arf_date))) > ar->time)
				{
					error(1, "member %s is newer than archive %s", mem, name);
					date = ar->time;
				}
				addfile(d, mem, date);
			}
			else if ((r = getrule(mem)) && r->status == TOUCH)
			{
				r->status = EXISTS;
				staterule(RULE, r, NiL, 1)->time = r->time = CURTIME;
				state.savestate = 1;
				swapput(0, member.data.V0.arf_date, sizeof(member.data.V0.arf_date), r->time);
				if (sfseek(ar->fp, -(Sfoff_t)sizeof(member.data.V0), 1) < 0)
					break;
				if (sfwrite(ar->fp, (char*)&member.data.V0, sizeof(member.data.V0)) != sizeof(member.data.V0))
				{
					op = "write";
					break;
				}
				if (!state.silent)
					error(0, "touch %s/%s", name, mem);
				ntouched--;
			}
			offset = swapget(0, member.data.V0.arf_size, sizeof(member.data.V0.arf_size));
			offset += (offset & 01);
		}
		break;
	case TYPE_port:
	case TYPE_rand:
		if (d && !ar->names)
			d->truncate = 14;
		mem = header.data.port.ar_name;
		header.term[sizeof(header.data.port)] = 0;
		while (sfseek(ar->fp, (Sfoff_t)offset, 1) >= 0)
		{
			if (sfread(ar->fp, (char*)&header.data.port, sizeof(header.data.port)) != sizeof(header.data.port) ||
			    strncmp(header.data.port.ar_fmag, ENDHDR_port, SENDHDR_port))
			{
				if (!sferror(ar->fp))
				{
					if (d)
						return 1;
					else if (!patch && ar->type == TYPE_rand && ar->sym_offset)
					{
						/*
						 * patch symbol directory time
						 */

						patch = SYMDIR_age;
						if (sfseek(ar->fp, (Sfoff_t)MSIZE_port, 0) < 0)
							break;
						offset = 0;
						continue;
					}
					else
						return 1;
				}
				else
				{
					op = "read";
					break;
				}
			}
			if (ar->names && *(mem = header.data.port.ar_name) == TERM_port)
				mem = ar->names + strtol(mem + 1, NiL, 10);
			if ((s = strchr(mem, TERM_port)) || (s = strchr(mem, TERM_rand)))
			{
				c = *s;
				*s = 0;
			}
			if (d)
			{
				if (!ar->names && (len = strlen(mem)) > d->truncate)
					d->truncate = len;
				if (sfsscanf(header.data.port.ar_date, "%lu", &date) != 1)
				{
					op = "date field";
					break;
				}
				if (date > ar->time)
				{
					error(1, "member %s is newer than archive %s", mem, name);
					date = ar->time;
				}
				addfile(d, mem, date);
			}
			else if (patch > 0 || (r = getrule(mem)) && r->status == TOUCH)
			{
				date = CURTIME;
				if (patch <= 0)
				{
					patch = 0;
					r->status = EXISTS;
					staterule(RULE, r, NiL, 1)->time = r->time = date;
					state.savestate = 1;
				}
				if (sfseek(ar->fp, -(Sfoff_t)sizeof(header.data.port), 1) < 0)
					break;
				sfsprintf(buf, sizeof(buf), "%-12lu", date + patch);
				strncpy(header.data.port.ar_date, buf, 12);
				if (s)
					*s = c;
				if (sfwrite(ar->fp, (char*)&header.data.port, sizeof(header.data.port)) != sizeof(header.data.port))
				{
					op = "write";
					break;
				}
				if (patch > 0)
					return 1;
				if (s)
					*s = 0;
				if (!state.silent)
					error(0, "touch %s/%s", name, mem);
				ntouched--;
			}
			if (sfsscanf(header.data.port.ar_size, "%ld", &offset) != 1)
			{
				op = "size field";
				break;
			}
			offset += (offset & 01);
		}
		break;
#endif
	case TYPE_aix:
		*(mem = buf) = 0;
		offset = ar->first;
		header.term[sizeof(header.data.aix)] = 0;
		for (;;)
		{
			if (offset >= ar->last)
				return 0;
			if (sfseek(ar->fp, (Sfoff_t)offset, 0) < 0)
				break;
			if (sfread(ar->fp, (char*)&member.data.aix, sizeof(member.data.aix)) != sizeof(member.data.aix))
			{
				if (!sferror(ar->fp))
					return 1;
				else
				{
					op = "read";
					break;
				}
			}
			member.term[sizeof(member.data.aix)] = 0;
			if (sfsscanf(member.data.aix.ar_namlen, "%d", &len) != 1)
			{
				op = "name length";
				break;
			}
			strncpy(mem, member.data.aix._ar_name.ar_name, 2);
			if (len > 2)
			{
				if (sfread(ar->fp, mem + 2, len - 2) != len - 2)
				{
					op = "name read";
					break;
				}
				if (sfseek(ar->fp, -(Sfoff_t)(len - 2), 1) < 0)
				{
					op = "name seek";
					break;
				}
			}
			mem[len] = 0;
			if (d)
			{
				if (sfsscanf(member.data.aix.ar_date, "%lu", &date) != 1)
				{
					op = "date field";
					break;
				}
				if (date > ar->time)
				{
					error(1, "member %s is newer than archive %s", mem, name);
					date = ar->time;
				}
				addfile(d, mem, date);
			}
			else if ((r = getrule(mem)) && r->status == TOUCH)
			{
				char	tm[13];

				r->status = EXISTS;
				staterule(RULE, r, NiL, 1)->time = r->time = CURTIME;
				state.savestate = 1;
				sfsprintf(tm, sizeof(tm), "%-12lu", r->time);
				strncpy(member.data.aix.ar_date, tm, 12);
				if (sfseek(ar->fp, -(Sfoff_t)sizeof(member.data.aix), 1) < 0)
					break;
				if (sfwrite(ar->fp, (char*)&member.data.aix, sizeof(member.data.aix)) != sizeof(member.data.aix))
				{
					op = "write";
					break;
				}
				if (!state.silent)
					error(0, "touch %s/%s", name, mem);
				ntouched--;
			}
			if (sfsscanf(member.data.aix.ar_nxtmem, "%ld", &offset) != 1)
			{
				op = "next member offset";
				break;
			}
		}
		break;
	case TYPE_local:
		if (!d)
		{
			op = "write";
			break;
		}
		offset = 0;
		while (s = sfgetr(ar->fp, '\n', 1))
		{
			/*
			 * assume ``junk Mmm ... member''
			 */

			if (!(mem = strrchr(s, ' ')))
				continue;
			*mem++ = 0;
			if (offset++ || !strmatch(mem, SYMDIR_local))
				while (s = strchr(s, ' '))
					if (isupper(*++s) && islower(s[1]) && islower(s[2]) && s[3] == ' ')
			{
				date = tmdate(s, &op, NiL);
				if (!*op)
				{
					/*
					 * secs may have been truncated
					 */

					if (offset <= 2)
					{
						time_t	tm;

						tm = date;
						tmform(buf, "%S", &tm);
						if (buf[0] == '0' && buf[1] == '0' && (r = staterule(RULE, NiL, mem, 0)) && (patch = r->time - date) > 0 && patch < 60)
						{
							date = r->time;
							offset = 1;
						}
						else
							offset = 2;
					}
					if (date > ar->time)
					{
						error(1, "member %s is newer than archive %s", mem, name);
						date = ar->time;
					}
					addfile(d, mem, date);
					if ((len = strlen(mem)) > d->truncate)
						d->truncate = len;
					break;
				}
			}
		}
		return 0;
	}
	error(1, "%s error %s %s%sarchive %s",
		op,
		d ? "scanning" : "touching",
		mem,
		*mem ? " in " : null,
		name);
	return 0;
}

/*
 * check for any untouched r->status==TOUCH members
 */

static int
chktouch(const char* s, char* v, void* h)
{
	struct rule*	r = (struct rule*)v;

	NoP(s);
	NoP(h);
	if (r->status == TOUCH)
	{
		r->status = FAILED;
		error(1, "archive member %s not touched", r->name);
	}
	return 0;
}

/*
 * touch the modify time of an archive member (and the archive itself!)
 */

void
touchar(char* name, register char* member)
{
	register struct rule*	r;
	int			arfd;

	if (member)
	{
		if (!(r = getrule(member)))
			error(PANIC, "%s[%s] not scanned", name, member);
		else
		{
			r->status = TOUCH;
			ntouched++;
		}
	}
	else if ((arfd = openar(name, "r+")) >= 0)
	{
		walkar(NiL, arfd, name);
		closear(arfd);
		if (ntouched > 0)
		{
			message((-2, "checking %d untouched members in %s", ntouched, name));
			hashwalk(table.rule, 0, chktouch, NiL);
		}
		ntouched = 0;
	}
}
