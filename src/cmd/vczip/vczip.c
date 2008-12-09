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
/*	Command to encode and decode with Vcodex methods.
**
**	Written by Kiem-Phong Vo (09/06/2003 - ILNGUYEN)
*/

#if _PACKAGE_ast

#define UNZIP		((char*)0)
#define DFLTZIP		"sieve.delta,bwt,mtf,rle.0,huffgroup"
#define ALIASES		"lib/vcodex/aliases"
#define VCZIPRC		".vcziprc"

static const char usage[] =
"[-?\n@(#)$Id: vczip (AT&T Research) 2008-11-04 $\n]"
USAGE_LICENSE
"[+NAME?vczip - vcodex method encode/decode filter]"
"[+DESCRIPTION?\bvczip\b is a filter that decodes the standard input "
    "and/or encodes the standard output. The \b--method\b option specifies "
    "the encoding. The default encoding is \b--method=" DFLTZIP "\b. The "
    "method is automatically determined when decoding.]"
"[+?For delta methods the \asource\a operand optionally specifies the "
    "file to delta against. If \asource\a is omitted then the input file is "
    "simply compressed. Delta-encoded data must be decoded with the same "
    "\asource\a.]"
"[+?Method aliases may be defined in \b../" ALIASES "\b in one of the "
    "directories on \b$PATH\b, or in \b$HOME/" VCZIPRC "\b, searched in "
    "order. Each alias is a \aname=value\a pair where \avalue\a is a "
    "\b--method\b option value, described below. Method names are searched "
    "before alias names.]"
"[i:input?Input data is read from \afile\a instead of the standard "
    "input.]:[file]"
"[m:method|encode?Set the transformation method from the \b,\b (or \b^\b) "
    "separated list of \amethod\a[.\aarg\a]] elements, where \amethod\a is "
    "a method alias or primitive method, \aarg\a is an optional method "
    "specific argument, and \b-\b denotes the default argument. Parenthesized "
    "values are implementation details. The \alibvcodex\a (\b-catalog\b) "
    "denotes a method supplied by the default library. Otherwise the method "
    "is a separate plugin; that plugin will be required to decode any "
    "encoded data. The primitive methods and method aliasess "
    "are:]:[method[.arg]][,method[.arg]]...]]:=" DFLTZIP "]"
    "{\fmethods\f}"
"[o:output?Output data is written to \afile\a instead of the standard "
    "output.]:[file]"
"[u:undo|decode?Decode data. If method is specified then that transformation "
    "is applied; otherwise the inverse of the encoding transofrmation is "
    "applied.]:?[method[.arg]][,method[.arg]]...]]]"
"[v:verbose?List the compresses size on the standard error.]"
"[w:window?Set the data partition window size to \awindow\a. "
    "\amethod\a specifies an optional window matching "
    "method. The window methods are:]:[window[,method]]]"
    "{\fwindows\f}"
"[q:identify?Identify the standard input encoding and write the "
    "\b--method\b transformation string on the standard output. "
    "If the standard input is not vczip encoded then nothing is "
    "printed.]"
"[f:from?Convert from \acodeset\a to the \b--to\b=\acodeset\a or the "
    "native codeset if \b--to\b is omitted. The conversion is done after the "
    "\bvcodex\b(3) methods have been applied. The codesets are matched by "
    "the following left-anchored case-insensitive \bksh\b(1) "
    "patterns:]:[codeset]{\fcodesets\f}"
"[t:to?Convert to \acodeset\a from the \b--from\b=\acodeset\a or the native "
    "codeset if \b--from\b is omitted. See \b--from\b for the list of "
    "supported codesets.]:[codeset]"
"[d:vcdiff|ietf?Encode as defined in IETF RFC3284.]"
"[p:plain?Do not encode transformation information in the data. This means "
    "the transform must be explicitly supplied to decode.]"
"[M:move?Use sfmove() for io.]"

"\n"
"\n[ source ] < input > output\n"
"\n"

"[+FILES]"
    "{"
        "[+../" ALIASES "?\b--method\b \aname=value\a alias file, found "
            "on \b$PATH\b.]"
    "}"
"[+SEE ALSO?\bcodex\b(1), \bcodex\b(3), \bvcodex\b(3)]"
;

#include	<ast.h>
#include	<error.h>
#include	<ccode.h>
#include	<vcodex.h>

static int
optmethod(Void_t* obj, char* name, char* desc, Void_t* handle)
{
	Sfio_t*		sp = (Sfio_t*)handle;
	Vcmethod_t*	mt = (Vcmethod_t*)obj;
	int		i;

	sfprintf(sp, "[+%s?", name);
	optesc(sp, desc, 0);
	if(mt->args)
	{	sfprintf(sp, " The arguments are:]{");
		for(i = 0; mt->args[i].desc; i++)
		{	sfprintf(sp, "[+%s?", mt->args[i].name ? mt->args[i].name : "-");
			if(mt->args[i].desc)
				optesc(sp, mt->args[i].desc, 0);
			sfputc(sp, ']');
			if(!mt->args[i].name)
				break;
		}
	}
	else
		sfputc(sp, ']');
	if(mt->about)
	{	if(!mt->args)
			sfputc(sp, '{');
		sfprintf(sp, "%s}", mt->about);
	}
	else if(mt->args)
		sfputc(sp, '}');
	return 0;
}

static int
optalias(Void_t* obj, char* name, char* desc, Void_t* handle)
{
	Sfio_t*		sp = (Sfio_t*)handle;

	sfprintf(sp, "[+%s?Equivalent to \b%s\b.]", name, desc);
	return 0;
}

static int
optwindow(Void_t* obj, char* name, char* desc, Void_t* handle)
{
	Sfio_t*		sp = (Sfio_t*)handle;

	sfprintf(sp, "[+%s?", name);
	optesc(sp, desc, 0);
	sfprintf(sp, "]");
	return 0;
}

/*
 * optget() info discipline function
 */

static int
optinfo(Opt_t* op, Sfio_t* sp, const char* s, Optdisc_t* dp)
{
	Ccmap_t		*mp;

	switch (*s)
	{
	case 'c':
		/* codesets */
		for (mp = ccmaplist(NiL); mp; mp = ccmaplist(mp))
		{
			sfputc(sp, '[');
			sfputc(sp, '+');
			sfputc(sp, '\b');
			optesc(sp, mp->match, '?');
			sfputc(sp, '?');
			optesc(sp, mp->desc, 0);
			sfputc(sp, ']');
		}
		break;
	case 'm':
		/* primitive methods */
		vcwalkmeth(optmethod, sp);
		/* aliases */
		vcwalkalias(optalias, sp);
		break;
	case 'w':
		vcwwalkmeth(optwindow, sp);
		break;
	}
	return 0;
}

static void
vcsferror(const char* mesg)
{	
	error(2, "%s", mesg);
}

int
main(int argc, char** argv)
{
	Vcchar_t	*data, *dt;
	ssize_t		dtsz, n;
	Vcsfdata_t	sfdt;			/* data passed to vcsf	*/
	Vcsfio_t	*sfio = 0;		/* IO handle		*/
	Vcodex_t	*vcm = 0;		/* ebcdic <-> ascii	*/
	int		action;			/* default is encoding	*/
	int		move = 0;		/* sfmove()		*/
	int		type = 0;		/* type of processing	*/
	char		*trans;			/* transformation spec	*/
	char		*window = 0;		/* window specification	*/
	char		*arg;
	Sfoff_t		donez = -1, lastz = -1;	/* amount processed	*/
	int		from = CC_NATIVE;	/* from codeset		*/
	int		to = CC_NATIVE;		/* to codeset		*/
	unsigned char*	map;			/* ccode conversion map	*/
	Optdisc_t	optdisc;		/* optget() dscipline	*/

	error_info.id = (arg = strrchr(argv[0], '/')) ? (arg + 1) : argv[0];
	if (strchr(error_info.id, 'u'))
	{	action = VC_DECODE;
		trans = UNZIP;
	}
	else
	{	action = VC_ENCODE;
		trans = DFLTZIP;
	}
	optinit(&optdisc, optinfo);
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'd':
			type = VCSF_VCDIFF;
			continue;
		case 'f':
			if ((from = ccmapid(opt_info.arg)) < 0)
				error(2, "%s: unknown codeset", opt_info.arg);
			continue;
		case 'i':
			if(sfopen(sfstdin, opt_info.arg, "r") != sfstdin)
				error(ERROR_SYSTEM|3, "%s: cannot read", opt_info.arg);
			continue;
		case 'm':
			trans = opt_info.arg;
			continue;
		case 'M':
			move = 1;
			continue;
		case 'o':
			if(sfopen(sfstdout, opt_info.arg, "w") != sfstdout)
				error(ERROR_SYSTEM|3, "%s: cannot write", opt_info.arg);
			continue;
		case 'p':
			type = VCSF_PLAIN;
			continue;
		case 'q':
			action = 0;
			continue;
		case 't':
			if ((to = ccmapid(opt_info.arg)) < 0)
				error(2, "%s: unknown codeset", opt_info.arg);
			continue;
		case 'u':
			action = VC_DECODE;
			trans = opt_info.arg ? opt_info.arg : UNZIP;
			continue;
		case 'w':
			window = opt_info.arg;
			continue;
		case 'v':
			donez = lastz = 0;
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	argc -= opt_info.index;
	if (error_info.errors || argc > 3)
		error(ERROR_USAGE|4, "%s", optusage(NiL));
	if((map = ccmap(from, to)) && !(vcm = vcopen(0, Vcmap, map, 0, VC_ENCODE)) )
		error(3, "cannot open codeset conversion handle");
	if(sfsize(sfstdin) == 0) /* a potentially empty data stream */
	{	Void_t *data;

		/* see if this is just a pipe showing up initially empty */
		if(!(data = sfreserve(sfstdin, -1, SF_LOCKR)) || sfvalue(sfstdin) == 0 )
			return 0; /* empty data transforms to empty output */
		else	sfread(sfstdin, data, 0); /* reset stream for normal transformation */
	}

	/* turn off share mode to avoid peeking on unseekable devices */
	sfset(sfstdin, SF_SHARE, 0);
	sfset(sfstdout, SF_SHARE, 0);

	/* use binary mode for file I/O */
	sfopen(sfstdin, NiL, "rb");
	sfopen(sfstdout, NiL, "wb");

	/* open stream for data processing */
	sfdt.type   = type;
	sfdt.trans  = trans;
	sfdt.source = argc == 2 ? argv[1] : 0;
	sfdt.window = window;
	sfdt.errorf = vcsferror;
	sfio = vcsfio(action == VC_ENCODE ? sfstdout : sfstdin, &sfdt, action);
	if(!action)
	{	if(sfio)
			sfprintf(sfstdout, "%s\n", sfdt.trans);
		return 0;
	}
	else if(!sfio)
		error(2, "cannot push io stream discipline");
	if (move)
	{
		sfmove(sfstdin, sfstdout, SF_UNBOUND, -1);
		if (!sfeof(sfstdin))
			error(ERROR_SYSTEM|2, "%s", "read error");
		else if (sfsync(sfstdout) || sferror(sfstdout))
			error(ERROR_SYSTEM|2, "%s", "read error");
	}
	else
	{
		/* get buffer for IO */
		data = 0;
		for(dtsz = 1024*1024; dtsz > 0; dtsz /= 2)
			if((data = (Void_t*)malloc(dtsz)) )
				break;
		if(!data)
			error(2, "cannot allocate io buffer");
		for(;;)
		{	if(action == VC_DECODE) /* get a chunk of data */
				n = vcsfread(sfio, data, dtsz);
			else	n = sfread(sfstdin, data, dtsz);
			if(n <= 0)
				break;

			if(donez >= 0) /* verbose mode */
			{	if(donez >= lastz + 64*(dtsz > 1024*1024 ? dtsz : 1024*1024)) 
				{	sfprintf(sfstderr, "done %I*d\n", sizeof(donez), donez);
					lastz = donez;
				}
				donez += n;
			}
			if(!vcm) /* do any ascii <-> ebcdic mapping required */
				dt = data;
			else if((n = vcapply(vcm, data, n, &dt)) <= 0)
				error(2, "character code map error");
			if(action == VC_DECODE) /* write out the data */
				n = sfwrite(sfstdout, dt, n);
			else	n = vcsfwrite(sfio, dt, n);
			if(n <= 0)
				error(2, "write error");
		}
		vcsfclose(sfio);
	}
	return error_info.errors != 0;
}

#else

#include	"vchdr.h"

#define	PR_METHOD	((Void_t*)1)
#define	PR_ALIAS	((Void_t*)2)

#define DFLTZIP	"sieve.delta,bwt,mtf,rle.0,huffgroup"

static char	*Mesg[] =
{
	"vczip [-Arguments] [SourceFile] < Input > Output\n",

	"\nBelow are the standard 'Arguments':\n",
	"-?: This prints this message.\n",
	"-i[InputFile]: This redefines the standard 'Input' to be 'InputFile'.\n",
	"-o[OutputFile]: This redefines the standard 'Output' to be 'OutputFile'.\n",
	"-w[size[.alg]]: This argument is ignored during decoding (i.e., -u).\n",
	"    On encoding, it defines the window processing for a large input file.\n",
	"    'size' is the window size, i.e., the size to break the input file into\n",
	"	chunks for processing. Units 'k' and 'm' mean kilo and megabytes.\n",
	"    'alg' selects a windowing matching algorithm for delta compression:\n",
	"	p: matching windows by a prefix matching method (default) or\n",
	"	m: matching windows by simply mirroring file positions.\n",
	"-E[type]: This translates between EBCDIC and ASCII during encoding or\n",
	"    decoding via the given 'type'. See also the 'map' transform below.\n",
	"-vcdiff: This encodes data as defined in IETF RFC3284.\n",
	"-plain: This means that the transformed data will not have information\n",
	"    about the transformation used for encoding. So, that information\n",
	"    will have to be supplied explicitly on decoding.\n", 
	"-u[transformation]: This decodes some previously encoded data.\n",
	"    If 'transformation' is not empty, it is the transformation used\n",
	"    to encode data. The data to be decoded will be treated as if it\n",
	"    was encoded with '-plain'. That is, '-utransformation' is the same\n",
	"    as '-u -plain -mtransformation'.\n",
	"-m[transformation]: A 'transformation' is defined as a comma-separated list:\n",
	"	transform1[.arg11.arg12...],transform2[.arg21.arg22],...'\n",
	"    For example, '-mbwt,mtf,rle.0,huffgroup' defines a transformation that\n"
	"    compresses data based on the Burrows-Wheeler transform. When the first\n",
	"    transform in a transformation is a delta compressor such as 'delta' or\n",
	"    'sieve', a 'SourceFile' can help enhancing compression. In that case,\n"
	"    the same source file must be given on both encoding and decoding.\n",
	0
};

static char	*Program;	/* name of executable	*/

static void error(const char* mesg, ...)
{
	va_list	args;
	va_start(args, mesg);
	sfprintf(sfstderr,"%s: ",Program);
	sfvprintf(sfstderr,mesg,args);
	sfprintf(sfstderr,"\n");
	va_end(args);
	exit(1);
}

static void vcsferror(const char* mesg)
{	sfprintf(sfstderr,"%s: %s\n", Program, mesg);
}

static int printdesc(Void_t* obj, char* name, char* value, Void_t* type)
{
	Vcmtarg_t	*arg;

	if(type == PR_METHOD)
	{	if(!obj)
			return -1;
		sfprintf(sfstderr,"%s: %s.\n", name, value);
		for(arg = ((Vcmethod_t*)obj)->args; arg && arg->name; ++arg)
			sfprintf(sfstderr, " %12s: %s.\n", arg->name, arg->desc);
		if(arg && !arg->name && arg->desc)
			sfprintf(sfstderr, " %12s: %s.\n", "None", arg->desc);
	}
	else if(type == PR_ALIAS)
		sfprintf(sfstderr, "-%s: %s.\n", name, value);
	else	return -1;

	return 0;
}

static void printmesg()
{
	int		i;

	for(i = 0; Mesg[i]; ++i)
		sfprintf(sfstderr,"%s", Mesg[i]);

	sfprintf(sfstderr, "\nThe default transformation is %s.\n", DFLTZIP);
	sfprintf(sfstderr, "Below are short-hands for common transformations:\n");
	vcwalkalias(printdesc, PR_ALIAS);

	/* print the set of primitive methods */
	sfprintf(sfstderr, "\nBelow are the available transforms and their arguments:\n");
	vcwalkmeth(printdesc, PR_METHOD);
}

int
main(int argc, char** argv)
{
	Vcchar_t	*data, *dt;
	ssize_t		dtsz, n;
	char		buf[1024];
	Vcsfdata_t	sfdt;			/* data passed to vcsf	*/
	Vcsfio_t	*sfio = NIL(Vcsfio_t*);	/* IO handle		*/
	Vcodex_t	*eavc = NIL(Vcodex_t*);	/* ebcdic <-> ascii	*/
	int		action = VC_ENCODE;	/* default is encoding	*/
	int		type = 0;		/* type of processing	*/
	char		*trans = DFLTZIP;	/* transformation spec	*/
	char		*window = NIL(char*);	/* window specification	*/
	ssize_t		donez = -1, lastz = -1;	/* amount processed	*/

	/* get program name */
	for(Program = argv[0]+strlen(argv[0]); Program > argv[0]; --Program)
		if(Program[-1] == '/')
			break;

	/* make list of default aliases */
	vcaddalias(Dfltalias);

	for(; argc > 1 && argv[1][0] == '-'; argc--, argv++)
	{	switch(argv[1][1])
		{
		case '?':
			printmesg();
			return 0;
		case 'o':
		case 'i':
		case 'S':
			if(argv[1][1] == 'S')
				; /* state file has been made obsolete */
			else if(argv[1][2] == 0)
				error("No file was given for %s.", argv[1]);
			else if(argv[1][1] == 'i')
			{	if(sfopen(sfstdin, argv[1]+2, "r") != sfstdin)
					error("Can't open input file '%s'.", argv[1]+2);
			}
			else
			{	if(sfopen(sfstdout, argv[1]+2, "w") != sfstdout)
					error("Can't open output file '%s'.", argv[1]+2);
			}
			break;
		case 'w':
			window = argv[1]+2;
			break;
		case 'E': /* ebcdic <-> ascii translation */
			if(eavc)
				vcclose(eavc);
			if(!(eavc = vcopen(0, Vcmap, argv[1]+2, 0, VC_ENCODE)) )
				error("'%s' specifies bad translation mode.", argv[1]);
			break;
		case 'v':
			if(strcmp(argv[1]+1,"vcdiff") == 0)
				type = VCSF_VCDIFF;
			else	goto dflt_arg;
			break;
		case 'p':
			if(strcmp(argv[1]+1,"plain") == 0)
				type = VCSF_PLAIN;
			else	goto dflt_arg;
			break;
		case 'u':
			action = VC_DECODE;
			if(argv[1][2])
			{	type = VCSF_PLAIN;
				trans = argv[1]+2;
			}
			break;
		case 'm':
			trans = argv[1]+2;
			break;
		case 'V':
			donez = lastz = 0;
			break;
		default:
		dflt_arg:
			trans = vcgetalias(argv[1]+1, buf, sizeof(buf));
			if(!trans || trans == argv[1]+1)
				error("'%s' is invalid. Use '-?' for help.", argv[1]);
		}
	}

	if(strcmp(Program, "vcunzip") == 0)
		action = VC_DECODE;
	else if(strncmp(Program, "vczip", 5) != 0 )
		error("Program name should be vczip or vcunzip.");

	if(sfsize(sfstdin) == 0) /* a potentially empty data stream */
	{	Void_t *data;

		/* see if this is just a pipe showing up initially empty */
		if(!(data = sfreserve(sfstdin, -1, SF_LOCKR)) || sfvalue(sfstdin) == 0 )
			return 0; /* empty data transforms to empty output */
		else	sfread(sfstdin, data, 0); /* reset stream for normal transformation */
	}

	/* turn off share mode to avoid peeking on unseekable devices */
	sfset(sfstdin, SF_SHARE, 0);
	sfset(sfstdout, SF_SHARE, 0);

#if _WIN32 /* on Windows systems, use binary mode for file I/O */
	setmode(0, O_BINARY);
	setmode(1, O_BINARY);
#endif

	/* open stream for data processing */
	sfdt.type   = type;
	sfdt.trans  = trans;
	sfdt.source = argc == 2 ? argv[1] : NIL(char*);
	sfdt.window = window;
	sfdt.errorf = vcsferror;
	if(!(sfio = vcsfio(action == VC_ENCODE ? sfstdout : sfstdin, &sfdt, action)) )
		error("Can't set up stream to encode or decode data.");

	/* get buffer for IO */
	data = NIL(Void_t*);
	for(dtsz = 1024*1024; dtsz > 0; dtsz /= 2)
		if((data = (Void_t*)malloc(dtsz)) )
			break;
	if(!data)
		error("Can't allocate I/O buffer.");

	for(;;)
	{	if(action == VC_DECODE) /* get a chunk of data */
			n = vcsfread(sfio, data, dtsz);
		else	n = sfread(sfstdin, data, dtsz);
		if(n <= 0)
			break;

		if(donez >= 0) /* verbose mode */
		{	if(donez >= lastz + 64*(dtsz > 1024*1024 ? dtsz : 1024*1024)) 
			{	sfprintf(sfstderr, "Done %d\n", donez);
				lastz = donez;
			}
			donez += n;
		}

		if(!eavc) /* do any ascii <-> ebcdic mapping required */
			dt = data;
		else if((n = vcapply(eavc, data, n, &dt)) <= 0)
			error("Byte mapping failed.");

		if(action == VC_DECODE) /* write out the data */
			n = sfwrite(sfstdout, dt, n);
		else	n = vcsfwrite(sfio, dt, n);
		if(n <= 0)
			error("Error writing out data.");
	}
	vcsfclose(sfio);

	return 0;
}

#endif
