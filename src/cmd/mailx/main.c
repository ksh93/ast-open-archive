#pragma prototyped
/*
 * Mail -- a mail program
 *
 * Startup -- interface with user.
 */

#include "mailx.h"

/*
 * Interrupt printing of the headers.
 */
static void
hdrstop(int sig)
{
	note(0, "\nInterrupt");
	longjmp(state.jump.header, sig);
}

/*
 * Set command line options and append to
 * op list for resetopt() after the rc's.
 */
static struct list*
setopt(register struct list* op, char* s, char* v)
{
	int		n;
	struct argvec	vec;

	n = strlen(s) + 1;
	if (v)
		n += strlen(v) + 1;
	if (!(op->next = newof(0, struct list, 1, n)))
		note(PANIC, "Out of space");
	op = op->next;
	s = strcopy(op->name, s);
	if (v) {
		*s++ = '=';
		strcpy(s, v);
	}
	state.onstack++;
	initargs(&vec);
	getargs(&vec, op->name);
	if (endargs(&vec) > 0) {
		state.cmdline = 1;
		set(vec.argv);
		state.cmdline = 0;
	}
	sreset();
	state.onstack--;
	return op;
}

/*
 * Reset the setopt() options after the rc's.
 */
static void
resetopt(register struct list* op)
{
	register struct list*	np;
	struct argvec		vec;

	np = op->next;
	while (op = np) {
		initargs(&vec);
		getargs(&vec, op->name);
		if (endargs(&vec) > 0) {
			state.cmdline = 1;
			set(vec.argv);
			state.cmdline = 0;
		}
		sreset();
		np = op->next;
		free(op);
	}
}

int
main(int argc, char** argv)
{
	register int	i;
	int		sig;
	char*		ef;
	int		flags = 0;
	sig_t		prevint;
	struct header	head;
	struct list	options;
	struct list*	op;

#if _PACKAGE_ast
	error_info.id = "mailx";
#endif

	/*
	 * Set up a reasonable environment.
	 * Figure out whether we are being run interactively,
	 * and so forth.
	 */
	memset(&head, 0, sizeof(head));
	(op = &options)->next = 0;
	if (!(state.path.buf = sfstropen()) || !(state.path.move = sfstropen()) || !(state.path.part = sfstropen()) || !(state.path.temp = sfstropen()))
		note(FATAL, "out of space");
	varinit();
	/*
	 * Now, determine how we are being used.
	 * We successively pick off - flags.
	 * If there is anything left, it is the base of the list
	 * of users to mail to.  Argp will be set to point to the
	 * first of these users.
	 */
	ef = 0;
	opterr = 0;
	for (;;) {
		switch (getopt(argc, argv, "AFHINPQ:ST:b:c:defino:s:tu:v")) {
		case EOF:
			break;
		case 'A':
			op = setopt(op, "news", NiL);
			continue;
		case 'F':
			flags |= FOLLOWUP;
			continue;
		case 'H':
			/*
			 * List all headers and exit.
			 */
			op = setopt(op, "justheaders", NiL);
			state.var.quiet = state.on;
			continue;
		case 'I':
			/*
			 * We're interactive
			 */
			op = setopt(op, "interactive", NiL);
			continue;
		case 'N':
			/*
			 * Avoid initial header printing.
			 */
			op = setopt(op, "noheader", NiL);
			state.var.quiet = state.on;
			continue;
		case 'P':
			/*
			 * Coprocess on pipe.
			 */
			op = setopt(op, "coprocess", NiL);
			continue;
		case 'Q':
			/*
			 * List all n most recent status and senders and exit.
			 */
			op = setopt(op, "justfrom", optarg);
			state.var.quiet = state.on;
			continue;
		case 'S':
			/*
			 * List all status and senders and exit.
			 */
			op = setopt(op, "justfrom", "-1");
			state.var.quiet = state.on;
			continue;
		case 'T':
			/*
			 * Next argument is temp file to write which
			 * articles have been read/deleted for netnews.
			 */
			op = setopt(op, "news", optarg);
			continue;
		case 'b':
			/*
			 * Get Blind Carbon Copy Recipient list
			 */
			extract(&head, GBCC|GMETOO, optarg);
			continue;
		case 'c':
			/*
			 * Get Carbon Copy Recipient list
			 */
			extract(&head, GCC|GMETOO, optarg);
			continue;
		case 'd':
			/*
			 * Debug output.
			 */
			op = setopt(op, "debug", NiL);
			continue;
		case 'e':
			/*
			 * Silently exit 0 if mail, 1, otherwise.
			 */
			op = setopt(op, "justcheck", NiL);
			state.var.quiet = state.on;
			continue;
		case 'f':
			/*
			 * User is specifying file to "edit" with Mail,
			 * as opposed to reading system mailbox.
			 * If no argument is given after -f, we read his
			 * mbox file.
			 *
			 * getopt() can't handle optional arguments, so here
			 * is an ugly hack to get around it.
			 */
			if (argv[optind] && argv[optind][0] != '-')
				ef = argv[optind++];
			else
				ef = "&";
			continue;
		case 'i':
			/*
			 * User wants to ignore interrupts.
			 * Set the variable "ignore"
			 */
			op = setopt(op, "ignore", NiL);
			continue;
		case 'n':
			/*
			 * User doesn't want to source state.var.master
			 */
			op = setopt(op, "nomaster", NiL);
			continue;
		case 'o':
			/*
			 * Set option(s) by name.
			 */
			op = setopt(op, optarg, NiL);
			continue;
		case 's':
			/*
			 * Give a subject field for sending from
			 * non terminal
			 */
			if (head.h_subject = optarg)
				head.h_flags |= GSUB;
			continue;
		case 't':
			/*
			 * Check for headers in message text.
			 */
			op = setopt(op, "sendheaders", optarg);
			state.mode = SEND;
			continue;
		case 'u':
			/*
			 * Next argument is person to pretend to be.
			 */
			op = setopt(op, "user", optarg);
			continue;
		case 'v':
			/*
			 * Send mailer verbose flag
			 */
			op = setopt(op, "verbose", NiL);
			continue;
		case '?':
			note(FATAL, "\
Usage: mail [-o [no]name[=value]] [-s subject] [-c cc] [-b bcc] to ...\n\
       mail [-o [no]name[=value]] [-f [folder]]");
		}
		break;
	}
	for (i = optind; (argv[i]) && (*argv[i] != '-'); i++)
		extract(&head, GTO|GMETOO, argv[i]);
	if (argv[i])
		head.h_options = argv;
	if (!state.mode)
		state.mode = (head.h_flags & GTO) ? SEND : RECEIVE;
	/*
	 * Check for inconsistent arguments.
	 */
	if (state.mode == RECEIVE && (head.h_flags & GSTD)) {
		if (!state.var.sendheaders)
			note(FATAL|IDENTIFY, "You must specify direct recipients with -s, -c, or -b");
		state.mode = SEND;
	}
	if (state.mode == RECEIVE)
		state.var.receive = state.on;
	if (state.mode == SEND && ef)
		note(FATAL|IDENTIFY, "Cannot give -f and people to send to");
	if (state.var.justcheck && state.mode == SEND)
		exit(1);
	tempinit();
	state.input = stdin;
	/*
	 * Up to this point salloc()==malloc() by default.
	 * From now on salloc() space cleared by sreset().
	 */
	state.onstack = 1;
	if (state.var.master)
		load(expand(state.var.master, 1));
	/*
	 * Expand returns a savestr, but load only uses the file name
	 * for fopen, so it's safe to do this.
	 */
	load(expand(state.var.mailrc, 1));
	/*
	 * Reset command line options so they take precedence over the rc's.
	 */
	resetopt(&options);
	if (state.mode == SEND) {
		sendmail(&head, flags);
		/*
		 * why wait?
		 */
		exit(state.senderr);
	}
	/*
	 * Ok, we are reading mail.
	 * Decide whether we are editing a mailbox or reading
	 * the system mailbox, and open up the right stuff.
	 */
	if (!ef)
		ef = "%";
	if (setfolder(ef) < 0)
		exit(1);
	if (sig = setjmp(state.jump.header))
		resume(sig);
	else {
		if ((prevint = signal(SIGINT, SIG_IGN)) != SIG_IGN)
			signal(SIGINT, hdrstop);
		if (!state.var.quiet)
			note(0, "Mail version %s.  Type ? for help", state.version);
		announce();
		fflush(stdout);
		signal(SIGINT, prevint);
	}
	if (!state.var.justheaders) {
		commands();
		signal(SIGHUP, SIG_IGN);
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		quit();
	}
	exit(0);
}
