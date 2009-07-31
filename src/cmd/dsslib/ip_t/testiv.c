/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 2000-2009 AT&T Intellectual Property          *
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
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#include <iv.h>
#include <ip6.h>
#include <error.h>
#include <option.h>
#include <ctype.h>

static const char usage[] =
"[-?\n@(#)$Id: testiv (AT&T Research) 2009-07-28 $\n]"
USAGE_LICENSE
"[+NAME?testiv - iv ipv6 longest prefix match test harness]"
"[+DESCRIPTION?\btestiv\b loads the ipv6 prefixes in \aprefix-file\a and "
    "then does a longest prefix match for each ipv6 address in each "
    "\aaddress-file\a. If \aaddress-file\a is omitted or \b-\b then the "
    "standard input is read. Matched addresses are listed with the min and "
    "max rangepoints of the matching interval and the string associated with "
    "the interval, one line per match. \b-\b is listed instead of "
    "rangepoints for unmatched addresses.]"
"[+?\aprefix-file\a must contain a sequence of lines of one or two space "
    "separated fields, with the \bipv6\b prefix in the first field and an "
    "optional string identifying the interval in the second field. Each "
    "\aaddress-file\a must contain a sequence of lines with the \bipv6\b "
    "address in the first space separated field.]"
"[d:dump?Dump the prefix table before matching the addresses.]"

"\n"
"\nprefix-file [ address-file ... ]\n"
"\n"

"[+SEE ALSO?\blpm\b(1), \bfv\b(3), \biv\b(3)]"
;

int
main(int argc, char** argv)
{
	char*		file;
	char*		s;
	char*		v;
	Iv_t*		iv;
	Ivseg_t*	pp;
	Sfio_t*		sp;
	unsigned char	prefix[IP6PREFIX];
	unsigned char	addr[IP6ADDR];
	Ivdisc_t	ivdisc;

	int		dump = 0;

	error_info.id = "testiv";
	ivinit(&ivdisc);
	ivdisc.errorf = errorf;
	for (;;)
	{
		switch (optget(argv, usage))
		{
		case 'd':
			dump = 1;
			continue;
		case '?':
			error(ERROR_USAGE|4, "%s", opt_info.arg);
			continue;
		case ':':
			error(2, "%s", opt_info.arg);
			continue;
		}
		break;
	}
	argv += opt_info.index;
	if (!(file = *argv++))
		error(ERROR_USAGE|4, "%s", optusage(0));
	if (!(sp = sfopen(0, file, "r")))
		error(ERROR_SYSTEM|3, "%s: cannot read prefix file", file);
	if (!(iv = ivopen(&ivdisc, ivmeth("nested"), 16, 0)))
		error(3, "cannot open nested iv table");
	while (s = sfgetr(sp, '\n', 1))
		if (strtoip6(s, &v, prefix, prefix + IP6BITS))
			error(1, "%s: invalid prefix", s);
		else
		{
			while (isspace(*v))
				v++;
			if (*v)
				v = strdup(v);
			else
				v = 0;
			if (dump)
				sfprintf(sfstderr, "set  %-42s  %-42s  %-42s  %s\n", fmtip6(fvplo(iv->size, prefix[IP6BITS], iv->r1, prefix), prefix[IP6BITS]), fmtip6(fvplo(iv->size, prefix[IP6BITS], iv->r2, prefix), -1), fmtip6(fvphi(iv->size, prefix[IP6BITS], iv->r2, prefix), -1), v);
			if (ivset(iv, fvplo(iv->size, prefix[IP6BITS], iv->r1, prefix), fvphi(iv->size, prefix[IP6BITS], iv->r2, prefix), v))
			{
				error(2, "%s: iv insertion error", s);
				break;
			}
		}
	sfclose(sp);
	file = *argv++;
	do
	{
		if (!file || streq(file, "-"))
			sp = sfstdin;
		else if (!(sp = sfopen(0, file, "r")))
			error(ERROR_SYSTEM|3, "%s: cannot read address file", file);
		while (s = sfgetr(sp, '\n', 1))
			if (strtoip6(s, 0, addr, 0))
				error(1, "%s: invalid address", s);
			else if (pp = ivseg(iv, addr))
				sfprintf(sfstdout, "%s%-42s  %-42s  %-42s  %s\n", dump ? "get  " : "", fmtip6(addr, -1), fmtip6(pp->lo, -1), fmtip6(pp->hi, -1), pp->data);
			else
				sfprintf(sfstdout, "%s%-42s  -\n", dump ? "get  " : "", fmtip6(addr, -1));
		if (sp != sfstdin)
			sfclose(sp);
	} while (file && (file = *argv++));
	ivclose(iv);
	return 0;
}
