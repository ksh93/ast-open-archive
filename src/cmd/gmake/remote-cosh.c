/*
 * coshell remote implementation for gnumake
 *
 *	Glenn Fowler
 *	The Shell as a Service
 *	USENIX Cincinnati 1993 Summer Conference Proceedings, pp. 267-277
 */

#include <ast.h>
#include <coshell.h>
#include <sig.h>

char*	remote_description = "coshell";

static struct				/* job state			*/
{
	Coshell_t*	coshell;	/* open coshell			*/
	int		id;		/* last job id			*/
	int		init;		/* tried initialization		*/
} remote;

/*
 * Call once at startup even if no commands are run.
 */

void
remote_setup()
{
}

/*
 * Return nonzero if the next job should be done remotely.
 * Return > 1 if remote will echo commands.
 */

int
start_remote_job_p()
{
	if (!remote.init)
	{
#ifdef SIGCHLD
		Handler_t	sigchld = signal(SIGCHLD, SIG_DFL);
#endif
#ifdef SIGCLD
		Handler_t	sigcld = signal(SIGCLD, SIG_DFL);
#endif

		remote.init = 1;
		remote.coshell = coopen((char*)0, CO_ANY, "label=gnumake");
#ifdef SIGCHLD
		signal(SIGCHLD, sigchld);
#endif
#ifdef SIGCLD
		signal(SIGCLD, sigcld);
#endif
	}
	return(remote.coshell ? 2 : 0);
}

/*
 * Start a remote job for TARGET running COMMAND that has been
 * blasted into ARGV.  Either COMMAND or ARGV may be used.
 * It gets standard input from STDIN_FD.  On failure, return
 * nonzero.  On success, return zero, and set *USED_STDIN to
 * nonzero if it will actually use STDIN_FD, zero if not, set
 * *ID_PTR to a unique identification, and set *IS_REMOTE to
 * zero if the job is local, nonzero if it is remote (meaning
 * *ID_PTR is a process ID).  If TRACE is nonzero then remote
 * should trace the commands, otherwise it should shutup.  If
 * local is nonzero then job has local affinity.
 */

int
start_remote_job(target, command, argv, envp, stdin_fd, is_remote, id_ptr, used_stdin, trace, local)
char*	target;
char*	command;
char**	argv;
char**	envp;
int	stdin_fd;
int*	is_remote;
int*	id_ptr;
int*	used_stdin;
int	trace;
int	local;
{
	register int		n;
	register Cojob_t*	jp;
	char			label[72];

	static char		format[] = "label=%s";

	if (!remote.coshell) return(-1);

	/*
	 * set up the label
	 */

	if ((n = strlen(target)) > sizeof(label) - sizeof(format))
		target += n - (sizeof(label) - sizeof(format));
	sprintf(label, format, target);

	/*
	 * send command to the coshell
	 */

	if (!(jp = coexec(remote.coshell, command, (trace ? 0 : CO_SILENT)|(local ? CO_LOCAL : 0), (char*)0, (char*)0, label))) return(-1);

	/*
	 * Cojob_t.local holds the external job id
	 */

	jp->local = (char*)++remote.id;

	/*
	 * finalize the return value args
	 */

	*is_remote = 1;
	*id_ptr = remote.id;
	*used_stdin = 0;
	return(0);
}

/*
 * Get the status of a dead remote child.  Block waiting for one to die
 * if BLOCK is nonzero.  Set *EXIT_CODE_PTR to the exit status, *SIGNAL_PTR
 * to the termination signal or zero if it exited normally, and *COREDUMP_PTR
 * nonzero if it dumped core.  Return the ID of the child that died,
 * 0 if we would have to block and !BLOCK, or < 0 if there were none.
 *
 * NOTE: coshell does not use a child termination signal, so
 *       remote_status() must have been called explicitly,
 *	 implying block!=0
 */

int
remote_status(exit_code_ptr, signal_ptr, coredump_ptr, block)
int*	exit_code_ptr;
int*	signal_ptr;
int*	coredump_ptr;
int	block;
{
	register Cojob_t*	jp;

	if (!remote.coshell) return(-1);
	if (block && remote.coshell->outstanding <= remote.coshell->running) return(0);
	if (!(jp = cowait(remote.coshell, (Cojob_t*)0))) return(-1);
	*exit_code_ptr = jp->status & 0177;
	*coredump_ptr = 0;
	*signal_ptr = (jp->status & 0200) ? *exit_code_ptr : 0;
	return((int)jp->local);
}

/*
 * Send signal SIG to child ID.  Return 0 if successful, -1 if not.
 */

int
remote_kill(id, sig)
int	id;
int	sig;
{
	/*
	 * kill them all, otherwise we'd have to maintain an active job list
	 */

	return(remote.coshell ? cokill(remote.coshell, (Cojob_t*)0, sig) : -1);
}

/*
 * Resolve any file system latencies on path.
 * Another example of how NFS isn't.
 */

void
remote_resolve_file(path)
char*	path;
{
	if (remote.coshell) cosync(remote.coshell, path, -1, O_RDONLY);
}

/*
 * Clean up any cached remote resources.
 */

void
remote_cleanup()
{
	if (remote.coshell)
	{
		coclose(remote.coshell);
		remote.coshell = 0;
	}
}
