/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*           Copyright (c) 2005-2006 AT&T Knowledge Ventures            *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped

/*
 * sort file io vcodex discipline
 */

static const char usage[] =
"[-1lp0?\n@(#)$Id: sortvcodex (AT&T Research) 2005-11-09 $\n]"
USAGE_LICENSE
"[+NAME?sortvcodex - sort io vcodex discipline library]"
"[+DESCRIPTION?The \bsortvcodex\b \bsort\b(1) discipline encodes and/or "
    "decodes input, output and temporary file data. By default temporary and "
    "output encoding is the same as the encoding used on the first encoded "
    "input file. Output encoding is only applied to the standard output or "
    "to files with a path suffix containing 'z'.]"
"[i:input?Decode the input files using \amethod\a. \b--noinput\b "
    "disables input encoding.]:[method]"
"[o:output?Encode the output file using \amethod\a. \b--nooutput\b "
    "disables output encoding.]:[method]"
"[r:regress?Massage \bverbose\b output for regression testing.]"
"[t:temporary?Encode temporary intermediate files using "
    "\amethod\a. \b--notemporary\b disables temporary encoding.]:[method]"
"[T:test?Enable test code defined by \amask\a. Test code is "
    "implementation specific. Consult the source for details.]#[mask]"
"[v:verbose?Enable file and stream encoding messages on the standard "
    "error.]"
"[+SEE ALSO?\bsort\b(1), \bvczip\b(1), \bvcodex\b(3)]"
;

#include <ast.h>
#include <error.h>
#include <recsort.h>
#include <vcsfio.h>

struct Delay_s;
typedef struct Delay_s Delay_t;

struct Delay_s
{
	Delay_t*	next;
	Sfio_t*		sp;
	char		name[1];
};

typedef struct Encoding_s
{
	Vcsfmeth_t*	list;
	ssize_t		size;
} Encoding_t;

typedef struct State_s
{
	Rsdisc_t	disc;
	Encoding_t	input;
	Encoding_t	output;
	Encoding_t	temporary;
	Delay_t*	delay;
	unsigned long	test;
	int		regress;
	int		verbose;
} State_t;

#define tempid(s,f)	((s)->regress?(++(s)->regress):sffileno((Sfio_t*)(f)))

static int
zipit(const char* path)
{
	char*	s;

	return !path || (s = strrchr(path, '.')) && strchr(s, 'z') && !strchr(s, '/');
}

static int
encode(State_t* state, Sfio_t* sp, const char* path)
{
	if (!vcsfmeth(sp, NiL))
	{
		if (!state->output.size)
			state->output = state->input;
		if (sfdcvcodex(sp, state->output.list, state->output.size, VC_ENCODE) < 0)
		{
			error(2, "%s: cannot push vcodex encode discipline", path);
			return -1;
		}
		if (state->verbose)
			error(0, "sort vcodex encode %s", path);
	}
	return 0;
}

static int
vcodex(Rs_t* rs, int op, Void_t* data, Void_t* arg, Rsdisc_t* disc)
{
	int		i;
	Delay_t*	delay;
	State_t*	state = (State_t*)disc;

	if (state->test & 0x10)
		error(0, "sort vcodex event %s %p %s"
			, op == RS_FILE_WRITE ? "RS_FILE_WRITE"
			: op == RS_FILE_READ ? "RS_FILE_READ"
			: op == RS_TEMP_WRITE ? "RS_TEMP_WRITE"
			: op == RS_TEMP_READ ? "RS_TEMP_READ"
			: "UNKNOWN"
			, data
			, arg);
	switch (op)
	{
	case RS_FILE_WRITE:
		if (((Sfio_t*)data == sfstdout || zipit(arg)) && (state->output.size > 0 || !state->output.size && state->input.size > 0))
			return encode(state, (Sfio_t*)data, (char*)arg);
		if (!state->output.size && zipit(arg) && (arg || (arg = (Void_t*)"(output-stream)")) && (delay = newof(0, Delay_t, 1, strlen(arg))))
		{
			delay->sp = (Sfio_t*)data;
			strcpy(delay->name, arg);
			delay->next = state->delay;
			state->delay = delay;
		}
		break;
	case RS_FILE_READ:
		if (state->input.size >= 0)
		{
			if ((i = sfdcvcodex((Sfio_t*)data, state->input.list, state->input.size, VC_DECODE)) < 0)
			{
				error(2, "%s: cannot push vcodex decode discipline", arg);
				return -1;
			}
			else if (i > 0)
			{
				if (state->verbose)
					error(0, "sort vcodex decode %s", arg);
				if (!state->input.size)
				{
					if (!(state->input.list = vcsfmeth((Sfio_t*)data, &state->input.size)))
					{
						error(2, "%s: cannot query vcodex decode discipline", arg);
						return -1;
					}
					if (!(state->input.list = vcsfdup(state->input.list, state->input.size)))
					{
						error(2, "%s: cannot dup vcodex decode discipline", arg);
						return -1;
					}
					if (state->delay)
					{
						i = 0;
						while (delay = state->delay)
						{
							if (!i && state->input.size > 0 && !sfseek(delay->sp, (Sfoff_t)0, SEEK_CUR))
								i = encode(state, delay->sp, delay->name);
							state->delay = delay->next;
							free(state->delay);
						}
						return i;
					}
				}
			}
		}
		break;
	case RS_TEMP_WRITE:
		if (state->temporary.size > 0 || !state->temporary.size && state->input.size > 0)
		{
			if (!state->temporary.size)
				state->temporary = state->input;
			if (sfdcvcodex((Sfio_t*)data, state->temporary.list, state->temporary.size, VC_ENCODE) < 0)
			{
				error(2, "temporary-%d: cannot push vcodex encode discipline", tempid(state, data));
				return -1;
			}
			if (state->verbose)
				error(0, "sort vcodex encode temporary-%d", tempid(state, data));
			return 1;
		}
		break;
	case RS_TEMP_READ:
		if (state->temporary.size > 0 || !state->temporary.size && state->input.size > 0)
		{
			if (!state->temporary.size)
				state->temporary = state->input;
			if (!sfdisc((Sfio_t*)data, SF_POPDISC) || sfseek((Sfio_t*)data, (Sfoff_t)0, SEEK_SET))
			{
				error(2, "temporary-%d: cannot rewind temporary data", tempid(state, data));
				return -1;
			}
			if ((i = sfdcvcodex((Sfio_t*)data, NiL, 0, VC_DECODE)) < 0)
			{
				error(2, "temporary-%d: cannot push vcodex decode discipline", tempid(state, data));
				return -1;
			}
			else if (i > 0 && state->verbose)
				error(0, "sort vcodex decode temporary-%d", tempid(state, data));
			return 1;
		}
		break;
	default:
		return -1;
	}
	return 0;
}

Rsdisc_t*
rs_disc(Rskey_t* key, const char* options)
{
	State_t*	state;

	if (!(state = newof(0, State_t, 1, 0)))
		error(ERROR_SYSTEM|3, "out of space");
	if (options)
	{
		for (;;)
		{
			switch (optstr(options, usage))
			{
			case 0:
				break;
			case 'i':
				if (!opt_info.arg)
					state->input.size = -1;
				else if (streq(opt_info.arg, "-"))
					state->input.size = 0;
				else if (!(state->input.list = vcsfcomp(opt_info.arg, &state->input.size, NiL, errorf)))
					goto drop;
				continue;
			case 'o':
				if (!opt_info.arg)
					state->output.size = -1;
				else if (streq(opt_info.arg, "-"))
					state->output.size = 0;
				else if (!(state->output.list = vcsfcomp(opt_info.arg, &state->output.size, NiL, errorf)))
					goto drop;
				continue;
			case 'r':
				state->regress = 1;
				continue;
			case 't':
				if (!opt_info.arg)
					state->temporary.size = -1;
				else if (streq(opt_info.arg, "-"))
					state->temporary.size = 0;
				else if (!(state->temporary.list = vcsfcomp(opt_info.arg, &state->temporary.size, NiL, errorf)))
					goto drop;
				continue;
			case 'v':
				state->verbose = 1;
				continue;
			case 'T':
				state->test |= opt_info.num;
				continue;
			case '?':
				error(ERROR_USAGE|4, "%s", opt_info.arg);
				goto drop;
			case ':':
				error(2, "%s", opt_info.arg);
				goto drop;
			}
			break;
		}
	}
	if (state->temporary.size >= 0)
		key->type |= RS_TEXT;
	state->disc.eventf = vcodex;
	state->disc.events = RS_FILE_WRITE|RS_FILE_READ|RS_TEMP_WRITE|RS_TEMP_READ;
	return &state->disc;
 drop:
	free(state);
	return 0;
}