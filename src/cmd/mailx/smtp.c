/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*Copyright (c) 1978-2004 The Regents of the University of California an*
*                                                                      *
*            Permission is hereby granted, free of charge,             *
*         to any person obtaining a copy of THIS SOFTWARE FILE         *
*              (the "Software"), to deal in the Software               *
*                without restriction, including without                *
*             limitation the rights to use, copy, modify,              *
*                  merge, publish, distribute, and/or                  *
*              sell copies of the Software, and to permit              *
*              persons to whom the Software is furnished               *
*            to do so, subject to the following disclaimer:            *
*                                                                      *
*THIS SOFTWARE IS PROVIDED BY The Regents of the University of Californ*
*           ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,           *
*              INCLUDING, BUT NOT LIMITED TO, THE IMPLIED              *
*              WARRANTIES OF MERCHANTABILITY AND FITNESS               *
*               FOR A PARTICULAR PURPOSE ARE DISCLAIMED.               *
*IN NO EVENT SHALL The Regents of the University of California and AT&T*
*           BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,            *
*             SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES             *
*             (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT              *
*            OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,             *
*             DATA, OR PROFITS; OR BUSINESS INTERRUPTION)              *
*            HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,            *
*            WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT            *
*             (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING              *
*             IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,              *
*          EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.          *
*                                                                      *
*              Information and Software Systems Research               *
*    The Regents of the University of California and AT&T Research     *
*                           Florham Park NJ                            *
*                                                                      *
*                          Kurt Shoens (UCB)                           *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * IMAP client
 *
 * Glenn Fowler
 * AT&T Labs Research
 */

#include "mailx.h"

#if _PACKAGE_ast

#include <css.h>
#include <tm.h>

#define SMTP_READY	220
#define SMTP_OK		250
#define SMTP_START	354
#define SMTP_CLOSE	221

/*
 * send the message in fp to the SMTP server on host
 * for recipients argv ...
 * if original!=0 then it is the size of the message
 * and the original sender is retrived from the
 * message and preserved
 */

int
sendsmtp(Sfio_t* fp, char* host, char** argv, off_t original)
{
	register char*	s;
	register char*	t;
	char*		e;
	int		n;
	int		fd;
	int		r;
	off_t		z;
	Sfio_t*		sp;
	Sfio_t*		rp;
	char		buf[PATH_MAX];
	char		svc[PATH_MAX];

	/*
	 * connect to the service
	 */

	sfsprintf(svc, sizeof(svc), "/dev/tcp/%s/inet.smtp", host);
	if ((fd = csopen(&cs, svc, 0)) < 0)
	{
		note(SYSTEM, "smtp: %s: cannot connect to service", svc);
		return -1;
	}
	if (!(sp = sfnew(NiL, NiL, SF_UNBOUND, fd, SF_WRITE)) ||
	    !(rp = sfnew(NiL, NiL, SF_UNBOUND, fd, SF_READ)))
	{
		if (sp)
			sfclose(sp);
		else
			close(fd);
		note(SYSTEM, "smtp: %s: cannot buffer service", svc);
		return -1;
	}

	/*
	 * verify
	 */

	do
	{
		if (!(s = sfgetr(rp, '\n', 1)))
			goto bad_recv;
		if (strtol(s, &e, 10) != SMTP_READY)
			goto bad_prot;
	} while (*e == '-');

	/*
	 * identify
	 */

	if (!(s = state.var.domain) || !*s)
		s = state.var.hostname;
	if (sfprintf(sp, "HELO %s\r\n", s) < 0)
		goto bad_send;
	do
	{
		if (!(s = sfgetr(rp, '\n', 1)))
			goto bad_recv;
		if (strtol(s, &e, 10) != SMTP_OK)
			goto bad_prot;
	} while (*(unsigned char*)e == SMTP_OK);

	/*
	 * from
	 */

	if (original)
	{
		if (!(s = sfgetr(fp, '\n', 1)) || !strneq(s, "From ", 5))
			goto bad_mesg;
		for (s += 5; isspace(*s); s++);
		for (t = s; *t && !isspace(*t); t++);
		if (!(n = t - s))
			goto bad_mesg;
		z = sfvalue(fp);
		if (sfprintf(sp, "MAIL FROM:<%*.*s>\r\n", n, n, s) < 0)
			goto bad_send;
	}
	else
	{
		z = 0;
		if ((state.var.domain ?
		     sfprintf(sp, "MAIL FROM:<%s@%s>\r\n", state.var.user, state.var.domain) :
		     sfprintf(sp, "MAIL FROM:<%s>\r\n", state.var.user)) < 0)
			goto bad_send;
	}
	do
	{
		if (!(s = sfgetr(rp, '\n', 1)))
			goto bad_recv;
		if (strtol(s, &e, 10) != SMTP_OK)
			goto bad_prot;
	} while (*e == '-');

	/*
	 * to
	 */

	while (s = *argv++)
	{
		if ((state.var.domain && !strchr(s, '@') ?
		     sfprintf(sp, "RCPT TO:<%s@%s>\r\n", s, state.var.domain) :
		     sfprintf(sp, "RCPT TO:<%s>\r\n", s)) < 0)
			goto bad_send;
		do
		{
			if (!(s = sfgetr(rp, '\n', 1)))
				goto bad_recv;
			if (strtol(s, &e, 10) != SMTP_OK)
				goto bad_prot;
		} while (*e == '-');
	}

	/*
	 * body
	 */

	if (sfprintf(sp, "DATA\r\n") < 0)
		goto bad_send;
	do
	{
		if (!(s = sfgetr(rp, '\n', 1)))
			goto bad_recv;
		if (strtol(s, &e, 10) != SMTP_START)
			goto bad_prot;
	} while (*e == '-');
	tmfmt(buf, sizeof(buf), "%+uDate: %a, %d %b %Y %H:%M:%S UT", NiL);
	if (sfputr(sp, buf, '\n') < 0)
		goto bad_send;
	if (sfprintf(sp, "From: <%s@%s>\n", state.var.user, host) < 0)
		goto bad_send;
	while (s = sfgetr(fp, '\n', 1))
	{
		if (sfprintf(sp, "%s%s\r\n", *s == '.' ? "." : "", s) < 0)
			goto bad_send;
		if (original && (z += sfvalue(fp)) >= original)
			break;
	}
	if (sfprintf(sp, ".\r\n") < 0)
		goto bad_send;
	do
	{
		if (!(s = sfgetr(rp, '\n', 1)))
			goto bad_recv;
		if (strtol(s, &e, 10) != SMTP_OK)
			goto bad_prot;
	} while (*e == '-');

	/*
	 * quit
	 */

	if (sfprintf(sp, "QUIT\r\n") < 0)
		goto bad_send;
	do
	{
		if (!(s = sfgetr(rp, '\n', 1)))
			goto bad_recv;
		if (strtol(s, &e, 10) != SMTP_CLOSE)
			goto bad_prot;
	} while (*e == '-');
	r = 0;
	goto done;
 bad_mesg:
	note(0, "smtp: bad message -- no From header");
	goto bad;
 bad_prot:
	if ((n = strlen(e)) > 0 && e[n - 1] == '\r')
		e[n - 1] = 0;
	note(0, "smtp: %s: service error:%s", svc, e);
	goto bad;
 bad_send:
	note(SYSTEM, "smtp: %s: service write error", svc);
	goto bad;
 bad_recv:
	note(SYSTEM, "smtp: %s: service read error", svc);
 bad:
	r = -1;
 done:
	sfclose(sp);
	sfclose(rp);
	return r;
}

#else

int
sendsmtp(FILE* fp, char** argv)
{
	note(0, "smtp: support not enabled");
	return -1;
}

#endif
