/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1984-2004 AT&T Corp.                *
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
 * compiler and loader for the architecture independent makefile object format
 *
 * The format fields are labeled by sequence number and type:
 *
 *	# sfputu()/sfgetu()
 *	$ size,string (no trailing 0)
 *	@ 0 terminated string
 *
 * The format is designed for per-section backward/forward compatible
 * additions to the header, rules, variables and trailer sections, with
 * the proviso that the section order is not changed and that sequence 1
 * fields must appear in all future formats.  This means that the
 * property, dynamic and status field bit values are permanently fixed
 * by each sequence.  Field addition semantics are controlled by the
 * sequence number.
 *
 * header:
 *
 *	1 4 magic	must match
 *	1 @ ident	identification string (information only)
 *	1 # size	header size
 *	1 # sequence	to label additions/changes
 *	1 # flags	OBJ_* flags
 *	1 # strings	string table size
 *	1 # lists	number of lists
 *	1 # rules	number of rules
 *	1 # rulenum	(RULENUM-MINRULENUM)
 *	1 # rulestr	(RULESTR-MINRULESTR)
 *	1 # variables	number of variables
 *	1 # varnum	(VARNUM-MINVARNUM)
 *	1 # varstr	(VARSTR-MINVARSTR)
 *	1 4 magic	again for verification
 *	* # ...		[header number fields additions here]
 *
 * optional headers:
 *
 *	1 # size	header size
 *	1 # type	header type
 *	* * ...		header contents
 *
 * HEADER_PREREQS:
 *
 *	1 # type	{0:end COMP_*:type}
 *	1 # time	time
 *	1 @ name	unbound name
 *	    ...
 *
 * variables:
 *
 *	1 # property
 *	* # ...		[variable number field additions here]
 *	1 $ name	name string
 *	1 $ value	value string
 *	* $ ...		[variable string field additions here]
 *
 * rules:
 *
 *	1 # property
 *	1 # dynamic
 *	1 # attribute
 *	1 # encoded	status|semaphore|view|scan
 *	1 # prereqs	prereq list index
 *	1 # time	rule time
 *	* # ...		[rule number field additions here]
 *	1 $ name	name string
 *	1 $ action	action string
 *	1 $ data	event time or state string
 *	* $ ...		[rule string field additions here]
 *
 * lists:
 *
 *	1 # rule	rule index
 *
 * trailer:
 *
 *	1 @ options	option string
 *	* @ ...		[trailer string fields additions here]
 *
 * NOTE: the old format compatibility code should probably be dropped in 95
 * NOTE: HA -- as of 08/11/97 2.1 was still in production use
 */

#include "make.h"

#include <ccode.h>

/*
 * old rule load() replacement puns on struct rule
 *
 * rule.mark is not used as load() may be triggered
 * by staterule() while marks are in use
 */

#define getoldrule(r)	((struct rule*)r->action)
#define isoldrule(r)	(r->status==OLDRULE)
#define setoldrule(r,o)	(r->status=OLDRULE,r->action=(char*)o,r->prereqs=(struct list*)oldrules,oldrules=r)

#define MAGIC		"\015\001\013\005"
#define MAGICSIZE	(sizeof(MAGIC)-1)

#define SEQUENCE	1		/* track semantic diffs		*/
#define HEADERSIZE	128		/* handle largest header	*/

#define HEADER_PREREQS	1		/* prereqs optional header	*/

#define MINRULENUM	6		/* min # rule number fields	*/
#define MINRULESTR	3		/* min # rule string fields	*/
#define MINVARNUM	1		/* min # variable number fields	*/
#define MINVARSTR	2		/* min # variable string fields	*/

#define RULENUM		(MINRULENUM+0)	/* # rule number fields		*/
#define RULESTR		(MINRULESTR+0)	/* # rule string fields		*/
#define VARNUM		(MINVARNUM+0)	/* # variable number fields	*/
#define VARSTR		(MINVARSTR+0)	/* # variable string fields	*/

struct compstate			/* compile state		*/
{
	char*		sp;		/* string table pointer		*/
	Sfio_t*		fp;		/* object file pointer		*/
	unsigned long	lists;		/* list index			*/
	unsigned long	rules;		/* rule index			*/
	unsigned long	strings;	/* string index			*/
	unsigned long	variables;	/* variable index		*/
};

struct loadstate			/* load state			*/
{
	char*		sp;		/* string table pointer		*/
};

static struct				/* object global state		*/
{
	Sfio_t*		pp;		/* prerequisite ref pointer	*/
	unsigned char*	a2n;		/* CC_ASCII=>CC_NATIVE		*/
	unsigned char*	n2a;		/* CC_NATIVE=>CC_ASCII		*/
	unsigned long	garbage;	/* state garbage count		*/
	unsigned long	rules;		/* state rule count		*/
	int		initialized;	/* state initialized		*/
} object;

/*
 * old object format compatibility
 * frozen 12/25/92
 */

#define OLD_MAGIC	0x0d010b05
#define OLD_OLD_MAGIC	0x0000ff5d
#define OLD_VERSION	"AT&T Bell Laboratories 08/11/89"
#define OLD_VERSION_2	"AT&T Bell Laboratories 01/24/89"
#define OLD_SEQUENCE	2
#define OLD_ALIGN	8

#define old_data	u2.u_data
#define old_event	u2.u_event

struct OLD_header			/* old make object file header	*/
{
	long		magic;		/* magic number			*/
	char		version[32];	/* compiler/loader version	*/
	unsigned char	null;		/* 0 byte for long version's	*/
	unsigned char	sequence;	/* different still compatible	*/
	unsigned char	sizes[10];	/* misc size checks		*/
};

struct OLD_trailer			/* old make object file trailer	*/
{
	long		magic;		/* magic number			*/
	char*		options;	/* options for set()		*/
	long		lists;		/* number of compiled lists	*/
	long		rules;		/* number of compiled rules	*/
	long		size;		/* total sizeof object file	*/
	long		variables;	/* number of compiled variables	*/
};

struct OLD_rule				/* old rule			*/
{
	char*		name;		/* rule name			*/

	union
	{
	struct frame*	u_active;	/* active target frame		*/
	unsigned long	u_complink;	/* compilation link		*/
	struct rule*	u_freelink;	/* free list link		*/
	}		u1;

	union
	{
	char*		u_uname;	/* unbound name			*/
	char*		u_data;		/* state value			*/
	unsigned long	u_event;	/* state rule event time	*/
	}		u2;

	struct OLD_list* prereqs;	/* prerequisites		*/
	char*		action;		/* update action		*/
	unsigned long	time;		/* modify time			*/

	long		attribute;	/* external named attributes	*/
	long		dynamic;	/* dynamic properties		*/
	long		property;	/* stable properties		*/

#define noswap		scan		/* 0 or char elts after here	*/

	unsigned char	scan;		/* file scan strategy index	*/
	unsigned char	semaphore;	/* semaphore + count		*/
	unsigned char	status;		/* disposition			*/
	unsigned char	view;		/* view bind index		*/

#if BINDINDEX
	unsigned char	source;		/* source bind index		*/
#else
	unsigned char	spare_1;	/* spare			*/
#endif
	unsigned char	preview;	/* min prereq view		*/

	unsigned short	must;		/* cancel if == 0		*/

	char*		runtime;	/* run time info		*/
};

struct OLD_var				/* old variable			*/
{
	char*		name;		/* name				*/
	char*		value;		/* value			*/
	long		property;	/* static and dynamic		*/
	long		length;		/* maximum length of value	*/
};

struct OLD_list				/* old rule cons cell		*/
{
	struct OLD_list* next;		/* next in list			*/
	struct OLD_rule* rule;		/* list item			*/
};

static struct OLD_header old_stamp =	/* old object header is fixed	*/
{
	OLD_MAGIC,
	OLD_VERSION,
	0,
	OLD_SEQUENCE,
	OLD_ALIGN,
	CHAR_BIT,
	sizeof(char),
	sizeof(short),
	sizeof(long),
	sizeof(char*),
	sizeof(struct OLD_list),
	sizeof(struct OLD_rule),
	sizeof(struct OLD_var),
	0,
};

/*
 * initialize the object ccode tables
 */

void
initcode(void)
{
	if (!object.initialized)
	{
		object.initialized = 1;
		object.a2n = ccmap(CC_ASCII, CC_NATIVE);
		object.n2a = ccmap(CC_NATIVE, CC_ASCII);
	}
}

/*
 * read canonical 0 terminated string from object file
 */

static char*
getstring(Sfio_t* sp)
{
	char*	s;

	if (s = sfgetr(sp, 0, 0))
		ccmapstr(object.a2n, s, sfvalue(sp));
	return s;
}

/*
 * write canonical 0 terminated string to file
 */

static void
putstring(register Sfio_t* sp, register const char* s, int sep)
{
	register int		c;
	register unsigned char*	map;

	if (map = object.n2a)
	{
		while (c = *(unsigned char*)s++)
			sfputc(sp, map[c]);
		if (sep >= 0)
			sfputc(sp, map[sep]);
	}
	else
		sfputr(sp, s, sep);
}

/*
 * recursively mark r and its prerequisites for compilation
 */

static void
markcompile(register struct rule* r)
{
	register struct list*	p;

	r->dynamic &= ~D_compiled;
	r->mark |= M_compile;
	for (p = r->prereqs; p; p = p->next)
		if (!(p->rule->mark & M_compile))
			markcompile(p->rule);
}

/*
 * mark state file garbage candidates
 */

static void
markgarbage(register struct rule* r, int garbage)
{
	register struct list*	p;
	register int		i;
	struct rule*		x;

	r->mark |= M_compile;
	if (garbage)
		r->dynamic |= D_garbage;
	else
		r->dynamic &= ~D_garbage;
	for (p = r->prereqs; p; p = p->next)
		if (!(p->rule->mark & M_compile))
			markgarbage(p->rule, garbage);
	for (i = RULE; i <= STATERULES; i++)
		if ((x = staterule(i, r, NiL, 0)) && !(x->mark & M_compile))
			markgarbage(x, garbage);
}

/*
 * compile a string
 */

static void
compstring(register struct compstate* cs, register char* s)
{
	register int		c;
	register unsigned char*	map;

	if (s)
	{
		c = strlen(s) + 1;
		cs->strings += c;
		sfputu(cs->fp, c);
		if (map = object.n2a)
			while (c = *s++)
				sfputc(cs->fp, map[c]);
		else
			sfwrite(cs->fp, s, c - 1);
	}
	else
		sfputu(cs->fp, 0);
}

/*
 * initialize rules for compilation
 * if h!=0 then all rules marked compiled
 */

static int
compinit(const char* s, char* v, void* h)
{
	register struct rule*	r = (struct rule*)v;

	NoP(s);
	r->complink = 0;
	r->mark &= ~M_compile;
	if (r->dynamic & D_alias)
		mergestate(makerule(r->name), r);
	if (h || (r->property & P_internal))
		r->dynamic |= D_compiled;
	return 0;
}

/*
 * mark selected rules and immediate prereqs for compilation
 *
 * NOTE: remember to clear r->mark (from markcompile())
 */

static int
compselect(const char* s, char* v, void* h)
{
	register struct rule*	r = (struct rule*)v;
	register char*		select = (char*)h;

	NoP(s);
	if (!(r->mark & M_compile) && (r->dynamic & D_compiled))
	{
		state.frame->target = r;
		expand(internal.met, select);
		if (sfstrtell(internal.met))
		{
			sfstrset(internal.met, 0);
			markcompile(r);
		}
	}
	return 0;
}

/*
 * mark rules so that only state vars and immediate prereqs will be compiled
 *
 * NOTE: remember to clear r->mark (from markgarbage())
 */

static int
compstate(const char* s, char* v, void* h)
{
	register struct rule*		r = (struct rule*)v;

	NoP(s);
	NoP(h);
	r->dynamic |= D_compiled;
	if (r->dynamic & D_garbage)
	{
		if (!object.garbage)
			r->dynamic &= ~D_garbage;
		else if (!(r->mark & M_compile))
			markgarbage(r, 1);
	}
	return 0;
}

/*
 * mark prerequisites for compilation
 *
 * NOTE: remember to clear r->mark (from markcompile())
 */

static int
compmark(const char* s, char* v, void* h)
{
	register struct rule*	r = (struct rule*)v;
	register struct list*	p;

	NoP(s);
	NoP(h);
	r->complink = 0;
	if (state.stateview == 0)
	{
		if ((r->property & P_state) && !r->view && !(r->dynamic & (D_garbage|D_lower)))
		{
			r->dynamic &= ~D_compiled;
			for (p = r->prereqs; p; p = p->next)
				p->rule->dynamic &= ~D_compiled;
		}
	}
	else if (!(r->dynamic & D_compiled) && !(r->mark & M_compile))
		markcompile(r);
	return 0;
}

/*
 * weed out the real garbage
 */

static int
compkeep(const char* s, char* v, void* h)
{
	register struct rule*	r = (struct rule*)v;

	NoP(s);
	NoP(h);
	if (!(r->mark & M_compile) && !(r->dynamic & D_garbage))
		markgarbage(r, 0);
	return 0;
}

/*
 * compile an individual rule
 */

static int
comprule(const char* s, char* v, void* h)
{
	register struct rule*		r = (struct rule*)v;
	register struct list*		p;
	register struct compstate*	cs = (struct compstate*)h;
	struct rule			x;

	NoP(s);

	/*
	 * compile each rule only once
	 */

	if (r->dynamic & D_compiled)
		return 0;
	r->dynamic |= D_compiled;
	r->mark |= M_compile;
#if DEBUG
	if (r->property & P_internal)
		error(1, "internal rule %s should not be compiled", r->name);
#endif

	/*
	 * set the current rule index for prerequisite list compilation
	 */

	r->complink = cs->rules++;
	x = *r;
	r = &x;

	/*
	 * make sure the unbound rule name is compiled
	 */

	if (!(r->property & P_state))
	{
		if (state.stateview == 0)
		{
			r->prereqs = 0;
			r->action = 0;
			r->dynamic &= ~D_compiled;
		}
		if (r->uname && !(r->property & P_metarule))
		{
			r->name = r->uname;
			r->uname = 0;
		}
#if BINDINDEX
		r->view = 0;
#endif
	}
#if !BINDINDEX
	r->view = 0;
#endif

	/*
	 * compile the fields
	 */

	sfputu(cs->fp, r->property);
	sfputu(cs->fp, r->dynamic & ~D_CLEAROBJECT);
	sfputu(cs->fp, r->attribute);
	sfputu(cs->fp, (r->semaphore<<16)|(r->view<<8)|(r->scan));
	if (p = r->prereqs)
	{
		sfputu(cs->fp, cs->lists);
		do cs->lists++; while (p = p->next);
	}
	else
		sfputu(cs->fp, 0);
	sfputu(cs->fp, r->time);
	compstring(cs, r->name);
	compstring(cs, r->action);
	if (r->property & P_staterule)
		sfputu(cs->fp, r->event);
	else
		compstring(cs, r->statedata);
	return 0;
}

/*
 * compile the prerequisite list for r
 */

static int
complist(const char* s, char* v, void* h)
{
	register struct rule*		r = (struct rule*)v;
	register struct list*		p;
	register struct compstate*	cs = (struct compstate*)h;
	register struct rule*		a;

	/*
	 * ignore aliases and rules not set up by comprule()
	 */

	if (!r->complink || !(r->mark & M_compile) || s != r->name && !(r->dynamic & D_alias) || state.stateview == 0 && !(r->property & P_state))
		return 0;
	r->mark &= ~M_compile;
	if (p = r->prereqs)
	{
		do
		{
			for (r = p->rule; !r->complink || !(r->dynamic & D_compiled); r = a)
				if (!(a = getrule(r->name)) || a == r)
					error(PANIC, "prerequisite %s of %s not compiled", r->name, s);
			sfputu(cs->fp, r->complink);

			/*
			 * list.next is always sequential on load
			 */

		} while (p = p->next);
		sfputu(cs->fp, 0);
	}
	return 0;
}

/*
 * compile an individual variable
 */

static int
compvar(const char* s, char* u, void* h)
{
	register struct var*		v = (struct var*)u;
	register struct compstate*	cs = (struct compstate*)h;
	char*				t;
	unsigned long			property;
	char*				value;

	/*
	 * compile each variable only once
	 * don't compile command arg variable definitions
	 */

	if ((v->property & V_compiled) || state.stateview < 0 && !(v->property & V_frozen) && ((v->property & V_import) || (v->property & (V_oldvalue|V_readonly)) == V_readonly) || state.stateview == 0 && !(v->property & V_retain))
		return 0;
	v->property |= V_compiled;
	property = v->property;
	value = v->value;

	/*
	 * check for possible old value
	 *
	 * if v->oldvalue is set in load() then the
	 * variable is frozen and the frozen value
	 * is different than the makefile value
	 */

	if (state.stateview == 0)
		property &= ~V_CLEAROBJECT;
	else
	{
		property &= ~V_CLEARSTATE;
		if (property & V_oldvalue)
		{
			if (t = getold(v->name))
			{
				if (!(property & V_frozen))
				{
					value = t;
					property &= ~V_oldvalue;
				}
				else if (streq(value, t))
					property &= ~V_oldvalue;
			}
#if DEBUG
			else
				error(PANIC, "%s->oldvalue set but not in table.oldvalue", s);
#endif
		}
		else if ((property & (V_frozen|V_readonly)) == (V_frozen|V_readonly))
			property |= V_oldvalue;
	}

	/*
	 * write the variable fields
	 */

	sfputu(cs->fp, property);
	compstring(cs, v->name);
	compstring(cs, value);
	cs->variables++;
	return 0;
}

/*
 * clear temporary marks on r
 */

static int
clearmarks(const char* s, char* v, void* h)
{
	register struct rule*	r = (struct rule*)v;

	NoP(s);
	NoP(h);
	r->complink = 0;
	r->mark &= ~M_compile;
	return 0;
}

/*
 * compile the current rules and variables into objfile
 */

void
compile(char* objfile, char* select)
{
	register Sfio_t*	sp;
	struct list*		p;
	struct list*		q;
	struct rule*		r;
	struct compstate	cs;

	static char*		tmpfile;

	/*
	 * initialize the object globals
	 */

	zero(cs);
	cs.lists++;
	cs.rules++;

	/*
	 * create the object temporary file
	 */

	if (!tmpfile)
		tmpfile = pathtemp(NiL, 0, null, idname, NiL);
	state.tmpfile = tmpfile;
	if (!(cs.fp = sfopen(NiL, state.tmpfile, "brw")))
	{
		error(ERROR_SYSTEM|1, "%s: cannot create temporary object file", state.tmpfile);
		return;
	}

	/*
	 * skip the header until everything else is done
	 */

	if (sfseek(cs.fp, (Sfoff_t)HEADERSIZE, SEEK_SET) != HEADERSIZE)
		error(ERROR_SYSTEM|3, "%s: object file header write error", state.tmpfile);

	/*
	 * write the optional headers
	 */

	if (sp = object.pp)
	{
		object.pp = 0;
		if (!state.base)
		{
			sfputu(sp, COMP_OPTIONS);
			sfputu(sp, 0);
			for (p = internal.preprocess->prereqs; p; p = p->next)
				putstring(sp, p->rule->name, p->next ? ' ' : -1);
			sfputc(sp, 0);
			sfputu(sp, 0);
			sfputu(internal.tmp, HEADER_PREREQS);
			sfputu(cs.fp, sfstrtell(internal.tmp) + sfstrtell(sp));
			sfstrset(internal.tmp, 0);
			sfputu(cs.fp, HEADER_PREREQS);
			sfwrite(cs.fp, sfstrbase(sp), sfstrtell(sp));
		}
		sfstrclose(sp);
	}
	sp = cs.fp;
	sfputu(sp, 0);
	if (sferror(sp))
		error(ERROR_SYSTEM|3, "%s: object file optional header write error", state.tmpfile);

	/*
	 * mark the rules and prerequisites for compilation
	 */

	if (select)
	{
		Sfio_t*	tmp;

		hashwalk(table.rule, 0, compinit, null);
		tmp = sfstropen();
		sfprintf(tmp, "$(<:V:%s)", select);
		r = state.frame->target;
		hashwalk(table.rule, 0, compselect, sfstruse(tmp));
		state.frame->target = r;
		sfstrclose(tmp);
	}
	else
	{
		/*
		 * compile and write the variables
		 */

		hashwalk(table.var, 0, compvar, &cs);
		if (sferror(sp))
			error(ERROR_SYSTEM|3, "%s: object file variable write error", state.tmpfile);
		hashwalk(table.rule, 0, compinit, NiL);
		if (state.stateview == 0)
		{
			/*
			 * check state file garbage collection
			 *
			 * NOTE: only the head of each garbage list is
			 *	 counted so the percentage threshold
			 *	 should probably be low
			 */

			if ((100 * object.garbage / (object.rules ? object.rules : 1) < PCTGARBAGE && !(state.test & 0x00008000)))
				object.garbage = 0;
			hashwalk(table.rule, 0, compstate, &cs);
			if (object.garbage)
			{
				hashwalk(table.rule, 0, clearmarks, &cs);
				hashwalk(table.rule, 0, compkeep, &cs);
			}
#if BINDINDEX
			for (n = 1; n <= state.maxsource; n++)
				markcompile(state.source[n].path);
			for (n = 1; n <= state.maxview; n++)
				markcompile(state.view[n].path);
#endif
		}
	}
	hashwalk(table.rule, 0, compmark, &cs);

	/*
	 * some rules must always be compiled and/or appear first
	 */

	for (p = internal.special->prereqs; p; p = p->next)
		markcompile(p->rule);
	for (p = internal.special->prereqs; p; p = p->next)
		for (q = p->rule->prereqs; q; q = q->next)
			comprule(q->rule->name, (char*)q->rule, &cs);
#if BINDINDEX
	if (state.stateview == 0)
	{
		for (n = 1; n <= state.maxsource; n++)
		{
			x = state.source[n].path;
			comprule(x->name, x);
		}
		for (n = 1; n <= state.maxview; n++)
		{
			x = state.view[n].path;
			comprule(x->name, x);
		}
	}
#endif

	/*
	 * compile and write the rules
	 */

	hashwalk(table.rule, 0, comprule, &cs);
	if (sferror(sp))
		error(ERROR_SYSTEM|3, "%s: object file rule write error", state.tmpfile);

	/*
	 * compile and write the prerequisite lists
	 */

	for (p = internal.special->prereqs; p; p = p->next)
		for (q = p->rule->prereqs; q; q = q->next)
			complist(q->rule->name, (char*)q->rule, &cs);
	hashwalk(table.rule, 0, complist, &cs);
	if (sferror(sp))
		error(ERROR_SYSTEM|3, "%s: object file prerequisite write error", state.tmpfile);

	/*
	 * write the trailer
	 */

	if (state.stateview < 0)
		listops(sp, 1);
	sfputc(sp, 0);

	/*
	 * clear temporary marks
	 */

	hashwalk(table.rule, 0, clearmarks, &cs);

	/*
	 * write the real header
	 */

	sfseek(sp, (Sfoff_t)0, SEEK_SET);
	sfwrite(sp, MAGIC, MAGICSIZE);
	putstring(sp, version, 0);
	sfputu(sp, HEADERSIZE);
	sfputu(sp, SEQUENCE);
	sfputu(sp, 0);
	sfputu(sp, cs.strings);
	sfputu(sp, cs.lists - 1);
	sfputu(sp, cs.rules - 1);
	sfputu(sp, RULENUM - MINRULENUM);
	sfputu(sp, RULESTR - MINRULESTR);
	sfputu(sp, cs.variables);
	sfputu(sp, VARNUM - MINVARNUM);
	sfputu(sp, VARSTR - MINVARSTR);
	sfwrite(sp, MAGIC, MAGICSIZE);
	if (sferror(sp))
		error(ERROR_SYSTEM|3, "%s: temporary object file header write error", state.tmpfile);

	/*
	 * commit to the temporary object and clean up
	 */

	sfclose(sp);
	remove(objfile);
	if (rename(state.tmpfile, objfile))
		error(1, "%s: object file not recompiled", objfile);
	remtmp(0);
	r = bindfile(NiL, objfile, BIND_DOT|BIND_FORCE|BIND_RULE);
	r->dynamic |= D_built|D_regular;
	r->view = 0;
	if (state.mam.dynamic || state.mam.regress)
	{
		mampush(state.mam.out, r, P_force);
		sfprintf(state.mam.out, "%sexec %s : compile into %s object\n", state.mam.label, state.mam.dynamic ? mamname(r) : null, error_info.id);
		mampop(state.mam.out, r, 0);
	}
	if (state.stateview == 0 && object.garbage && object.garbage > cs.rules)
		message((-1, "%d%% [%d/%d] state file garbage collection recovery", (object.garbage - cs.rules) * 100 / object.garbage, object.garbage - cs.rules, object.garbage));
}

/*
 * register input file prerequisite
 */

void
compref(int type, const char* name, unsigned long date)
{
	if (object.pp)
	{
		if (type)
		{
			sfputu(object.pp, type);
			sfputu(object.pp, date);
			putstring(object.pp, name, 0);
		}
		else
		{
			sfstrclose(object.pp);
			object.pp = 0;
		}
	}
	else if (!type && !state.makefile)
		object.pp = sfstropen();
}

/*
 * promote lower view prereqs of top view state
 */

static int
promote(const char* s, char* v, void* h)
{
	register struct rule*	r = (struct rule*)v;
	register struct list*	p;

	NoP(s);
	NoP(h);
	if (r->mark & M_compile)
	{
		r->mark &= ~M_compile;
		for (p = r->prereqs; p; p = p->next)
		{
			r = p->rule;
			if (r->dynamic & D_lower)
			{
				unviewname(r->name);
				p->rule = makerule(r->name);
				viewname(r->name, r->view);
			}
		}
	}
	return 0;
}

/*
 * associate one rule with each name
 */

static int
atomize(const char* s, char* v, void* h)
{
	register struct rule*	r = (struct rule*)v;
	register struct list*	p;

	NoP(s);
	NoP(h);
	if (isoldrule(r))
	{
#if DEBUG
		error(PANIC, "old rule %s still in table.rule%s", r->name, r == getrule(r->name) ? null : " -- duplicate hash");
#endif
		return 0;
	}
	for (p = r->prereqs; p; p = p->next)
		if (isoldrule(p->rule))
			p->rule = getoldrule(p->rule);
	return 0;
}

/*
 * remove temporary compilation files
 */

void
remtmp(int fatal)
{
	if (fatal && state.stateview == 0 && state.tmpfile)
		error(2, "%s: state file not updated", statefile());
	if (state.tmpfile)
	{
		if (remove(state.tmpfile) && errno != ENOENT)
			error(ERROR_SYSTEM|1, "%s: temporary file not removed", state.tmpfile);
		state.tmpfile = 0;
	}
	if (fatal)
		lockstate(0);
}

/*
 * load a string
 */

static char*
loadstring(struct loadstate* ls, Sfio_t* sp)
{
	register int	n;
	register char*	s;

	if (!(n = sfgetu(sp)))
		return 0;
	s = ls->sp;
	ls->sp += n--;
	sfread(sp, s, n);
	ccmapstr(object.a2n, s, n);
	return s;
}

/*
 * initialize object state
 */

static void
loadinit(void)
{
	object.garbage = object.rules = 0;
}

/*
 * check if file is loadable object
 * if source!=0 then source prereqs are checked
 */

int
loadable(register Sfio_t* sp, register struct rule* r, int source)
{
	register struct rule*	x;
	register struct list*	p;
	char*			s;
	char*			t;
	long			n;
	Sfoff_t			off;
	time_t			m;
	int			ok = 1;
	long			old = 0;

	loadinit();
	if ((s = sfreserve(sp, 0, 0)) && (n = sfvalue(sp)) >= 0)
	{
		if (n >= MAGICSIZE && !memcmp(s, MAGIC, MAGICSIZE) || n >= sizeof(old) && ((old = OLD_MAGIC, swapop(s, &old, sizeof(old)) >= 0) || (old = OLD_OLD_MAGIC, swapop(s, &old, sizeof(old)) >= 0)))
		{
			if (!source)
				return state.base || !state.forceread || state.global || state.list;

			/*
			 * check previous source prerequisites
			 */

			if (!old && !sfseek(sp, (Sfoff_t)0, SEEK_SET) && sfseek(sp, (Sfoff_t)MAGICSIZE, SEEK_SET) == MAGICSIZE)
			{
				if (getstring(sp) && (off = sfgetu(sp)) && sfgetu(sp))
					while (!sfeof(sp) && sfseek(sp, off, SEEK_SET) == off && (off = sfgetu(sp)))
					{
						off += sfseek(sp, (Sfoff_t)0, SEEK_CUR);
						if (sfgetu(sp) == HEADER_PREREQS)
						{
							/*UNDENT...*/

	state.init++;
	while (n = sfgetu(sp))
	{
		m = sfgetu(sp);
		if (!(s = getstring(sp)))
			break;
		if (n & (COMP_BASE|COMP_FILE|COMP_GLOBAL|COMP_INCLUDE))
		{
			x = bindfile(NiL, s, BIND_FORCE|BIND_MAKEFILE|BIND_RULE);
			if (!(x->dynamic & D_regular))
				x->dynamic &= ~D_bound;

			/*
			 * put makefile prereqs in state as query courtesy
			 */

			if (!(n & COMP_BASE) && *x->name != '/')
				staterule(RULE, x, NiL, 1)->time = x->time;

			/*
			 * check prerequisite file time with previous
			 */

			message((-4, "%s%s%s%s%s%sprerequisite %s [%s] state [%s]", (x->dynamic & D_regular) ? null : "non-regular ", (n & COMP_DONTCARE) ? "optional " : null, (n & COMP_BASE) ? "base " : null, (n & COMP_FILE) ? "-f " : null, (n & COMP_GLOBAL) ? "-g " : null, (n & COMP_INCLUDE) ? "include " : null, s, strtime(x->time), strtime(m)));
			if (((x->dynamic & D_regular) || (n & COMP_BASE)) && timecmp(m, x->time))
			{
				error(state.exec || state.mam.out ? -1 : 1, "%s: out of date with %s", r->name, x->name);
				break;
			}

			/*
			 * check that explicit prerequisite still specified
			 */

			if (n & (COMP_FILE|COMP_GLOBAL))
			{
				for (p = ((n & COMP_FILE) ? internal.makefiles : internal.globalfiles)->prereqs; p; p = p->next)
					if (!(p->rule->mark & M_compile))
					{
						if (streq(p->rule->name, s) || p->rule->uname && streq(p->rule->uname, s))
							p->rule->mark |= M_compile;
						else
						{
							error(state.exec || state.mam.out ? -1 : 1, "%s: %sfile %s option order changed", r->name, (n & COMP_GLOBAL) ? "global " : null, s);
							goto nope;
						}
						break;
					}
				if (!p)
				{
					error(state.exec || state.mam.out ? -1 : 1, "%s: %sfile %s was specified last time", r->name, (n & COMP_GLOBAL) ? "global " : null, s);
					break;
				}
			}
			else if ((n & COMP_BASE) && !state.rules)
				state.rules = makerule(s)->name;
		}
		else if (n & COMP_OPTIONS)
		{
			/*
			 * compare with current preprocessor options
			 */

			for (p = internal.preprocess->prereqs; p; p = p->next)
				sfputr(internal.nam, p->rule->name, p->next ? ' ' : -1);
			t = sfstruse(internal.nam);
			if (!streq(s, t))
			{
				error(state.exec || state.mam.out ? -1 : 1, "%s: options changed%s%s", r->name, *s ? " from " : null, s);
				break;
			}
		}
	}
 nope:
	state.init--;
	if (n)
		ok = 0;

	/*
	 * check for explicit file prereqs not specified last time
	 */

	for (p = internal.globalfiles->prereqs; p; p = p->next)
		if (p->rule->mark & M_compile)
			p->rule->mark &= ~M_compile;
		else if (ok)
		{
			ok = 0;
			if (state.writeobject)
				error(state.exec || state.mam.out ? -1 : 1, "%s: global file %s not specified last time", r->name, p->rule->name);
		}
	for (p = internal.makefiles->prereqs; p; p = p->next)
		if (p->rule->mark & M_compile)
			p->rule->mark &= ~M_compile;
		else if (ok)
		{
			ok = 0;
			if (state.writeobject)
				error(state.exec || state.mam.out ? -1 : 1, "%s: file %s not specified last time", r->name, p->rule->name);
		}
	if (ok && !sfseek(sp, (Sfoff_t)0, SEEK_SET))
		return 1;

							/*...INDENT*/
							break;
						}
					}
				sfseek(sp, (Sfoff_t)0, SEEK_SET);
			}
		}
	}
	return 0;
}

/*
 * load compiled rules and variables from objfile
 * return:
 *	-1	partially loaded => punt
 *	 0	not loaded
 *	 1	loaded
 */

int
load(register Sfio_t* sp, const char* objfile, int ucheck)
{
	register int		n;
	register struct rule*	r;
	register struct var*	v;
	register struct list*	d;
	register char*		s;
	char*			p = 0;
	int			promoted = 0;
	int			recompile = 0;
	struct rule*		oldrules = 0;
	struct rule*		or;
	struct rule*		xr;
	struct var*		ov;
	struct var*		xv;
	struct var*		x;
	struct list*		xd;
	struct list*		a;
	char*			ident;
	int			flags;
	int			strings;
	int			lists;
	int			rules;
	int			rulenum;
	int			rulestr;
	int			variables;
	int			varnum;
	int			varstr;
	int			attrclash;
	int			garbage;
	int			oscan;
	int			scanclash;
	int			sequence;
	unsigned long		attr;
	unsigned long		attrclear;
	unsigned long		oattribute;
	struct frame*		fp;
	Sfoff_t			off;
	struct stat		st;
	unsigned long		attrmap[CHAR_BIT * sizeof(unsigned long)];
	unsigned char		scanmap[UCHAR_MAX + 1];
	struct loadstate	ls;

	int			old;
	int			old_swap;
	long			old_magic;
	struct OLD_rule		old_rule;
	struct OLD_var		old_var;
	struct OLD_list		old_list;
	struct OLD_header	old_header;
	struct OLD_trailer	old_trailer;

	loadinit();
	zero(ls);

	/*
	 * empty object files are ok
	 */

	if (fstat(sffileno(sp), &st))
	{
		error(1, "%s: cannot stat object file", objfile);
		return 0;
	}
	if (!st.st_size)
	{
		if (state.stateview >= 0)
		{
			error(1, "%s: empty state file", objfile);
			return 0;
		}
		return 1;
	}

	/*
	 * check for other users within last ucheck minutes
	 */

	if (ucheck && st.st_uid != geteuid() && (n = (CURTIME - (unsigned long)st.st_mtime)) < ucheck * 60)
		error(1, "%s: another user was here %s ago", state.makefile, fmtelapsed(n, 1));

	/*
	 * check the header
	 */

	if (!(s = sfreserve(sp, MAGICSIZE, 0)))
		goto badmagic;
	if (memcmp(s, MAGIC, MAGICSIZE) || !(s = getstring(sp)) || streq(s, OLD_VERSION))
	{
		old = 1;
		if (sfseek(sp, (Sfoff_t)0, SEEK_SET) ||
		    sfread(sp, (char*)&old_header, sizeof(old_header)) != sizeof(old_header) ||
		    sfseek(sp, -(Sfoff_t)sizeof(old_trailer), SEEK_END) == -1 ||
		    sfread(sp, (char*)&old_trailer, sizeof(old_trailer)) != sizeof(old_trailer))
			goto badio;
		if ((old_magic = OLD_MAGIC, (old_swap = swapop(&old_header.magic, &old_magic, sizeof(old_magic)))) < 0 && (old_magic = OLD_OLD_MAGIC, (old_swap = swapop(&old_header.magic, &old_magic, sizeof(old_magic)))) < 0)
			goto badmagic;
		sequence = old_header.sequence;
		old_header.sequence = old_stamp.sequence;
		if (old_swap)
		{
			swapmem(old_swap, &old_header, &old_header, sizeof(old_header));
			swapmem(old_swap, &old_trailer, &old_trailer, sizeof(old_trailer));
		}
		if ((st.st_size - sizeof(old_trailer)) & (OLD_ALIGN - 1))
			goto badversion;
		strcpy(old_stamp.version, OLD_VERSION);
		if (memcmp(((char*)&old_header) + sizeof(old_header.magic), ((char*)&old_stamp) + sizeof(old_stamp.magic), sizeof(old_header) - sizeof(old_header.magic)))
		{
			strcpy(old_stamp.version, OLD_VERSION_2);
			if (memcmp(((char*)&old_header) + sizeof(old_header.magic), ((char*)&old_stamp) + sizeof(old_stamp.magic), sizeof(old_header) - sizeof(old_header.magic)))
				goto badversion;
			sequence = 2;
		}
		ident = strrchr(old_stamp.version, ' ') + 1;
		if (old_trailer.magic != old_header.magic || old_trailer.size != st.st_size)
			goto badmagic;
		if (state.exec && streq(objfile, state.objectfile))
			return 0;
		lists = old_trailer.lists;
		rules = old_trailer.rules;
		variables = old_trailer.variables;
		off = sizeof(old_header) + rules * sizeof(old_rule) + lists * sizeof(old_list) + variables * sizeof(old_var);
		strings = st.st_size - sizeof(old_trailer) - off;
		if (sfeof(sp) || sfseek(sp, off, SEEK_SET) != off)
			goto badio;
	}
	else
	{
		old = 0;
		if (s = strrchr(s, ' '))
			s++;
		else
			s = "old";
		ident = strcpy((char*)scanmap, s);
		off = sfgetu(sp);
		sequence = sfgetu(sp);
		flags = sfgetu(sp);
		strings = sfgetu(sp);
		lists = sfgetu(sp);
		rules = sfgetu(sp);
		rulenum = sfgetu(sp);
		rulestr = sfgetu(sp);
		variables = sfgetu(sp);
		varnum = sfgetu(sp);
		varstr = sfgetu(sp);
		if (!(s = sfreserve(sp, MAGICSIZE, 0)) || memcmp(s, MAGIC, MAGICSIZE))
			goto badmagic;

		/*
		 * read the optional headers
		 */

		for (;;)
		{
			if (sfeof(sp) || sfseek(sp, off, SEEK_SET) != off)
				goto badio;
			if (!sequence || !(off = sfgetu(sp)))
				break;
			off += sfseek(sp, (Sfoff_t)0, SEEK_CUR);
		}
	}
	message((-3, "%s sequence=%d lists=%d rules=%d variables=%d strings=%d", ident, sequence, lists, rules, variables, strings));

	/*
	 * allocate strings and structs in one chunk
	 * and compute pointers to the compiled data
	 */

	if (!(p = newof(0, char, lists * sizeof(struct list) + rules * sizeof(struct rule) + variables * sizeof(struct var) + strings, 0)))
	{
		error(3, "out of space");
		goto bad;
	}
	r = or = (struct rule*)p;
	d = (struct list*)((char*)r + rules * sizeof(struct rule));
	v = ov = (struct var*)((char*)d + lists * sizeof(struct list));
	s = ((char*)v + variables * sizeof(struct var));

	/*
	 * read the string table
	 */

	if (!old)
		ls.sp = s;
	else if (sfread(sp, s, strings) != strings)
		goto badio;

	/*
	 * load the variables and check for any frozen
	 * variables that may have changed
	 */

	if (old)
	{
		off = sizeof(old_header) + rules * sizeof(old_rule) + lists * sizeof(old_list);
		if (sfseek(sp, off, SEEK_SET) != off)
			goto badio;
	}
	oattribute = internal.attribute->attribute;
	oscan = internal.scan->scan;
	attrclash = scanclash = 0;
	for (xv = v + variables; v < xv; v++)
	{
		if (old)
		{
			if (sfread(sp, (char*)&old_var, sizeof(old_var)) != sizeof(old_var))
				goto badio;
			if (old_swap)
				swapmem(old_swap, &old_var, &old_var, sizeof(old_var));
			v->property = old_var.property;
			if (old_var.name)
				v->name = s + (unsigned long)old_var.name - 1;
			if (old_var.value)
				v->value = s + (unsigned long)old_var.value - 1;
			switch (sequence)
			{
			case 2: /* 01/24/89 */
				v->property = (v->property & 0x000003ffL);
				break;
			}
		}
		else
		{
			v->property = sfgetu(sp);
#if !_HUH_1993_10_01 /* drop this eventually */
			v->property &= ~V_free;
#endif

			/*
			 * variable number field additions here
			 */

			for (n = varnum; n > 0; n--)
				sfgetu(sp);
			v->name = loadstring(&ls, sp);
			v->value = loadstring(&ls, sp);

			/*
			 * variable string field additions here
			 */

			for (n = varstr; n > 0; n--)
				loadstring(&ls, sp);
		}
		if ((v->property & V_frozen) && (!(x = getvar(v->name)) && ((v->property & V_oldvalue) || (v->property & V_import) && *v->value) || x && ((x->property & (V_append|V_readonly)) == (V_append|V_readonly) || ((v->property|x->property) & (V_import|V_readonly)) && !streq(v->value, x->value)) || (v->property & V_functional)))
		{
			error((state.exec || state.mam.out) && !state.explain ? -1 : 1, "%s: frozen %svariable %s changed", objfile, ((v->property|(x ? x->property : 0)) & V_import) ? "environment " : ((x ? x->property : 0) & V_readonly) ? "command argument " : null, v->name);
			recompile = 1;
			v->property &= ~V_readonly;
		}
		else
			v->property &= ~(V_oldvalue|V_readonly);
	}
	if (sfeof(sp))
		goto badio;
	if (recompile)
	{
		recompile = 0;
		goto bad;
	}
	recompile = -1;
	p = 0;
	v = ov;

	/*
	 * enter the variables
	 */

	hashclear(table.var, HASH_ALLOCATE);
	for (xv = v + variables; v < xv; v++)
	{
		if (!(x = getvar(v->name)) || !(x->property & (V_readonly|V_restored)) && (!(x->property & V_import) || !state.global))
		{
			putvar(v->name, v);
			if (x)
				freevar(x);
		}
		else
		{
			if ((x->property & (V_append|V_readonly)) == (V_append|V_readonly))
			{
				n = state.reading;
				state.reading = 1;
				setvar(x->name, v->value, 0);
				state.reading = n;
			}
			x->property |= v->property & (V_functional|V_scan);
		}
		if ((v->property & V_retain) && state.stateview >= 0)
			v->property |= V_restored;
	}
	hashset(table.var, HASH_ALLOCATE);

#if BINDINDEX
	/*
	 * initialize the bind index maps
	 */

	for (n = 0; n <= state.maxsource; n++)
		state.source[i].map = 0;
	for (n = 0; n <= state.maxview; n++)
		state.view[i].map = 0;
	state.view[0].map = state.stateview;
#endif

	/*
	 * load and enter the rules
	 */

	if (old)
	{
		off = sizeof(old_header);
		if (sfseek(sp, off, SEEK_SET) != off)
			goto badio;
	}
	garbage = 0;
	hashclear(table.rule, HASH_ALLOCATE);
	for (xr = r + rules; r < xr; r++)
	{
		register struct rule*	o;

		if (old)
		{
			if (sfread(sp, (char*)&old_rule, sizeof(old_rule)) != sizeof(old_rule))
				goto badio;
			if (old_swap)
				swapmem(old_swap, &old_rule, &old_rule, (char*)&old_rule.noswap - (char*)&old_rule);
			r->property = old_rule.property;
			r->dynamic = old_rule.dynamic;
			r->attribute = old_rule.attribute;
			switch (sequence)
			{
			case 2: /* 01/24/89 */
				r->property =
					((r->property & 0x0000ffffL)) |
					((r->property & 0x3ffb0000L) << 1);
				r->dynamic =
					((r->dynamic & 0x000003ffL)) |
					((r->dynamic & 0x00000800L) << 1) |
					((r->dynamic & 0x00034000L) << 2) |
					((r->dynamic & 0x00040000L) << 4);
				break;
			}
			if (old_rule.name)
				r->name = s + (unsigned long)old_rule.name - 1;
			if (r->property & P_staterule)
				r->event = old_rule.old_event;
			else if (old_rule.old_data)
				r->statedata = s + (unsigned long)old_rule.old_data - 1;
			if (old_rule.prereqs)
				r->prereqs = d + (unsigned long)old_rule.prereqs / sizeof(old_list) - 1;
			if (old_rule.action)
				r->action = s + (unsigned long)old_rule.action - 1;
			r->scan = old_rule.scan;
			r->semaphore = old_rule.semaphore;
			r->view = old_rule.view;
			r->time = old_rule.time;
			switch (sequence)
			{
			case 0: /* 09/11/89 */
				if ((r->property & P_attribute) && (r->attribute && !(r->property & P_use) && !streq(r->name, internal.attribute->name) || r->scan && !streq(r->name, internal.scan->name)))
					r->dynamic |= D_index;
				/*FALLTHROUGH*/
			case 1: /* 07/17/91 */
				if (r->property & P_staterule)
				{
					if (isaltstate(r->name))
					{
						r->property |= P_implicit;
						r->time = 0;
					}
					r->event = r->time;
				}
				/*FALLTHROUGH*/
			}
		}
		else
		{
			r->property = sfgetu(sp);
			r->dynamic = sfgetu(sp);
			r->attribute = sfgetu(sp);
			if (n = sfgetu(sp))
			{
				r->semaphore = (n>>16) & ((1<<8)-1);
				r->view = (n>>8) & ((1<<8)-1);
				r->scan = (n) & ((1<<8)-1);
			}
			if (n = sfgetu(sp))
				r->prereqs = d + n - 1;
			r->time = sfgetu(sp);

			/*
			 * rule number field additions here
			 */

			for (n = rulenum; n > 0; n--)
				sfgetu(sp);
			r->name = loadstring(&ls, sp);
			r->action = loadstring(&ls, sp);
			if (r->property & P_staterule)
				r->event = sfgetu(sp);
			else
				r->statedata = loadstring(&ls, sp);

			/*
			 * rule string field additions here
			 */

			for (n = rulestr; n > 0; n--)
				loadstring(&ls, sp);
		}
		r->preview = state.maxview + 1;
		o = getrule(r->name);
		if (r->dynamic & D_index)
		{
			/*
			 * check for index atom consistency
			 * remap inconsistent state file atoms
			 */

			if (r->scan)
			{
				attr = ~0;
				if (o && (o->property & P_attribute) && o->scan)
				{
					if (r->scan != o->scan)
					{
						error((state.exec || state.mam.out) && !state.explain ? -1 : 1, "%s: %s %s definition changed", objfile, r->name, internal.scan->name);
						if (state.stateview < 0)
							return -1;
						attr = o->scan;
					}
				}
				else
				{
					for (a = internal.scan->prereqs; a; a = a->next)
						if (r->scan == a->rule->scan)
						{
							error((state.exec || state.mam.out) && !state.explain ? -1 : 1, "%s: %s %s definition clashes with %s", objfile, r->name, internal.scan->name, a->rule->name);
							if (state.stateview < 0)
								return -1;
							attr = 0;
							break;
						}
					if (state.stateview >= 0)
						attr = 0;
				}
				if (attr != ~0)
				{
					if (!scanclash)
					{
						scanclash = 1;
						for (n = 0; n < elementsof(scanmap); n++)
							scanmap[n] = n;
					}
					if (!(r->scan = scanmap[r->scan] = attr))
						r->property &= ~P_attribute;
					if (o)
						continue;
				}
			}
			else if (r->attribute)
			{
				attr = ~0;
				if (o && (o->property & P_attribute) && o->attribute)
				{
					if (r->attribute != o->attribute)
					{
						error((state.exec || state.mam.out) && !state.explain ? -1 : 1, "%s: %s %s definition changed", objfile, r->name, internal.attribute->name);
						if (state.stateview < 0)
							return -1;
						attr = o->attribute;
					}
				}
				else
					for (a = internal.attribute->prereqs; a; a = a->next)
						if (r->attribute == a->rule->attribute)
						{
							error((state.exec || state.mam.out) && !state.explain ? -1 : 1, "%s: %s %s definition clashes with %s", objfile, r->name, internal.attribute->name, a->rule->name);
							if (state.stateview < 0)
								return -1;
							attr = 0;
						}
				if (attr != ~0)
				{
					if (!attrclash)
					{
						attrclash = 1;
						attrclear = 0;
						for (n = 0; n < CHAR_BIT * sizeof(unsigned long); n++)
							attrmap[n] = (1<<n);
					}
					attrclear |= r->attribute;
					for (n = 0; n < CHAR_BIT * sizeof(unsigned long); n++)
						if (r->attribute == (1<<n))
						{
							attrmap[n] = attr;
							break;
						}
					continue;
				}
			}
		}
		else
		{
			if (attrclash)
			{
				attr = r->attribute & ~attrclear;
				for (n = 0; n < CHAR_BIT * sizeof(unsigned long); n++)
					if (r->attribute & (1<<n))
						attr |= attrmap[n];
				r->attribute = attr;
			}
			if (scanclash)
				r->scan = scanmap[r->scan];
		}
		if (r->dynamic & D_compiled)
		{
			n = state.stateview > 0 && viewable(r);
			if (n && (o || (state.questionable & 0x00000020)))
			{
				r->dynamic |= D_compiled|D_lower;
				viewname(r->name, state.stateview);
				r->view = state.stateview;
				putrule(r->name, r);
			}
			else
			{
				if (n)
				{
					r->mark |= M_compile;
					promoted = 1;
				}
				if (!o)
				{
					if (state.stateview >= 0 && (r->property & P_target))
					{
						r->dynamic |= D_garbage;
						garbage++;
					}
				}
				else if (state.stateview >= 0 && (r->property & P_statevar) && (r->prereqs || r->action))
				{
					/*
					 * ignore state file prereqs and action
					 */

					if (o->action && r->action && !streq(o->action, r->action))
						r->time = CURTIME;
					o->time = r->time;
					o->statedata = r->statedata;
					setoldrule(r, o);
					continue;
				}
				else if (state.stateview >= 0 && (r->property & P_staterule) && o->event > r->event)
				{
					/*
					 * o was updated after r was saved 
					 */

					setoldrule(r, o);
					continue;
				}
				else if (!isoldrule(o))
				{
					if (o->dynamic & (D_bound|D_scanned))
					{
						r->statedata = o->statedata;
						r->time = o->time;
						r->status = o->status;
						r->view = o->view;
						r->property &= ~(P_parameter|P_state|P_staterule|P_statevar);
						r->property |= o->property & (P_parameter|P_state|P_staterule|P_statevar);
						r->dynamic &= ~(D_bound|D_entries|D_global|D_regular|D_scanned);
						r->dynamic |= o->dynamic & (D_bound|D_entries|D_global|D_regular|D_scanned);
						if (!(o->property & P_state) && o->uname)
						{
							r->uname = putrule(o->uname, r);
							getrule(o->name);
						}
					}
					else if ((r->property & (P_parameter|P_statevar)) == P_statevar && (o->property & (P_parameter|P_statevar)) == (P_parameter|P_statevar))
						r->property |= P_parameter;
					setoldrule(o, r);
				}
				r->name = putrule(0, r);
			}
		}
		else if (o)
		{
			/*
			 * r is just a reference -- keep the old rule
			 * but retain some attributes
			 */

			o->attribute |= r->attribute & internal.retain->attribute;
			o->property |= r->property & (P_dontcare|P_ignore|P_terminal);
			setoldrule(r, o);
		}
		else
		{
			r->name = putrule(0, r);
			if (!(r->property & P_state))
				r->time = 0;
			if (state.stateview >= 0)
			{
				if (state.stateview >= 0 && (r->property & P_target))
				{
					r->dynamic |= D_garbage;
					garbage++;
				}

				/*
				 * clear reference secondary attributes
				 */

				r->attribute &= internal.retain->attribute;
				r->property &= (P_attribute|P_dontcare|P_ignore|P_parameter|P_state|P_staterule|P_statevar|P_terminal);
				r->dynamic &= D_garbage;
			}
		}
#if BINDINDEX
		if (r->dynamic & D_bindindex)
		{
			if (o)
			{
				if (o->dynamic & D_bindindex)
				{
					if (r->source)
					{
						state.source[r->source].map = o->source;
						if (isoldrule(o))
							state.source[o->source].path = r;
					}
					else if (r->view)
					{
						state.view[r->view].map = o->view;
						if (isoldrule(o))
							state.view[o->view].path = r;
					}
				}
				else if (r->source)
				{
					if (++state.maxsource >= elementsof(state.source))
						error(3, "%s: too many %s directories -- %d max", r->name, internal.source->name, elementsof(state.source));
					state.source[r->source].map = state.maxsource;
					if (isoldrule(o))
						state.source[state.maxsource].path = r;
					else
					{
						o->dynamic |= D_bindindex;
						o->source = state.maxsource;
						state.source[o->source].path = o;
					}
				}
			}
			else if (r->source)
			{
				if (++state.maxsource >= elementsof(state.source))
					error(3, "%s: too many %s directories -- %d max", r->name, internal.source->name, elementsof(state.source));
				state.source[r->source].map = state.maxsource;
				state.source[state.maxsource].path = r;
			}
		}
		r->source = state.source[r->source].map;
		r->view = state.view[r->view].map;
#endif
	}
	hashset(table.rule, HASH_ALLOCATE);
	r = or;

	/*
	 * load the prerequisite lists
	 */

	if (old)
	{
		off = sizeof(old_header) + rules * sizeof(old_rule);
		if (sfseek(sp, off, SEEK_SET) != off)
			goto badio;
	}
	if (lists)
	{
		for (xd = d + lists; d < xd;)
		{
			if (old)
			{
				if (sfread(sp, (char*)&old_list, sizeof(old_list)) != sizeof(old_list))
					goto badio;
				if (old_swap)
					swapmem(old_swap, &old_list, &old_list, sizeof(old_list));
				if (old_list.rule)
					d->rule = r + (unsigned long)old_list.rule / sizeof(old_rule) - 1;
				if (old_list.next)
					d->next = d + 1;
				d++;
			}
			else if (n = sfgetu(sp))
			{
				d->rule = r + n - 1;
				d->next = d + 1;
				d++;
			}
			else
				(d - 1)->next = 0;
		}
		if (!old)
		{
			sfgetu(sp);
			(d - 1)->next = 0;
		}
	}
	if (sfeof(sp))
		goto badio;

	/*
	 * collect state file garbage collection stats
	 */

	if (state.stateview >= 0)
	{
		object.garbage += garbage;
		object.rules += rules;
	}

	/*
	 * readjust the internal rule pointers
	 */

	initrule();

	/*
	 * make sure top view state has top view prereqs
	 */

	if (promoted)
		hashwalk(table.rule, 0, promote, NiL);

	/*
	 * associate one rule with each name
	 */
	
	if (oldrules)
	{
		hashwalk(table.rule, 0, atomize, NiL);
		fp = state.frame;
		for (;;)
		{
			if (isoldrule(fp->target))
			{
				fp->target = getoldrule(fp->target);
				fp->target->active = fp;
			}
			if (fp == fp->parent)
				break;
			fp = fp->parent;
		}
		do
		{
			xr = (struct rule*)oldrules->prereqs;
			freerule(oldrules);
		} while (oldrules = xr);
	}

	/*
	 * check special indices
	 */

	if (internal.attribute->attribute == 1)
		internal.attribute->attribute = oattribute;
	if (internal.scan->scan == SCAN_USER)
		internal.scan->scan = oscan;

	/*
	 * reset compiled options
	 */

	if (!state.list)
	{
		if (old)
		{
			if (old_trailer.options)
				set(s + (unsigned long)old_trailer.options - 1);
		}
		else if ((s = getstring(sp)) && *s)
			set(s);
		if (state.stateview < 0)
		{
			/*
			 * check for load time actions
			 *
			 * NOTE: state.global++ allows var definitions
			 *       to override readonly vars here
			 */

			if (state.global)
				state.global++;
			for (xr = r + rules; r < xr; r++)
				if ((r->property & (P_immediate|P_target)) == (P_immediate|P_target))
					immediate(r);
			if (state.global)
				state.global--;
		}
	}
	return 1;
 badversion:
	if (strncmp(old_header.version, old_stamp.version, sizeof(old_header.version)))
	{
		/*
		 * old old versions were only numbers
		 */

		if (!isalpha(*old_header.version))
			sfsprintf(old_header.version, sizeof(old_header.version), "%d", *((long*)old_header.version));
		error(1, "%s: old format (%s) incompatible with make loader (%s)", objfile, old_header.version, old_stamp.version);
	}
	else
		error(1, "%s: old format (%s) generated on an incompatible architecture", objfile, old_header.version);
	return 0;
 badio:
	error(ERROR_SYSTEM|2, "%s: object file io error", objfile);
	goto bad;
 badmagic:
	error(1, "%s: not a %s object file", objfile, version);
 bad:
	if (p)
		free(p);
	return recompile;
}
