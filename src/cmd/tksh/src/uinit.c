#pragma prototyped

#include "tksh.h"

static char **av;
static int ac;

static void tksh_userinit(int subshell)
{
	char *end = av[0] + strlen(av[0]);
	int len = strlen(av[0]);
	char *args[2];

	if (subshell)
	{
		TkshSubShell();
		return;
	}

#ifndef DO_TK
	/* sfsetbuf(sfstdout, NULL, 0); */
	args[0] = av[0]; args[1] = NULL;
	if ((len >= 4) && (strcmp(end-4, "tksh") == 0))
		/* b_tkinit(0, (char **) 0, (void *) 0); */
		b_tkinit(1, args, (void *) 0);
	else if ((len >= 6) && (strcmp(end-6, "tclksh") == 0))
                /* b_tclinit(0, (char **) 0, (void *) 0); */
                b_tclinit(1, args, (void *) 0);
	else
	{
		sh_addbuiltin("tclinit", b_tclinit, (void *) 0);
		sh_addbuiltin("tkinit", b_tkinit, (void *) 0);
	}
#else
	sh_addbuiltin("tkinit", b_tkinit, (void *) 0);
	if ((len >= 6) && (strcmp(end-6, "tclksh") == 0))
                b_tclinit(0, (char **) 0, (void *) 0);
	else
		sh_addbuiltin("tclinit", b_tclinit, (void *) 0);
#endif
}

int main(int argc, char *argv[])
{
	return (sh_main(ac=argc, av=argv, tksh_userinit));
}
