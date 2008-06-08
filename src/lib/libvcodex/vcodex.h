/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2003-2008 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
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

#define VCSFIO		1	/* one for sfio.h, zero for stdio.h	*/
#define VCPROPRIETARY	0	/* one to include proprietary functions	*/

#if _PACKAGE_ast
#include	<ast_std.h>
#undef	VCSFIO
#define VCSFIO		1
#else
#include	<ast_common.h>
#endif

#if VCSFIO == 1
#include	<sfio.h>
#else
#include	<stdio.h>
#endif

#include	<cdt.h>

/*	VCODEX = COmpression + DElta + X (encryption, etc.)
**
**	Written by Kiem-Phong Vo (kpv@research.att.com)
*/

#define VC_VERSION	20080604L	/* what/when/whereever, ILUNDV	*/
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

/* define 32-bit integer types */
#if !defined(Vcint32_t) && _typ_int32_t
#define Vcint32_t	int32_t
#endif
#if !defined(Vcint32_t) && _ast_int4_t
#define Vcint32_t	_ast_int4_t
#endif
#if !defined(Vcint32_t)
Help needed to define signed 32-bit integer type.
#endif

#if !defined(Vcuint32_t) && _typ_uint32_t
#define Vcuint32_t	uint32_t
#endif
#if !defined(Vcuint32_t) && _ast_int4_t
#define Vcuint32_t	unsigned _ast_int4_t
#endif
#if !defined(Vcuint32_t)
Help needed to define unsigned 32-bit integer type.
#endif

typedef unsigned char		Vcchar_t;
typedef struct _vcodex_s	Vcodex_t;
typedef struct _vcdisc_s	Vcdisc_t;
typedef struct _vcmethod_s	Vcmethod_t;
typedef struct _vcmtarg_s	Vcmtarg_t;
typedef struct _vcmtcode_s	Vcmtcode_t;
typedef struct _vccontext_s	Vccontext_t;
typedef int			(*Vcevent_f)_ARG_((Vcodex_t*, int, Void_t*, Vcdisc_t*));
typedef ssize_t			(*Vcapply_f)_ARG_((Vcodex_t*, const Void_t*, size_t, Void_t**));

typedef int			(*Vcwalk_f)_ARG_((Void_t*, char*, char*, Void_t*));

/* type of buffers allocated for transformed data */
typedef struct _vcbuffer_s	Vcbuffer_t;
typedef int			(*Vcbuffer_f)_ARG_((Vcodex_t*, Vcbuffer_t*, Vcmethod_t*));

/* types to encode/decode bits and integers */
typedef Vcuint32_t		Vcbit_t; 	/* 32-bit accumulator	*/
#define VC_BITSIZE		(sizeof(Vcbit_t)*8 )
typedef Vcint32_t		Vcint_t;	/* 32-bit integers	*/
#define VC_INTSIZE		(sizeof(Vcint_t)*8 )

/* Vcio_t: handle to perform IO functions on strings */
typedef struct _vcio_s
{	Vcchar_t*	data;		/* the data buffer		*/
	Vcchar_t*	next;		/* current position in buffer	*/
	Vcchar_t*	endb;		/* end of buffer or data in it	*/
	Vcbit_t		bits;		/* buffer usable for bit-coding	*/
	ssize_t		nbits;		/* # of bits cached in "bits"	*/
} Vcio_t;

/* Discipline structure: what application supplies */
struct _vcdisc_s
{	Void_t*		data;		/* data, key string, etc.	*/
	ssize_t		size;		/* size of data or just an int	*/
	Vcevent_f	eventf;		/* call-back function on events	*/
};

/* Arguments to a method */
struct _vcmtarg_s
{	char*		name;		/* argument name - alphanumeric	*/
	char*		desc;		/* short description of arg	*/
	Void_t*		data;		/* predefined data, if any	*/
};

/* structure to extract/restore a handle by its private code */
struct _vcmtcode_s
{	Vcchar_t*	data;		/* the encoding data for handle	*/
	ssize_t		size;		/* the data size		*/
	Vcodex_t*	coder;		/* the reconstructed coder	*/
};

/* Method structure: what a tranform looks like */
struct _vcmethod_s
{	Vcapply_f	encodef;	/* function to encode		*/
	Vcapply_f	decodef;	/* function to decode		*/
	int		(*eventf)_ARG_((Vcodex_t*, int, Void_t*));
	char*		name;		/* string name, 0-terminated	*/
	char*		desc;		/* description, 0-terminated	*/
	char*		about;		/* [-name?value]...0-terminated	*/
	Vcmtarg_t*	args;		/* list of possible arguments	*/
	ssize_t		window;		/* suggested window size	*/
	int		type;		/* flags telling type of method	*/
};

/* Method writers: note that this should be first in any context type */
struct _vccontext_s
{	Vccontext_t	*next;
};

/* Vcodex_t handle structure: to keep states */
struct _vcodex_s
{	Vcapply_f	applyf;		/* function to process data	*/
	Vcdisc_t*	disc;		/* supplied discipline 		*/
	Vcmethod_t*	meth;		/* selected coding method 	*/
	Vcodex_t*	coder;		/* continuation coder		*/
	size_t		undone;		/* left-over after vcapply()	*/
	unsigned int	flags;		/* bits to control operations	*/
	Vccontext_t*	ctxt;		/* list of contexts		*/
	Void_t*		data;		/* private method data		*/
	ssize_t		head;		/* required buffer head size	*/
	char*		file;		/* file with allocation request	*/
	int		line;		/* line number in file		*/
#ifdef _VCODEX_PRIVATE
	_VCODEX_PRIVATE
#endif
};

/* flags passable to vcopen() */
#define VC_FLAGS		007777	/* all supported flags		*/
#define VC_ENCODE		000001	/* handle for encoding data	*/
#define VC_DECODE		000002	/* handle to decode data	*/
#define VC_CLOSECODER		000010	/* 2nd-ary coders to be closed	*/

/* event types passable to discipline event handlers */
#define VC_OPENING		1	/* opening event		*/
#define VC_CLOSING		2	/* closing event		*/
#define VC_DISC			3	/* changing discipline		*/

/* event types to be processed by method event handlers */
#define VC_INITCONTEXT		101	/* setting/creating a context	*/
#define VC_FREECONTEXT		102	/* freeing one or all contexts	*/
#define VC_FREEBUFFER		103	/* free all associated buffers	*/
#define VC_SETMTARG		104	/* set an argument to a method	*/
#define VC_EXTRACT		105	/* extract code for handle	*/
#define VC_RESTORE		106	/* restoring a handle from code	*/
#define VC_GETIDENT		107	/* get method ID		*/
#define VC_TELLBUFFER		108	/* announce buffer data	*/

/* flags defining certain method attributes */
#define VC_MTSOURCE		000001	/* use source data (Vcdelta)	*/

/* separators for arguments */
#define VC_ARGSEP		'.'	/* separator for method args	*/
#define VC_METHSEP		','	/* separator for methods	*/

/* function to initialize a discipline structure */
#define VCDISC(dc,dt,sz,fn) \
		((dc)->data = (dt), (dc)->size = (sz), (dc)->eventf = (fn) )

/* return vcodex discipline event */
#define VCSF_DISC	((((int)('V'))<<7)|((int)('C')))

_BEGIN_EXTERNS_

extern Vcmethod_t*	Vcdelta;	/* delta compression		*/
extern Vcmethod_t*	Vchamming;	/* byte-wise delta of data	*/

extern Vcmethod_t*	Vchuffman;	/* Huffman compression		*/
extern Vcmethod_t*	Vchuffgroup;	/* Huffman with grouping	*/
extern Vcmethod_t*	Vchuffpart;	/* Huffman with partitioning	*/

extern Vcmethod_t*	Vcbwt;		/* Burrows-Wheeler transform	*/
extern Vcmethod_t*	Vcrle;		/* run-length compressor	*/
extern Vcmethod_t*	Vcmtf;		/* move-to-front transform	*/

extern Vcmethod_t*	Vcmap;		/* mapping codeset to codeset	*/
extern Vcmethod_t*	Vctranspose;	/* swap rows and columns	*/

/* public functions */
extern Vcodex_t*	vcopen _ARG_((Vcdisc_t*, Vcmethod_t*, Void_t*, Vcodex_t*, int));
extern Vcodex_t*	vcmake _ARG_((char*, int));
extern ssize_t		vcextract _ARG_((Vcodex_t*, Void_t**));
extern Vcodex_t*	vcrestore _ARG_((Void_t*, size_t));
extern int		vcclose _ARG_((Vcodex_t*));
extern ssize_t		vcapply _ARG_((Vcodex_t*, Void_t*, size_t, Void_t**));
extern size_t		vcundone _ARG_((Vcodex_t*));
extern Vcdisc_t*	vcdisc _ARG_((Vcodex_t*, Vcdisc_t*));

extern int		vcaddmeth _ARG_((Vcmethod_t**, ssize_t));
extern Vcmethod_t*	vcgetmeth _ARG_((char*, int));
extern int		vcwalkmeth _ARG_((Vcwalk_f, Void_t*));

extern char*		vcgetident _ARG_((Vcmethod_t*, char*, ssize_t));
extern char*		vcgetmtarg _ARG_((char*, char*, ssize_t, Vcmtarg_t*, Vcmtarg_t**));
extern int              vcsetmtarg _ARG_((Vcodex_t*, char*, Void_t*, int));

extern char*		vcsubstring _ARG_((char*, int, char*, ssize_t, int));
extern double		vclog _ARG_((size_t)); /* fast log2 */
extern size_t		vclogi _ARG_((size_t)); /* integer part of log2 */
extern ssize_t		vcbcktsort _ARG_((ssize_t*, ssize_t*, ssize_t, Vcchar_t*, ssize_t*));

typedef int		(*Vccompare_f)_ARG_((Void_t*, Void_t*, Void_t*));
extern Void_t		vcqsort _ARG_((Void_t*, ssize_t, ssize_t, Vccompare_f, Void_t*));

extern Vcint_t		vcatoi _ARG_((char*));
extern ssize_t		vcitoa _ARG_((Vcint_t, char*, ssize_t));
extern Vcint_t		vcintcode _ARG_((Vcint_t, Vcint_t, Vcint_t, Vcint_t, int));
extern char*		vcstrcode _ARG_((char*, char*, ssize_t));

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
extern ssize_t		vcioputlist _ARG_((Vcio_t*, Vcint_t*, ssize_t));
extern ssize_t		vciogetlist _ARG_((Vcio_t*, Vcint_t*, ssize_t));

extern ssize_t		_vcioputu _ARG_((Vcio_t*, Vcint_t));
extern Vcint_t		_vciogetu _ARG_((Vcio_t*));
extern ssize_t		_vcioputm _ARG_((Vcio_t*, Vcint_t, Vcint_t));
extern Vcint_t		_vciogetm _ARG_((Vcio_t*, Vcint_t));
extern ssize_t		_vcioput2 _ARG_((Vcio_t*, Vcint_t, Vcchar_t, Vcchar_t));
extern Vcint_t		_vcioget2 _ARG_((Vcio_t*, Vcchar_t, Vcchar_t));
extern ssize_t		_vcioputg _ARG_((Vcio_t*, Vcint_t));
extern Vcint_t		_vciogetg _ARG_((Vcio_t*));

_END_EXTERNS_

#define vcapply(vc,s,sz,b)	(*(vc)->applyf)((vc), (Void_t*)(s), (size_t)(sz), (Void_t**)(b) )
#define vcundone(vc)		((vc)->undone)	/* amount left unprocessed by transform	*/

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

/* size of an integer coded by vcioputu() and vcputm() */
#define vcsizeu(v)		((v)<(1<<7) ? 1 : (v)<(1<<14) ? 2 : (v)<(1<<21) ? 3 : 4)
#define vcsizem(v)		((v)<(1<<8) ? 1 : (v)<(1<<16) ? 2 : (v)<(1<<24) ? 3 : 4)

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
	TYPES AND FUNCTIONS RELATED TO STRING, SUFFIX SORTING 
*************************************************************************/

#if VCSFXINT /* may be desirable when sizeof(int) < sizeof(ssize_t) */
#define Vcsfxint_t	int
#else
#define Vcsfxint_t	ssize_t
#endif
typedef struct _vcsfx_s
{	Vcsfxint_t*	idx;	/* the sorted suffix array		*/
	Vcsfxint_t*	inv;	/* the inverted indices/ranks		*/
	Vcchar_t*	str;	/* the source string			*/
	Vcsfxint_t	nstr;
} Vcsfx_t;

_BEGIN_EXTERNS_
extern Vcsfx_t*	vcsfxsort _ARG_((const Void_t*, ssize_t));
extern ssize_t	vcperiod _ARG_((const Void_t*, ssize_t));
_END_EXTERNS_


/*************************************************************************
	TYPES AND FUNCTIONS RELATED TO GENERALIZED LZ-PARSING
*************************************************************************/

/* (*parsef)(Vclzparse_t* vcpa, int type, Vcmatch_t* mtch, ssize_t n)
   Arguments:
	vcpa: structure describing source&target data and matching requirements.
	type: type of match, VC_REVERSE, VCP_MAP, etc.
	mtch: array of matched data.
	n:    number of elements in mtch.
   Return values:
	> 0: amount of data used up.
	= 0: all given data should be considered unmatchable.
	< 0: processing failed, quit and return immediately.
*/

#define VCLZ_REVERSE	0001	/* use/do reverse matching 	*/
#define VCLZ_MAP	0002	/* matching by mapping bytes	*/

typedef struct _vclzparse_s	Vclzparse_t;
typedef struct _vclzmatch_s	Vclzmatch_t;
typedef ssize_t			(*Vclzparse_f)_ARG_((Vclzparse_t*,int,Vclzmatch_t*,ssize_t));

/* tpos is monotonically increasing in an array of these */
struct _vclzmatch_s
{	ssize_t		tpos;	/* position in target to match	*/
	ssize_t		mpos;	/* match pos or <0 for none	*/
	ssize_t		size;	/* length of data involved	*/
};

struct _vclzparse_s
{	Vclzparse_f	parsef;	/* function to process parsed segments	*/
	Vcchar_t*	src;	/* source string, if any, to learn from	*/
	ssize_t		nsrc;
	Vcchar_t*	tar;	/* target string to be parsed		*/
	ssize_t		ntar;
	ssize_t		mmin;	/* minimum acceptable match length	*/
	Vcchar_t*	cmap;	/* character map for matching		*/
	int		type;	/* VCP_REVERSE, VCP_MAP			*/
};

_BEGIN_EXTERNS_
extern int	vclzparse _ARG_((Vclzparse_t*, ssize_t));
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
extern ssize_t		vchsize _ARG_((ssize_t, ssize_t*, ssize_t*, int*));
extern ssize_t		vchbits _ARG_((ssize_t, ssize_t*, Vcbit_t*));
extern Vchtrie_t*	vchbldtrie _ARG_((ssize_t, ssize_t*, Vcbit_t*));
extern Void_t		vchdeltrie _ARG_((Vchtrie_t*));
extern ssize_t		vchgetcode _ARG_((ssize_t, ssize_t*, ssize_t, Vcchar_t*, size_t));
extern ssize_t		vchputcode _ARG_((ssize_t, ssize_t*, ssize_t, Vcchar_t*, size_t));
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
extern ssize_t		vcdputtable _ARG_((Vcdtable_t*, Void_t*, size_t));
extern int		vcdgettable _ARG_((Vcdtable_t*, Void_t*, size_t));
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
	char*		desc;
	char*		about;		/* [-name?value]...0-terminated	*/
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

extern Vcwmethod_t*	Vcwmirror;
extern Vcwmethod_t*	Vcwvote;
extern Vcwmethod_t*	Vcwprefix;

extern Vcwindow_t*	vcwopen _ARG_((Vcwdisc_t*, Vcwmethod_t*));
extern int		vcwclose _ARG_((Vcwindow_t*));
extern Vcwmethod_t*	vcwgetmeth _ARG_((char*));
extern int		vcwwalkmeth _ARG_((Vcwalk_f, Void_t*));

_END_EXTERNS_


/*************************************************************************
	TYPES AND FUNCTIONS FOR FIXED-LENGTH RECORDS TABLE TRANSFORM
*************************************************************************/

#define VCTBL_RLE	0001	/* use run-length entropy (default)	*/
#define VCTBL_CEE	0002	/* use conditional entropy		*/
#define VCTBL_LEFT	0010	/* left to right dependency (default) 	*/
#define VCTBL_RIGHT	0020	/* right to left dependency		*/
#define VCTBL_SINGLE	0100	/* single predictor			*/

typedef struct _vctblcolumn_s /* transform specification for each column	*/
{	ssize_t		index;		/* column index			*/
	ssize_t		pred1;		/* <0 if self compressing	*/
	ssize_t		pred2;		/* >=0 if supporting pred1	*/
} Vctblcolumn_t;

typedef struct _vctblplan_s /* transform plan for all columns		*/
{	ssize_t		ncols;		/* # of columns	or row size	*/
	Vctblcolumn_t*	trans;		/* the plan to transform data	*/
	Vcodex_t*	zip;		/* to compress a plan encoding	*/
	ssize_t		train;		/* size of training data	*/
	ssize_t		dtsz;		/* last data size compressed	*/
	ssize_t		cmpsz;		/* and result			*/
	int		good;		/* training deemed good		*/
} Vctblplan_t;

_BEGIN_EXTERNS_
extern Vctblplan_t*	vctblmakeplan _ARG_((const Void_t*, size_t, size_t, int));
extern void		vctblfreeplan _ARG_((Vctblplan_t*));
extern ssize_t		vctblencodeplan _ARG_((Vctblplan_t*, Void_t**));
extern Vctblplan_t*	vctbldecodeplan _ARG_((Void_t*, size_t));
_END_EXTERNS_


/*************************************************************************
	TYPES AND FUNCTIONS RELATED TO RELATIONAL DATABASE TRANSFORM
*************************************************************************/


#define VCRD_RECORD	00000010	/* data is record-oriented	*/
#define VCRD_FIELD	00000020	/* data is field-oriented	*/
#define VCRD_PAD	00000040	/* pad fields/records to full	*/
#define VCRD_FIXED	00000100	/* fixed-length field		*/

#define VCRD_DOT	00001000	/* field was of the form x.y...	*/
#define VCRD_SLASH	00002000	/* like above but x.y.../z	*/

#define VCRD_VECTOR	00100000	/* did transform vector already	*/

/* function to weight string matching while computing a plan */
typedef ssize_t		(*Vcrdweight_f)_ARG_((Vcchar_t*, ssize_t, Vcchar_t*, ssize_t));
#define VCRD_NOPLAN	((Vcrdweight_f)(-1)) /* use the identity plan	*/

/* Information about a set of relational data. There are two cases:
** fldn > 0: fields may be fixed length or variable lengths:
**	flen[f] > 0: field f is fixed length.
**	flen[f] = 0: field f is variable length with delimiters.
**	flen[f] < 0: field f was variable length but made into fixed.
** fldn <= 0: records and fields with delimiters rsep and fsep.
*/
typedef struct _vcrdinfo_s
{	ssize_t		recn;		/* >0 for total # of records	*/
	ssize_t		fldn;		/* >0 for total # of fields	*/
	ssize_t*	flen;		/* schema field lengths if any 	*/
	int		fsep;		/* >= 0 for field separator	*/
	int		rsep;		/* >= 0 for record separator	*/
	int		dot;		/* the . character		*/
	int		slash;		/* the / character		*/
	int		digit[10];	/* the characters 0-9		*/
} Vcrdinfo_t;

/* structure of a field */
typedef struct _vcrdrecord_s
{	Vcchar_t*	data;		/* pointer to record data	*/
	ssize_t		dtsz;		/* record length in a field	*/
} Vcrdrecord_t;
typedef struct _vcrdfield_s
{	int		type;		/* DOT|SLASH|FIXED|... 		*/
	Vcrdrecord_t*	rcrd;		/* data in all records		*/
	ssize_t		maxz;		/* max size of any record	*/
	ssize_t*	vect;		/* transform vector by this fld	*/
	Vcchar_t*	data;		/* locally allocated field data	*/
} Vcrdfield_t;

/* structure of a table */
typedef struct _vcrdtable_s
{	Vcrdinfo_t*	info;
	ssize_t		parz;		/* size of data just parsed	*/
	ssize_t		recn;		/* number of records		*/
	ssize_t		fldn;		/* number of fields per record	*/
	Vcrdfield_t*	fld;		/* list of fields		*/
} Vcrdtable_t;

/* prediction plan */
typedef struct _vcrdplan_s
{	ssize_t		fldn;		/* number of fields		*/
	ssize_t		pred[1];	/* predictor list		*/
} Vcrdplan_t;

/* to compute field and record separators */
typedef struct _vcrdsepar_s
{	int		fsep;		/* field separator		*/
	int		rsep;		/* record separator		*/
	ssize_t		size;		/* size of data used to compute	*/
	ssize_t		recn;		/* number of records		*/
	ssize_t		recf;		/* # of records containing fsep	*/
	double		allz;		/* coding size over all		*/
	double		fldz;		/* coding size by fields	*/
} Vcrdsepar_t;

#define vcrdsize(tbl)	((tbl)->parz)	/* size of data just parsed	*/

_BEGIN_EXTERNS_
extern ssize_t		vcrdsepar _ARG_((Vcrdsepar_t*, ssize_t, Vcchar_t*, ssize_t, int));

extern ssize_t		vcrdweight _ARG_((Vcrdtable_t*, ssize_t, ssize_t*, Vcrdweight_f));
extern Vcrdplan_t*	vcrdmakeplan _ARG_((Vcrdtable_t*, Vcrdweight_f));
extern void		vcrdfreeplan _ARG_((Vcrdplan_t*));
extern int		vcrdexecplan _ARG_((Vcrdtable_t*, Vcrdplan_t*, int));

extern Vcrdtable_t*	vcrdparse _ARG_((Vcrdinfo_t*, Vcchar_t*, ssize_t, int));
extern ssize_t		vcrdfield _ARG_((Vcrdtable_t*, ssize_t, ssize_t, Vcchar_t* data, ssize_t dtsz));
extern int		vcrdattrs _ARG_((Vcrdtable_t*, ssize_t, int, int));
extern ssize_t		vcrdextract _ARG_((Vcrdtable_t*, ssize_t, Vcchar_t*, ssize_t, int));
extern void		vcrdclose _ARG_((Vcrdtable_t*));
extern int		vcrdvector _ARG_((Vcrdtable_t*, ssize_t, Vcchar_t*, ssize_t, int));
_END_EXTERNS_


/*************************************************************************
	TYPES AND FUNCTIONS RELATED TO STREAM PROCESSING
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

/* header processing modes */
#define VCSF_VCDIFF	000001	/* output RFC3284 VCDIFF header	*/
#define VCSF_PLAIN	000002	/* no header to be output	*/
#define VCSF_TRANS	000004	/* set trans on VC_DECODE	*/
#define VCSF_FREE	000010	/* free sfdt on disc pop	*/

/* application-defined function to process error messages */
typedef void		(*Vcsferror_f)_ARG_((const char*));

/* data passed to vcsfio() to initialize a stream */
typedef struct _vcsfdata_s
{	int		type;	/* various types of processing	*/
	char*		trans;	/* transformation specification	*/
	char*		window;	/* window specification		*/
	char*		source;	/* source file to delta against	*/
	Vcsferror_f	errorf;	/* to process errors		*/
} Vcsfdata_t;

_BEGIN_EXTERNS_
extern Vcsfio_t*	vcsfio _ARG_((Sfio_t*, Vcsfdata_t*, int));
extern ssize_t		vcsfread _ARG_((Vcsfio_t*, Void_t*, size_t));
extern ssize_t		vcsfwrite _ARG_((Vcsfio_t*, const Void_t*, size_t));
extern int		vcsfsync _ARG_((Vcsfio_t*));
extern int		vcsfclose _ARG_((Vcsfio_t*));

extern void		vcaddalias _ARG_((char**));
extern char*		vcgetalias _ARG_((char*, char*, ssize_t));
extern int		vcwalkalias _ARG_((Vcwalk_f, Void_t*));
_END_EXTERNS_


/*************************************************************************
	TYPES AND MACROS/FUNCTIONS USEFUL FOR WRITING METHODS
*************************************************************************/


typedef Vcuint32_t		Vchash_t;	/* 32-bit hash value	*/
#define VCHASH(ky)		(((ky)>>13)^0x1add2b3^(ky) )
#define	VCINDEX(ky,msk)		(VCHASH(ky) & (msk) ) 

/* get/set private method data */
#define vcgetmtdata(vc, tp)	((tp)(vc)->data)
#define vcsetmtdata(vc, mt)	((vc)->data = (Void_t*)(mt))

/* get disciplines */
#define vcgetdisc(vc)		((vc)->disc)

/* get context and coerce to the true type */
#define vcgetcontext(vc, tp)	((tp)(vc)->ctxt)

/* allocate/deallocate buffer.
** bb: if !NULL, current buffer to resize.
** zz: new size, 0 to deallocate.
** hh: some slack amount at head of buffer to allow backward overwriting.
*/
#if defined(__FILE__) && defined(__LINE__)
#define vcbuffer(vv,bb,zz,hh) \
	(!(vv) ? NIL(Vcchar_t*) : \
		((vv)->file = __FILE__, (vv)->line = __LINE__, _vcbuffer((vv),(bb),(zz),(hh)) ) )
#else
#define vcbuffer(vv,bb,zz,hh)	_vcbuffer((vv),(bb),(zz),(hh))
#endif

struct _vcbuffer_s /* type of a buffer */
{	Vcbuffer_t*	next;
	size_t		size;	/* total buffer size	*/
	char*		file;	/* file allocating it	*/
	int		line;	/* line number in file	*/
	unsigned char	buf[1];	/* actual data buffer	*/
};

_BEGIN_EXTERNS_
extern ssize_t		vcrecode _ARG_((Vcodex_t*, Vcchar_t**, ssize_t*, ssize_t));
extern Vccontext_t*	vcinitcontext _ARG_((Vcodex_t*, Vccontext_t*));
extern int		vcfreecontext _ARG_((Vcodex_t*, Vccontext_t*));
extern Vcchar_t*	_vcbuffer _ARG_((Vcodex_t*, Vcchar_t*, ssize_t, ssize_t));
extern int		vctellbuf _ARG_((Vcodex_t*, Vcbuffer_f));
_END_EXTERNS_

/*************************************************************************
	BUILTIN AND PLUGIN METHOD LIST DEFINITIONS
*************************************************************************/

#ifdef _BLD_vcodex

#if __STD_C
#define VCLIB(m)	Vcmethod_t* m = &_##m;
#else
#define VCLIB(m)	Vcmethod_t* m = &_/**/m;
#endif

#else

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
