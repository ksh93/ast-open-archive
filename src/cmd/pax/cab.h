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
 * cab interface definitions
 */

#include <int.h>

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

typedef struct				/* cab file header		*/
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

typedef struct				/* cab optional header info	*/
{
	struct
	{
	char*		name;		/* cab file name		*/
	char*		disk;		/* disk id			*/
	}		prev,next;
} Cabopt_t;

typedef struct				/* cab chunk header		*/
{
	unsigned int_4	offset;		/* data offset			*/
	unsigned int_2	blocks;		/* block count			*/
	unsigned int_2	compress;	/* compression type		*/
} Cabchunk_t;

typedef struct				/* chunk block header		*/
{
	unsigned int_4	checksum;	/* decompressed checksum	*/
	unsigned int_2	compressed;	/* compressed size		*/
	unsigned int_2	decompressed;	/* decompressed size		*/
} Cabblock_t;

typedef struct				/* file entry header		*/
{
	unsigned int_4	size;		/* uncompressed data size	*/
	unsigned int_4	offset;		/* decompressed chunk offset	*/
	unsigned int_2	chunk;		/* chunk index			*/
	unsigned int_2	date;		/* dos date			*/
	unsigned int_2	time;		/* dos time			*/
	unsigned int_2	attr;		/* CAB_ATTR_*			*/
} Cabentry_t;

typedef struct				/* file entry			*/
{
	Cabentry_t	entry;		/* entry header			*/
	char*		name;		/* file name			*/
	time_t		date;		/* mtime			*/
	int		append;		/* append this part to existing	*/
} Cabfile_t;
