/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 2003-2007 AT&T Knowledge Ventures            *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                      by AT&T Knowledge Ventures                      *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#ifndef _VCODEX_H
#define _VCODEX_H	1

#define VCSFIO		1	/* 1 for using sfio.h, 0 for stdio.h	*/

#if _PACKAGE_ast
#include	<ast_std.h>
#else
#include	<ast_common.h>
#endif

#if VCSFIO == 1
#include	<sfio.h>
#else
#include	<stdio.h>
#endif

/*	VCODEX = COmpression + DElta + X (encryption, etc.)
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

#define VC_VERSION	20050805L
#define VC_ID		"vcodex"	/* package identification	*/
#define VC_LIB		"vcodex_lib"	/* function name		*/

#if !_SFIO_H /* Emulate a few Sfio features to keep single source code */
#define Sfoff_t		long	/* file offset type	*/
#define Sfio_t		FILE	/* stream structure	*/
#define Sfdisc_t	int	/* Sfio discipline	*/

#define sfstderr	stderr	/* standard streams	*/
#define sfstdout	stdout
#define sfstdin		stdin

#define SF_IOCHECK	010

#define SF_LOCKR	001
#define SF_LASTR	002

#define SF_SYNC		001
#define SF_CLOSING	002
#define SF_DPOP		004
#define	SF_ATEXIT	010

#define sfprintf	fprintf
#define sfvprintf	vfprintf
#define sfopen(s,f,m)	((s) ? freopen((f),(m),(s)) : fopen((f), (m)) )
#define sfread(s,b,n)	fread((b), 1, (n), (s))
#define sfwrite(s,b,n)	fwrite((b), 1, (n), (s))
#define sfrd(s,b,n,d)	fread((b), 1, (n), (s))
#define sfwr(s,b,n,d)	fwrite((b), 1, (n), (s))
#define sfseek(s,p,t)	(fseek((s),(long)(p),(t)) >= 0 ? (Sfoff_t)ftell(s) : (Sfoff_t)(-1))
#define sfdisc(s,d)	(d)
#define sfset(f,m,t)	(0)
#define sfsetbuf(f,b,n)	(0)
#define sffileno(f)	fileno(f)
#define sftmp(n)	tmpfile()

extern Void_t*		sfreserve _ARG_((Sfio_t*, ssize_t, int));
extern char*		sfgetr _ARG_((Sfio_t*, int, int));
extern ssize_t		sfvalue _ARG_((Sfio_t*));
extern Sfoff_t		sfsize _ARG_((Sfio_t*));
extern int		sfclose _ARG_((Sfio_t*));
#endif

typedef unsigned char		Vcchar_t;
typedef struct _vcodex_s	Vcodex_t;
typedef struct _vcdisc_s	Vcdisc_t;
typedef struct _vcmethod_s	Vcmethod_t;
typedef struct _vcmtarg_s	Vcmtarg_t;
typedef int			(*Vcevent_f)_ARG_((Vcodex_t*, int, Void_t*, Vcdisc_t*));
typedef ssize_t			(*Vcapply_f)_ARG_((Vcodex_t*, const Void_t*, size_t, Void_t**));

/* types to encode/decode bits and integers */
typedef uint32_t		Vcbit_t; 	/* 32-bit accumulator	*/
#define VC_BITSIZE		(sizeof(Vcbit_t)*8 )
typedef int32_t			Vcint_t;	/* 32-bit integers	*/
#define VC_INTSIZE		(sizeof(Vcint_t)*8 )

/* Vcio_t: handle to perform IO functions on strings */
typedef struct _vcio_s
{	Vcchar_t*	data;		/* the data buffer		*/
	Vcchar_t*	next;		/* current position in buffer	*/
	Vcchar_t*	endb;		/* end of buffer or data in it	*/
	Vcbit_t		bits;		/* buffer usable for bit-coding	*/
	ssize_t		nbits;		/* # of bits cached in "bits"	*/
} Vcio_t;

/* discipline structure: what application supplies */
struct _vcdisc_s
{	Void_t*		data;		/* data, key string, etc.	*/
	ssize_t		size;		/* size of data or just an int	*/
	Vcevent_f	eventf;		/* call-back function on events	*/
};

struct _vcmtarg_s
{	
	char*		name;		/* string name for arg		*/
	char*		ident;		/* 'name' written in ASCII	*/
	char*		desc;		/* short description of arg	*/
	Void_t*		data;		/* associated data, if any	*/
};

/* method structure: what a tranform looks like */
struct _vcmethod_s
{	Vcapply_f	encodef;	/* function to encode		*/
	Vcapply_f	decodef;	/* function to decode		*/
	ssize_t		(*extractf)_ARG_((Vcodex_t*, Void_t**, int));
	Vcodex_t*	(*restoref)_ARG_((Vcodex_t*, Void_t*, ssize_t));
	int		(*eventf)_ARG_((Vcodex_t*, int, Void_t*));
	char*		name;		/* string name, 0-terminated	*/
	char*		ident;		/* same as above but in ASCII	*/
	char*		desc;		/* description, 0-terminated	*/
	Vcmtarg_t*	args;		/* list of possible arguments	*/
	ssize_t		window;		/* suggested window size	*/
	Vcmethod_t*	link;		/* library private link		*/
};

/* Vcodex_t handle structure: to keep states */
struct _vcodex_s
{	Vcapply_f	applyf;		/* function to process data	*/
	Vcdisc_t*	disc;		/* supplied discipline 		*/
	Vcmethod_t*	meth;		/* selected coding method 	*/
	Vcodex_t*	coder;		/* continuation coder		*/
	size_t		undone;		/* left-over after vcapply()	*/
	unsigned int	flags;		/* bits to control operations	*/
#ifdef _VCODEX_PRIVATE
	_VCODEX_PRIVATE
#endif
};

/* flags passable to vcopen() or vcsfio() */
#define VC_FLAGS		000777	/* all supported flags		*/
#define VC_ENCODE		000001	/* handle for encoding data	*/
#define VC_DECODE		000002	/* handle to decode data	*/
#define VC_CLOSECODER		000010	/* 2nd-ary coders to be closed	*/

/* the below are for vcsfio() only. */
#define VCSF_CLOSEWINDOW	000100	/* close window handle		*/
#define VCSF_VCDIFFHEADER	000200	/* output RFC3284 VCDIFF header	*/

/* event types passable to discipline event handlers */
#define VC_OPENING		1	/* opening event		*/
#define VC_CLOSING		2	/* closing event		*/
#define VC_DISC			3	/* changing discipline		*/

/* event types to be processed by method event handlers */
#define VC_INITCTXT		101	/* initializing a context	*/
#define VC_ENDCTXT		102	/* closing a context		*/
#define VC_FREEBUF		103	/* free all associated buffers	*/
#define VC_GETARG		104	/* get method argument		*/

/* function to initialize a discipline structure */
#define VCDISC(dc,dt,sz,fn) \
		((dc)->data = (dt), (dc)->size = (sz), (dc)->eventf = (fn) )

_BEGIN_EXTERNS_

#if _BLD_vcodex && defined(__EXPORT__)
#define extern		extern __EXPORT__
#endif
#if !_BLD_vcodex && defined(__IMPORT__)
#define extern		extern __IMPORT__
#endif

extern Vcmethod_t*	Vcdelta;	/* delta compression		*/
extern Vcmethod_t*	Vcapprox;	/* approximate matching		*/
extern Vcmethod_t*	Vchuffman;	/* Huffman compression		*/
extern Vcmethod_t*	Vchuffgroup;	/* Huffman with grouping	*/
extern Vcmethod_t*	Vchuffpart;	/* Huffman with partitioning	*/
extern Vcmethod_t*	Vcbwt;		/* Burrows-Wheeler transform	*/
extern Vcmethod_t*	Vcrle;		/* run-length compressor	*/
extern Vcmethod_t*	Vcmtf;		/* move-to-front transform	*/

extern Vcmethod_t*	Vcmap;		/* mapping codeset to codeset	*/
extern Vcmethod_t*	Vctranspose;	/* swap rows and columns	*/

#undef	extern

#if _BLD_vcodex && defined(__EXPORT__)
#define extern		extern __EXPORT__
#endif

/* public functions */
extern Vcodex_t*	vcopen _ARG_((Vcdisc_t*, Vcmethod_t*, Void_t*, Vcodex_t*, int));
extern ssize_t		vcextract _ARG_((Vcodex_t*, Void_t**, int));
extern Vcodex_t*	vcrestore _ARG_((Vcodex_t*, Void_t*, size_t));
extern int		vcclose _ARG_((Vcodex_t*));
extern int		vccontext _ARG_((Vcodex_t*, int));
extern ssize_t		vcapply _ARG_((Vcodex_t*, Void_t*, size_t, Void_t**));
extern size_t		vcundone _ARG_((Vcodex_t*));
extern Vcdisc_t*	vcdisc _ARG_((Vcodex_t*, Vcdisc_t*));
extern Vcchar_t*	vcsetbuf _ARG_((Vcodex_t*, Vcchar_t*, ssize_t, ssize_t));

extern Vcmethod_t*	vcgetmeth _ARG_((const char*, const char*, char**));
extern Vcmethod_t*	vcnextmeth _ARG_((Vcmethod_t*));

extern double		vclog _ARG_((unsigned int)); /* fast log2 */
extern ssize_t		vcbcktsort _ARG_((ssize_t*, ssize_t*, ssize_t, Vcchar_t*, ssize_t*));

typedef int		(*Vcqsort_f)_ARG_((Void_t*, Void_t*, Void_t*));
extern Void_t		vcqsort _ARG_((Void_t*, ssize_t, ssize_t, Vcqsort_f, Void_t*));

extern ssize_t		vcioputc _ARG_((Vcio_t*, int));
extern int		vciogetc _ARG_((Vcio_t*));
extern ssize_t		vcioputs _ARG_((Vcio_t*, Void_t*, size_t));
extern ssize_t		vciogets _ARG_((Vcio_t*, Void_t*, size_t));

extern ssize_t		vcioputu _ARG_((Vcio_t*, Vcint_t));
extern Vcint_t		vciogetu _ARG_((Vcio_t*));
extern ssize_t		vcioputm _ARG_((Vcio_t*, Vcint_t, Vcint_t));
extern Vcint_t		vciogetm _ARG_((Vcio_t*, Vcint_t));
extern ssize_t		vcioput2 _ARG_((Vcio_t*, Vcchar_t, Vcchar_t, Vcint_t));
extern Vcint_t		vcioget2 _ARG_((Vcio_t*, Vcchar_t, Vcchar_t));
extern ssize_t		vcioputg _ARG_((Vcio_t*, Vcint_t));
extern Vcint_t		vciogetg _ARG_((Vcio_t*));
extern ssize_t		vcioputl _ARG_((Vcio_t*, Vcint_t*, ssize_t));
extern ssize_t		vciogetl _ARG_((Vcio_t*, Vcint_t*, ssize_t));

extern ssize_t		_vcioputu _ARG_((Vcio_t*, Vcint_t));
extern Vcint_t		_vciogetu _ARG_((Vcio_t*));
extern ssize_t		_vcioputm _ARG_((Vcio_t*, Vcint_t, Vcint_t));
extern Vcint_t		_vciogetm _ARG_((Vcio_t*, Vcint_t));
extern ssize_t		_vcioput2 _ARG_((Vcio_t*, Vcint_t, Vcchar_t, Vcchar_t));
extern Vcint_t		_vcioget2 _ARG_((Vcio_t*, Vcchar_t, Vcchar_t));
extern ssize_t		_vcioputg _ARG_((Vcio_t*, Vcint_t));
extern Vcint_t		_vciogetg _ARG_((Vcio_t*));

#undef	extern

_END_EXTERNS_

#define vcapply(vc,s,sz,b)	(*(vc)->applyf)((vc), \
					(Void_t*)(s), (size_t)(sz), (Void_t**)(b) )
#define vcundone(vc)		((vc)->undone)

/* subsystem for fast I/O on strings */
#define vcioinit(io,b,n)	((io)->data = (io)->next = (Vcchar_t*)(b), \
				 (io)->endb = (io)->data + (n) )
#define vciosize(io)		((io)->next - (io)->data)
#define vciomore(io)		((io)->endb - (io)->next)
#define vcioextent(io)		((io)->endb - (io)->data)
#define vciodata(io)		((io)->data)
#define vcionext(io)		((io)->next)
#define vcioskip(io, n)		((io)->next += (n))
#define vcioputc(io, v)		(*(io)->next++ = (Vcchar_t)(v) )
#define vciogetc(io)		(*(io)->next++)
#define vcioputs(io, str, sz)	(memcpy((io)->next, (str), (sz)), (io)->next += (sz) )
#define vciogets(io, str, sz)	(memcpy((str), (io)->next, (sz)), (io)->next += (sz) )
#define vciomove(io1, io2, sz)	(memcpy((io2)->next, (io1)->next, (sz)), \
				 (io1)->next += (sz), (io2)->next += (sz) )
#define vcioputm(io, v, m)	_vcioputm((io), (Vcint_t)(v), (Vcint_t)(m))
#define vciogetm(io, m)		_vciogetm((io), (Vcint_t)(m))
#define vcioputu(io, v)	\
	((Vcint_t)(v) < (Vcint_t)(1<<7) ? (*(io)->next++ = (Vcchar_t)(v), 1) : \
		_vcioputu((io), (Vcint_t)(v)) )
#define vciogetu(io)		_vciogetu((io))
#define vcioput2(io, v, a, z)	_vcioput2((io),(Vcint_t)(v),(Vcchar_t)(a),(Vcchar_t)(z))
#define vcioget2(io, a, z)	_vcioget2((io),(Vcchar_t)(a),(Vcchar_t)(z))
#define vcioputg(io, v)		_vcioputg((io), (Vcint_t)(v))
#define vciogetg(io)		_vciogetg((io))

/* The below bit I/O macro functions use (bt,nb) for bit accumulation. These
** are (register) variables that will be modified in place to keep states.
** The variables must be initialized by vciosetb() before starting bit coding
** and finalized by vcioendb() before restarting to byte coding.
** For convenience, the Vcio_t structure itself provides two fields (bits,nbits)
** usable as (bt,nb). In this way, applications that perform bits ops in function
** calls does not need extra (static) variables to keep states.
*/
#define vciosetb(io, bt, nb, tp) /* (tp) is only for symmetry with vcioendb */ \
do {	(bt) = 0; (nb) = 0; \
} while(0)

#define vciofilb(io, bt, nb, mb) /* getting bits from stream into (bt) */ \
do {	if((nb) < (mb)) while((nb) <= (VC_BITSIZE-8) && (io)->next < (io)->endb) \
			{ (nb) += 8; (bt) |= (*(io)->next++ << (VC_BITSIZE - (nb))); } \
} while(0)

#define vciodelb(io, bt, nb, nd) /* consume bits already read */ \
do {	(bt) <<= (nd); (nb) -= (nd); \
} while(0)

#define vcioflsb(io, bt, nb) /* putting accumulated bits out to stream */ \
do {	for(; (nb) >= 8 && (io)->next < (io)->endb; (nb) -= 8) \
		{ *(io)->next++ = (Vcchar_t)((bt) >> (VC_BITSIZE-8)); (bt) <<= 8; } \
} while(0)

#define vcioaddb(io, bt, nb, ad, na) /* add new bits to accumulator */ \
do {	if(((nb)+(na)) > VC_BITSIZE ) \
		vcioflsb((io), (bt), (nb)); \
	(bt) |= (ad) >> (nb); (nb) += (na); \
} while(0)

#define vcioendb(io, bt, nb, tp) /* finishing bit operations */ \
do {	if((tp) == VC_ENCODE) \
	{ vcioflsb(io, bt, nb); \
	  if((nb) > 0) /* finish up the partial last byte */ \
		{ *(io)->next++ = (Vcchar_t)((bt) >> (VC_BITSIZE-8)); (bt) = 0; } \
	} else \
	{ while(((nb) -= 8) >= 0) \
		(io)->next--;\
	  (bt) = 0; (nb) = 0; \
	} \
} while(0)


/*************************************************************************
	TYPES AND FUNCTIONS RELATED TO THE STREAM INTERFACE
*************************************************************************/

#if _SFIO_H
#define Vcsfio_t	Sfio_t
#define vcsfread	sfread
#define vcsfwrite	sfwrite
#define vcsfsync	sfsync
#define vcsfclose	sfclose
#else
#define Vcsfio_t	Void_t
#endif /*_SFIO_H*/

#define VCSF_MAX	64		/* max # of methods allowed 	*/

#define VCSF_WINDOW	(Vcmethod_t*)(-1) /* window data		*/
#define VCSF_STATES	(Vcmethod_t*)(-3) /* file storing states	*/
#define VCSF_CHAIN	(Vcmethod_t*)(-5) /* list chaining		*/

typedef struct _vcsfmeth_s
{	Vcmethod_t*	meth;	/* the requested method to invoke 	*/
	Void_t*		data;	/* method argument or init parameters	*/
	ssize_t		size;	/* data size, if windowing		*/
} Vcsfmeth_t;

_BEGIN_EXTERNS_

#if _BLD_vcodex && defined(__EXPORT__)
#define extern		extern __EXPORT__
#endif

extern Vcsfio_t*	vcsfio _ARG_((Sfio_t*, Vcsfmeth_t*, ssize_t, unsigned int));
extern Vcsfmeth_t*	vcsfmeth _ARG_((Vcsfio_t*, ssize_t*));

#if !_SFIO_H

extern ssize_t		vcsfread _ARG_((Vcsfio_t*, Void_t*, size_t));
extern ssize_t		vcsfwrite _ARG_((Vcsfio_t*, const Void_t*, size_t));
extern int		vcsfsync _ARG_((Vcsfio_t*));
extern int		vcsfclose _ARG_((Vcsfio_t*));

#endif

#undef	extern

_END_EXTERNS_


/*************************************************************************
	TYPES AND FUNCTIONS RELATED TO SUFFIX ARRAYS
*************************************************************************/

typedef struct _vcsfx_s
{	ssize_t*	idx;	/* the sorted suffix array		*/
	ssize_t*	inv;	/* the inverted indices/ranks		*/
	Vcchar_t*	str;	/* the source string			*/
	size_t		nstr;
} Vcsfx_t;

_BEGIN_EXTERNS_

#if _BLD_vcodex && defined(__EXPORT__)
#define extern		extern __EXPORT__
#endif

extern Vcsfx_t*	vcsfxsort _ARG_((const Void_t*, size_t));
extern ssize_t	vcperiod _ARG_((const Void_t*, size_t));

#undef	extern

_END_EXTERNS_


/*************************************************************************
	TYPES AND FUNCTIONS RELATED TO STRING PARSING
*************************************************************************/

/* (*parsef)(Vcparse_t* vcpa, int type, ssize_t undt, ssize_t here, ssize_t mtsz, ssize_t mtch);
   Arguments:
	vcpa: structure describing source&target data and matching requirements.
	type: VC_REVERSE, VC_APPROX, VC_MAP.
	unmt: starting position of unmatchable data.
	here: current position in target data.
	mtsz: matchable data size.
	mtch: matching address if mtsz > 0.
   Return values:
	> 0: instruction processed successfully.
	= 0: instruction cannot be used, treat as unmatched.
	< 0: processing failed, quit and return immediately.
*/

#define VC_REVERSE	0020	/* reverse matching		*/
#define VC_MAP		0040	/* matching by mapping bytes 	*/
#define VC_APPROX	0100	/* use approximate matching	*/

typedef struct _vcparse_s	Vcparse_t;
typedef int			(*Vcparse_f)_ARG_((Vcparse_t*,int,ssize_t,ssize_t,ssize_t,ssize_t));

struct _vcparse_s
{	Vcparse_f	parsef;	/* function to process a parsed string	*/
	Vcchar_t*	src;	/* source string, if any, to learn from	*/
	ssize_t		nsrc;
	Vcchar_t*	tar;	/* target string to be parsed		*/
	ssize_t		ntar;
	int		type;	/* VC_REVERSE, VC_APPROX, etc. 		*/
	ssize_t		mmin;	/* minimum acceptable match length	*/
	ssize_t		miss;	/* max consecutive misses allowed 	*/	
	Vcchar_t*	map;	/* map of characters for matching	*/
};

_BEGIN_EXTERNS_

#if _BLD_vcodex && defined(__EXPORT__)
#define extern		extern __EXPORT__
#endif

extern int	vcparse _ARG_((Vcparse_t*, ssize_t));

#undef	extern

_END_EXTERNS_


/************************************************************************
	TYPES AND FUNCTIONS RELATED TO HUFFMAN ENCODING
************************************************************************/

/* A Huffman decoding trie is stored in Vchtrie_t.node and Vchtrie_t.size.
** size[p] > 0: a data byte has been decoded. In this case, size[p]
**		is the number of bits that should be consumed to finish
**		the bits corresponding to this byte. node[p] is the byte.
** size[p] < 0: need to recurse to the next level of the trie. In this
**		case, -size[p] is the number of bits needed to index the
**		next level. node[p] is the base of the next level.
** size[p] == 0: an undecipherable bit string. Data is likely corrupted.
*/
typedef struct _vchtrie_s
{	short*		node;	/* data or next trie base to look up	*/
	short*		size;	/* >0: code sizes, 0: internal nodes 	*/
	short		ntop;	/* # of bits to index top trie level	*/
	short		trsz;	/* allocated memory for the trie	*/
	short		next;
} Vchtrie_t;

_BEGIN_EXTERNS_

#if _BLD_vcodex && defined(__EXPORT__)
#define extern		extern __EXPORT__
#endif

extern ssize_t		vchsize _ARG_((ssize_t, ssize_t*, ssize_t*, int*));
extern ssize_t		vchbits _ARG_((ssize_t, ssize_t*, Vcbit_t*));
extern Vchtrie_t*	vchbldtrie _ARG_((ssize_t, ssize_t*, Vcbit_t*));
extern Void_t		vchdeltrie _ARG_((Vchtrie_t*));
extern ssize_t		vchgetcode _ARG_((ssize_t, ssize_t*, ssize_t, Vcchar_t*, size_t));
extern ssize_t		vchputcode _ARG_((ssize_t, ssize_t*, ssize_t, Vcchar_t*, size_t));

#undef	extern

_END_EXTERNS_

/*************************************************************************
	HEADER DATA FOR PERSISTENT STORAGE.
	This is based on and extending IETF RFC3284,
	the Vcdiff delta compression coding format.
	The 4th byte can be changed for different versions.
*************************************************************************/

#define VC_HEADER0		(0126|0200)	/* ASCII 'V' | 0200	*/
#define VC_HEADER1		(0103|0200)	/* ASCII 'C' | 0200	*/
#define VC_HEADER2		(0104|0200)	/* ASCII 'D' | 0200	*/
#define VC_HEADER3		(0130|0200)	/* ASCII 'X' | 0200	*/

/* Bits in the file control byte.
** The first two bits are for windowing with respect to IETF RFC3284.
** They are no longer needed with VCDX since information about secondary
** compression, etc. are now packaged along with method encoding. However,
** we keep them for compatibility.
*/
#define	VCD_COMPRESSOR		(1<<0)	/* using a secondary compressor	*/
#define	VCD_CODETABLE		(1<<1)	/* alternative code table	*/
#define VC_EXTRAHEADER		(1<<2)	/* application-defined header	*/
#define VC_CHECKSUM		(1<<3)	/* window has a checksum	*/
#define	VC_INITS		(0xf)

/* Bits in the window control byte. Again, the first two bits are for
** windowing with respect to IETF RFC3284.
*/
#define	VCD_SOURCEFILE		(1<<0)	/* match window in source file	*/
#define	VCD_TARGETFILE		(1<<1)	/* match window in target file	*/
#define VC_RAW			(1<<2)	/* data was left uncoded	*/
#define VC_EOF			(1<<7)	/* end-of-file			*/


/*************************************************************************
	TYPES AND FUNCTIONS RELATED TO THE DELTA COMPRESSOR VCDELTA
*************************************************************************/

/* bits in the delta indicator byte */
#define	VCD_DATACOMPRESS	(1<<0)	/* compressed unmatched data	*/
#define	VCD_INSTCOMPRESS	(1<<1)	/* compressed instructions	*/
#define	VCD_ADDRCOMPRESS	(1<<2)	/* compressed COPY addrs	*/

/* delta instruction types */
#define	VCD_NOOP	0	
#define VCD_ADD		1	/* data immediately follow		*/
#define VCD_RUN		2	/* a run of a single byte		*/
#define VCD_COPY	3	/* copy data from some earlier address	*/
#define VCD_BYTE	4	/* Vcinst_t.mode is the byte encoded	*/

/* Address modes are limited to 16 (VCD_ADDR). Of these, VCD_SELF
   and VCD_HERE are reserved. The remaining modes are s+n < 16 where
   s*256 is the size of "same address" cache and n is the size of the
   "near address" cache.
*/
#define VCD_ADDR	16	/* the maximum number of address modes	*/
#define VCD_SELF	0	/* COPY addr is coded as itself		*/
#define VCD_HERE	1	/* COPY addr is offset from current pos	*/

/* buffer size requirement for encoding/decoding code tables */
#define VCD_TBLSIZE	(6*256 + 64)

typedef struct _vcdinst_s	Vcdinst_t;	/* instruction type	*/
typedef struct _vcdcode_s	Vcdcode_t;	/* a pair of insts	*/
typedef struct _vcdtable_s	Vcdtable_t;	/* entire code table	*/

struct _vcdinst_s
{	Vcchar_t	type;	/* COPY, RUN, ADD, NOOP, BYTE		*/
	Vcchar_t	size;	/* if 0, size coded separately		*/
	Vcchar_t	mode;	/* address mode for COPY		*/
};

struct _vcdcode_s
{	Vcdinst_t	inst1;
	Vcdinst_t	inst2;
};

struct _vcdtable_s
{	Vcchar_t	s_near;		/* size of near address cache	*/
	Vcchar_t	s_same;		/* size of same address cache	*/
	Vcdcode_t	code[256];	/* codes -> instructions	*/
};

_BEGIN_EXTERNS_

#if _BLD_vcodex && defined(__EXPORT__)
#define extern		extern __EXPORT__
#endif

extern ssize_t		vcdputtable _ARG_((Vcdtable_t*, Void_t*, size_t));
extern int		vcdgettable _ARG_((Vcdtable_t*, Void_t*, size_t));

#undef	extern

_END_EXTERNS_


/*************************************************************************
	TYPES AND FUNCTIONS RELATED TO MATCHING WINDOWS
*************************************************************************/

typedef struct _vcwmatch_s	Vcwmatch_t;
typedef struct _vcwmethod_s	Vcwmethod_t;
typedef struct _vcwdisc_s	Vcwdisc_t;
typedef struct _vcwindow_s	Vcwindow_t;
typedef int	(*Vcwevent_f)_ARG_((Vcwindow_t*, int, Void_t*, Vcwdisc_t*));

struct _vcwmatch_s
{	int		type;	/* VCD_[SOURCE|TARGET]FILE		*/
	Sfoff_t		wpos;	/* position in file			*/
	ssize_t		wsize;	/* size of matching window		*/
	Void_t*		wdata;	/* window data				*/
	ssize_t		msize;	/* amount of data actually matched	*/
	int		more;	/* more subwindows to process		*/
};

struct _vcwmethod_s
{	Vcwmatch_t*	(*applyf)_ARG_((Vcwindow_t*, Void_t*, size_t, Sfoff_t));
	int		(*eventf)_ARG_((Vcwindow_t*, int));
	char*		name;
}; 

struct _vcwdisc_s
{	Sfio_t*		srcf;	/* source file				*/
	Sfio_t*		tarf;	/* target file if any			*/
	Vcwevent_f	eventf;
};

struct _vcwindow_s
{	Vcwmethod_t*	meth;
	Vcwdisc_t*	disc;
	ssize_t		cmpsz;	/* size of result of last comp. attempt	*/
	Vcwmatch_t	match;	/* space to return the matching window	*/
	Void_t*		mtdata;
};

/* window events */
#define VCW_OPENING	0
#define VCW_CLOSING	1

#define vcwfeedback(vcw, sz)	((vcw)->cmpsz = (sz))
#define vcwapply(vcw, dt, dtsz, p) \
			(*(vcw)->meth->applyf)((vcw), (dt), (dtsz), (p))

_BEGIN_EXTERNS_

#if _BLD_vcodex && defined(__EXPORT__)
#define extern		extern __EXPORT__
#endif
#if !_BLD_vcodex && defined(__IMPORT__)
#define extern		extern __IMPORT__
#endif

extern Vcwmethod_t*	Vcwmirror;
extern Vcwmethod_t*	Vcwvote;
extern Vcwmethod_t*	Vcwprefix;

#undef	extern

#if _BLD_vcodex && defined(__EXPORT__)
#define extern		extern __EXPORT__
#endif

extern Vcwindow_t*	vcwopen _ARG_((Vcwdisc_t*, Vcwmethod_t*));
extern int		vcwclose _ARG_((Vcwindow_t*));
extern Vcwmethod_t*	vcwgetmeth _ARG_((const char*));
extern Vcwmethod_t*	vcwnextmeth _ARG_((Vcwmethod_t*));

#undef	extern

_END_EXTERNS_


/*************************************************************************
	TYPES AND FUNCTIONS RELATED TO TABLE COMPRESSION
*************************************************************************/

typedef struct _vcplcol_s /* transform specification for each column	*/
{	ssize_t		index;		/* column index			*/
	ssize_t		pred1;		/* <0 if self compressing	*/
	ssize_t		pred2;		/* >=0 if supporting pred1	*/
} Vcplcol_t;

typedef struct _vcplplan_s /* transform plan for all columns		*/
{	ssize_t		ncols;		/* # of columns	or row size	*/
	Vcplcol_t*	trans;		/* the plan to transform data	*/
	ssize_t		train;		/* size of training data	*/
	ssize_t		dtsz;		/* last data size compressed	*/
	ssize_t		cmpsz;		/* and result			*/
	int		good;		/* training deemed good		*/
	Vcodex_t*	zip;		/* to compress a plan encoding	*/
} Vcplan_t;

_BEGIN_EXTERNS_

#if _BLD_vcodex && defined(__EXPORT__)
#define extern		extern __EXPORT__
#endif

extern Vcplan_t*	vcplopen _ARG_((const Void_t*, size_t, size_t));
extern void		vcplclose _ARG_((Vcplan_t*));
extern ssize_t		vcplencode _ARG_((Vcplan_t*, Void_t**));
extern Vcplan_t*	vcpldecode _ARG_((Void_t*, size_t));

#undef	extern

_END_EXTERNS_

/*************************************************************************
	BUILTIN AND PLUGIN METHOD LIST DEFINITIONS
*************************************************************************/

#ifndef VCNEXT
#define VCNEXT(m)	0
#endif

#ifndef VCLIB

#ifdef __STDC__
#if defined(__EXPORT__)
#define VCLIB(m)	extern __EXPORT__ Vcmethod_t* vcodex_lib(const char* path) { return &_##m; }
#else
#define VCLIB(m)	extern Vcmethod_t* vcodex_lib(const char* path) { return &_##m; }
#endif
#else
#define VCLIB(m)	extern Vcmethod_t* vcodex_lib(path) char* path; { return &_/**/m; }
#endif

#endif

#endif /*_VCODEX_H*/
