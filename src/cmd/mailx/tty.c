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
 * Mail -- a mail program
 *
 * Interactive header editing.
 */

#include "mailx.h"

/*
 * tty stuff swiped from ksh
 */

#ifdef _hdr_termios
#   include	<termios.h>
#   if __sgi__ || sgi	/* special hack to eliminate ^M problem */
#	ifndef ECHOCTL
#	    define ECHOCTL	ECHOE
#	endif /* ECHOCTL */
#	ifndef CNSUSP
#	    define CNSUSP	CNSWTCH
#	endif /* CNSUSP */
#   endif /* sgi */
#   ifdef _NEXT_SOURCE
#	define _lib_tcgetattr	1
#	define _lib_tcgetpgrp	1
#   endif /* _NEXT_SOURCE */
#else
#   if defined(_sys_termios) && defined(_lib_tcgetattr)
#	include	<sys/termios.h>
#	define _hdr_termios
#   else
#	undef _sys_termios
#   endif /* _sys_termios */
#endif /* _hdr_termios */

#ifdef _hdr_termios
#   undef _hdr_sgtty
#   undef tcgetattr
#   undef tcsetattr
#   undef tcgetpgrp
#   undef tcsetpgrp
#   undef cfgetospeed
#   ifndef TCSANOW
#	define TCSANOW		TCSETS
#	define TCSADRAIN	TCSETSW
#	define TCSAFLUSH	TCSETSF
#   endif /* TCSANOW */
    /* The following corrects bugs in some implementations */
#   if defined(TCSADFLUSH) && !defined(TCSAFLUSH)
#	define TCSAFLUSH	TCSADFLUSH
#   endif /* TCSADFLUSH */
#   ifndef _lib_tcgetattr
#	undef  tcgetattr
#	define tcgetattr(fd,tty)	ioctl(fd, TCGETS, tty)
#	undef  tcsetattr
#	define tcsetattr(fd,action,tty)	ioctl(fd, action, tty)
#	undef  cfgetospeed
#	define cfgetospeed(tp)		((tp)->c_cflag & CBAUD)
#   endif /* _lib_tcgetattr */
#   undef TIOCGETC
#   ifdef SHOPT_OLDTERMIO  /* use both termios and termio */
#	ifdef _hdr_termio
#	    undef	ESCAPE
#	    include	<termio.h>
#	else
#	    ifdef _sys_termio
#		include	<sys/termio.h>
#		define _hdr_termio 1
#	    else
#		undef SHOPT_OLDTERMIO
#	    endif /* _sys_termio */
#	endif /* _hdr_termio */
#   endif /* SHOPT_OLDTERMIO */
#else
#   define cfgetospeed(tp)	((tp)->c_cflag & CBAUD)
#   undef SHOPT_OLDTERMIO
#   ifdef _hdr_termio
#	include	<termio.h>
#   else
#	ifdef _sys_termio
#	    include	<sys/termio.h>
#	    define _hdr_termio 1
#	endif /* _sys_termio */
#   endif /* _hdr_termio */
#   ifdef _hdr_termio
#	define termios termio
#	undef TIOCGETC
#	define tcgetattr(fd,tty)		ioctl(fd, TCGETA, tty)
#	define tcsetattr(fd,action,tty)	ioctl(fd, action, tty)

#	ifdef _sys_bsdtty
#	   include	<sys/bsdtty.h>
#	endif /* _sys_bsdtty */
#   else
#	ifdef _hdr_sgtty
#	    include	<sgtty.h>
#	    ifndef LPENDIN
#	        ifdef _sys_nttyio
#		    include	<sys/nttyio.h>
#	        endif /* _sys_nttyio */
#	    endif /* LPENDIN */
#	    define termios sgttyb
#	    ifdef TIOCSETN
#	 	undef TCSETAW
#	    endif /* TIOCSETN */
#	    ifdef TIOCGETP
#		define tcgetattr(fd,tty)		ioctl(fd, TIOCGETP, tty)
#		define tcsetattr(fd,action,tty)	ioctl(fd, action, tty)
#	    else
#		define tcgetattr(fd,tty)	gtty(fd, tty)
#		define tcsetattr(fd,action,tty)	stty(fd, tty)
#	    endif /* TIOCGETP */
#	endif /* _hdr_sgtty */
#   endif /* hdr_termio */

#   ifndef TCSANOW
#	ifdef TCSETAW
#	    define TCSANOW	TCSETA
#	    ifdef u370
	    /* delays are too long, don't wait for output to drain */
#		define TCSADRAIN	TCSETA
#	    else
#		define TCSADRAIN	TCSETAW
#	    endif /* u370 */
#	    define TCSAFLUSH	TCSETAF
#	else
#	    ifdef TIOCSETN
#		define TCSANOW	TIOCSETN
#		define TCSADRAIN	TIOCSETN
#		define TCSAFLUSH	TIOCSETP
#	    endif /* TIOCSETN */
#	endif /* TCSETAW */
#   endif /* TCSANOW */
#endif /* _hdr_termios */

/* set ECHOCTL if driver can echo control charaters as ^c */
#ifdef LCTLECH
#   ifndef ECHOCTL
#	define ECHOCTL	LCTLECH
#   endif /* !ECHOCTL */
#endif /* LCTLECH */
#ifdef LNEW_CTLECH
#   ifndef ECHOCTL
#	define ECHOCTL  LNEW_CTLECH
#   endif /* !ECHOCTL */
#endif /* LNEW_CTLECH */
#ifdef LNEW_PENDIN
#   ifndef PENDIN
#	define PENDIN LNEW_PENDIN
#  endif /* !PENDIN */
#endif /* LNEW_PENDIN */
#ifndef ECHOCTL
#   ifndef VEOL
#	define RAWONLY	1
#   endif /* !VEOL */
#endif /* !ECHOCTL */

/*
 * Output label on wfd and return next char on rfd with no echo.
 * Return < -1 is -(signal + 1).
 */

int
ttyquery(int rfd, int wfd, const char* label)
{
	register int	r;
	int		n;
	unsigned char	c;
	struct termios	old;
	struct termios	tty;

	if (!label)
		n = 0;
	else if (n = strlen(label))
		write(wfd, label, n);
	tcgetattr(rfd, &old);
	tty = old;
	tty.c_cc[VTIME] = 0;
	tty.c_cc[VMIN] = 1;
	tty.c_lflag &= ~(ICANON|ECHO|ECHOK|ISIG);
	tcsetattr(rfd, TCSADRAIN, &tty);
	if ((r = read(rfd, &c, 1)) == 1) {
		if (c == old.c_cc[VEOF])
			r = -1;
		else if (c == old.c_cc[VINTR])
			r = -(SIGINT + 1);
		else if (c == old.c_cc[VQUIT])
			r = -(SIGQUIT + 1);
		else if (c == '\r')
			r = '\n';
		else
			r = c;
	}
	tcsetattr(rfd, TCSADRAIN, &old);
	if (n) {
		write(wfd, "\r", 1);
		while (n-- > 0)
			write(wfd, " ", 1);
		write(wfd, "\r", 1);
	}
	return r;
}

/*
 * Edit buf on rfd,wfd with label.
 * Do not backspace over label.
 */

int
ttyedit(int rfd, int wfd, const char* label, char* buf, size_t size)
{
	register int	r;
	register int	last = strlen(buf);
	unsigned char	c;
	struct termios	old;
	struct termios	tty;

	size--;
	if (label)
		write(wfd, label, strlen(label));
	if (last)
		write(wfd, buf, last);
	tcgetattr(rfd, &old);
	tty = old;
	tty.c_cc[VTIME] = 0;
	tty.c_cc[VMIN] = 1;
	tty.c_lflag &= ~(ICANON|ECHO|ECHOK|ISIG);
	tcsetattr(rfd, TCSADRAIN, &tty);
	for (;;) {
		if ((r = read(rfd, &c, 1)) <= 0)
			break;
		if (c == old.c_cc[VERASE]) {
			if (last == 0)
				write(wfd, "\a", 1);
			else {
				write(wfd, "\b \b", 3);
				last--;
			}
		}
		else if (c == old.c_cc[VKILL]) {
			memset(buf, '\b', last);
			write(wfd, buf, last);
			memset(buf, ' ', last);
			write(wfd, buf, last);
			memset(buf, '\b', last);
			write(wfd, buf, last);
			last = 0;
		}
		else if (c == old.c_cc[VEOF]) {
			r = last;
			break;
		}
		else if (c == old.c_cc[VINTR]) {
			r = -(SIGINT + 1);
			break;
		}
		else if (c == old.c_cc[VQUIT]) {
			r = -(SIGQUIT + 1);
			break;
		}
		else if (last > size) {
			r = -1;
			break;
		}
		else {
			if (c == '\r')
				c = '\n';
			buf[last++] = c;
			write(wfd, &buf[last - 1], 1);
			if (c == '\n') {
				r = --last;
				break;
			}
			r = last;
		}
	}
	tcsetattr(rfd, TCSADRAIN, &old);
	if (r >= 0)
		buf[last] = 0;
	return r;
}

/*
 * Edit the fields in type.
 */

void
grabedit(struct header* hp, unsigned long type)
{
	register char*			s;
	register const struct lab*	lp;
	int				r;
	sig_t				saveint;
	sig_t				savequit;
	char				buf[LINESIZE];

	fflush(stdout);
	if ((saveint = signal(SIGINT, SIG_IGN)) == SIG_DFL)
		signal(SIGINT, SIG_DFL);
	if ((savequit = signal(SIGQUIT, SIG_IGN)) == SIG_DFL)
		signal(SIGQUIT, SIG_DFL);
	r = 0;
	for (lp = state.hdrtab; lp->name; lp++)
		if (type & lp->type) {
			if (!(s = detract(hp, lp->type)))
				s = "";
			if (strlen(s) >= sizeof(buf)) {
				note(0, "%sfield too long to edit", lp->name);
				continue;
			}
			strcpy(buf, s);
			if ((r = ttyedit(0, 1, lp->name, buf, sizeof(buf))) < 0)
				break;
			headclear(hp, lp->type);
			extract(hp, lp->type, buf);
		}
	if (saveint != SIG_DFL)
		signal(SIGINT, saveint);
	if (savequit != SIG_DFL)
		signal(SIGQUIT, savequit);
	if (r < -1)
		kill(0, -(r + 1));
}
