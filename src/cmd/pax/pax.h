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
 * AT&T Research
 *
 * pax common definitions
 */

#include <ast.h>
#include <ls.h>
#include <sig.h>
#include <ftwalk.h>
#include <ctype.h>
#include <ccode.h>
#include <hash.h>
#include <proc.h>
#include <regex.h>
#include <error.h>
#include <times.h>
#include <swap.h>
#include <align.h>
#include <modex.h>
#include <sfstr.h>
#include <debug.h>
#include <stdarg.h>
#include <int.h>
#include <sum.h>
#include <ardir.h>

#include "FEATURE/local"

#define PANIC		ERROR_PANIC|ERROR_SOURCE,__FILE__,__LINE__

#define bcount(ap)	((ap)->io->last-(ap)->io->next)
#define bsave(ap)	(state.backup=(*(ap)->io))

#define BUFFER_FD_MAX	(-2)
#define BUFFER_FD_MIN	(-3)

#undef	getbuffer
#define getbuffer(n)	(((n)>=BUFFER_FD_MIN&&(n)<=BUFFER_FD_MAX)?&state.buffer[BUFFER_FD_MAX-(n)]:(Buffer_t*)0)
#undef	setbuffer
#define setbuffer(n)	(BUFFER_FD_MAX-(n))

#define HOLE_MIN	1024

#define holeinit(fd)	(state.hole=0)
#define holedone(fd)	do if(state.hole){lseek(fd,state.hole-1,SEEK_CUR);state.hole=0;write(fd,"",1);} while(0)

#define ropath(p)	(p[0]=='.'&&(p[1]==0||p[1]=='.'&&(p[2]==0||p[2]=='.'&&p[3]==0))||p[0]=='-'&&p[1]==0)

#define SECTION_CONTROL	1			/* control io		*/
#define SECTION_DATA	2			/* data io		*/

#define SECTION_MAX	3

#define SECTION(p)	(p)->section

#define METER_parts	20

#define NOW		time(NiL)

/*
 * info per archive format
 *
 * NOTES:
 *
 *	format indices must agree with format[] table below
 *
 *	the *_NAMESIZE maximum file name lengths include the trailing \0
 *
 *	PATH_MAX must be >= max(*_NAMESIZE)
 */

#define ALAR		0
#define ALAR_NAME	"ansi"
#define ALAR_DESC	"ANSI standard label tape; for tape devices only"
#define ALAR_REGULAR	4
#define ALAR_SPECIAL	ALAR_REGULAR
#define ALAR_LABEL	2048
#define ALAR_HEADER	80
#define ALAR_VARHDR	9
#define ALAR_NAMESIZE	17
#define ALAR_RECSIZE	(BLOCKSIZE>>2)
#define ALAR_RECFORMAT	'D'
#define ALAR_ALIGN	0
#define ALAR_FLAGS	IN|OUT
#define ALAR_ID		"SLT"

#define BINARY		1
#define BINARY_NAME	"binary"
#define BINARY_DESC	"cpio binary with symlinks"
#define BINARY_REGULAR	DEFBUFFER
#define BINARY_SPECIAL	DEFBLOCKS
#define BINARY_HEADER	26
#define BINARY_NAMESIZE	256
#define BINARY_ALIGN	2
#define BINARY_FLAGS	IN|OUT

typedef struct Hdr_binary_s		/* binary header		*/
{
	short		magic;
	unsigned short	dev;
	unsigned short	ino;
	unsigned short	mode;
	unsigned short	uid;
	unsigned short	gid;
	short		links;
	unsigned short	rdev;
	unsigned short	mtime[2];
	short		namesize;
	unsigned short	size[2];
} Hdr_binary_t;

#define CPIO		2
#define CPIO_NAME	"cpio"
#define CPIO_DESC	"cpio character with symlinks"
#define CPIO_REGULAR	DEFBUFFER
#define CPIO_SPECIAL	DEFBLOCKS
#define CPIO_HEADER	76
#define CPIO_MAGIC	070707
#define CPIO_EXTENDED	1
#define CPIO_TRAILER	"TRAILER!!!"
#define CPIO_TRUNCATE(x) ((x)&0177777)
#define CPIO_NAMESIZE	256
#define CPIO_ALIGN	0
#define CPIO_FLAGS	IN|OUT

#define IBMAR		3
#define IBMAR_NAME	"ibm"
#define IBMAR_DESC	"EBCDIC standard label tape; for tape devices only"
#define IBMAR_REGULAR	ALAR_REGULAR
#define IBMAR_SPECIAL	ALAR_SPECIAL
#define IBMAR_ALIGN	ALAR_ALIGN
#define IBMAR_FLAGS	IN|OUT

/*
 * NOTE: since USTAR is an extension of TAR they share the same header
 */

#define TAR		4
#define TAR_NAME	"tar"
#define TAR_DESC	"tar with symlinks"
#define TAR_REGULAR	DEFBUFFER
#define TAR_SPECIAL	DEFBLOCKS
#define TAR_HEADER	TBLOCK
#define TAR_NAMESIZE	100
#define TAR_SUMASK	((1L<<(sizeof(tar_header.chksum)-1)*3)-1)
#define TAR_ALIGN	BLOCKSIZE
#define TAR_FLAGS	IN|OUT

#define header		tar_header_info
#define hblock		tar_header_block
#include <tar.h>
#undef header
#undef hblock

typedef union tar_header_block Hdr_tar_t;

#define USTAR		5
#define USTAR_NAME	TMAGIC
#define USTAR_DESC	"POSIX 1003.1-1988 tar"
#define USTAR_REGULAR	TAR_REGULAR
#define USTAR_SPECIAL	TAR_SPECIAL
#define USTAR_HEADER	TAR_HEADER
#define USTAR_ALIGN	TAR_ALIGN
#define USTAR_FLAGS	IN|OUT

#define tar_header	tar_header_block.dbuf
#define tar_block	tar_header_block.dummy

/*
 * s5r4 expanded cpio(5) char format
 * pax won't even bother with the binary counterpart
 */

#define ASC		6
#define ASC_NAME	"asc"
#define ASC_DESC	"s5r4 extended cpio character"
#define ASC_REGULAR	CPIO_REGULAR
#define ASC_SPECIAL	CPIO_SPECIAL
#define ASC_HEADER	110
#define ASC_MAGIC	070701
#define ASC_NAMESIZE	1024
#define ASC_ALIGN	4
#define ASC_FLAGS	IN|OUT

/*
 * s5r4 expanded cpio(5) char format with checksum
 * (misnamed ``crc'' -- the check is a simple sum of the header)
 * pax won't even bother with the binary counterpart
 */

#define ASCHK		7
#define ASCHK_NAME	"aschk"
#define ASCHK_DESC	"s5r4 extended cpio character with checksum"
#define ASCHK_REGULAR	ASC_REGULAR
#define ASCHK_SPECIAL	ASC_SPECIAL
#define ASCHK_MAGIC	070702
#define ASCHK_ALIGN	ASC_ALIGN
#define ASCHK_FLAGS	IN|OUT

/*
 * backup saveset format
 */

#define SAVESET		8
#define SAVESET_NAME	"vmsbackup"
#define SAVESET_DESC	"VMS backup savset; for tape devices only"
#define SAVESET_REGULAR	0
#define SAVESET_SPECIAL	0
#define SAVESET_ALIGN	0
#define SAVESET_FLAGS	IN
#define SAVESET_ID	"DECVMS"
#define SAVESET_IMPL	"BACKUP"

#define BLKHDR_SIZE	256
#define BLKHDR_hdrsiz	0
#define BLKHDR_blksiz	40

#define FILHDR_SIZE	4
#define FILHDR_MAGIC	257
#define FILHDR_namelen	128
#define FILHDR_size	0
#define FILHDR_type	2
#define FILHDR_data	4

#define FILATT_blocks	10
#define FILATT_frag	12
#define FILATT_recatt	1
#define FILATT_recfmt	0
#define FILATT_reclen	2
#define FILATT_recvfc	15

#define RECHDR_SIZE	16
#define RECHDR_size	0
#define RECHDR_type	2

#define REC_file	3
#define REC_vbn		4

/*
 * the proposed POSIX IEEE Std 1003.1B-1990 interchange format
 */

#define PAX		9
#define PAX_NAME	"pax"
#define PAX_DESC	"POSIX 1003.2b-1995 extended ustar"
#define PAX_REGULAR	TAR_REGULAR
#define PAX_SPECIAL	TAR_SPECIAL
#define PAX_HEADER	TAR_HEADER
#define PAX_ALIGN	TAR_ALIGN
#define PAX_FLAGS	IN|OUT

/*
 * object archive/library
 */

#define AR		10
#define AR_NAME		"library"
#define AR_DESC		"object library archive"
#define AR_REGULAR	0
#define AR_SPECIAL	0
#define AR_ALIGN	2
#define AR_FLAGS	IN

/*
 * cql virtual directory archive
 */

#include <vdb.h>

#define VDB		11
#define VDB_NAME	VDB_MAGIC
#define VDB_DESC	"virtual database"
#define VDB_REGULAR	DEFBUFFER
#define VDB_SPECIAL	DEFBLOCKS
#define VDB_ALIGN	0
#define VDB_FLAGS	IN|OUT

/*
 * zip archive
 */

#define ZIP		12
#define ZIP_NAME	"zip"
#define ZIP_DESC	"zip 2.1 / PKZIP 2.04g archive"
#define ZIP_REGULAR	DEFBUFFER
#define ZIP_SPECIAL	DEFBLOCKS
#define ZIP_ALIGN	0
#define ZIP_FLAGS	IN

#include "zip.h"

/*
 * ms cabinet file
 */

#define CAB		13
#define CAB_NAME	"cab"
#define CAB_DESC	"MS cabinet file"
#define CAB_REGULAR	DEFBUFFER
#define CAB_SPECIAL	DEFBLOCKS
#define CAB_ALIGN	0
#define CAB_FLAGS	IN

#include "cab.h"

typedef struct Cab_s
{
	char*		format;
	Cabheader_t	header;
	Cabopt_t	optional;
	Cabchunk_t*	chunk;
	Cabfile_t*	file;
	struct
	{
	unsigned int	header;
	unsigned int	chunk;
	unsigned int	block;
	}		reserved;
	int		index;
} Cab_t;

/*
 * redhat rpm file
 */

#define RPM		14
#define RPM_NAME	"rpm"
#define RPM_DESC	"Redhat rpm package encapsulated cpio"
#define RPM_REGULAR	DEFBUFFER
#define RPM_SPECIAL	DEFBLOCKS
#define RPM_ALIGN	0
#define RPM_FLAGS	IN

#include "rpm.h"

/*
 * mime multipart
 */

#define MIME		15
#define MIME_NAME	"mime"
#define MIME_DESC	"encapsulated mime"
#define MIME_MAGIC	"--"
#define MIME_REGULAR	DEFBUFFER
#define MIME_SPECIAL	DEFBLOCKS
#define MIME_ALIGN	0
#define MIME_FLAGS	IN

/*
 * ms outlook tnef -- how about zip + text name=value attributes?
 */

#define TNEF		16
#define TNEF_NAME	"tnef"
#define TNEF_DESC	"MS outlook transport neutral encapsulation format"
#define TNEF_MAGIC	0x223e9f78
#define TNEF_REGULAR	DEFBUFFER
#define TNEF_SPECIAL	DEFBLOCKS
#define TNEF_ALIGN	0
#define TNEF_FLAGS	IN

typedef struct Tnef_s
{
	char*		format;
	off_t		offset;
} Tnef_t;

/*
 * compression pseudo formats -- COMPRESS is first
 */

#define COMPRESS		17
#define COMPRESS_NAME		"compress"
#define COMPRESS_DESC		"Lempel-Ziv compression"
#define COMPRESS_ALGORITHM	0
#define COMPRESS_UNDO		{"zcat"}
#define COMPRESS_MAGIC		0x1f9d0000
#define COMPRESS_MAGIC_MASK	0xffff0000

#define GZIP			18
#define GZIP_NAME		"gzip"
#define GZIP_DESC		"gzip compression"
#define GZIP_ALGORITHM		"-9"
#define GZIP_UNDO		{"gunzip"},{"ratz","-c"}
#define GZIP_MAGIC		0x1f8b0000
#define GZIP_MAGIC_MASK		0xffff0000

#define BZIP			19
#define BZIP_NAME		"bzip2"
#define BZIP_DESC		"bzip compression"
#define BZIP_ALGORITHM		0
#define BZIP_UNDO		{"bunzip2"}
#define BZIP_MAGIC		0x425a6800
#define BZIP_MAGIC_MASK		0xffffff00

/*
 * delta pseudo formats -- DELTA is first
 */

#define DELTA			20
#define DELTA_NAME		"delta"
#define DELTA_DESC		"vdelta difference/compression"
#define DELTA_ALGORITHM		"94"

#define DELTA_88		21
#define DELTA_88_NAME		"delta88"
#define DELTA_88_DESC		"delta88 difference/compression"
#define DELTA_88_ALGORITHM	"88"

#define DELTA_IGNORE		22
#define DELTA_IGNORE_NAME	"ignore"
#define DELTA_IGNORE_DESC	"ignore delta headers"

#define DELTA_PATCH		23
#define DELTA_PATCH_NAME	"patch"
#define DELTA_PATCH_DESC	"delta using standard archive formats"

/*
 * format generic definitions
 */

#define IN_DEFAULT	CPIO		/* first getheader() state	*/
#define OUT_DEFAULT	CPIO		/* default output format	*/

#define IN		(1<<0)		/* copy in			*/
#define OUT		(1<<1)		/* copy out			*/

#define SETIDS		(1<<0)		/* set explicit uid and gid	*/

#define INFO_MATCH	"*([A-Z0-9!])!!!"/* info header file name match	*/
#define INFO_SEP	'!'		/* info header field separator	*/
#define INFO_ORDERED	'O'		/* delta on ordered base	*/

#define ID		"PAX"		/* info header id		*/
#define IDLEN		(sizeof(ID)-1)	/* strlen(ID)			*/

#define IMPLEMENTATION	"ATTPAX1"	/* implementation id		*/

#define TYPE_COMPRESS	'C'		/* compress encoding type	*/
#define TYPE_DELTA	'D'		/* delta encoding type		*/

#define DELTA_SRC	0		/* source info			*/
#define DELTA_TAR	1		/* target info			*/
#define DELTA_DEL	2		/* delta info			*/
#define DELTA_DATA	3		/* data info mask		*/
#define DELTA_BIO	(1<<2)		/* bio (no arg)			*/
#define DELTA_BUFFER	(1<<3)		/* buffer arg			*/
#define DELTA_FD	(1<<4)		/* fd arg			*/
#define DELTA_HOLE	(1<<5)		/* holewrite() fd arg		*/
#define DELTA_OFFSET	(1<<6)		/* offset arg			*/
#define DELTA_SIZE	(1<<7)		/* size arg			*/
#define DELTA_OUTPUT	(1<<8)		/* output data			*/
#define DELTA_TEMP	(1<<9)		/* temp output fd		*/
#define DELTA_FREE	(1<<10)		/* free fd or buffer		*/
#define DELTA_COUNT	(1<<11)		/* set in|out count		*/
#define DELTA_LIST	(1<<12)		/* listentry() if ok		*/

#define DELTA_create	'c'		/* delta create data op		*/
#define DELTA_delete	'd'		/* delta delete data op		*/
#define DELTA_nop	'x'		/* no delta operation pseudo op	*/
#define DELTA_pass	'p'		/* delta pass pseudo op		*/
#define DELTA_update	'u'		/* delta update data op		*/
#define DELTA_verify	'v'		/* delta verify data op		*/
#define DELTA_zip	'z'		/* delta zip data op		*/

#define DELTA_TRAILER	10		/* delta trailer output size	*/

#define DELTA_checksum	'c'		/* delta member checksum	*/
#define DELTA_index	'i'		/* delta header index		*/
#define DELTA_trailer	't'		/* delta trailer size		*/

#define DELTA_LO(c)	((c)&0xffff)	/* lo order checksum bits	*/
#define DELTA_HI(c)	DELTA_LO(c>>16)	/* hi order checksum bits	*/

#define INVALID_ignore		0	/* silently ignore		*/
#define INVALID_prompt		1	/* prompt for new name		*/
#define INVALID_translate	2	/* translate and/or truncate	*/
#define INVALID_UTF8		3	/* convert to UTF8		*/

#define NOLINK		0		/* not a link			*/
#define HARDLINK	'1'		/* hard link to previous entry	*/
#define SOFTLINK	'2'		/* soft link to previous entry	*/

#define BLOCKSIZE	512		/* block size			*/
#define IOALIGN		ALIGN_BOUND1	/* io buffer alignment		*/
#define MINBLOCK	1		/* smallest block size		*/
#define DEFBLOCKS	20		/* default blocking		*/
#define DEFBUFFER	16		/* default io buffer blocking	*/
#define MAXBLOCKS	40		/* largest zero fill blocking	*/
#define FILBLOCKS	1024		/* file to file blocking	*/
#define MAXUNREAD	BLOCKSIZE	/* max bunread() count		*/
#define RESETABLE	(-1)		/* default option can be reset	*/

typedef struct Bio_s			/* buffered io info		*/
{
	char*		next;		/* next char pointer		*/
	char*		last;		/* last char+1 pointer		*/
	char*		buffer;		/* io buffer			*/
	off_t		buffersize;	/* buffer size			*/
	off_t		count;		/* char transfer count		*/
	off_t		offset;		/* volume offset		*/
	off_t		size;		/* total size if seekable	*/
	int		fd;		/* file descriptor		*/
	int		skip;		/* volume skip			*/
	int		keep;		/* volume keep after skip	*/
	int		mode;		/* open() O_* mode		*/
	unsigned int	all:1;		/* read all volumes		*/
	unsigned int	blocked:1;	/* blocked device io		*/
	unsigned int	blok:1;		/* BLOK io file			*/
	unsigned int	blokflag:1;	/* io file BLOK flag		*/
	unsigned int	empty:1;	/* last read was empty		*/
	unsigned int	eof:1;		/* hit EOF			*/
	unsigned int	seekable:1;	/* seekable			*/
	unsigned int	unblocked:1;	/* set unblocked device io	*/
} Bio_t;

typedef struct Buffer_s			/* pseudo fd buffer		*/
{
	char*		base;		/* buffer base			*/
	char*		next;		/* current position		*/
	char*		past;		/* overflow position		*/
} Buffer_t;

typedef struct Fileid_s			/* unique file identifier	*/
{
	int		dev;		/* device			*/
	int		ino;		/* inode			*/
} Fileid_t;

typedef struct Link_s			/* link info			*/
{
	char*		name;		/* name				*/
	char*		checksum;	/* hard link checksum		*/
	int		namesize;	/* name size with null byte	*/
	Fileid_t	id;		/* generated link file id	*/
} Link_t;

typedef struct File_s			/* common internal file info	*/
{
	struct Archive_s*ap;		/* !=0 if from buffer		*/
	int		extended;	/* extended header file		*/
	int		fd;		/* >=0 read fd			*/
	char*		id;		/* archive file id		*/
	char*		intermediate;	/* intermediate output name	*/
	int		magic;		/* header magic number		*/
	char*		name;		/* archive file name		*/
	int		namesize;	/* name size with null byte	*/
	char*		path;		/* local file name for reading	*/
	int		perm;		/* original st_mode perm	*/
	struct
	{
	int		op;		/* op				*/
	struct Member_s*base;		/* base file pointer		*/
	off_t		size;		/* target file size		*/
	}		delta;		/* delta info			*/
	struct stat*	st;		/* stat() info from ftwalk()	*/
	int		type;		/* st_mode type			*/
	Link_t*		link;		/* hard link state		*/
	int		linktype;	/* NOLINK, HARDLINK, SOFTLINK	*/
	char*		linkname;	/* link to this path		*/
	int		linknamesize;	/* linkname size with null byte	*/
	char*		uidname;	/* user id name			*/
	char*		gidname;	/* group id name		*/
	struct
	{
	int		blocks;		/* io block count		*/
	int		format;		/* format			*/
	int		section;	/* file section number		*/
	}		record;		/* record format info		*/
	long		checksum;	/* checksum			*/
	unsigned int	chmod:1;	/* must restore mode		*/
	unsigned int	ordered:1;	/* ordered fileout() override	*/
	unsigned int	ro:1;		/* readonly { . .. ... - }	*/
	unsigned int	skip:1;		/* skip this entry		*/
} File_t;

typedef struct Member_s			/* cached member info		*/
{
	File_t*		info;		/* deltapass() file info	*/
	long		mtime;		/* modify time			*/
	off_t		offset;		/* data offset			*/
	off_t		size;		/* data size			*/
	off_t		expand;		/* expanded size		*/
	short		dev;		/* dev				*/
	short		ino;		/* ino				*/
	short		mode;		/* mode				*/
	unsigned int	mark:1;		/* visit mark			*/
} Member_t;

typedef struct Format_s			/* format info			*/
{
	char*		name;		/* name				*/
	char*		desc;		/* description			*/
	unsigned long	regular;	/* default regular blocking	*/
	unsigned long	special;	/* default special blocking	*/
	int		align;		/* trailer alignment		*/
	int		flags;		/* io info			*/
	char*		algorithm;	/* compress algorithm name/flag	*/
	char*		undo[2];	/* compress undo name and arg	*/
	char*		undotoo[2];	/* alternate undo		*/
} Format_t;

typedef struct List_s			/* generic list			*/
{
	struct List_s*	next;		/* next in list			*/
	void*		item;		/* list item pointer		*/
} List_t;

typedef struct Map_s			/* file name map list		*/
{
	struct Map_s*	next;		/* next in list			*/
	regex_t		re;		/* compiled match re		*/
} Map_t;

typedef struct Post_s			/* post processing restoration	*/
{
	time_t		mtime;		/* modify time			*/
	int		mode;		/* permissions			*/
	int		uid;		/* user id			*/
	int		gid;		/* group id			*/
	unsigned int	chmod:1;	/* must restore mode		*/
} Post_t;

typedef union Integral_u		/* byte|half swap probe		*/
{
	unsigned int_4	l;
	unsigned int_2	s[2];
	unsigned int_1	c[4];
} Integral_t;

typedef struct Delta_s			/* delta archive info		*/
{
	struct Archive_s*base;		/* base archive			*/
	int		epilogue;	/* epilogue hit			*/
	int		index;		/* member index			*/
	int		format;		/* 0,DELTA,DELTA_IGNORE,DELTA_PATCH */
	char*		hdr;		/* header pointer		*/
	char		hdrbuf[64];	/* header buffer		*/
	Hash_table_t*	tab;		/* entry table			*/
	int		trailer;	/* optional trailer size	*/
	int		version;	/* encoding type version	*/
} Delta_t;

typedef struct Convert_s		/* char code conversion		*/
{
	int		on;		/* do the conversion		*/
	int		from;		/* convert from this char code	*/
	int		to;		/* convert to this char code	*/
} Convert_t;

typedef struct Archive_s		/* archive info			*/
{
	Ardir_t*	ardir;		/* ardir(3) handle		*/
	Ardirent_t*	ardirent;	/* ardir(3) current entry	*/
	unsigned long	checksum;	/* running checksum		*/
	int		compress;	/* compression index		*/
	Convert_t	convert[SECTION_MAX];/* data/header conversion	*/
	void*		data;		/* format specific data		*/
	Delta_t*	delta;		/* delta info			*/
	int		entries;	/* total number of entries	*/
	int		entry;		/* current entry index		*/
	int		errors;		/* intermediate error count	*/
	int		expected;	/* expected format		*/
	File_t		file;		/* current member file info	*/
	int		format;		/* format			*/
	Bio_t*		io;		/* current buffered io		*/
	Bio_t		mio;		/* main buffered io		*/
	unsigned long	memsum;		/* checksum			*/
	char*		name;		/* io pathname			*/
	char*		names;		/* PORTAR long name directory	*/
	off_t		offset;		/* relative byte offset		*/
	struct
	{
	unsigned long	checksum;	/* old running checksum		*/
	unsigned long	memsum;		/* old checksum			*/
	}		old;
	char*		package;	/* package id			*/
	struct
	{
	char		header[PATH_MAX*2];/* header path name		*/
	char		link[PATH_MAX];	/* link text			*/
	char		name[PATH_MAX];	/* real path name		*/
	char		prev[PATH_MAX];	/* previous entry order check	*/
	char		temp[PATH_MAX];	/* temp intermediate name	*/
	}		path;
	struct Archive_s*parent;	/* parent (delta) for base	*/
	int		part;		/* media change count		*/
	int		peek;		/* already peeked at file entry */
	File_t*		record;		/* record output file		*/
	int		raw;		/* don't convert sections	*/
	int		section;	/* current archive section	*/
	int		selected;	/* number of selected members	*/
	off_t		size;		/* size				*/
	off_t		skip;		/* base archive skip offset	*/
	struct stat	st;		/* memver stat			*/
	int		sum;		/* collect running checksum	*/
	int		swap;		/* swap operation		*/
	int		swapio;		/* io swap operation		*/
	Hash_table_t*	tab;		/* entries to verify		*/
	Bio_t		tio;		/* temporary buffered io	*/
	int		verified;	/* number of verified entries	*/
	int		volume;		/* volume number		*/
} Archive_t;

typedef struct State_s			/* program state		*/
{
	int		acctime;	/* reset file access times	*/
	int		append;		/* append -- must be 0 or 1 !!!	*/
	Bio_t		backup;		/* backup() position		*/
	long		blocksize;	/* explicit buffer size		*/
	Buffer_t	buffer[BUFFER_FD_MAX-BUFFER_FD_MIN+1];/* fd buf	*/
	long		buffersize;	/* io buffer size		*/
	struct
	{
	char*		name;		/* archive member name		*/
	char*		path;		/* local file path		*/
	Sfio_t*		sp;		/* tmp stream			*/
	Sum_t*		sum;		/* method handle		*/
	}		checksum;	/* --checksum state		*/
	int		chmod;		/* must preserve mode		*/
	int		clobber;	/* overwrite output files	*/
	int		complete;	/* files completely in volume	*/
	int		current;	/* current file[] index		*/
	int		delta2delta;	/* -rz- -wz- : retain delta info*/
	int		descend;	/* dir names self+descendents	*/
	char*		destination;	/* pass mode destination dir	*/
	dev_t		dev;		/* . device number		*/
	unsigned short	devcnt;		/* dev assignment count		*/
	int		drop;		/* drop a `.' for each file	*/
	int		dropcount;	/* current line drop count	*/
	int		exact;		/* exact archive read		*/
	char**		files;		/* alternate file name list	*/
	struct
	{
	char**		argv;		/* filter command argv		*/
	char**		patharg;	/* filter file arg in argv	*/
	char*		options;	/* line mode options		*/
	char*		command;	/* line mode command		*/
	char*		path;		/* line mode physical path	*/
	char*		name;		/* line mode logical path	*/
	int		line;		/* line mode			*/
	}		filter;		/* file output filter state	*/
	int		flags;		/* flags			*/
	int		ftwflags;	/* ftwalk() flags		*/
	int		format;		/* default output format	*/
	int		gid;		/* current group id		*/
	struct
	{
	char*		comment;	/* comment text			*/
	int		invalid;	/* invalid path INVALID_ action	*/
	int		linkdata;	/* data with each hard link	*/
	char*		listformat;	/* verbose listing format	*/
	char*		name;		/* name format			*/
	}		header;		/* header specific options	*/
	off_t		hole;		/* one past last hole		*/
	struct
	{
	char		volume[64];	/* volume id			*/
	char		format[7];	/* format id			*/
	char		implementation[8];/* implementation id		*/
	char		owner[15];	/* owner id			*/
	char		standards[20];	/* standards id			*/
	}		id;		/* id strings (including '\0')	*/
	Archive_t*	in;		/* input archive info		*/
	unsigned short	inocnt;		/* ino assignment count		*/
	struct
	{
	char*		name;		/* archive member name		*/
	char*		path;		/* local file path		*/
	Sfio_t*		sp;		/* tmp stream			*/
	}		install;	/* --install state		*/
	int		intermediate;	/* intermediate ouput - rename	*/
	int		interrupt;	/* this signal caused exit	*/
	int		keepgoing;	/* keep going on error		*/
					/* use when possible for -rw	*/
	int		(*linkf)(const char*, const char*);
	Hash_table_t*	linktab;	/* hard link table		*/
	int		list;		/* full file trace		*/
	char*		listformat;	/* verbose listing format	*/
	int		local;		/* reject files/links outside .	*/
	Map_t*		maps;		/* file name maps		*/
	int		matchsense;	/* pattern match sense		*/
	off_t		maxout;		/* max volume/part output count	*/
	struct
	{
	off_t		size;		/* total size			*/
	int		fancy;		/* fancy display		*/
	int		last;		/* length of last display path	*/
	int		on;		/* meter progress		*/
	int		width;		/* display line width		*/
	}		meter;		/* meter state			*/
	int		mkdir;		/* make intermediate dirs	*/
	struct
	{
	char*		magic;		/* separator magic		*/
	size_t		length;		/* separator magic length	*/
	size_t		fill;		/* last member filler size	*/
	}		mime;
	int		modemask;	/* & with mode for chmod()	*/
	int		modtime;	/* retain mtime			*/
	int		newer;		/* append only if newer		*/
	int		operation;	/* IN|OUT operation mode	*/
	Hash_table_t*	options;	/* option table			*/
	int		ordered;	/* sorted base and file list	*/
	int		owner;		/* set owner info		*/
	Archive_t*	out;		/* output archive info		*/
	int		pass;		/* archive to archive		*/
	char**		patterns;	/* name match patterns		*/
	char*		peekfile;	/* stdin file list peek		*/
	char		pwd[PATH_MAX];	/* full path of .		*/
	int		pwdlen;		/* pwd length sans null		*/
	List_t*		proc;		/* procopen() list for finish	*/
	struct
	{
	int		charset;	/* convert record charset	*/
	int		delimiter;	/* directory delimiter		*/
	File_t*		file;		/* current output file		*/
	int		format;		/* record format		*/
	char*		header;		/* file header			*/
	int		headerlen;	/* file header length		*/
	int		line;		/* convert records<->lines	*/
	int		pad;		/* pad output record blocks	*/
	char*		pattern;	/* format match pattern		*/
	int		offset;		/* data buffer offset		*/
	long		size;		/* io size			*/
	char*		trailer;	/* file trailer			*/
	int		trailerlen;	/* file trailer length		*/
	}		record;		/* record info			*/
	Hash_table_t*	restore;	/* post proc restoration table	*/
	Sfio_t*		rtty;		/* tty file read pointer	*/
#if SAVESET
	struct
	{
	char*		block;		/* current block		*/
	long		blocksize;	/* max block size		*/
	char*		bp;		/* block pointer		*/
	int		recatt;		/* record attributes		*/
	int		recfmt;		/* record format		*/
	int		reclen;		/* record length		*/
	int		recvfc;		/* record fixed control length	*/
	int		lastsize;	/* size of last record		*/
	time_t		time;		/* backup time			*/
	}		saveset;	/* backup saveset state		*/
#endif
	int		setgid;		/* set file gid to this value	*/
	int		setuid;		/* set file uid to this value	*/
					/* -L=pathstat() -P=lstat()	*/
	int		(*statf)(const char*, struct stat*);
	int		strict;		/* strict standard conformance	*/
	int		summary;	/* output summary info		*/
	int		test;		/* debug test bits		*/
	struct
	{
	char*		buffer;		/* temporary buffer		*/
	long		buffersize;	/* temporary buffer size	*/
	char*		file;		/* tmp file name		*/
	Sfio_t*		lst;		/* temporary list stream	*/
	Sfio_t*		str;		/* temporary string stream	*/
	}		tmp;		/* temporary stuff		*/
	int		uid;		/* current user id		*/
	int		update;		/* copy file only if newer	*/
	char*		usage;		/* optget() usage string	*/
	struct
	{
	unsigned char	delimiter;	/* header delimiter		*/
	unsigned char	variant;	/* variant header delimiters	*/
	Sfio_t*		directory;	/* directory trailer stream	*/
	struct stat	st;		/* member stat prototype	*/
	struct
	{
	char*		base;		/* header buffer base		*/
	char*		next;		/* next header			*/
	off_t		size;		/* total size			*/
	}		header;
	}		vdb;
	int		verbose;	/* trace files when acted upon	*/
	int		verify;		/* verify action on file	*/
	Sfio_t*		wtty;		/* tty file write pointer	*/
	int		yesno;		/* interactive answer is yes/no	*/
} State_t;

extern char*			definput;
extern char*			defoutput;
extern char*			eomprompt;
extern Format_t			format[];
extern State_t			state;

extern char			alar_header[ALAR_LABEL];
extern Hdr_binary_t		binary_header;
extern Hdr_tar_t		tar_header_block;
extern char			zip_header[ZIP_HEADER];

extern int		addlink(Archive_t*, File_t*);
extern void		append(Archive_t*);
extern long		asc_checksum(char*, int, unsigned long);
extern void		backup(Archive_t*);
extern long		bblock(int);
extern void		bflushin(Archive_t*, int);
extern void		bflushout(Archive_t*);
extern char*		bget(Archive_t*, off_t, off_t*);
extern void		binit(Archive_t*);
extern void		bput(Archive_t*, int);
extern off_t		bread(Archive_t*, void*, off_t, off_t, int);
extern void		brestore(Archive_t*);
extern off_t		bseek(Archive_t*, off_t, int, int);
extern int		bskip(Archive_t*);
extern void		bunread(Archive_t*, void*, int);
extern void		bwrite(Archive_t*, void*, int);
extern int		closeout(Archive_t*, File_t*, int);
extern int		cmpftw(Ftw_t*, Ftw_t*);
extern void		convert(Archive_t*, int, int, int);
extern void		copy(Archive_t*, int(*)(Ftw_t*));
extern void		copyin(Archive_t*);
extern int		copyinout(Ftw_t*);
extern int		copyout(Ftw_t*);
extern long		cpio_long(unsigned short*);
extern void		cpio_short(unsigned short*, long);
extern void		deltabase(Archive_t*);
extern void		deltadelete(Archive_t*);
extern void		deltaout(Archive_t*, Archive_t*, File_t*);
extern void		deltapass(Archive_t*, Archive_t*);
extern void		deltaverify(Archive_t*);
extern int		dirprefix(char*, char*);
extern void		filein(Archive_t*, File_t*);
extern void		fileout(Archive_t*, File_t*);
extern void		fileskip(Archive_t*, File_t*);
extern int		filter(Archive_t*, File_t*);
extern void		finish(int);
extern Archive_t*	getarchive(int);
extern void		getdeltaheader(Archive_t*, File_t*);
extern void		getdeltatrailer(Archive_t*, File_t*);
extern void		getepilogue(Archive_t*);
extern int		getfile(Archive_t*, File_t*, Ftw_t*);
extern int		getformat(char*);
extern int		getheader(Archive_t*, File_t*);
extern void		getidnames(File_t*);
extern int		getlabel(Archive_t*, File_t*);
extern long		getlabnum(char*, int, int, int);
extern char*		getlabstr(char*, int, int, char*);
extern int		getprologue(Archive_t*);
extern int		getsaveset(Archive_t*, File_t*, int);
extern void		gettrailer(Archive_t*, File_t*);
extern ssize_t		holewrite(int, void*, size_t);
extern Archive_t*	initarchive(const char*, int);
extern void		initdelta(Archive_t*);
extern void		initfile(Archive_t*, File_t*, char*, int);
extern char**		initmatch(char**);
extern void		interactive(void);
extern void		listentry(File_t*);
extern int		listprintf(Sfio_t*, Archive_t*, File_t*, const char*);
extern char*		map(char*);
extern int		match(char*);
extern void		newio(Archive_t*, int, int);
extern unsigned long	omemsum(const void*, int, unsigned long);
extern int		openin(Archive_t*, File_t*);
extern int		openout(Archive_t*, File_t*);
extern int		paxdelta(Archive_t*, Archive_t*, File_t*, int, ...);
extern int		portable(const char*);
extern int		prune(Archive_t*, File_t*, struct stat*);
extern void		putdeltaheader(Archive_t*, File_t*);
extern void		putdeltatrailer(Archive_t*, File_t*);
extern void		putepilogue(Archive_t*);
extern void		putheader(Archive_t*, File_t*);
extern void		putlabels(Archive_t*, File_t*, char*);
extern void		putprologue(Archive_t*);
extern void		puttrailer(Archive_t*, File_t*);
extern int		restore(const char*, char*, void*);
extern int		selectfile(Archive_t*, File_t*);
extern void		setdeltaheader(Archive_t*, File_t*);
extern void		setfile(Archive_t*, File_t*);
extern void		setidnames(File_t*);
extern void		setoptions(char*, char**, char*, Archive_t*);
extern void		settime(const char*, time_t, time_t);
extern char*		strlower(char*);
extern char*		strupper(char*);
extern long		tar_checksum(Archive_t*);
extern int		validout(Archive_t*, File_t*);
extern int		verify(Archive_t*, File_t*, char*);
