/***************************************************************
*                                                              *
*           This software is part of the ast package           *
*              Copyright (c) 1996-2000 AT&T Corp.              *
*      and it may only be used by you under license from       *
*                     AT&T Corp. ("AT&T")                      *
*       A copy of the Source Code Agreement is available       *
*              at the AT&T Internet web site URL               *
*                                                              *
*     http://www.research.att.com/sw/license/ast-open.html     *
*                                                              *
*      If you have copied this software without agreeing       *
*      to the terms of the license you are infringing on       *
*         the license and copyright and are violating          *
*             AT&T's intellectual property rights.             *
*                                                              *
*               This software was created by the               *
*               Network Services Research Center               *
*                      AT&T Labs Research                      *
*                       Florham Park NJ                        *
*                                                              *
*               Phong Vo <kpv@research.att.com>                *
*             Glenn Fowler <gsf@research.att.com>              *
*                                                              *
***************************************************************/
/*
 * Glenn Fowler
 * AT&T Research
 *
 * rskey coding for recsort
 *
 * Doug McIlroy did the hard part here
 * (and with regression tests too)
 */

#include "rskeyhdr.h"

#if _sys_resource && _lib_getrlimit

#include <sys/resource.h>

static size_t
datasize(void)
{
	struct rlimit	rlim;

	getrlimit(RLIMIT_DATA, &rlim);
	return rlim.rlim_cur;
}

#else

#define datasize()	(size_t)(128*1024*1024)

#endif

/*
 * Canonicalize the number string pointed to by dp, of length
 * len.  Put the result in kp.
 *
 * A field of length zero, or all blank, is regarded as 0.
 * Over/underflow is rendered as huge or zero and properly signed.
 * It happens 1e+-1022.
 *
 * Canonicalized strings may be compared as strings of unsigned
 * chars.  For good measure, a canonical string has no zero bytes.
 *
 * Syntax: optionally signed floating point, with optional
 * leading spaces.  A syntax deviation ends the number.
 *
 * Form of output: packed in 4-bit nibbles.  First
 * 3 nibbles count the number N of significant digits
 * before the decimal point.  The quantity actually stored
 * is 2048+sign(x)*(N+1024).  Further nibbles contain
 * 1 decimal digit d each, stored as d+2 if x is positive
 * and as 10-d if x is negative.  Leading and trailing
 * zeros are stripped, and a trailing "digit" d = -1 
 * is appended.  (The trailing digit handled like all others,
 * so encodes as 1 or 0xb according to the sign of x.)
 * An odd number of nibbles is padded with zero.
 *
 * Buglet: overflow is reported if output is exactly filled.
 */

#define encode(x)	(neg?(10-(x)):((x)+2))
#define putdig(x)	(nib?(*dig=encode(x)<<4,nib=0):(*dig++|=encode(x),nib=1))

static int
#if __STD_C
key_n_code(Rskey_t* kp, Field_t* f, unsigned char* dp, unsigned char* cp, int len)
#else
key_n_code(kp, f, dp, cp, len)
Rskey_t*	kp;
Field_t*	f;
unsigned char*	dp;
unsigned char*	cp;
int		len;
#endif
{
	unsigned char*	dig = cp + 1;	/* byte for next digit */
	int		nib = 0;	/* high nibble 1, low nibble 0 */
	unsigned char*	xp = dp;
	unsigned char*	ep = xp + len;	/* end pointer */
	int		zeros = 0;	/* count zeros seen but not installed */
	int		sigdig = 1024;
	int		neg = f->rflag;	/* 0 for +, 1 for - */
	int		decimal = 0;
	int		n;
	int		inv;

	cp[1] = 0;

	/*
	 * eat blanks
	 */

	while (xp < ep && blank(*xp)) xp++;

	/*
	 * eat sign
	 */

	if (xp < ep)
		switch (*xp)
		{
		case '-':
			neg ^= 1;
			/*FALLTHROUGH*/
		case '+':
			xp++;
			break;
		}

	/*
	 * eat leading zeros
	 */

	while (xp < ep && *xp == '0') xp++;
	if (xp < ep && *xp == '.')
	{
		decimal++;
		for (xp++; xp < ep && *xp == '0'; xp++)
			sigdig--;
	}
	if (xp >= ep || *xp > '9' || *xp < '0')
	{
		/*
		 * no significant digit
		 */

		sigdig = 0;
		neg = 0;
		goto retzero;
	}
	for (; xp < ep; xp++)
	{
		switch (*xp)
		{
		case '.':
			if (decimal)
				goto out;
			decimal++;
			continue;
		case '0':
			zeros++;
			if (!decimal)
				sigdig++;
			continue;
		case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9':
			for (; zeros > 0; zeros--)
				putdig(0);
			n = *xp - '0';
			putdig(n);
			if (!decimal)
				sigdig++;
			continue;
		case 'e':
		case 'E':
			if (f->flag != 'g')
				goto out;
			inv = 1;
			if (xp < ep) switch(*++xp)
			{
			case '-':
				inv = -1;
				/*FALLTHROUGH*/
			case '+':
				xp++;
				break;
			}
			if (xp >= ep || *xp > '9' || *xp < '0')
				goto out;
			for (n = 0; xp < ep; xp++)
			{
				int	c = *xp;

				if (c < '0' || c > '9')
					break;
				if ((n = 10 * n + c - '0') >= 0)
					continue;
				sigdig = 2047 * inv;
				goto out;
			}
			sigdig += n * inv;
			goto out;
		default:
			goto out;
		}
	}
 out:
	if (sigdig < 0 || sigdig >= 2047)
	{
		sigdig = sigdig < 0 ? 0 : 2047;
		if (kp->keydisc->errorf)
			(*kp->keydisc->errorf)(kp, kp->keydisc, 1, "%-.*s: numeric field overflow", dp);
		dig = cp + 1;
		*dig = 0;
		nib = 0;
	}
 retzero:
	if (neg) sigdig = 2048 - sigdig;
	else sigdig = 2048 + sigdig;
	cp[0] = sigdig >> 4;
	cp[1] |= sigdig << 4;
	putdig(-1);
	return dig - cp + 1 - nib;
}

/*
 * Encode text field subject to options -r -fdi -b.
 * Fields are separated by 0 (or 255 if rflag is set)
 * the anti-ambiguity stuff prevents such codes from
 * happening otherwise by coding real zeros and ones
 * as 0x0101 and 0x0102, and similarly for complements
 */

static int
#if __STD_C
key_t_code(Rskey_t* kp, Field_t* f, unsigned char* dp, unsigned char* cp, int len)
#else
key_t_code(kp, f, dp, cp, len)
Rskey_t*	kp;
Field_t*	f;
unsigned char*	dp;
unsigned char*	cp;
int		len;
#endif
{
	unsigned char*	xp = cp;
	int		c;
	unsigned char*	keep = f->keep;
	unsigned char*	trans = f->trans;
	int		reverse = f->rflag ? ~0: 0;

	while (--len >= 0)
	{
		c = *dp++;
		if (keep[c])
		{
			c = trans[c];
			if(c <= 1)
			{
				/*
				 * anti-ambiguity
				 */

				*xp++ = 1 ^ reverse;
				c++;
			}
			else if (c >= 254)
			{
				*xp++ = 255 ^ reverse;
				c--;
			}
			*xp++ = c ^ reverse;
		}
	}
	*xp++ = reverse;
	return xp - cp;
}

/*
 * sort by month name
 */

static const char*	month[] =
{
	"jan", "feb", "mar", "apr", "may", "jun",
	"jul", "aug", "sep", "oct", "nov", "dec"
};

static int
#if __STD_C
key_m_code(Rskey_t* kp, Field_t* f, unsigned char* dp, unsigned char* cp, int len)
#else
key_m_code(kp, f, dp, cp, len)
Rskey_t*	kp;
Field_t*	f;
unsigned char*	dp;
unsigned char*	cp;
int		len;
#endif
{
	register int	c;
	int		j = -1;
	int		i;
	unsigned char*	xp;

	for (; len > 0 && blank(*dp); dp++, len--);
	if (len >= 3)
		while (++j < elementsof(month))
		{
			xp = (unsigned char*)month[j];
			for (i = 0; i < 3; i++)
			{
				c = dp[i];
				if (isupper(c))
					c = tolower(c);
				if (c != *xp++)
					break;
			}
			if (i >= 3)
				break;
		}
	*cp = j >= elementsof(month) ? 0 : j + 1;
	if (f->rflag)
		*cp ^= ~0;
	return 1;
}

/*
 * the recsort defkeyf
 * return encoded key for dat,datlen in key,keylen
 */

static int
#if __STD_C
code(Rs_t* rs, unsigned char* dat, int datlen, unsigned char* key, int keylen, Rsdisc_t* disc)
#else
code(rs, dat, datlen, key, keylen, disc)
Rs_t*		rs;
unsigned char*	dat;
int		datlen;
unsigned char*	key;
int		keylen;
Rsdisc_t*	disc;
#endif
{
	Rskey_t*	kp = (Rskey_t*)((char*)rs->disc - offsetof(Rskey_t, disc));
	unsigned char*	cp;
	Field_t*	fp;
	unsigned char*	ep;
	unsigned char*	op = key;
	unsigned char*	xp = dat + datlen;
	int		t;
	int		np;
	int		m = kp->field.maxfield;
	unsigned char**	pp = kp->field.positions;

	pp[0] = dat;
	np = 1;
	switch (t = kp->tab)
	{
	case 0:
		for (cp = dat; cp < xp && np < m;)
		{
			while (blank(*cp) && cp < xp)
				cp++;
			while (!blank(*cp) && cp < xp)
				cp++;
			pp[np++] = cp;
		}
		break;
	case '\n':
		break;
	default:
		for (cp = dat; cp < xp && np < m;)
			if (*cp++ == t)
				pp[np++] = cp;
		break;
	}
	for (fp = kp->field.head; fp; fp = fp->next)
	{
		t = fp->begin.field;
		if (t < np)
		{
			cp = pp[t];
			if (fp->bflag && kp->field.global.next)
				while (cp < xp && blank(*cp))
					cp++;
			cp += fp->begin.index;
			if (cp > xp)
				cp = xp;
		}
		else cp = xp;
		t = fp->end.field;
		if (t < np)
		{
			if (fp->end.index < 0)
			{
				if (t >= np - 1)
					ep = xp;
				else
				{
					ep = pp[t + 1];
					if (kp->tab) ep--;
				}
			}
			else
			{
				ep = pp[t];
				if (fp->eflag)
					while(ep < xp && blank(*ep))
						ep++;
				ep += fp->end.index;
			}
			if (ep > xp)
				ep = xp;
			else if (ep < cp)
				ep = cp;
		}
		else ep = xp;
		op += (*fp->coder)(kp, fp, cp, op, ep - cp);
	}
	return op - key;
}

/*
 * conflict message
 */

static void
#if __STD_C
conflict(Rskey_t* kp, int c)
#else
conflict(kp, c)
Rskey_t*	kp;
int		c;
#endif
{
	(*kp->keydisc->errorf)(kp, kp->keydisc, 1, "%c: key type conflicts with previous value", c);
}

/*
 * nice band
 */

static int
#if __STD_C
checkfield(Rskey_t* kp, Field_t* fp, const char* key, int c)
#else
checkfield(kp, fp, key, c)
Rskey_t*	kp;
Field_t*	fp;
char*		key;
int		c;
#endif
{
	if (c || fp->begin.field < 0 || fp->end.field < 0 || fp->begin.index < 0 || fp->end.index < -1)
	{
		if (kp->keydisc->errorf)
		{
			if (key)
				(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "%s: invalid key field specification", key + fp->standard - 1);
			else
				(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "field[%d]: invalid key field specification", fp->index);
		}
		kp->keydisc->flags |= RSKEY_ERROR;
		return -1;
	}
	if (kp->keydisc->errorf && fp->coder == key_n_code && fp->keep)
		(*kp->keydisc->errorf)(kp, kp->keydisc, 1, "n: key type conflicts with d,i");
	return 0;
}

/*
 * add coding function
 */

static void
#if __STD_C
addcoder(Rskey_t* kp, Field_t* fp, Coder_t np, int c)
#else
addcoder(kp, fp, np, c)
Rskey_t*	kp;
Field_t*	fp;
Coder_t		np;
int		c;
#endif
{
	NoP(kp);
	if (kp->keydisc->errorf && fp->coder && fp->coder != np)
		conflict(kp, c);
	fp->coder = np;
	fp->flag = c;
}

/*
 * add translation table
 */

static void
#if __STD_C
addtable(Rskey_t* kp, int c, unsigned char** op, unsigned char* np)
#else
addtable(kp, c, op, np)
Rskey_t*	kp;
int		c;
unsigned char**	op;
unsigned char*	np;
#endif
{
	NoP(kp);
	if (kp->keydisc->errorf && *op && *op != np)
		conflict(kp, c);
	*op = np;
}

/*
 * add a sort key field option c
 */

static int
#if __STD_C
addopt(Rskey_t* kp, register Field_t* fp, int c, int end)
#else
addopt(kp, fp, c, end)
Rskey_t*		kp;
register Field_t*	fp;
int			c;
int			end;
#endif
{
	switch (c)
	{
	case 'a':
		if (!fp->aflag)
		{
			fp->aflag = 1;
			if (!kp->field.prev)
			{
				if (kp->keydisc->errorf)
					(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "field[%d]: global accumulate invalid", fp->index);
				kp->keydisc->flags |= RSKEY_ERROR;
				return 0;
			}
			(kp->field.tail = kp->field.prev)->next = 0;
			kp->field.prev = 0;
			if (kp->accumulate.tail)
				kp->accumulate.tail->next = fp;
			else
				kp->accumulate.head = kp->accumulate.tail = fp;
		}
		return 1;
	case 'b':
		if (end) fp->eflag = 1;
		else fp->bflag = 1;
		return 1;
	case 'd':
		addtable(kp, c, &fp->keep, kp->state->dict);
		break;
	case 'f':
		addtable(kp, c, &fp->trans, kp->state->fold);
		break;
	case 'g':
	case 'n':
		addcoder(kp, fp, key_n_code, c);
		break;
	case 'i':
		addtable(kp, c, &fp->keep, kp->state->print);
		break;
	case 'M':
		addcoder(kp, fp, key_m_code, c);
		break;
	case 'r':
		fp->rflag = 1;
		return 1;
	default:
		return 0;
	}
	kp->coded = 1;
	if (kp->keydisc->errorf && fp != kp->field.tail)
		(*kp->keydisc->errorf)(kp, kp->keydisc, 1, "field spec precedes global option %c", c);
	return 1;
}

/*
 * add sort key options in s
 * all!=0 applies to all fields,
 * otherwise the current field
 */

int
#if __STD_C
rskeyopt(Rskey_t* kp, const char* key, int all)
#else
rskeyopt(kp, key, all)
Rskey_t*	kp;
char*		key;
int		all;
#endif
{
	register Field_t*	fp;
	register int		c;
	char*			s;

	fp = all ? kp->field.head : kp->field.tail;
	s = (char*)key;
	while (addopt(kp, fp, c = *s++, 0));
	if (fp->standard && (c == ',' || c == ' '))
	{
		if ((fp->end.field = (int)strtol(s, (char**)&s, 10) - 1) > kp->field.maxfield)
			kp->field.maxfield = fp->end.field;
		if (*s == '.' && !(fp->end.index = (int)strtol(s + 1, &s, 10)))
			fp->end.index = -1;
		while (addopt(kp, fp, c = *s++, 1));
	}
	return checkfield(kp, fp, key, c);
}

/*
 * add a sort key
 */

int
#if __STD_C
rskey(Rskey_t* kp, const char* key, int obsolete)
#else
rskey(kp, key, obsolete)
Rskey_t*	kp;
char*		key;
int		obsolete;
#endif
{
	register Field_t*	fp;
	int			n;
	int			m;
	int			standard;
	char*			s = (char*)key;
	char			buf[32];

	n = (int)strtol(s, &s, 10);
	if (standard = !obsolete)
	{
		if (*s == ':')
		{
			kp->fixed = n;
			if (!*++s)
				return 0;
			n = strtol(s, &s, 10);
			m = 0;
			if (*s)
			{
				if (*s != ':')
				{
					if (kp->keydisc->errorf)
						(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "%s: invalid fixed record key length", key);
					kp->keydisc->flags |= RSKEY_ERROR;
					return -1;
				}
				if (*++s)
				{
					m = strtol(s, &s, 10);
					if (*s && (*s != ':' || *++s))
					{
						if (kp->keydisc->errorf)
							(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "%s: invalid fixed key offset", key);
						kp->keydisc->flags |= RSKEY_ERROR;
						return -1;
					}
				}
			}
			key = (const char*)(s = buf);
			sfsprintf(s, sizeof(buf), ".%d,1.%d", m + 1, m + n);
			n = 1;
		}
	}
	if (obsolete == '-')
	{
		if (!kp->field.global.next && rskey(kp, "0", 1))
			return -1;
		s = (char*)key;
		if ((kp->field.tail->end.field = *s == '.' ? kp->field.tail->begin.field : (int)strtol(s, &s, 10)) > kp->field.maxfield)
			kp->field.maxfield = kp->field.tail->end.field;
		if (*s == '.')
			kp->field.tail->end.index = (int)strtol(s + 1, &s, 10);
		else
			kp->field.tail->end.field--;
		if (!kp->field.tail->end.index)
			kp->field.tail->end.index = -1;
	}
	else if (!(fp = vmnewof(Vmheap, 0, Field_t, 1, 0)))
	{
		if (kp->keydisc->errorf)
			(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "out of space [field]");
		kp->keydisc->flags |= RSKEY_ERROR;
		return -1;
	}
	else
	{
		fp->index = ++kp->field.index;
		kp->field.prev = kp->field.tail;
		kp->field.tail = kp->field.tail->next = fp;
		fp->bflag = fp->eflag = 0;
		fp->standard = standard;
		if ((fp->begin.field = n - fp->standard) > kp->field.maxfield)
			kp->field.maxfield = fp->begin.field;
		fp->end.field = MAXFIELD;
		if (*s == '.')
			fp->begin.index = (int)strtol(s + 1, &s, 10) - fp->standard;
	}
	return *s ? rskeyopt(kp, s, 0) : 0;
}

/*
 * initialize key info after all rskey()/rskeyargs() calls
 */

int
#if __STD_C
rskeyinit(register Rskey_t* kp)
#else
rskeyinit(kp)
register Rskey_t*	kp;
#endif
{
	register long		n;
	register Field_t*	fp;
	long			m;
	size_t			z;

	static char*		in[] = { "-", 0 };

	/*
	 * finalize the fields
	 */

	if (checkfield(kp, kp->field.tail, NiL, 0))
		return -1;
	fp = kp->field.head;
	if (!fp->coder)
	{
		fp->coder = key_t_code;
		fp->flag = 't';
	}
	if (!fp->trans)
		fp->trans = kp->state->ident;
	if (!fp->keep)
		fp->keep = kp->state->all;
	if (fp->rflag)
	{
		fp->rflag = 0;
		kp->type |= RS_REVERSE;
	}
	while (fp = fp->next)
	{
		n = 0;
		if (!fp->coder)
		{
			fp->coder = key_t_code;
			fp->flag = 't';
		}
		else n = 1;
		if(!fp->trans) fp->trans = kp->state->ident;
		else n = 1;
		if(!fp->keep) fp->keep = kp->state->all;
		else n = 1;
		if (!n && !fp->bflag && !fp->eflag && !fp->rflag)
		{
			fp->coder = kp->field.global.coder;
			fp->flag = kp->field.global.flag;
			fp->trans = kp->field.global.trans;
			fp->keep = kp->field.global.keep;
			fp->rflag = kp->field.global.rflag;
			fp->bflag = kp->field.global.bflag;
			if (fp->standard)
				fp->eflag = kp->field.global.bflag;
		}
		else if (kp->type & RS_REVERSE)
			fp->rflag = !fp->rflag;
		if (fp->standard)
		{
			if (!fp->end.index)
				fp->end.index--;
		}
		else if (!fp->end.index && fp->end.field)
		{
			if (kp->tab && fp->eflag)
			{
				if (kp->keydisc->errorf)
					(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "skipping blanks right after tab-char is ill-defined");
				kp->keydisc->flags |= RSKEY_ERROR;
				return -1;
			}
			fp->end.index--;
		}
		if (kp->fixed)
		{
			if (fp->begin.index > kp->fixed)
			{
				if (kp->keydisc->errorf)
					(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "field[%d]: begin index %d is greater than fixed record size", fp->index, fp->begin.index);
				kp->keydisc->flags |= RSKEY_ERROR;
				return -1;
			}
			if (fp->end.index > kp->fixed)
			{
				if (kp->keydisc->errorf)
					(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "field[%d]: end index %d is greater than fixed record size", fp->index, fp->end.index);
				kp->keydisc->flags |= RSKEY_ERROR;
				return -1;
			}
		}
	}
	fp = kp->field.head;
	if (fp = fp->next)
	{
		kp->field.head = fp;
		if (!fp->next && !kp->tab && !fp->begin.field && !fp->end.field && fp->end.index > 0 && fp->flag == 't' && fp->trans == kp->state->ident && fp->keep == kp->state->all && !fp->bflag && !fp->eflag && !fp->rflag)
		{
			kp->disc.type |= RS_KSAMELEN;
			kp->disc.key = fp->begin.index;
			kp->disc.keylen = fp->end.index - fp->begin.index;
		}
		else
			kp->coded = 1;
	}
	if (kp->coded)
	{
		kp->field.maxfield += 2;
		kp->disc.defkeyf = code;
		kp->disc.key = 2 * kp->field.maxfield;
		if (!(kp->field.positions = vmnewof(Vmheap, 0, unsigned char*, kp->field.maxfield, 0)))
		{
			if (kp->keydisc->errorf)
				(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "out of space [positions]");
			kp->keydisc->flags |= RSKEY_ERROR;
			return -1;
		}
	}
	if (kp->fixed)
	{
		kp->disc.type |= RS_DSAMELEN;
		kp->disc.data = kp->fixed;
	}
	else
	{
		if (!kp->disc.keylen)
			kp->disc.keylen = -1;
		kp->disc.data = '\n';
	}

	/*
	 * limit the sizes
	 */

	z = datasize() / 3;
	if (kp->nproc > 1)
		z /= 2;
	if (kp->insize > z)
		kp->insize = z;
	if (kp->outsize > z)
		kp->outsize = z;

	/*
	 * reconcile the sizes
	 */

	if (!(n = kp->alignsize))
		n = SF_BUFSIZE;
	if (n & (n - 1))
	{
		for (m = 1; m < n; m <<= 1)
			if (m >= (LONG_MAX >> CHAR_BIT))
			{
				if (kp->keydisc->errorf)
					(*kp->keydisc->errorf)(kp, kp->keydisc, 2, "%ld: invalid alignment size", n);
				kp->keydisc->flags |= RSKEY_ERROR;
				return -1;
			}
		if (kp->keydisc->errorf)
			(*kp->keydisc->errorf)(kp, kp->keydisc, 1, "%ld: alignment size rounded to %ld", n, m);
		n = m;
	}
	kp->alignsize = n--;
	kp->insize = (kp->insize < kp->alignsize) ? kp->alignsize : roundof(kp->insize, kp->alignsize);
	kp->outsize = (kp->outsize && kp->outsize < kp->alignsize) ? kp->alignsize : roundof(kp->outsize, kp->alignsize);
	kp->procsize = (kp->procsize < kp->alignsize) ? kp->alignsize : roundof(kp->procsize, kp->alignsize);
	if (kp->procsize > kp->insize)
		kp->procsize = kp->insize;
	if (kp->insize == kp->alignsize && kp->alignsize > 1)
		kp->alignsize /= 2;

	/*
	 * no input files equivalent to "-"
	 */

	if (!kp->input || !*kp->input)
		kp->input = in;
	return (kp->keydisc->flags & RSKEY_ERROR) ? -1 : 0;
}
