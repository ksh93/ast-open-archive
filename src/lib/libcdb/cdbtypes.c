/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1997-2000 AT&T Corp.                *
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
/*
 * Glenn Fowler
 * AT&T Research
 *
 * cdb builtin external types
 *
 * bcd and ibm hacked from Griff Smith's ttu library
 */

#include "cdblib.h"
#include "FEATURE/lib"

#include <math.h>
#if _hdr_float
#include <float.h>
#endif
#include <hashpart.h>
#include <tm.h>

/*
 * bcd_unpack[x] is the binary value for bcd byte x
 * invalid codes convert to 0
 */

static const unsigned char	bcd_unpack[] =
{
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
    10,11,12,13,14,15,16,17,18,19, 0, 0, 0, 0, 0, 0,
    20,21,22,23,24,25,26,27,28,29, 0, 0, 0, 0, 0, 0,
    30,31,32,33,34,35,36,37,38,39, 0, 0, 0, 0, 0, 0,
    40,41,42,43,44,45,46,47,48,49, 0, 0, 0, 0, 0, 0,
    50,51,52,53,54,55,56,57,58,59, 0, 0, 0, 0, 0, 0,
    60,61,62,63,64,65,66,67,68,69, 0, 0, 0, 0, 0, 0,
    70,71,72,73,74,75,76,77,78,79, 0, 0, 0, 0, 0, 0,
    80,81,82,83,84,85,86,87,88,89, 0, 0, 0, 0, 0, 0,
    90,91,92,93,94,95,96,97,98,99, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/*
 * bcd_negative[x]!=0 if bcd sign is negative
 */

static const unsigned char	bcd_negative[] =
{
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
};

/*
 * bcd_hichar[x] is the character for the high order bcd digit in x
 */

static const unsigned char	bcd_hichar[] =
{
    '0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0',
    '1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1',
    '2','2','2','2','2','2','2','2','2','2','2','2','2','2','2','2',
    '3','3','3','3','3','3','3','3','3','3','3','3','3','3','3','3',
    '4','4','4','4','4','4','4','4','4','4','4','4','4','4','4','4',
    '5','5','5','5','5','5','5','5','5','5','5','5','5','5','5','5',
    '6','6','6','6','6','6','6','6','6','6','6','6','6','6','6','6',
    '7','7','7','7','7','7','7','7','7','7','7','7','7','7','7','7',
    '8','8','8','8','8','8','8','8','8','8','8','8','8','8','8','8',
    '9','9','9','9','9','9','9','9','9','9','9','9','9','9','9','9',
    '?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
    '?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
    '?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
    '?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
    '?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
    '?','?','?','?','?','?','?','?','?','?','?','?','?','?','?','?',
};

/*
 * bcd_lochar[x] is the character for the low order bcd digit in x
 */

static const unsigned char	bcd_lochar[] =
{
    '0','1','2','3','4','5','6','7','8','9','?','?','?','?','?','?',
    '0','1','2','3','4','5','6','7','8','9','?','?','?','?','?','?',
    '0','1','2','3','4','5','6','7','8','9','?','?','?','?','?','?',
    '0','1','2','3','4','5','6','7','8','9','?','?','?','?','?','?',
    '0','1','2','3','4','5','6','7','8','9','?','?','?','?','?','?',
    '0','1','2','3','4','5','6','7','8','9','?','?','?','?','?','?',
    '0','1','2','3','4','5','6','7','8','9','?','?','?','?','?','?',
    '0','1','2','3','4','5','6','7','8','9','?','?','?','?','?','?',
    '0','1','2','3','4','5','6','7','8','9','?','?','?','?','?','?',
    '0','1','2','3','4','5','6','7','8','9','?','?','?','?','?','?',
    '0','1','2','3','4','5','6','7','8','9','?','?','?','?','?','?',
    '0','1','2','3','4','5','6','7','8','9','?','?','?','?','?','?',
    '0','1','2','3','4','5','6','7','8','9','?','?','?','?','?','?',
    '0','1','2','3','4','5','6','7','8','9','?','?','?','?','?','?',
    '0','1','2','3','4','5','6','7','8','9','?','?','?','?','?','?',
    '0','1','2','3','4','5','6','7','8','9','?','?','?','?','?','?',
};

static int
internal_bcd(Cdb_t* cdb, Cdbformat_t* fp, Cdbdata_t* dp, const char* b, size_t n, Cdbtype_t* type)
{
	register unsigned char*		s = (unsigned char*)b;
	register unsigned char*		e = s + fp->width - 1;
	register Cdbuint_t		u;
	register Sfulong_t		w;
	register unsigned const char*	p;
	register char*			t;
	int				k;

	switch (fp->ultype)
	{
	case CDB_INTEGER:
	case CDB_UNSIGNED|CDB_INTEGER:
		u = 0;
		p = bcd_unpack;
		while (s < e && !*s)
			s++;
		switch (e - s)
		{
		case 11: u *= 100; u += p[*s++];
		case 10: u *= 100; u += p[*s++];
		case  9: u *= 100; u += p[*s++];
		case  8: u *= 100; u += p[*s++];
		case  7: u *= 100; u += p[*s++];
		case  6: u *= 100; u += p[*s++];
		case  5: u *= 100; u += p[*s++];
		case  4: u *= 100; u += p[*s++];
		case  3: u *= 100; u += p[*s++];
		case  2: u *= 100; u += p[*s++];
		case  1: u *= 100; u += p[*s++];
		case  0: u *=  10; u += p[*s >> 4];
		case -1:if (bcd_negative[*s]) u = -u;
		}
		if (fp->flags & CDB_UNSIGNED)
		{
			dp->number.uinteger = u;
			SETUINTEGER(dp, fp, 0);
		}
		else
		{
			dp->number.integer = u;
			SETINTEGER(dp, fp, 0);
		}
		break;
	case CDB_LONG|CDB_INTEGER:
	case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
		w = 0;
		p = bcd_unpack;
		while (s < e && !*s)
			s++;
		switch (e - s)
		{
		case 21: w *= 100; w += p[*s++];
		case 20: w *= 100; w += p[*s++];
		case 19: w *= 100; w += p[*s++];
		case 18: w *= 100; w += p[*s++];
		case 17: w *= 100; w += p[*s++];
		case 16: w *= 100; w += p[*s++];
		case 15: w *= 100; w += p[*s++];
		case 14: w *= 100; w += p[*s++];
		case 13: w *= 100; w += p[*s++];
		case 12: w *= 100; w += p[*s++];
		case 11: w *= 100; w += p[*s++];
		case 10: w *= 100; w += p[*s++];
		case  9: w *= 100; w += p[*s++];
		case  8: w *= 100; w += p[*s++];
		case  7: w *= 100; w += p[*s++];
		case  6: w *= 100; w += p[*s++];
		case  5: w *= 100; w += p[*s++];
		case  4: w *= 100; w += p[*s++];
		case  3: w *= 100; w += p[*s++];
		case  2: w *= 100; w += p[*s++];
		case  1: w *= 100; w += p[*s++];
		case  0: w *=  10; w += p[*s >> 4];
		case -1:if (bcd_negative[*s]) w = -w;
		}
		if (fp->flags & CDB_UNSIGNED)
		{
			dp->number.winteger = w;
			SETWINTEGER(dp, fp, 0);
		}
		else
		{
			dp->number.linteger = w;
			SETLINTEGER(dp, fp, 0);
		}
		break;
	case CDB_STRING:
		k = (n + 1) * 2 + 1;
		if (!(t = vmnewof(cdb->record->vm, 0, char, k, 0)))
			return -1;
		dp->string.base = t;
		*t++ = bcd_negative[*e] ? '-' : '0';
		while (s < e)
		{
			*t++ = bcd_hichar[*s];
			*t++ = bcd_lochar[*s++];
		}
		*t++ = bcd_hichar[*s];
		*t = 0;
		dp->string.length = t - dp->string.base;
		dp->flags |= CDB_STRING|CDB_CACHED|CDB_TERMINATED;
		break;
	}
	return fp->width;
}

/*
 * bcd_pack[x] is bcd byte for 0<=x<=99
 */

static const unsigned char	bcd_pack[] =
{
       0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
      16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
      32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
      48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
      64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
      80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
      96, 97, 98, 99,100,101,102,103,104,105,
     112,113,114,115,116,117,118,119,120,121,
     128,129,130,131,132,133,134,135,136,137,
     144,145,146,147,148,149,150,151,152,153,
};

static int
external_bcd(Cdb_t* cdb, Cdbformat_t* fp, Cdbdata_t* dp, char* b, size_t n, Cdbtype_t* type)
{
	register unsigned char*	s = (unsigned char*)b;
	register unsigned char*	e = s + fp->width;
	register Cdbuint_t	v;
	register Sfulong_t	w;
	register unsigned char*	t;
	int			k;
	int			x;

	if (e-- <= s)
		return -1;
	if (fp->width > n)
		return -(fp->width + 1);
	switch (fp->ultype)
	{
	case CDB_INTEGER:
		v = dp->number.integer;
		if (dp->number.integer < 0)
		{
			v = -v;
			*e = 0xD;
			goto encode_integer;
		}
		/*FALLTHROUGH*/
	case CDB_UNSIGNED|CDB_INTEGER:
		v = dp->number.uinteger;
		*e = 0xC;
	encode_integer:
		*e |= bcd_pack[(v % 10) * 10];
		v /= 10;
		while (e-- > s)
		{
			*e = bcd_pack[v % 100];
			v /= 100;
		}
		break;
	case CDB_LONG|CDB_INTEGER:
		w = dp->number.linteger;
		if (dp->number.linteger < 0)
		{
			w = -w;
			*e = 0xD;
			goto encode_linteger;
		}
		/*FALLTHROUGH*/
	case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
		w = dp->number.linteger;
		*e = 0xC;
	encode_linteger:
		*e |= bcd_pack[(w % 10) * 10];
		w /= 10;
		while (e-- > s)
		{
			*e = bcd_pack[w % 100];
			w /= 100;
		}
		break;
	case CDB_STRING:
		t = (unsigned char*)dp->string.base;
		e = t + dp->string.length;
		if (t < e)
		{
			if (*t == '-')
			{
				t++;
				x = 0xD;
			}
			else
				x = 0xC;
			k = fp->width * 2 - 1;
			if ((k -= (e - t)) < 0)
				t -= k;
			else for (k /= 2; k > 0; k--)
				*s++ = 0;
			*s = ((e - t) & 1) ? ((*t++ - '0') << 4) : 0;
			while (t < e)
			{
				*s++ |= *t++ - '0';
				*s = (*t++ - '0') << 4;
			}
			*s |= x;
		}
		break;
	}
	return fp->width;
}

static int
validate_bcd(Cdb_t* cdb, register Cdbformat_t* fp, Cdbtype_t* type)
{
	if (!fp->width)
	{
		fp->width = 1;
		return 0;
	}
	switch (fp->type)
	{
	case CDB_INTEGER:
		return (fp->width <= ((fp->flags & CDB_LONG) ? 11 : 5)) ? 0 : -1;
	case CDB_STRING:
		return 0;
	}
	return -1;
}

/*
 * positive exponent values for IBM 370 floating point
 */

#if DBL_MAX_10_EXP > 37

#define IBM_FP_HI		7.23700557733226210e+75
#define IBM_FP_LO		5.39760534693402789e-79

static const double	ibm_exp[] =
{
    8.63616855509444463e-78, 1.38178696881511114e-76,
    2.21085915010417782e-75, 3.53737464016668452e-74,
    5.65979942426669523e-73, 9.05567907882671237e-72,
    1.44890865261227398e-70, 2.31825384417963837e-69,
    3.70920615068742139e-68, 5.93472984109987422e-67,
    9.49556774575979875e-66, 1.51929083932156780e-64,
    2.43086534291450848e-63, 3.88938454866321357e-62,
    6.22301527786114171e-61, 9.95682444457782673e-60,
    1.59309191113245228e-58, 2.54894705781192364e-57,
    4.07831529249907783e-56, 6.52530446799852453e-55,
    1.04404871487976392e-53, 1.67047794380762228e-52,
    2.67276471009219565e-51, 4.27642353614751303e-50,
    6.84227765783602085e-49, 1.09476442525376334e-47,
    1.75162308040602134e-46, 2.80259692864963414e-45,
    4.48415508583941463e-44, 7.17464813734306340e-43,
    1.14794370197489014e-41, 1.83670992315982423e-40,
    2.93873587705571877e-39, 4.70197740328915003e-38,
    7.52316384526264005e-37, 1.20370621524202241e-35,
    1.92592994438723585e-34, 3.08148791101957736e-33,
    4.93038065763132378e-32, 7.88860905221011805e-31,
    1.26217744835361890e-29, 2.01948391736579022e-28,
    3.23117426778526435e-27, 5.16987882845642297e-26,
    8.27180612553027675e-25, 1.32348898008484430e-23,
    2.11758236813575085e-22, 3.38813178901720136e-21,
    5.42101086242752217e-20, 8.67361737988403547e-19,
    1.38777878078144568e-17, 2.22044604925031308e-16,
    3.55271367880050093e-15, 5.68434188608080149e-14,
    9.09494701772928238e-13, 1.45519152283668518e-11,
    2.32830643653869629e-10, 3.72529029846191406e-09,
    5.96046447753906250e-08, 9.53674316406250000e-07,
    1.52587890625000000e-05, 2.44140625000000000e-04,
    3.90625000000000000e-03, 6.25000000000000000e-02,
    1.00000000000000000e+00, 1.60000000000000000e+01,
    2.56000000000000000e+02, 4.09600000000000000e+03,
    6.55360000000000000e+04, 1.04857600000000000e+06,
    1.67772160000000000e+07, 2.68435456000000000e+08,
    4.29496729600000000e+09, 6.87194767360000000e+10,
    1.09951162777600000e+12, 1.75921860444160000e+13,
    2.81474976710656000e+14, 4.50359962737049600e+15,
    7.20575940379279360e+16, 1.15292150460684700e+18,
    1.84467440737095516e+19, 2.95147905179352826e+20,
    4.72236648286964521e+21, 7.55578637259143234e+22,
    1.20892581961462920e+24, 1.93428131138340668e+25,
    3.09485009821345069e+26, 4.95176015714152110e+27,
    7.92281625142643376e+28, 1.26765060022822940e+30,
    2.02824096036516704e+31, 3.24518553658426727e+32,
    5.19229685853482763e+33, 8.30767497365572421e+34,
    1.32922799578491587e+36, 2.12676479325586540e+37,
    3.40282366920938463e+38, 5.44451787073501542e+39,
    8.71122859317602466e+40, 1.39379657490816395e+42,
    2.23007451985306231e+43, 3.56811923176489970e+44,
    5.70899077082383952e+45, 9.13438523331814324e+46,
    1.46150163733090292e+48, 2.33840261972944467e+49,
    3.74144419156711147e+50, 5.98631070650737835e+51,
    9.57809713041180536e+52, 1.53249554086588886e+54,
    2.45199286538542217e+55, 3.92318858461667548e+56,
    6.27710173538668076e+57, 1.00433627766186892e+59,
    1.60693804425899028e+60, 2.57110087081438444e+61,
    4.11376139330301511e+62, 6.58201822928482417e+63,
    1.05312291668557190e+65, 1.68499666669691499e+66,
    2.69599466671506398e+67, 4.31359146674410237e+68,
    6.90174634679056379e+69, 1.10427941548649021e+71,
    1.76684706477838433e+72, 2.82695530364541493e+73,
    4.52312848583266388e+74, 7.23700557733226210e+75,
};

#else

/*
 * full ibm range not supported
 */

#define IBM_FP_HI		1.70141183460469230e+38
#define IBM_FP_LO		4.70197740328915000e-38

static const double	ibm_exp[] =
{
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    IBM_FP_LO,               IBM_FP_LO,
    7.52316384526264005e-37, 1.20370621524202241e-35,
    1.92592994438723585e-34, 3.08148791101957736e-33,
    4.93038065763132378e-32, 7.88860905221011805e-31,
    1.26217744835361890e-29, 2.01948391736579022e-28,
    3.23117426778526435e-27, 5.16987882845642297e-26,
    8.27180612553027675e-25, 1.32348898008484430e-23,
    2.11758236813575085e-22, 3.38813178901720136e-21,
    5.42101086242752217e-20, 8.67361737988403547e-19,
    1.38777878078144568e-17, 2.22044604925031308e-16,
    3.55271367880050093e-15, 5.68434188608080149e-14,
    9.09494701772928238e-13, 1.45519152283668518e-11,
    2.32830643653869629e-10, 3.72529029846191406e-09,
    5.96046447753906250e-08, 9.53674316406250000e-07,
    1.52587890625000000e-05, 2.44140625000000000e-04,
    3.90625000000000000e-03, 6.25000000000000000e-02,
    1.00000000000000000e+00, 1.60000000000000000e+01,
    2.56000000000000000e+02, 4.09600000000000000e+03,
    6.55360000000000000e+04, 1.04857600000000000e+06,
    1.67772160000000000e+07, 2.68435456000000000e+08,
    4.29496729600000000e+09, 6.87194767360000000e+10,
    1.09951162777600000e+12, 1.75921860444160000e+13,
    2.81474976710656000e+14, 4.50359962737049600e+15,
    7.20575940379279360e+16, 1.15292150460684700e+18,
    1.84467440737095516e+19, 2.95147905179352826e+20,
    4.72236648286964521e+21, 7.55578637259143234e+22,
    1.20892581961462920e+24, 1.93428131138340668e+25,
    3.09485009821345069e+26, 4.95176015714152110e+27,
    7.92281625142643376e+28, 1.26765060022822940e+30,
    2.02824096036516704e+31, 3.24518553658426727e+32,
    5.19229685853482763e+33, 8.30767497365572421e+34,
    1.32922799578491587e+36, 2.12676479325586540e+37,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
    IBM_FP_HI,               IBM_FP_HI,
};

#endif

/*
 * ibm floating point
 */

static int
internal_ibm(Cdb_t* cdb, Cdbformat_t* fp, Cdbdata_t* dp, const char* b, size_t n, Cdbtype_t* type)
{
	register unsigned char*	s = (unsigned char*)b;
	register Cdbint_t	i;
	register double		f;

	switch (fp->width)
	{
	case 4:
		i = (s[1] << 16)
                  | (s[2] <<  8)
                  |  s[3];
		f = i * (1.0 / 0x1000000) * ((s[0] < 0x80) ? ibm_exp[s[0]] : -ibm_exp[s[0] & 0x7F]);
		dp->number.floating = f;
		SETFLOATING(dp, fp, 0);
		return 4;
	case 8:
		i = (s[1] << 16)
                  | (s[2] <<  8)
                  |  s[3];
		f = i * (1.0 / 0x1000000);
		i = (s[4] <<  8)
	          |  s[5];
		f += i * ((1.0 / 0x1000000) / (double)0x10000);
		i = (s[6] <<  8)
		  |  s[7];
		f += i * (((1.0 / 0x1000000) / (double)0x10000) / (double)0x10000);
		f *= (s[0] < 0x80) ? ibm_exp[s[0]] : -ibm_exp[s[0] & 0x7F];
		dp->number.floating = f;
		SETFLOATING(dp, fp, 0);
		return 8;
	}
	return -1;
}

static int
external_ibm(Cdb_t* cdb, Cdbformat_t* fp, Cdbdata_t* dp, char* b, size_t n, Cdbtype_t* type)
{
	register unsigned char*	s = (unsigned char*)b;
	register unsigned int	lo;
	register unsigned int	hi;
	register unsigned int	ex;
	register double		f;
	int			negative;
	double			hi3;
	double			md2;
	double			lo2;
	Cdbint_t		ihi3;
	Cdbint_t		imd2;
	Cdbint_t		ilo2;

	if (negative = (f = dp->number.floating) < 0.0)
		f = -f;
	switch (fp->width)
	{
	case 4:
		if (f < IBM_FP_LO)
			s[0] = s[1] = s[2] = s[3] = 0x00;
		else
		{
			if (f > IBM_FP_HI)
				f = IBM_FP_HI;

			/*
			 * find the closest exponent in ibm_exp[]
			 */

			lo = 0;
			hi = elementsof(ibm_exp) - 1;
			while (lo != hi + 1)
			{
				ex = lo + ((hi - lo) >> 1);
				if (ibm_exp[ex] < f)
					lo = ex + 1;
				else
					hi = ex - 1;
			}
			if (ibm_exp[ex] < f)
				ex++;

			/*
			 * scale by the exponent to reduce
			 * to the range 0.0625 -> 0.9999...
			 */

			f /= ibm_exp[ex];

			/*
			 * extract the fraction bits as integers
			 */

			f *= (double)0x1000000;
			hi3 = floor(f + 0.5);
			ihi3 = hi3;

			/*
			 * correct for overflow
			 */

			if (ihi3 > 0xFFFFFF)
			{
				ihi3 = 0x100000;
				ex++;
			}
			if (ex > 0x7F)
			{
				ex = 0x7F;
				ihi3 = 0xFFFFFF;
			}

			/*
			 * set the sign bit
			 */

			if (negative)
				ex |= 0x80;

			/*
			 * done
			 */

			s[0] = ex;
			s[1] = ihi3 >> 16;
			s[2] = (ihi3 >> 8) & 0xFF;
			s[3] = ihi3 & 0xFF;
		}
		return 4;
	case 8:
		if (f < IBM_FP_LO)
			s[0] = s[1] = s[2] = s[3] = s[4] = s[5] = s[6] = s[7] = 0x00;
		else
		{
			if (f > IBM_FP_HI)
				f = IBM_FP_HI;

			/*
			 * find the closest exponent in ibm_exp[]
			 */

			lo = 0;
			hi = elementsof(ibm_exp) - 1;
			while (lo != hi + 1)
			{
				ex = lo + ((hi - lo) >> 1);
				if (ibm_exp[ex] < f)
					lo = ex + 1;
				else
					hi = ex - 1;
			}
			if (ibm_exp[ex] < f)
				ex++;

			/*
			 * scale by the exponent to reduce
			 * to the range 0.0625 -> 0.9999...
			 */

			f /= ibm_exp[ex];

			/*
			 * extract the fraction bits as integers
			 */

			f *= (double)0x1000000;
			hi3 = floor(f);
			ihi3 = hi3;
			f -= hi3;
			f *= (double)0x10000;
			md2 = floor(f);
			imd2 = md2;
			f -= md2;
			f *= (double)0x10000;
			lo2 = floor(f + 0.5);
			ilo2 = lo2;

			/*
			 * correct for overflow
			 */

			if (ilo2 > 0xFFFF)
			{
				ilo2 -= 0x10000;
				imd2++;
			}
			if (imd2 > 0xFFFF)
			{
				imd2 -= 0x10000;
				ihi3++;
			}
			if (ihi3 > 0xFFFFFF)
			{
				ihi3 = 0x100000;
				ex++;
			}
			if (ex > 0x7F)
			{
				ex = 0x7F;
				ihi3 = 0xFFFFFF;
				imd2 = 0xFFFF;
				ilo2 = 0xFFFF;
			}

			/*
			 * set the sign bit
			 */

			if (negative)
				ex |= 0x80;

			/*
			 * done
			 */

			s[0] = ex;
			s[1] = ihi3 >> 16;
			s[2] = (ihi3 >> 8) & 0xFF;
			s[3] = ihi3 & 0xFF;
			s[4] = imd2 >> 8;
			s[5] = imd2 & 0xFF;
			s[6] = ilo2 >> 8;
			s[7] = ilo2 & 0xFF;
		}
		return 8;
	}
	return -1;
}

static int
validate_ibm(Cdb_t* cdb, register Cdbformat_t* fp, Cdbtype_t* type)
{
	switch (fp->type)
	{
	case CDB_FLOATING:
		switch (fp->width)
		{
		case 0:
			fp->width = 8;
		case 4:
		case 8:
			return 0;
		}
		break;
	}
	return -1;
}

/*
 * big endian binary
 */

static int
internal_be(Cdb_t* cdb, Cdbformat_t* fp, Cdbdata_t* dp, const char* b, size_t n, Cdbtype_t* type)
{
	register unsigned char*	s;
	register unsigned char*	e;
	register unsigned char*	t;
	register Cdbuint_t	v;
	register Sfulong_t	w;
	register int		c;

	switch (fp->ultype)
	{
	case CDB_FLOATING:
		if ((c = fp->width) != sizeof(double))
			return -1;
		if (n < c)
			return -(c + 1);
		s = (unsigned char*)b;
		e = s + (c = fp->width);
		t = (unsigned char*)&dp->number.floating;
		while (s < e)
			*t++ = *s++;
		SETFLOATING(dp, fp, 0);
		return c;
	case CDB_INTEGER:
	case CDB_UNSIGNED|CDB_INTEGER:
	case CDB_STRING:
		if (n < (c = fp->width))
			return -(c + 1);
		v = 0;
		s = (unsigned char*)b;
		e = s + c;
		while (s < e)
			v = (v << 8) + *s++;
		if (fp->type == CDB_INTEGER)
		{
			if (!(fp->flags & CDB_UNSIGNED) && c < sizeof(v) && (v & (1 << ((c << 3) - 1))))
				v |= ((1 << ((sizeof(v) - c) << 3)) - 1) << (c << 3);
			dp->number.integer = v;
			SETINTEGER(dp, fp, 0);
		}
		else if (!(dp->string.length = v))
			cdbempty(cdb, dp, fp, 1);
		else if ((c += v) > n)
			c = -(c + 1);
		else
		{
			if (!s[v - 1])
			{
				dp->string.length--;
				dp->flags = CDB_STRING|CDB_TERMINATED;
			}
			if (fp->code != CC_ASCII || !s[v - 1] && (cdb->flags & CDB_TERMINATED))
			{
				v = dp->string.length;
				if (!(e = vmoldof(cdb->record->vm, 0, unsigned char, v + 1, 0)))
					return -1;
				s = (unsigned char*)ccmapcpy((char*)e, (char*)s, v, CC_ASCII, fp->code);
				s[v] = 0;
				dp->flags = CDB_STRING|CDB_TERMINATED|CDB_CACHED;
			}
			dp->string.base = (char*)s;
		}
		return c;
	case CDB_LONG|CDB_INTEGER:
	case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
		if (n < (c = fp->width))
			return -(c + 1);
		w = 0;
		s = (unsigned char*)b;
		e = s + c;
		while (s < e)
			w = (w << 8) + *s++;
		if (!(fp->flags & CDB_UNSIGNED) && c < sizeof(w) && (w & (1 << ((c << 3) - 1))))
			w |= ((1 << ((sizeof(w) - c) << 3)) - 1) << (c << 3);
		if (fp->flags & CDB_UNSIGNED)
		{
			dp->number.winteger = w;
			SETWINTEGER(dp, fp, 0);
		}
		else
		{
			dp->number.linteger = w;
			SETLINTEGER(dp, fp, 0);
		}
		return c;
	}
	return -1;
}

static int
external_be(Cdb_t* cdb, Cdbformat_t* fp, Cdbdata_t* dp, char* b, size_t n, Cdbtype_t* type)
{
	register unsigned char*	s;
	register unsigned char*	e;
	register unsigned char*	t;
	register Cdbuint_t	v;
	register Sfulong_t	w;
	register int		c;

	switch (fp->ultype)
	{
	case CDB_FLOATING:
		if (n < (c = fp->width))
			return -(c + 1);
		s = (unsigned char*)b;
		e = s + c;
		t = (unsigned char*)&dp->number.floating;
		while (s < e)
			*s++ = *t++;
		return c;
	case CDB_INTEGER:
	case CDB_UNSIGNED|CDB_INTEGER:
		v = dp->number.uinteger;
		if (n < (c = fp->width))
			return -(c + 1);
		s = (unsigned char*)b;
		e = s + c;
		while (e > s)
		{
			*--e = v;
			v >>= 8;
		}
		return c;
	case CDB_LONG|CDB_INTEGER:
	case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
		w = dp->number.winteger;
		if (n < (c = fp->width))
			return -(c + 1);
		s = (unsigned char*)b;
		e = s + c;
		while (e > s)
		{
			*--e = w;
			w >>= 8;
		}
		return c;
	case CDB_STRING:
		if (n < (c = fp->width + (v = dp->string.length + 1)))
			return -(c + 1);
		s = (unsigned char*)b;
		e = s + fp->width;
		while (e > s)
		{
			*--e = v;
			v >>= 8;
		}
		s += fp->width;
		ccmapcpy((char*)s, dp->string.base, dp->string.length, fp->code, CC_ASCII);
		s[dp->string.length] = 0;
		return c;
	}
	return -1;
}

static int
validate_be(Cdb_t* cdb, register Cdbformat_t* fp, Cdbtype_t* type)
{
	switch (fp->ultype)
	{
	case CDB_FLOATING:
		if (!fp->width)
		{
			fp->width = sizeof(double);
			return 0;
		}
		if (fp->width <= sizeof(double))
			return 0;
		break;
	case CDB_INTEGER:
	case CDB_UNSIGNED|CDB_INTEGER:
		if (!fp->width)
		{
			fp->width = 4;
			return 0;
		}
		if (fp->width <= sizeof(Cdbint_t))
			return 0;
		break;
	case CDB_LONG|CDB_INTEGER:
	case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
		if (!fp->width)
		{
			fp->width = 8;
			return 0;
		}
		if (fp->width <= sizeof(Sflong_t))
			return 0;
		break;
	case CDB_STRING:
		if (!fp->width)
		{
			fp->width = 2;
			return 0;
		}
		if (fp->width <= sizeof(Cdbint_t))
			return 0;
		break;
	}
	return -1;
}

/*
 * little endian binary
 */

static int
internal_le(Cdb_t* cdb, Cdbformat_t* fp, Cdbdata_t* dp, const char* b, size_t n, Cdbtype_t* type)
{
	register unsigned char*	s;
	register unsigned char*	e;
	register unsigned char*	t;
	register Cdbuint_t	v;
	register Sfulong_t	w;
	register int		c;

	switch (fp->ultype)
	{
	case CDB_FLOATING:
		if ((c = fp->width) != sizeof(double))
			return -1;
		if (n < c)
			return -(c + 1);
		s = (unsigned char*)b;
		e = s + (c = fp->width);
		t = (unsigned char*)&dp->number.floating;
		while (s < e)
			*t++ = *s++;
		SETFLOATING(dp, fp, 0);
		return c;
	case CDB_INTEGER:
	case CDB_UNSIGNED|CDB_INTEGER:
	case CDB_STRING:
		if (n < (c = fp->width))
			return -(c + 1);
		v = 0;
		s = (unsigned char*)b;
		e = s + c;
		while (e > s)
			v = (v << 8) + *--e;
		s += c;
		if (fp->type == CDB_INTEGER)
		{
			if (!(fp->flags & CDB_UNSIGNED) && c < sizeof(w) && (v & (1 << ((c << 3) - 1))))
				v |= ((1 << ((sizeof(w) - c) << 3)) - 1) << (c << 3);
			dp->number.integer = v;
			SETINTEGER(dp, fp, 0);
		}
		else if (!(dp->string.length = v))
			cdbempty(cdb, dp, fp, 1);
		else if ((c += v) > n)
			c = -(c + 1);
		else
		{
			if (!s[v - 1])
			{
				dp->string.length--;
				dp->flags = CDB_STRING|CDB_TERMINATED;
			}
			if (fp->code != CC_ASCII || !s[v - 1] && (cdb->flags & CDB_TERMINATED))
			{
				v = dp->string.length;
				if (!(e = vmoldof(cdb->record->vm, 0, unsigned char, v + 1, 0)))
					return -1;
				s = (unsigned char*)ccmapcpy((char*)e, (char*)s, v, CC_ASCII, fp->code);
				s[v] = 0;
				dp->flags = CDB_STRING|CDB_TERMINATED|CDB_CACHED;
			}
			dp->string.base = (char*)s;
		}
		return c;
	case CDB_LONG|CDB_INTEGER:
	case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
		if (n < (c = fp->width))
			return -(c + 1);
		w = 0;
		s = (unsigned char*)b;
		e = s + c;
		while (e > s)
			w = (w << 8) + *--e;
		s += c;
		if (!(fp->flags & CDB_UNSIGNED) && c < sizeof(w) && (w & (1 << ((c << 3) - 1))))
			w |= ((1 << ((sizeof(w) - c) << 3)) - 1) << (c << 3);
		if (fp->flags & CDB_UNSIGNED)
		{
			dp->number.winteger = w;
			SETWINTEGER(dp, fp, 0);
		}
		else
		{
			dp->number.linteger = w;
			SETLINTEGER(dp, fp, 0);
		}
		return c;
	}
	return -1;
}

static int
external_le(Cdb_t* cdb, Cdbformat_t* fp, Cdbdata_t* dp, char* b, size_t n, Cdbtype_t* type)
{
	register unsigned char*	s;
	register unsigned char*	e;
	register unsigned char*	t;
	register Cdbuint_t	v;
	register Sfulong_t	w;
	register int		c;

	switch (fp->ultype)
	{
	case CDB_FLOATING:
		if (n < (c = fp->width))
			return -(c + 1);
		s = (unsigned char*)b;
		e = s + c;
		t = (unsigned char*)&dp->number.floating;
		while (s < e)
			*s++ = *t++;
		return c;
	case CDB_INTEGER:
	case CDB_UNSIGNED|CDB_INTEGER:
		v = dp->number.uinteger;
		if (n < (c = fp->width))
			return -(c + 1);
		s = (unsigned char*)b;
		e = s + c;
		while (s < e)
		{
			*s++ = v;
			v >>= 8;
		}
		return c;
	case CDB_LONG|CDB_INTEGER:
	case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
		w = dp->number.winteger;
		if (n < (c = fp->width))
			return -(c + 1);
		s = (unsigned char*)b;
		e = s + c;
		while (s < e)
		{
			*s++ = w;
			w >>= 8;
		}
		return c;
	case CDB_STRING:
		if (n < (c = fp->width + (v = dp->string.length + 1)))
			return -(c + 1);
		s = (unsigned char*)b;
		e = s + fp->width;
		while (s < e)
		{
			*s++ = v;
			v >>= 8;
		}
		ccmapcpy((char*)s, dp->string.base, dp->string.length, fp->code, CC_ASCII);
		s[dp->string.length] = 0;
		return c;
	}
	return -1;
}

#define validate_le	validate_be

/*
 * sfio int,float,string
 */

static int
internal_sf(Cdb_t* cdb, Cdbformat_t* fp, Cdbdata_t* dp, register const char* b, size_t n, Cdbtype_t* type)
{
	register unsigned char*	s;
	register unsigned char*	e;
	register Sfulong_t	u;
	register Sflong_t	v;
	register int		c;

	switch (fp->type)
	{
	case CDB_FLOATING:
		sfstrtmp(cdb->buf, SF_READ, (void*)b, n);
		dp->number.floating = sfgetd(cdb->buf);
		if (sferror(cdb->buf))
			return -(n + 2);
		SETFLOATING(dp, fp, 0);
		return sfstrtell(cdb->buf);
	case CDB_INTEGER:
	case CDB_LONG|CDB_INTEGER:
		s = (unsigned char*)b;
		e = s + n;
		switch ((int)((v = *s++) & (SF_MORE|SF_SIGN)))
		{
		case 0:
			break;
		case SF_SIGN:
			v = -(v & (SF_SIGN - 1)) - 1;
			break;
		default:
			v &= (SF_MORE - 1);
			for (;;)
			{
				if (s >= e)
					return -(n + 2);
				if ((c = *s++) & SF_MORE)
					v = (v << SF_UBITS) | (c & (SF_MORE - 1));
				else
				{
					v = (v << SF_SBITS) | (c & (SF_SIGN - 1));
					if (c & SF_SIGN)
						v = -v - 1;
					break;
				}
			}
			break;
		}
		if (fp->flags & CDB_LONG)
		{
			dp->number.linteger = v;
			SETLINTEGER(dp, fp, 0);
		}
		else
		{
			dp->number.integer = v;
			SETINTEGER(dp, fp, 0);
		}
		return s - (unsigned char*)b;
	case CDB_UNSIGNED|CDB_INTEGER:
	case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
		s = (unsigned char*)b;
		e = s + n;
		u = 0;
		do
		{
			if (s >= e)
				return -(n + 2);
			c = *s++;
			u = (u << SF_UBITS) | (c & (SF_MORE - 1));
		} while (c & SF_MORE);
		if (fp->flags & CDB_LONG)
		{
			dp->number.winteger = u;
			SETWINTEGER(dp, fp, 0);
		}
		else
		{
			dp->number.uinteger = u;
			SETUINTEGER(dp, fp, 0);
		}
		return s - (unsigned char*)b;
	case CDB_STRING:
		v = 0;
		s = (unsigned char*)b;
		e = s + n;
		do
		{
			if (s >= e)
				return -(n + 2);
			c = *s++;
			v = (v << SF_UBITS) | (c & (SF_MORE - 1));
		} while (c & SF_MORE);
		c = s - (unsigned char*)b;
		if (!(dp->string.length = v))
			cdbempty(cdb, dp, fp, 1);
		else if ((c += v) > n)
			c = -(c + 1);
		else
		{
			dp->string.length--;
			dp->flags = CDB_STRING|CDB_TERMINATED;
			if (fp->code != CC_ASCII)
			{
				if (!(s = (unsigned char*)vmstrdup(cdb->record->vm, (char*)s)))
					return -1;
				ccmaps((char*)s, dp->string.length, CC_ASCII, fp->code);
			}
			dp->string.base = (char*)s;
		}
		return c;
	}
	return -1;
}

static int
external_sf(Cdb_t* cdb, Cdbformat_t* fp, Cdbdata_t* dp, char* b, size_t n, Cdbtype_t* type)
{
	register unsigned char*	s;
	register unsigned char*	t;
	register int		c;
	register Sflong_t	v;
	register Sfulong_t	u;
	unsigned char		tmp[4 * sizeof(u)];

	switch (fp->type)
	{
	case CDB_FLOATING:
		sfstrtmp(cdb->buf, SF_WRITE, b, n);
		if (sfputd(cdb->buf, dp->number.floating) < 0)
			return -(n + 2);
		return sfstrtell(cdb->buf);
	case CDB_INTEGER:
		v = dp->number.integer;
		goto encode_integer;
	case CDB_LONG|CDB_INTEGER:
		v = dp->number.linteger;
	encode_integer:
		s = t = &tmp[elementsof(tmp) - 1];
		if (v < 0)
		{
			v = -(v + 1);
			*t = (unsigned char)((v & (SF_MORE - 1)) | SF_SIGN);
		}
		else
			*t = (unsigned char)(v & (SF_SIGN - 1));
		v >>= SF_SBITS;
		while (v > 0)
		{
			*--t = (unsigned char)(v | SF_MORE);
			v >>= SF_UBITS;
		}
		if ((c = s - t + 1) > n)
			return -(c + 1);
		s = (unsigned char*)b;
		switch (c)
		{
		default:memcpy(s, t, c); break;
		case 7:	*s++ = *t++;
		case 6:	*s++ = *t++;
		case 5:	*s++ = *t++;
		case 4:	*s++ = *t++;
		case 3:	*s++ = *t++;
		case 2:	*s++ = *t++;
		case 1:	*s++ = *t++;
		}
		return c;
	case CDB_UNSIGNED|CDB_INTEGER:
		u = dp->number.integer;
		goto encode_uinteger;
	case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
		u = dp->number.linteger;
	encode_uinteger:
		s = t = &tmp[elementsof(tmp) - 1];
		*t = (unsigned char)(u);
		while ((u >>= SF_UBITS) > 0)
			*--t = (unsigned char)(u | SF_MORE);
		if ((c = s - t + 1) > n)
			return -(c + 1);
		s = (unsigned char*)b;
		switch (c)
		{
		default:memcpy(s, t, c); break;
		case 7:	*s++ = *t++;
		case 6:	*s++ = *t++;
		case 5:	*s++ = *t++;
		case 4:	*s++ = *t++;
		case 3:	*s++ = *t++;
		case 2:	*s++ = *t++;
		case 1:	*s++ = *t++;
		}
		return c;
	case CDB_STRING:
		u = dp->string.length + 1;
		s = t = &tmp[elementsof(tmp) - 1];
		*t = (unsigned char)(u);
		while ((u >>= SF_UBITS) > 0)
			*--t = (unsigned char)(u | SF_MORE);
		if ((c = s - t + 1) > n)
			return -(c + 1);
		s = (unsigned char*)b;
		switch (c)
		{
		default:memcpy(s, t, c); break;
		case 7:	*s++ = *t++;
		case 6:	*s++ = *t++;
		case 5:	*s++ = *t++;
		case 4:	*s++ = *t++;
		case 3:	*s++ = *t++;
		case 2:	*s++ = *t++;
		case 1:	*s++ = *t++;
		}
		if (dp->string.length)
		{
			if ((c += dp->string.length + 1) > n)
				return -(c + 1);
			ccmapcpy(s, dp->string.base, dp->string.length, fp->code, CC_ASCII);
			s[dp->string.length] = 0;
		}
		return c;
	}
	return -1;
}

static int
validate_sf(Cdb_t* cdb, register Cdbformat_t* fp, Cdbtype_t* type)
{
	return fp->width ? -1 : 0;
}

/*
 * date
 */

static int
internal_date(Cdb_t* cdb, Cdbformat_t* fp, Cdbdata_t* dp, const char* b, size_t n, Cdbtype_t* type)
{
	char*	e;
	char*	f;

	sfwrite(cdb->tmp, b, n);
	dp->number.integer = tmscan(sfstruse(cdb->tmp), &e, fp->details ? fp->details : "", &f, NiL, 0L);
	SETINTEGER(dp, fp, *e != 0 || *f != 0);
	return n;
}

static int
external_date(Cdb_t* cdb, Cdbformat_t* fp, Cdbdata_t* dp, char* b, size_t n, Cdbtype_t* type)
{
	time_t		v = dp->number.integer;

	return tmfmt(b, n, fp->details ? fp->details : "%K", &v) - b;
}

static int
validate_date(Cdb_t* cdb, register Cdbformat_t* fp, Cdbtype_t* type)
{
	return 0;
}

/*
 * hash/rand -- hashed/random field -- handy for obfuscating sensitive data
 */

typedef struct
{
	Cdbuint_t	hash;
	int		rand;
} Hash_t;

static const char	lower_hash[] = "abcdefghijklmnopqrstuvwxyz";
static const char	upper_hash[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char	digit_hash[] = "0123456789";

extern time_t		time(time_t*);

static int
init_hash(Cdb_t* cdb, Cdbtype_t* type)
{
	register const char*	s;
	register int		n;
	register Cdbuint_t	h;
	register Hash_t*	hp;

	if (!(hp = newof(0, Hash_t, 1, 0)))
		return -1;
	h = 0;
	for (s = type->name; n = *s++;)
		HASHPART(h, n);
	h ^= (Cdbuint_t)time(NiL) ^ (((Cdbuint_t)getpid()) << (h & 077));
	for (n = ((h >> 7) & 077) | 0100; n > 0; n--)
		HASHPART(h, n);
	hp->hash = h;
	hp->rand = type->name[0] == 'r';
	type->data = (void*)hp;
	return 0;
}

static int
internal_hash(Cdb_t* cdb, Cdbformat_t* fp, Cdbdata_t* dp, const char* b, size_t n, Cdbtype_t* type)
{
	Hash_t*			hp = (Hash_t*)type->data;
	register unsigned char*	s;
	register unsigned char*	e;
	register unsigned char*	t;
	register int		c;
	register Cdbuint_t	h;

	if (!(t = vmoldof(cdb->record->vm, 0, unsigned char, n + 1, 0)))
		return -1;
	dp->string.base = (char*)t;
	dp->string.length = n;
	dp->flags = CDB_STRING|CDB_TERMINATED|CDB_CACHED;
	s = (unsigned char*)b;
	e = s + n;
	h = hp->hash;
	while (s < e)
	{
		c = *s++;
		HASHPART(h, c);
		if (islower(c))
			c = lower_hash[h % (sizeof(lower_hash) - 1)];
		else if (isupper(c))
			c = upper_hash[h % (sizeof(upper_hash) - 1)];
		else if (c != '+' && c != '-' && c != '_' && c != '.')
			c = digit_hash[h % (sizeof(digit_hash) - 1)];
		*t++ = c;
	}
	*t = 0;
	if (hp->rand)
		hp->hash = h;
	return n;
}

static int
external_hash(Cdb_t* cdb, Cdbformat_t* fp, Cdbdata_t* dp, char* b, size_t n, Cdbtype_t* type)
{
	Hash_t*			hp = (Hash_t*)type->data;
	register unsigned char*	s;
	register unsigned char*	e;
	register unsigned char*	t;
	register int		c;
	register Cdbuint_t	h;

	if (n < dp->string.length)
		return -(dp->string.length + 1);
	s = (unsigned char*)dp->string.base;
	e = s + dp->string.length;
	t = (unsigned char*)b;
	h = hp->hash;
	while (s < e)
	{
		c = *s++;
		HASHPART(h, c);
		if (islower(c))
			c = lower_hash[h % (sizeof(lower_hash) - 1)];
		else if (isupper(c))
			c = upper_hash[h % (sizeof(upper_hash) - 1)];
		else if (c != '+' && c != '-' && c != '_' && c != '.')
			c = digit_hash[h % (sizeof(digit_hash) - 1)];
		*t++ = c;
	}
	if (hp->rand)
		hp->hash = h;
	return dp->string.length;
}

static int
validate_hash(Cdb_t* cdb, register Cdbformat_t* fp, Cdbtype_t* type)
{
	return 0;
}

/*
 * base 100 integers
 */

static const unsigned char heka_unpack[UCHAR_MAX+1] =
{
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,
   0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
  10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
  20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
  30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
  40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
  50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
  60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
  70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
  80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
  90,  0, 91, 92,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
   0,  0,  0,  0,  0, 
   0,  0,  0, 93, 94, 95, 96, 97, 98, 99
};

static const unsigned char heka_unpack1[UCHAR_MAX+1] =
{
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',
   '0',  '0',  '0',
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',
   '1',  '1',  '1',  '1',  '1',  '1',  '1',  '1',  '1',  '1',
   '2',  '2',  '2',  '2',  '2',  '2',  '2',  '2',  '2',  '2',
   '3',  '3',  '3',  '3',  '3',  '3',  '3',  '3',  '3',  '3',
   '4',  '4',  '4',  '4',  '4',  '4',  '4',  '4',  '4',  '4',
   '5',  '5',  '5',  '5',  '5',  '5',  '5',  '5',  '5',  '5',
   '6',  '6',  '6',  '6',  '6',  '6',  '6',  '6',  '6',  '6',
   '7',  '7',  '7',  '7',  '7',  '7',  '7',  '7',  '7',  '7',
   '8',  '8',  '8',  '8',  '8',  '8',  '8',  '8',  '8',  '8',
   '9',  '0',  '9',  '9',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '9',  '9',  '9',  '9',  '9',  '9',  '9'
};

static const unsigned char heka_unpack2[UCHAR_MAX+1] =
{
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',
   '0',  '0',  '0',
   '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',
   '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',
   '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',
   '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',
   '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',
   '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',
   '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',
   '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',
   '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',
   '0',  '0',  '1',  '2',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '0',  '0', 
   '0',  '0',  '0',  '3',  '4',  '5',  '6',  '7',  '8',  '9'
};

static int
internal_heka(Cdb_t* cdb, Cdbformat_t* fp, Cdbdata_t* dp, const char* b, size_t n, Cdbtype_t* type)
{
	register unsigned char*	s = (unsigned char*)b;
	register unsigned char*	e = s + n;
	register Cdbuint_t	u;
	register Sfulong_t	w;
	register char*		t;
	register int		c;
	int			neg;

	neg = 0;
	switch (fp->ultype)
	{
	case CDB_INTEGER:
		if (*s == '-')
		{
			s++;
			neg = 1;
		}
		else if (*s == '+')
			s++;
		/*FALLTHROUGH*/
	case CDB_UNSIGNED|CDB_INTEGER:
		u = 0;
		while (s < e)
			u = u * 100 + heka_unpack[*s++];
		if (fp->ultype & CDB_UNSIGNED)
		{
			dp->number.uinteger = u;
			SETUINTEGER(dp, fp, 0);
		}
		else
		{
			dp->number.integer = u;
			if (neg)
				dp->number.integer = -dp->number.integer;
			SETINTEGER(dp, fp, 0);
		}
		break;
	case CDB_LONG|CDB_INTEGER:
		if (*s == '-')
		{
			s++;
			neg = 1;
		}
		else if (*s == '+')
			s++;
		/*FALLTHROUGH*/
	case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
		w = 0;
		while (s < e)
			w = w * 100 + heka_unpack[*s++];
		if (fp->ultype & CDB_UNSIGNED)
		{
			dp->number.winteger = w;
			SETUINTEGER(dp, fp, 0);
		}
		else
		{
			dp->number.linteger = w;
			if (neg)
				dp->number.linteger = -dp->number.linteger;
			SETLINTEGER(dp, fp, 0);
		}
		break;
	case CDB_STRING:
		c = (n + 2) * 2;
		if (!(t = vmnewof(cdb->record->vm, 0, char, c, 0)))
			return -1;
		dp->string.base = t;
		if (*s == '-')
		{
			s++;
			*t++ = '-';
		}
		else if (*s == '+')
			s++;
		while (s < e)
		{
			c = *s++;
			*t++ = heka_unpack1[c];
			*t++ = heka_unpack2[c];
		}
		*t = 0;
		dp->string.length = t - dp->string.base;
		dp->flags |= CDB_STRING|CDB_CACHED|CDB_TERMINATED;
		break;
	}
	return n;
}

static const unsigned char heka_pack[100] =
{
      33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
      43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
      53, 54, 55, 56, 57, 58, 59, 60, 61, 62,
      63, 64, 65, 66, 67, 68, 69, 70, 71, 72,
      73, 74, 75, 76, 77, 78, 79, 80, 81, 82,
      83, 84, 85, 86, 87, 88, 89, 90, 91, 92,
      93, 94, 95, 96, 97, 98, 99,100,101,102,
     103,104,105,106,107,108,109,110,111,112,
     113,114,115,116,117,118,119,120,121,122,
     123,125,126,241,242,243,244,245,246,247,
};   

static int
external_heka(Cdb_t* cdb, Cdbformat_t* fp, Cdbdata_t* dp, char* b, size_t n, Cdbtype_t* type)
{
	register unsigned char*	s;
	register unsigned char*	t;
	register unsigned char*	e;
	register int		c;
	register Sflong_t	v;
	register Sfulong_t	u;
	int			neg;
	unsigned char		tmp[128];

	switch (fp->ultype)
	{
	case CDB_INTEGER:
		v = dp->number.integer;
		goto encode_integer;
	case CDB_LONG|CDB_INTEGER:
		v = dp->number.linteger;
	encode_integer:
		s = t = &tmp[elementsof(tmp) - 1];
		if (v == 0)
		{
			*--t = heka_pack[0];
			neg = 0;
		}
		else
		{
			if (neg = v < 0)
				v = -v;
			while (v > 0)
			{
				*--t = heka_pack[v % 100];
				v /= 100;
			}
		}
		if ((c = fp->width) > 0 && c < (elementsof(tmp) - 1))
		{
			e = s - c + 1;
			c = heka_pack[0];
			while (t > e)
				*--t = c;
		}
		*--t = neg ? '-' : '+';
		if ((c = s - t) > n)
			return -(c + 1);
		s = (unsigned char*)b;
		switch (c)
		{
		default:memcpy(s, t, c); break;
		case 7:	*s++ = *t++;
		case 6:	*s++ = *t++;
		case 5:	*s++ = *t++;
		case 4:	*s++ = *t++;
		case 3:	*s++ = *t++;
		case 2:	*s++ = *t++;
		case 1:	*s++ = *t++;
		}
		return c;
	case CDB_UNSIGNED|CDB_INTEGER:
		u = dp->number.integer;
		goto encode_uinteger;
	case CDB_UNSIGNED|CDB_LONG|CDB_INTEGER:
		u = dp->number.linteger;
	encode_uinteger:
		s = t = &tmp[elementsof(tmp) - 1];
		if (u == 0)
			*--t = heka_pack[0];
		else
			while (u > 0)
			{
				*--t = heka_pack[u % 100];
				u /= 100;
			}
		if ((c = fp->width) > 0 && c < elementsof(tmp))
		{
			e = s - c;
			c = heka_pack[0];
			while (t > e)
				*--t = c;
		}
		if ((c = s - t) > n)
			return -(c + 1);
		s = (unsigned char*)b;
		switch (c)
		{
		default:memcpy(s, t, c); break;
		case 7:	*s++ = *t++;
		case 6:	*s++ = *t++;
		case 5:	*s++ = *t++;
		case 4:	*s++ = *t++;
		case 3:	*s++ = *t++;
		case 2:	*s++ = *t++;
		case 1:	*s++ = *t++;
		}
		return c;
	}
	return -1;
}

static int
validate_heka(Cdb_t* cdb, register Cdbformat_t* fp, Cdbtype_t* type)
{
	return 0;
}

/*
 * binary key string with format
 */

static int
internal_key(Cdb_t* cdb, Cdbformat_t* fp, Cdbdata_t* dp, const char* b, size_t n, Cdbtype_t* type)
{
#if 0
	const char*	e;
	char*		f;

	e = b + n;
	if (f = fp->details)
	{
	}
	else
	{
	}
#endif
	return 0;
}

static int
external_key(Cdb_t* cdb, Cdbformat_t* fp, Cdbdata_t* dp, char* b, size_t n, Cdbtype_t* type)
{
	return 0;
}

static int
validate_key(Cdb_t* cdb, register Cdbformat_t* fp, Cdbtype_t* type)
{
	return fp->width ? 0 : -1;
}

/*
 * builtin external type list
 */

Cdbtype_t	cdbexternal[] =
{
	{
		"bcd",
		"binary coded decimal",
		CDB_INTEGER|CDB_STRING|CDB_BINARY,
		0,
		internal_bcd,
		external_bcd,
		validate_bcd,
		0,
		&cdbexternal[1]
	},
	{
		"be",
		"big endian binary",
		CDB_FLOATING|CDB_INTEGER|CDB_STRING|CDB_BINARY,
		0,
		internal_be,
		external_be,
		validate_be,
		0,
		&cdbexternal[2]
	},
	{
		"date",
		"date string <=> time_t",
		CDB_INTEGER|CDB_STRING,
		0,
		internal_date,
		external_date,
		validate_date,
		0,
		&cdbexternal[3]
	},
	{
		"hash",
		"hashed obfuscation",
		CDB_STRING,
		init_hash,
		internal_hash,
		external_hash,
		validate_hash,
		0,
		&cdbexternal[4]
	},
	{
		"heka",
		"base 100 integers",
		CDB_INTEGER|CDB_STRING,
		0,
		internal_heka,
		external_heka,
		validate_heka,
		0,
		&cdbexternal[5]
	},
	{
		"ibm",
		"ibm 2 and 4 byte floating point",
		CDB_FLOATING|CDB_BINARY,
		0,
		internal_ibm,
		external_ibm,
		validate_ibm,
		0,
		&cdbexternal[6]
	},
	{
		"key",
		"binary key string",
		CDB_STRING,
		0,
		internal_key,
		external_key,
		validate_key,
		0,
		&cdbexternal[7]
	},
	{
		"le",
		"big endian binary",
		CDB_FLOATING|CDB_INTEGER|CDB_STRING|CDB_BINARY,
		0,
		internal_le,
		external_le,
		validate_le,
		0,
		&cdbexternal[8]
	},
	{
		"rand",
		"randomized obfuscation",
		CDB_STRING,
		init_hash,
		internal_hash,
		external_hash,
		validate_hash,
		0,
		&cdbexternal[9]
	},
	{
		"sf",
		"sfputu/sfputl encoding",
		CDB_FLOATING|CDB_INTEGER|CDB_STRING|CDB_BINARY,
		0,
		internal_sf,
		external_sf,
		validate_sf,
		0,
		0
	},
};
