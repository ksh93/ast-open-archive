/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1995-2002 AT&T Corp.                *
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
*            Information and Software Systems Research             *
*                        AT&T Labs Research                        *
*                         Florham Park NJ                          *
*                                                                  *
*               Glenn Fowler <gsf@research.att.com>                *
*******************************************************************/
#pragma prototyped

#include "sed.h"

#include <ctype.h>
#include <ccode.h>

#define ustrlen(p) strlen((char*)(p))
#define ustrcmp(p, q) strcmp((char*)(p), (char*)(q))
#define ustrcpy(p, q) (unsigned char*)strcpy((char*)(p), (char*)(q))
#define ustrchr(p, c) (unsigned char*)strchr((char*)(p), c)

int blank(Text*);
void fixlabels(Text*);
void fixbrack(Text*);
void ckludge(Text*, int, int, int, Text*);
int addr(Text*, Text*);
word pack(int, int, int);
word* instr(unsigned char*);
unsigned char *succi(unsigned char*);

#if DEBUG
extern void regdump(regex_t*);	/* secret entry into regex pkg */
#endif

int semicolon;
int spaces;
Text rebuf;

unsigned char adrs[UCHAR_MAX+1] = {	/* max no. of addrs, 3 is illegal */
	0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 3, 3, 3, 3, 3, /* <nl> */
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 2, 3, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	/* !# */
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 3, 1, 3, 3, /* := */
	3, 3, 3, 3, 2, 3, 3, 2, 2, 3, 3, 3, 3, 3, 2, 3, /* DGHN */
	2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,	/* P */
	3, 1, 2, 2, 2, 3, 3, 2, 2, 1, 3, 3, 2, 3, 2, 3, /* a-n */
	2, 1, 2, 2, 2, 3, 3, 2, 2, 2, 3, 2, 3, 0, 3, 3, /* p-y{} */
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
};

#define Ec Tc	/* commands that have same compilation method */
#define Dc Tc
#define Gc Tc
#define Hc Tc
#define Nc Tc
#define Pc Tc
#define dc Tc
#define gc Tc
#define hc Tc
#define lc Tc
#define nc Tc
#define pc Tc
#define xc Tc
#define tc bc
#define ic ac
#define cc ac

unsigned char *synl;	/* current line pointer for syntax errors */

/* COMMAND LAYOUT */

int
blank(Text *t)
{
	if(*t->w==' ' || *t->w=='\t' || *t->w=='\r') {
		t->w++;
		return 1;
	} else
		return 0;
}

word *
instr(unsigned char *p)		/* get address of command word */
{
	word *q = (word*)p;
	while((*q & IMASK) != IMASK)
		q++;
	return q;
}

unsigned char *
succi(unsigned char *p)
{
	word *q = instr(p);
	if(code(*q) == '{')
		return (unsigned char*)(q+1);
	else
		return p + (*q & LMASK);
}

word
pack(int neg, int cmd, int length)
{
	int l = length & LMASK;
	if(length != l)
		syntax("<command-list> or <text> too long");
	return IMASK | neg | cmd << 2*BYTE | l;
}

void
putint(Text *s, int n)
{
	assure(s, sizeof(word));
	*(word*)s->w = n;
	s->w += sizeof(word);
}

int
number(Text *t)
{
	unsigned n = 0;
	while(isdigit(*t->w)) {
		if(n > (INT_MAX-9)/10)
			syntax("number too big");
		n = n*10 + *t->w++ - '0';
	}
	return n;
}

int
addr(Text *script, Text *t)
{
	int n;
	if(reflags & REG_LENIENT)
		while(*t->w == ' ' || *t->w == '\t' || *t->w == '\r')
			t->w++;
	switch(*t->w) {
	default:
		return 0;
	case '$':
		t->w++;
		n = DOLLAR;
		break;
	case '\\':
		t->w++;
		if(*t->w=='\n' ||*t->w=='\\')
			syntax("bad regexp delimiter");
	case '/':
		n = recomp(&rebuf, t) | REGADR;
		break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		n = number(t);
		if(n == 0)
			syntax("address is zero");
	}
	putint(script, n);
	if(reflags & REG_LENIENT)
		while(*t->w == ' ' || *t->w == '\t' || *t->w == '\r')
			t->w++;
	return 1;
}

regex_t *
readdr(int x)
{
	return (regex_t*)(rebuf.s + (x&AMASK));
}

/* LABEL HANDLING */

/* the labels array consists of int values followed by strings.
   value -1 means unassigned; other values are relative to the
   beginning of the script

   on the first pass, every script ref to a label becomes the
   integer offset of that label in the labels array, or -1 if
   it is a branch to the end of script

   on the second pass (fixlabels), the script ref is replaced
   by the value from the labels array. */

Text labels;

word *
lablook(unsigned char *l, Text *labels)
{
	unsigned char *p, *q;
	int n, m;
	assure(labels, 1);
	for(p = labels->s; p < labels->w; ) {
		q = p + sizeof(word);
		if(ustrcmp(q, l) == 0)
			return (word*)p;
		q += ustrlen(q) + 1;
		p = (unsigned char*)wordp(q);
	}
	n = ustrlen(l);
	m = p - labels->s;
	assure(labels, sizeof(word)+n+1+sizeof(word));
	p = labels->s + m;
	*(word*)p = -1;
	q = p + sizeof(word);
	ustrcpy(q, l);
	q += ustrlen(q) + 1;
	labels->w = (unsigned char*)wordp(q);
	return (word*)p;
}

/* find pos in label list; assign value i to label if i>=0 */

int
getlab(Text *t, int i)
{
	word *p;
	unsigned char *u;
	while(blank(t));	/* not exactly posix */
	for(u=t->w; *t->w!='\n'; t->w++)
		if(!isprint(*t->w) || *t->w==' ' || *t->w=='\t' || *t->w=='\r')
			synwarn("invisible character in name");
	if(u == t->w)
		return -1;
	*t->w = 0;
	p = lablook(u, &labels);
	if(*p == -1)
		*p = i;
	else if(i != -1)
		syntax("duplicate label");
	*t->w = '\n';
	return (unsigned char*)p - labels.s;
}

void
Cc(Text *script, Text *t)	/* colon */
{
	if(getlab(t, script->w - sizeof(word) - script->s) == -1)
		syntax("missing label");
}

void
bc(Text *script, Text *t)
{
	int g;
	g = getlab(t, -1);	/* relative pointer to label list */
	putint(script, g);
}

void
fixlabels(Text *script)
{
	unsigned char *p;
	word *q;
	for(p=script->s; p<script->w; p=succi(p)) {
		q = instr(p);
		switch(code(*q)) {
		case 't':
		case 'b':
			if(q[1] == -1)
				q[1] = script->w - script->s;
			else if(*(word*)(labels.s+q[1]) != -1)
				q[1] = *(word*)(labels.s+q[1]);
			else
				error(3, "undefined label: %s",
					labels.s+q[1]+sizeof(word));
		}
	}
	free(labels.s);
}

/* FILES */

Text files;

void
rc(Text *script, Text *t)
{
	unsigned char *u;
	if(!blank(t))
		synwarn("no space before file name");
	while(blank(t)) ;
	for(u=t->w; *t->w!='\n'; t->w++) ;
	if(u == t->w)
		syntax("missing file name");
	*t->w = 0;
	putint(script, (unsigned char*)lablook(u, &files) - files.s);
	*t->w = '\n';
}

void
wc(Text *script, Text *t)
{
	word *p;
	rc(script, t);
	p = (word*)(files.s + ((word*)script->w)[-1]);
	if(*p != -1)
		return;
	*(Sfio_t**)p = sfopen(NiL, (char*)(p+1), "w");
	if(*(Sfio_t**)p == 0)
		syntax("can't open file for writing");
}

/* BRACKETS */

Text brack;

/* Lc() stacks (in brack) the location of the { command word.
   Rc() stuffs into that word the offset of the } sequel
   relative to the command word.
   fixbrack() modifies the offset to be relative to the
   beginning of the instruction, including addresses. */

void				/* { */
Lc(Text *script, Text *t)
{
	while(blank(t));
	putint(&brack, script->w - sizeof(word) - script->s);
}

void				/* } */
Rc(Text *script, Text *t)
{
	int l;
	word *p;
	t = t;
	if(brack.w == 0 || (brack.w-=sizeof(word)) < brack.s)
		syntax("unmatched }");
	l = *(word*)brack.w;
	p = (word*)(script->s + l);
	l = script->w - script->s - l;
	if(l >= LMASK - 3*sizeof(word))	/* fixbrack could add 3 */
		syntax("{command-list} too long)");
	*p = (*p&~LMASK) | l;
}

void
fixbrack(Text *script)
{
	unsigned char *p;
	word *q;
	if(brack.w == 0)
		return;
	if(brack.w > brack.s)
		syntax("unmatched {");
	for(p=script->s; p<script->w; p=succi(p)) {
		q = instr(p);
		if(code(*q) == '{')
			*q += (unsigned char*)q - p;
	}
	free(brack.s);
}

/* EASY COMMANDS */

void
Xc(Text *script, Text *t)	/* # */
{
	script = script;	/* avoid use/set diagnostics */
	if(t->s[1]=='n')
		nflag = 1;
	while(*t->w != '\n')
		t->w++;
}

void
Ic(Text *script, Text *t)	/* ignore */
{
	script = script;
	t->w--;
}

void
Tc(Text *script, Text *t)	/* trivial to compile */
{
	script = script;
	t = t;
}

void
xx(Text *script, Text *t)
{
	script = script;
	t = t;
	syntax("unknown command");
}

/* MISCELLANY */

void
ac(Text *script, Text *t)
{
	if(*t->w++ != '\\' || *t->w++ != '\n')
		syntax("\\<newline> missing after command");
	for(;;) {
		while(bflag && blank(t)) ;
		assure(script, 2 + sizeof(word));
		switch(*t->w) {
		case 0:
			error(ERROR_PANIC|4, "bug: missed end of <text>");
		case '\n':
			*script->w++ = *t->w;
			*script->w++ = 0;
			script->w = (unsigned char*)wordp(script->w);
			return;
		case '\\':
			t->w++;
		default:
			*script->w++ = *t->w++;
		}
	}
}

void
qc(Text *script, Text *t)
{
	sfset(sfstdin, SF_SHARE, 1);
	script = script;
	t = t;
}

void
sc(Text *script, Text *t)
{
	int c, flags, re, nsub;
	word *q;
	int n = -1;
	int delim = *t->w;
	switch(delim) {
	case '\n':
	case '\\':
		syntax("improper delimiter");
	}
	re = recomp(&rebuf, t);
	putint(script, re);
	nsub = readdr(re)->re_nsub;
	flags = script->w - script->s;
	putint(script, 0);		/* space for flags */
	while((c=*t->w++) != delim) {
		assure(script, 3+sizeof(word*));
		if(c == '\n')
			syntax("unterminated command");
		else if(c == '\\') {
			int d = *t->w++;
			if(d==delim)
				;
			else if(d=='&' || d=='\\')
				*script->w++ = c;
			else if(d>='0' && d<='9') {
				if(d > '0'+nsub)
					syntax("improper backreference");
				*script->w++ = c;
			}
			c = d;
		}
		*script->w++ = c;
	}
	*script->w++ = 0;
	script->w = (unsigned char*)wordp(script->w);
	q = (word*)(script->s + flags);
	*q = 0;
	for(;;) {
		switch(*t->w) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			if(n != -1)
				syntax("extra flags");
			n = number(t);
			if(n == 0 || (n&(PFLAG|WFLAG)) != 0)
				syntax("count out of range");
			continue;
		case 'p':
			if(*q & PFLAG)
				syntax("extra flags");
			t->w++;
			*q |= PFLAG;
			continue;
		case 'g':
			t->w++;
			if(n != -1)
				syntax("extra flags");
			n = 0;
			continue;
		case 'w':
			t->w++;
			*q |= WFLAG;
			wc(script, t);
		}
		break;
	}
	*q |= n==-1? 1: n;
}

void
yc(Text *script, Text *t)
{
	int i;
	int delim;
	unsigned char *s;
	unsigned char *p, *q;
	int pc, qc;
	if((delim = *t->w++) == '\n' || delim=='\\')
		syntax("missing delimiter");
	assure(script, UCHAR_MAX+1);
	s = script->w;
	for(i=0; i<UCHAR_MAX+1; i++)
		s[i] = 0;
	for(q=t->w; (qc = *q++)!=delim; ) {
		if(qc == '\n')
			syntax("missing delimiter");
		if(qc=='\\' && *q==delim)
			q++;
	}
	for(p=t->w; (pc = *p++) != delim; ) {
		if(pc=='\\') {
			if(*p==delim)
				pc = *p++;
			else if(*p=='n' && (reflags & REG_LENIENT)) {
				p++;
				pc = '\n';
			}
		}
		if((qc = *q++) == '\n')
			syntax("missing delimiter");
		if(qc==delim)
			syntax("string lengths differ");
		if(qc=='\\') {
			if(*q==delim)
				qc = *q++;
			else if(*q=='n' && (reflags & REG_LENIENT)) {
				q++;
				qc = '\n';
			}
		}
		if(s[pc]) {
			if(s[pc]!=qc)
				syntax("ambiguous map");
			synwarn("redundant map");
		}
		s[pc] = qc;
	}
	if(*q++ != delim)
		syntax("string lengths differ");
	for(i=0; i<UCHAR_MAX+1; i++)
		if(s[i] == 0)
			s[i] = i;
	t->w = q;
	script->w += UCHAR_MAX+1;
}

void
synwarn(char *s)
{
	unsigned char *t = ustrchr(synl, '\n');
	error(1, "%s: %.*s", s, t-synl, synl);
}

void
syntax(char *s)
{
	unsigned char *t = ustrchr(synl, '\n');
	error(3, "%s: %.*s", s, t-synl, synl);
}

void
badre(regex_t* re, int code)
{
	unsigned char *t = ustrchr(synl, '\n');
	if(code && code!= REG_NOMATCH) {
		char buf[UCHAR_MAX+1];
		regerror(code, re, buf, sizeof(buf));
		error(3, "%s: %.*s", buf, t-synl, synl);
	}
	else
		error(3, "bad regular expression: %.*s", t-synl, synl);
}

#if DEBUG

void
printscript(Text *script)
{
	unsigned char *s;
	word *q;
	for(s=script->s; s<script->w; s = succi(s)) {
		q = (word*)s;
		if((*q&IMASK) != IMASK) {
			if((*q&REGADR) == 0)
				printf("%d", *q);
			else
				regdump((regex_t*)(*q & AMASK));
			q++;
		}
		if((*q&IMASK) != IMASK) {
			if((*q&REGADR) == 0)
				printf(",%d", *q);
			else
				regdump((regex_t*)(*q & AMASK));
			q += 2;
		}
		if(code(*q) == '\n')
			continue;
		printf("%s%c\n", *q&NEG?"!":"", code(*q));
	}
}

#endif

#if DEBUG & 2

/* debugging code 2; execute stub.
   prints the compiled script (without arguments)
   then each input line with line numbers */

void
execute(Text *script, Text *y)
{
	if(recno == 1)
		printscript(script);
	printf("%d:%s",recno,y->s);
}

#endif

typedef void (*cmdf)(Text*, Text*);

static cmdf docom[128] = {
	xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,Ic,xx,xx,xx,xx,xx, /* <nl> */
	xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,
	xx,Ic,xx,Xc,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx, /* !# */
	xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,Cc,Ic,xx,Ec,xx,xx, /* :;= */
	xx,xx,xx,xx,Dc,xx,xx,Gc,Hc,xx,xx,xx,xx,xx,Nc,xx, /* DGHN */
	Pc,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx, /* P */
	xx,ac,bc,cc,dc,xx,xx,gc,hc,ic,xx,xx,lc,xx,nc,xx, /* a-n */
	pc,qc,rc,sc,tc,xx,xx,wc,xc,yc,xx,Lc,xx,Rc,xx,xx  /* p-y{} */
};

void
compile(Text *script, Text *t)
{
	int loc;	/* progam counter */
	int neg;	/* ! in effect */
	int cmd;
	int naddr;
	word *q;	/* address of instruction word */
	t->w = t->s;	/* here w is a read pointer */
	while(*t->w) {
		assure(script, 4*sizeof(word));
		loc = script->w - script->s;
		synl = t->w;
		naddr = 0;
		while(blank(t)) ;
		naddr += addr(script, t);
		if(naddr && *t->w ==',') {
			t->w++;
			naddr += addr(script, t);
			if(naddr < 2)
				syntax("missing address");
		}
		q = (word*)script->w;
		if(naddr == 2)
			*q++ = INACT;
		script->w = (unsigned char*)(q+1);
		neg = 0;
		for(;;) {
			while(blank(t));
			cmd = *t->w++;
			if(neg && docom[ccmapc(cmd,CC_NATIVE,CC_ASCII)&0x7f]==Ic)
				syntax("improper !");
			if(cmd != '!')
				break;
			neg = NEG;
		}
		if(!neg) {
			switch(adrs[ccmapc(cmd,CC_NATIVE,CC_ASCII)]) {
			case 1:
				if(naddr <= 1)
					break;
			case 0:
				if(naddr == 0)
					break;
				syntax("too many addresses");
			}
		}
		(*docom[ccmapc(cmd,CC_NATIVE,CC_ASCII)&0x7f])(script, t);
		while(*t->w == ' ' || *t->w == '\t' || *t->w == '\r')
		{
			t->w++;
			if(!(reflags & REG_LENIENT) && !spaces++)
				synwarn("space separators");
		}
		switch(*t->w) {
		case 0:
			script->w = script->s + loc;
			break;
		case ';':
			if(!(reflags & REG_LENIENT) && !semicolon++)
				synwarn("semicolon separators");
		case '\n':
			t->w++;
			break;
		default:
			if(cmd == '{')
				break;
			syntax("junk after command");
		}
		*q = pack(neg,cmd,script->w-script->s-loc);
	}
	fixbrack(script);
	fixlabels(script);
}
