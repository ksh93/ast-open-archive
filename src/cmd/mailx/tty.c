#pragma prototyped
/*
 * Mail -- a mail program
 *
 * Interactive header editing.
 */

#include "mailx.h"

#include <termios.h>

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
