#pragma prototyped

/*
 * rar decoder snarfed from the GPL unrarlib.c
 * this code is also GPL
 */

#include <codex.h>

#define NC		298
#define DC		48
#define RC		28
#define BC		19
#define MC		257

#define MAXWINSIZE	0x100000
#define MAXWINMASK	(MAXWINSIZE-1)

#define CODE_HUFFMAN	0
#define CODE_LZ		1
#define CODE_LZ2	2
#define CODE_REPEATLZ	3
#define CODE_CACHELZ	4
#define CODE_STARTFILE	5
#define CODE_ENDFILE	6
#define CODE_STARTMM	8
#define CODE_ENDMM	7
#define CODE_MMDELTA	9

typedef unsigned _ast_int1_t ui1;
typedef unsigned _ast_int2_t ui2;
typedef unsigned _ast_int4_t ui4;

typedef struct AudioVariables_s
{
	int			K1,K2,K3,K4,K5;
	int			D1,D2,D3,D4;
	int			LastDelta;
	unsigned int		Dif[11];
	unsigned int		ByteCount;
	int			LastChar;
} AudioVariables_t;

typedef struct LitDecode_s
{
	unsigned int		MaxNum;
	unsigned int		DecodeLen[16];
	unsigned int		DecodePos[16];
	unsigned int		DecodeNum[NC];
} LitDecode_t;

typedef struct DistDecode_s
{
	unsigned int		MaxNum;
	unsigned int		DecodeLen[16];
	unsigned int		DecodePos[16];
	unsigned int		DecodeNum[DC];
} DistDecode_t;

typedef struct RepDecode_s
{
	unsigned int		MaxNum;
	unsigned int		DecodeLen[16];
	unsigned int		DecodePos[16];
	unsigned int		DecodeNum[RC];
} RepDecode_t;

typedef struct MultDecode_s
{
	unsigned int		MaxNum;
	unsigned int		DecodeLen[16];
	unsigned int		DecodePos[16];
	unsigned int		DecodeNum[MC];
} MultDecode_t;

typedef struct BitDecode_s
{
	unsigned int		MaxNum;
	unsigned int		DecodeLen[16];
	unsigned int		DecodePos[16];
	unsigned int		DecodeNum[BC];
} BitDecode_t;

typedef struct Decode_s
{
	unsigned int		MaxNum;
	unsigned int		DecodeLen[16];
	unsigned int		DecodePos[16];
	unsigned int		DecodeNum[2];
} Decode_t;

typedef struct Io_s
{
	char*			bp;
	char*			be;
} Io_t;

typedef struct Pos_s
{
	ui4			offset;
	ui4			size;
} Pos_t;

typedef struct State_s
{
	Codex_t*		codex;

	int			bad;
	int			bitcount;
	int			bof;
	int			eof;
	int			eofbitcount;
	int			flush;

	Pos_t			flushpos[2];

	ui4			bitbuf;

	unsigned char*		ip;
	unsigned char*		ie;
	unsigned char		buf[8192];

	int			UnpAudioBlock;
	int			UnpChannels;
	int			CurChannel;
	int			ChannelDelta;
	unsigned int		Distance;
	unsigned int		InBit;
	unsigned int		LastDist;
	unsigned int		LastLength;
	unsigned int		OldDistPtr;
	unsigned int		OldDist[4];
	unsigned int		UnpPtr;
	unsigned int		WrPtr;
	unsigned char		UnpBuf[MAXWINSIZE];
	unsigned char		UnpOldTable[MC*4];
	AudioVariables_t	AudV[4];
	LitDecode_t		LD;
	DistDecode_t		DD;
	RepDecode_t		RD;
	MultDecode_t		MD[4];
	BitDecode_t		BD;
} State_t;

#define GETCHAR(p)	((p)->ip < (p)->ie ? (int)*(p)->ip++ : fill(p))

static int
fill(State_t* state)
{
	ssize_t	r;

	if ((r = sfrd(state->codex->sp, state->buf, sizeof(state->buf), &state->codex->sfdisc)) <= 0)
		return -1;
	state->ie = (state->ip = state->buf) + r;
	return *state->ip++;
}

#define GETBITS(p,n)	((n)<=(p)->bitcount ? (int)((((p)->bitbuf)>>((p)->bitcount-=(n)))&((1L<<(n))-1)) : getbits(p,n,0))
#define PEEKBITS(p,n)	((n)<=(p)->bitcount ? (int)((((p)->bitbuf)>>((p)->bitcount-(n)))&((1L<<(n))-1)) : getbits(p,n,1))

static int
getbits(register State_t* state, register int n, int peek)
{
	register int	c;

	if (state->eof)
	{
		if (!peek)
		{
			if (n >= state->eof)
				return -1;
			state->eof -= n;
		}
		state->bitcount = state->eofbitcount;
	}
	while (n > state->bitcount)
	{
		if ((c = GETCHAR(state)) < 0)
		{
			if (!peek)
				return -1;
			if (!state->eof)
				state->eof = state->bitcount + 1;
			c = 0;
		}
		state->bitbuf <<= 8;
		state->bitbuf |= c;
		state->bitcount += 8;
	}
	c = (state->bitbuf>>(state->bitcount-n))&((1L<<(n))-1);
	if (!peek)
		state->bitcount -= n;
	if (state->eof)
	{
		state->eofbitcount = state->bitcount;
		state->bitcount = 0;
	}
	return c;
}

static int
getcode(register State_t* state, Decode_t *dp)
{
	register unsigned int	n;
	register unsigned int	i;

	if (!(n = PEEKBITS(state, 16) & 0xfffe) && state->eof && state->codex->size < 0)
		return -1;
	if (n < dp->DecodeLen[8])
	{
		if (n < dp->DecodeLen[4])
		{
			if (n < dp->DecodeLen[2])
			{
				if (n < dp->DecodeLen[1])
					i = 1;
				else
					i = 2;
			}
			else
			{
				if (n < dp->DecodeLen[3])
					i = 3;
				else
					i = 4;
			}
		}
		else
		{
			if (n < dp->DecodeLen[6])
			{
				if (n < dp->DecodeLen[5])
					i = 5;
				else
					i = 6;
			}
			else
			{
				if (n < dp->DecodeLen[7])
					i = 7;
				else
					i = 8;
			}
		}
	}
	else
	{
		if (n < dp->DecodeLen[12])
		{
			if (n < dp->DecodeLen[10])
			{
				if (n < dp->DecodeLen[9])
					i = 9;
				else
					i = 10;
			}
			else
			{
				if (n < dp->DecodeLen[11])
					i = 11;
				else
					i = 12;
			}
		}
		else
		{
			if (n < dp->DecodeLen[14])
			{
				if (n < dp->DecodeLen[13])
					i = 13;
				else
					i = 14;

			}
			else
					i = 15;
		}
	}
	if (GETBITS(state, i) < 0)
		return -1;
	if ((n = dp->DecodePos[i] + ((n - dp->DecodeLen[i-1])>>(16-i))) >= dp->MaxNum)
		n = 0;
	return dp->DecodeNum[n];
}

static ui4 LDecode[] =
{
	0x00000000, 0x00000001, 0x00000002, 0x00000003,
	0x00000004, 0x00000005, 0x00000006, 0x00000007,
	0x00000008, 0x0000000a, 0x0000000c, 0x0000000e,
	0x00000010, 0x00000014, 0x00000018, 0x0000001c,
	0x00000020, 0x00000028, 0x00000030, 0x00000038,
	0x00000040, 0x00000050, 0x00000060, 0x00000070,
	0x00000080, 0x000000a0, 0x000000c0, 0x000000e0,
};

static ui1 LBits[] =
{
	0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  2,  2,
	2,  2,  3,  3,  3,  3,  4,  4,  4,  4,  5,  5,  5,  5
};

static ui4 DDecode[] =
{
	0x00000000, 0x00000001, 0x00000002, 0x00000003,
	0x00000004, 0x00000006, 0x00000008, 0x0000000c,
	0x00000010, 0x00000018, 0x00000020, 0x00000030,
	0x00000040, 0x00000060, 0x00000080, 0x000000c0,
	0x00000100, 0x00000180, 0x00000200, 0x00000300,
	0x00000400, 0x00000600, 0x00000800, 0x00000c00,
	0x00001000, 0x00001800, 0x00002000, 0x00003000,
	0x00004000, 0x00006000, 0x00008000, 0x0000c000,
	0x00010000, 0x00018000, 0x00020000, 0x00030000,
	0x00040000, 0x00050000, 0x00060000, 0x00070000,
	0x00080000, 0x00090000, 0x000a0000, 0x000b0000,
	0x000c0000, 0x000d0000, 0x000e0000, 0x000f0000,
};

static ui1 DBits[] =
{
	 0,  0,  0,  0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,
	 7,  7,  8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14,
	15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16
};

static ui4 SDDecode[] =
{
	0x00000000, 0x00000004, 0x00000008, 0x00000010,
	0x00000020, 0x00000040, 0x00000080, 0x000000c0,
};

static ui1 SDBits[] =
{
	 2,  2,  3,  4,  5,  6,  6,  6
};

static void
maketable(register State_t* state, unsigned char* LenTab, Decode_t* Dec, int Size)
{
	int	i;
	long	m;
	long	n;
	int	LenCount[16];
	int	TmpPos[16];

	memset(LenCount, 0, sizeof(LenCount));
	for (i = 0; i < Size; i++)
		LenCount[LenTab[i] & 0xF]++;
	LenCount[0] = 0;
	for (TmpPos[0] = Dec->DecodePos[0] = Dec->DecodeLen[0] = 0, n = 0, i = 1; i < 16; i++)
	{
		n = 2 * (n + LenCount[i]);
		m = n<<(15-i);
		if (m > 0xFFFF)
			m=0xFFFF;
		Dec->DecodeLen[i] = (unsigned int)m;
		TmpPos[i] = Dec->DecodePos[i] = Dec->DecodePos[i-1] + LenCount[i-1];
	}
	for (i = 0; i < Size; i++)
		if (LenTab[i])
			Dec->DecodeNum[TmpPos[LenTab[i] & 0xF]++] = i;
	Dec->MaxNum = Size;
}

static ui1
getaudio(register State_t* state, int delta)
{
	register AudioVariables_t*	v;
	int				c;
	int				i;
	unsigned int			n;
	unsigned int			m;

	v = &state->AudV[state->CurChannel];
	v->ByteCount++;
	v->D4 = v->D3;
	v->D3 = v->D2;
	v->D2 = v->LastDelta-v->D1;
	v->D1 = v->LastDelta;
	i = 8 * v->LastChar + v->K1 * v->D1 + v->K2 * v->D2 + v->K3 * v->D3 + v->K4 * v->D4 + v->K5 * state->ChannelDelta;
	i = (i>>3) & 0xFF;
	c = i - delta;
	i = ((signed char)delta)<<3;
	v->Dif[0] += abs(i);
	v->Dif[1] += abs(i - v->D1);
	v->Dif[2] += abs(i + v->D1);
	v->Dif[3] += abs(i - v->D2);
	v->Dif[4] += abs(i + v->D2);
	v->Dif[5] += abs(i - v->D3);
	v->Dif[6] += abs(i + v->D3);
	v->Dif[7] += abs(i - v->D4);
	v->Dif[8] += abs(i + v->D4);
	v->Dif[9] += abs(i - state->ChannelDelta);
	v->Dif[10] += abs(i + state->ChannelDelta);
	state->ChannelDelta = v->LastDelta = (signed char)(c - v->LastChar);
	v->LastChar = c;
	if (!(v->ByteCount & 0x1F))
	{
		m = v->Dif[0];
		n = 0;
		v->Dif[0] = 0;
		for (i = 1; (unsigned int)i < sizeof(v->Dif) / sizeof(v->Dif[0]); i++)
		{
			if (v->Dif[i] < m)
			{
				m = v->Dif[i];
				n = i;
			}
			v->Dif[i] = 0;
		}
		switch (n)
		{
		case 1:
			if (v->K1 >= -16)
				v->K1--;
			break;
		case 2:
			if (v->K1 < 16)
				v->K1++;
			break;
		case 3:
			if (v->K2 >= -16)
				v->K2--;
			break;
		case 4:
			if (v->K2 < 16)
				v->K2++;
			break;
		case 5:
			if (v->K3 >= -16)
				v->K3--;
			break;
		case 6:
			if (v->K3 < 16)
				v->K3++;
			break;
		case 7:
			if (v->K4 >= -16)
				v->K4--;
			break;
		case 8:
			if (v->K4 < 16)
				v->K4++;
			break;
		case 9:
			if (v->K5 >= -16)
				v->K5--;
			break;
		case 10:
			if (v->K5 < 16)
				v->K5++;
			break;
		}
	}
	return c;
}

static int
gettables(register State_t* state)
{
	ui1		BitLength[BC];
	unsigned char	Table[MC*4];
	int		TableSize;
	int		n;
	int		i;
	int		c;

	if ((c = GETBITS(state, 1)) < 0)
		goto bad;
	state->UnpAudioBlock = c;
	if ((c = GETBITS(state, 1)) < 0)
		goto bad;
	if (!c)
		memset(state->UnpOldTable, 0, sizeof(state->UnpOldTable));
	if (state->UnpAudioBlock)
	{
		if ((c = GETBITS(state, 2)) < 0)
			goto bad;
		state->UnpChannels = c + 1;
		if (state->CurChannel >= state->UnpChannels)
			state->CurChannel = 0;
		TableSize = MC * state->UnpChannels;
	}
	else
		TableSize = NC + DC + RC;
	for (i = 0; i < BC; i++)
	{
		if ((c = GETBITS(state, 4)) < 0)
			goto bad;
		BitLength[i] = c;
	}
	maketable(state, BitLength, (Decode_t*)&state->BD, BC);
	i = 0;
	while (i < TableSize)
	{
		if ((c = getcode(state, (Decode_t*)&state->BD)) < 0)
			goto bad;
		if (c < 16)
		{
			Table[i] = (c + state->UnpOldTable[i]) & 0xf;
			i++;
		}
		else if (c == 16)
		{
			if ((c = GETBITS(state, 2)) < 0)
				goto bad;
			n = c + 3;
			while (n-- > 0 && i < TableSize)
			{
				Table[i] = Table[i-1];
				i++;
			}
		}
		else
		{
			if (c == 17)
			{
				if ((c = GETBITS(state, 3)) < 0)
					goto bad;
				n = c + 3;
			}
			else
			{
				if ((c = GETBITS(state, 7)) < 0)
					goto bad;
				n = c + 11;
			}
			while (n-- > 0 && i < TableSize)
				Table[i++] = 0;
		}
	}
	if (state->UnpAudioBlock)
		for (i = 0; i < state->UnpChannels; i++)
			maketable(state, &Table[i*MC], (Decode_t*)&state->MD[i], MC);
	else
	{
		maketable(state, &Table[0], (Decode_t*)&state->LD, NC);
		maketable(state, &Table[NC], (Decode_t*)&state->DD, DC);
		maketable(state, &Table[NC+DC], (Decode_t*)&state->RD, RC);
	}
	memcpy(state->UnpOldTable, Table, sizeof(state->UnpOldTable));
	return 0;
 bad:
	if (state->codex->disc->errorf)
		(*state->codex->disc->errorf)(NiL, state->codex->disc, 2, "%s: corrupt compression table", state->codex->meth->name);
	return -1;
}

static int
rar_open(Codex_t* p, char* const args[], Codexnum_t flags)
{
	register State_t*	state;
	char*			e;
	int			method;

	if (args[2])
	{
		method = (int)strtol(args[2], &e, 0);
		if (*e)
			method = 0;
	}
	else
		method = 0;
	if (method <= 0 || method >= 256)
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "%s: method version expected", args[0]);
		return -1;
	}
	if (method != 20)
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "%s: unknown method version", args[0]);
		return -1;
	}
	if (args[3])
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "%s: %s: unknown method option", args[1], args[3]);
		return -1;
	}
	if (!(state = newof(0, State_t, 1, 0)))
	{
		if (p->disc->errorf)
			(*p->disc->errorf)(NiL, p->disc, 2, "out of space");
		return -1;
	}
	state->codex = p;
	p->data = state;
	return 0;
}

static int
rar_init(Codex_t* p)
{
	register State_t*	state = (State_t*)p->data;

	state->ip = state->ie = 0;
	state->bitcount = 0;
	state->eof = 0;
	state->flush = 0;
	if (!(p->flags & CODEX_RETAIN))
	{
		state->ChannelDelta = state->CurChannel = 0;
		memset(state->AudV, 0, sizeof(state->AudV));
		memset(state->OldDist, 0, sizeof(state->OldDist));
		state->OldDistPtr = 0;
		state->LastDist = state->LastLength = 0;
		memset(state->UnpBuf, 0, sizeof(state->UnpBuf));
		memset(state->UnpOldTable, 0, sizeof(state->UnpOldTable));
		state->UnpPtr = state->WrPtr = 0;
		if (gettables(state))
			return -1;
	}
	return 0;
}

static int
flush(register State_t* state, register Io_t* io)
{
	register Pos_t*	pos;
	size_t		n;

	if (!state->flush)
	{
		if (state->UnpPtr < state->WrPtr)
		{
			if (n = state->UnpPtr)
			{
				state->flushpos[state->flush].offset = 0;
				state->flushpos[state->flush].size = n;
				state->flush++;
			}
			n = (0 - state->WrPtr) & MAXWINMASK;
		}
		else
			n = state->UnpPtr - state->WrPtr;
		if (n)
		{
			state->flushpos[state->flush].offset = state->WrPtr;
			state->flushpos[state->flush].size = n;
			state->flush++;
		}
		state->WrPtr = state->UnpPtr;
	}
	while (state->flush)
	{
		pos = &state->flushpos[state->flush - 1];
		if ((n = pos->size) > (io->be - io->bp))
			n = io->be - io->bp;
		memcpy(io->bp, state->UnpBuf + pos->offset, n);
		if (pos->size -= n)
			pos->offset += n;
		else
			state->flush--;
		if ((io->bp += n) >= io->be)
			return 1;
	}
	return 0;
}

static ssize_t
rar_read(Sfio_t* sp, void* buf, size_t n, Sfdisc_t* disc)
{
	register State_t*	state = (State_t*)CODEX(disc)->data;
	register int		b;
	register int		c;
	register int		z;
	Io_t			io;

	io.be = (io.bp = buf) + n;
	if (state->flush && flush(state, &io))
		return io.bp - (char*)buf;
	for (;;)
	{
		if (((state->WrPtr - (state->UnpPtr &= MAXWINMASK)) & MAXWINMASK) < 270 && flush(state, &io))
			return io.bp - (char*)buf;
		else if (state->UnpAudioBlock)
		{
			if ((c = getcode(state, (Decode_t*)&state->MD[state->CurChannel])) < 0)
				break;
			else if (c == 256)
			{
				if (gettables(state))
					break;
			}
			else
			{
				if (state->codex->size >= 0)
				{
					if (!state->codex->size)
						break;
					state->codex->size--;
				}
				state->UnpBuf[state->UnpPtr++] = getaudio(state, c);
				if (++state->CurChannel == state->UnpChannels)
					state->CurChannel = 0;
			}
		}
		else if ((c = getcode(state, (Decode_t*)&state->LD)) < 0)
			break;
		else if (c < 256)
		{
			if (state->codex->size >= 0)
			{
				if (!state->codex->size)
					break;
				state->codex->size--;
			}
			state->UnpBuf[state->UnpPtr++] = c;
		}
		else if (c == 269)
		{
			if (gettables(state))
				break;
		}
		else
		{
			if (c > 269)
			{
				z = LDecode[c-=270] + 3;
				if ((b = LBits[c]) > 0)
				{
					if ((c = GETBITS(state, b)) < 0)
						break;
					z += c;
				}
				if ((c = getcode(state, (Decode_t*)&state->DD)) < 0)
					break;
				state->Distance = DDecode[c] + 1;
				if ((b = DBits[c]) > 0)
				{
					if ((c = GETBITS(state, b)) < 0)
						break;
					state->Distance += c;
				}
				if (state->Distance >= 0x40000L)
					z++;
				if (state->Distance >= 0x2000)
					z++;
			}
			else if (c == 256)
			{
				state->Distance = state->LastDist;
				z = state->LastLength;
			}
			else if (c < 261)
			{
				state->Distance = state->OldDist[(state->OldDistPtr-(c-256)) & 3];
				if ((c = getcode(state, (Decode_t*)&state->RD)) < 0)
					break;
				z = LDecode[c] + 2;
				if ((b = LBits[c]) > 0)
				{
					if ((c = GETBITS(state, b)) < 0)
						break;
					z += c;
				}
				if (state->Distance >= 0x40000L)
					z++;
				if (state->Distance >= 0x2000)
					z++;
				if (state->Distance >= 0x101)
					z++;
			}
			else
			{
				state->Distance = SDDecode[c-=261] + 1;
				if ((b = SDBits[c]) > 0)
				{
					if ((c = GETBITS(state, b)) < 0)
						break;
					state->Distance += c;
				}
				z = 2;
			}
			if (state->codex->size >= 0)
			{
				if (state->codex->size < z)
					break;
				state->codex->size -= z;
			}
			state->LastDist = state->OldDist[state->OldDistPtr++ & 3] = state->Distance;
			state->LastLength = z;
			while (z--)
			{
				state->UnpBuf[state->UnpPtr] = state->UnpBuf[(state->UnpPtr - state->Distance) & MAXWINMASK];
				state->UnpPtr = (state->UnpPtr + 1) & MAXWINMASK;
			}
		}
	}
	flush(state, &io);
	return io.bp - (char*)buf;
}

Codexmeth_t	codex_rar =
{
	"rar",
	"\brar\b(1) 2.0 archive compression.",
	"[+(version)?codex-rar 2003-12-23]"
	"[+(author)?Eugene Roshal]"
	"[+(copyright)?Copyright (c) 1999-2003 Eugene Roshal]"
	"[+(license)?GPL]",
	CODEX_DECODE|CODEX_COMPRESS,
	0,
	0,
	rar_open,
	0,
	rar_init,
	0,
	rar_read,
	0,
	0,
	0,
	0,
	0,
	0,
	CODEXNEXT(codex_rar_next)
};

CODEXLIB(&codex_rar)
