/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1995-2000 AT&T Corp.                *
*        and it may only be used by you under license from         *
*                       AT&T Corp. ("AT&T")                        *
*         A copy of the Source Code Agreement is available         *
*                at the AT&T Internet web site URL                 *
*                                                                  *
*       http://www.research.att.com/sw/license/ast-open.html       *
*                                                                  *
*        If you have copied this software without agreeing         *
*        to the terms of the license you are infringing on         *
*           the license and copyright and are violating            *
*               AT&T's intellectual property rights.               *
*                                                                  *
*                 This software was created by the                 *
*                 Network Services Research Center                 *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*                                                                  *
*******************************************************************/
#pragma prototyped

#include <ast.h>
#include <error.h>

#include "regex.h"

#if LONG_MAX > INT_MAX
typedef long word;
#else
typedef int word;
#endif

typedef struct {
	unsigned char *w;		/* write pointer */
	unsigned char *e;		/* end */
	unsigned char *s;		/* start */
} Text;

extern void	compile(Text*, Text*);
extern void	execute(Text*, Text*);
extern int	recomp(Text*, Text*);
extern int	reexec(regex_t*, char*, size_t, regmatch_t*, int);
extern int	match(unsigned char*, Text*, int);
extern int	substitute(regex_t*, Text*, unsigned char*, int);
extern regex_t*	readdr(int);
extern void	tcopy(Text*, Text*);
extern void	printscript(Text*);
extern void	vacate(Text*);
extern void	synwarn(char*);
extern void	syntax(char*);
extern void	badre(regex_t*, int);
extern int	readline(Text*);
extern int	ateof(void);
extern void	coda(void);

#define exch(a, b, t) ((t)=(a), (a)=(b), (b)=(t))
	
	/* space management; assure room for n more chars in Text */
#define assure(t, n) \
	do if((t)->s==0 || (t)->w>=(t)->e-n-1) grow(t, n);while(0)
extern void	grow(Text*, int);

	/* round character pointer up to word pointer.
	   portable to the cray; simpler tricks are not */

#define wordp(p) (word*)(p + sizeof(word) - 1 \
			- (p+sizeof(word)-1 - (unsigned char*)0)%sizeof(word))

extern int	reflags;
extern int	recno;
extern int	nflag;
extern int	qflag;
extern int	sflag;
extern int	bflag;
extern char*	stdouterr;

extern Text	files;

/* SCRIPT LAYOUT

   script commands are packed thus:
   0,1,or2 address words signed + for numbers - for regexp
   if 2 addresses, then another word indicates activity
	positive: active, the record number where activated
	negative: inactive, sign or-ed with number where deactivated
   instruction word
	high byte IMASK+flags; flags are NEG and SEL
	next byte command code (a letter)
	next two bytes, length of this command, including addrs
        (length is a multiple of 4; capacity could be expanded
	by counting the length in words instead of bytes)
   after instruction word
	on s command
		offset of regexp in rebuf
		word containing flags p,w plus n (g => n=0)
		replacement text
		word containing file designator, if flag w
	on y command
		256-byte transliteration table
	on b and t command
		offset of label in script
*/

#define BYTE		CHAR_BIT
#define IMASK		0xC0000000	/* instruction flag */
#define NEG  		0x01000000	/* instruction written with ! */
#define LMASK		0xffff		/* low half word */
#define AMASK		0x7fffffff	/* address mask, clear sign bit */
#define INACT		(~AMASK)	/* inactive bit, the sign bit */
#define DOLLAR		AMASK		/* huge address */
#define REGADR		(~AMASK)	/* context address */
#define PFLAG		0x80000000	/* s/../../p */
#define WFLAG		0x40000000	/* s/../../w */

extern word	pack(int, int, int);
extern word*	instr(unsigned char*);

#define code(inst)	((inst)>>2*BYTE & 0xff)
#define nexti(p)	((p) + (*instr(p)&LMASK))
