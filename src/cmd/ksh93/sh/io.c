/*******************************************************************
*                                                                  *
*             This software is part of the ast package             *
*                Copyright (c) 1982-2002 AT&T Corp.                *
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
*                David Korn <dgk@research.att.com>                 *
*                                                                  *
*******************************************************************/
#pragma prototyped
/*
 * Input/output file processing
 *
 *   David Korn
 *   AT&T Labs
 *   Room 2B-102
 *   Murray Hill, N. J. 07974
 *   Tel. x7975
 *   research!dgk
 *
 */

#include	"defs.h"
#include	<fcin.h>
#include	<ls.h>
#include	<stdarg.h>
#include	<ctype.h>
#include	"variables.h"
#include	"io.h"
#include	"jobs.h"
#include	"shnodes.h"
#include	"history.h"
#include	"edit.h"
#include	"timeout.h"
#include	"FEATURE/externs"
#include	"FEATURE/dynamic"
#include	"FEATURE/poll"

#ifdef	FNDELAY
#   ifdef EAGAIN
#	if EAGAIN!=EWOULDBLOCK
#	    undef EAGAIN
#	    define EAGAIN       EWOULDBLOCK
#	endif
#   else
#	define EAGAIN   EWOULDBLOCK
#   endif /* EAGAIN */
#   ifndef O_NONBLOCK
#	define O_NONBLOCK	FNDELAY
#   endif /* !O_NONBLOCK */
#endif	/* FNDELAY */

#ifndef O_SERVICE
#   define O_SERVICE	O_NOCTTY
#endif

#define RW_ALL	(S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH)

static void	*timeout;
static int	(*fdnotify)(int,int);

#if defined(_lib_socket) && defined(_sys_socket) && defined(_hdr_netinet_in)
#   include <sys/socket.h>
#   include <netdb.h>
#   include <netinet/in.h>
#   if !defined(htons) && !_lib_htons
#      define htons(x)	(x)
#   endif
#   if !defined(htonl) && !_lib_htonl
#      define htonl(x)	(x)
#   endif
    static int str2inet(const char*, struct sockaddr_in*);
#   define SOCKET	1
#else
#   undef SOCKET
#endif

struct fdsave
{
	int	orig_fd;	/* original file descriptor */
	int	save_fd;	/* saved file descriptor */
};


#ifdef SF_BUFCONST
    static int  	subexcept(Sfio_t*, int, void*, Sfdisc_t*);
    static int  	eval_exceptf(Sfio_t*, int, void*, Sfdisc_t*);
    static int  	slowexcept(Sfio_t*, int, void*, Sfdisc_t*);
    static int  	pipeexcept(Sfio_t*, int, void*, Sfdisc_t*);
    static ssize_t	piperead(Sfio_t*, void*, size_t, Sfdisc_t*);
    static ssize_t	slowread(Sfio_t*, void*, size_t, Sfdisc_t*);
    static ssize_t	subread(Sfio_t*, void*, size_t, Sfdisc_t*);
    static ssize_t	tee_write(Sfio_t*,const void*,size_t,Sfdisc_t*);
#else
    static int	subexcept(Sfio_t*, int, Sfdisc_t*);
    static int 	eval_exceptf(Sfio_t*, int, Sfdisc_t*);
    static int	slowexcept(Sfio_t*, int, Sfdisc_t*);
    static int	pipeexcept(Sfio_t*, int, Sfdisc_t*);
    static int	piperead(Sfio_t*, void*, int, Sfdisc_t*);
    static int	slowread(Sfio_t*, void*, int, Sfdisc_t*);
    static int	subread(Sfio_t*, void*, int, Sfdisc_t*);
    static int	tee_write(Sfio_t*,const void*,int,Sfdisc_t*);
#endif
static int	io_prompt(Sfio_t*,int);
static int	io_heredoc(register struct ionod*);
static void	sftrack(Sfio_t*,int,int);
static const Sfdisc_t eval_disc = { NULL, NULL, NULL, eval_exceptf, NULL};
static Sfdisc_t tee_disc = {NULL,tee_write,NULL,NULL,NULL};
static Sfio_t *subopen(Sfio_t*, off_t, long);
static const Sfdisc_t sub_disc = { subread, 0, 0, subexcept, 0 };

struct subfile
{
	Sfdisc_t	disc;
	Sfio_t		*oldsp;
	off_t		offset;
	long		size;
	long		left;
};

static struct fdsave	*filemap;
static short		filemapsize;

/* ======== input output and file copying ======== */

void sh_ioinit(void)
{
	register int n;
	filemapsize = 8;
	filemap = (struct fdsave*)malloc(8*sizeof(struct fdsave));
#ifdef SHOPT_FASTPIPE
	n = sh.lim.open_max+2;
#else
	n = sh.lim.open_max;
#endif /* SHOPT_FASTPIPE */
	sh.fdstatus = (unsigned char*)malloc((unsigned)n);
	memset((char*)sh.fdstatus,0,n);
	sh.fdptrs = (int**)malloc(n*sizeof(int*));
	memset((char*)sh.fdptrs,0,n*sizeof(int*));
	sh.sftable = (Sfio_t**)malloc(n*sizeof(Sfio_t*));
	memset((char*)sh.sftable,0,n*sizeof(Sfio_t*));
	sh.sftable[0] = sfstdin;
	sh.sftable[1] = sfstdout;
	sh.sftable[2] = sfstderr;
	sfnotify(sftrack);
	sh_iostream(0);
	/* all write steams are in the same pool and share outbuff */
	sh.outpool = sfopen(NIL(Sfio_t*),NIL(char*),"sw");  /* pool identifier */
	sh.outbuff = (char*)malloc(IOBSIZE);
	sh.errbuff = (char*)malloc(IOBSIZE/4);
	sfsetbuf(sfstderr,sh.errbuff,IOBSIZE/4);
	sfsetbuf(sfstdout,sh.outbuff,IOBSIZE);
	sfpool(sfstdout,sh.outpool,SF_WRITE);
	sfpool(sfstderr,sh.outpool,SF_WRITE);
	sfset(sfstdout,SF_LINE,0);
}

/*
 * create or initialize a stream corresponding to descriptor <fd>
 * a buffer with room for a sentinal is allocated for a read stream.
 * A discipline is inserted when read stream is a tty or a pipe
 * For output streams, the buffer is set to sh.output and put into
 * the sh.outpool synchronization pool
 */
Sfio_t *sh_iostream(register int fd)
{
	register Sfio_t *iop;
	register int status = sh_iocheckfd(fd);
	register int flags = SF_WRITE;
	char *bp;
#ifdef SHOPT_FASTPIPE
	if(fd>=sh.lim.open_max)
		return(sh.sftable[fd]);
#endif /* SHOPT_FASTPIPE */
	if(status==IOCLOSE)
	{
		switch(fd)
		{
		    case 0:
			return(sfstdin);
		    case 1:
			return(sfstdout);
		    case 2:
			return(sfstderr);
		}
		return(NIL(Sfio_t*));
	}
	if(status&IOREAD)
	{
		if(!(bp = (char *)malloc(IOBSIZE+1)))
			return(NIL(Sfio_t*));
		flags |= SF_READ;
		if(!(status&IOWRITE))
			flags &= ~SF_WRITE;
	}
	else
		bp = sh.outbuff;
	if(status&IODUP)
		flags |= SF_SHARE|SF_PUBLIC;
	if((iop = sh.sftable[fd]) && sffileno(iop)>=0)
		sfsetbuf(iop, bp, IOBSIZE);
	else if(!(iop=sfnew((fd<=2?iop:0),bp,IOBSIZE,fd,flags)))
		return(NIL(Sfio_t*));
	if(status&IOREAD)
	{
		Sfdisc_t *dp;
		sfset(iop,SF_MALLOC,1);
		{
			dp = newof(0,Sfdisc_t,1,0);
			if(status&IOTTY)
				dp->readf = slowread;
			else if(status&IONOSEEK)
				dp->readf = piperead;
			else
				dp->readf = 0;
			dp->seekf = 0;
			dp->writef = 0;
			dp->exceptf = slowexcept;
			sfdisc(iop,dp);
		}
	}
	else
		sfpool(iop,sh.outpool,SF_WRITE);
	sh.sftable[fd] = iop;
	return(iop);
}

/*
 * preserve the file descriptor or stream by moving it
 */
static void io_preserve(register Sfio_t *sp, register int f2)
{
	register int fd;
	if(sp)
		fd = sfsetfd(sp,10);
	else
		fd = sh_fcntl(f2,F_DUPFD,10);
	if(f2==sh.infd)
		sh.infd = fd;
	if(fd<0)
		errormsg(SH_DICT,ERROR_system(1),e_toomany);
	if(sh.fdptrs[fd]=sh.fdptrs[f2])
	{
		if(f2==job.fd)
			job.fd=fd;
		*sh.fdptrs[fd] = fd;
		sh.fdptrs[f2] = 0;
	}
	sh.sftable[fd] = sp;
	sh.fdstatus[fd] = sh.fdstatus[f2];
	if(fcntl(f2,F_GETFD,0)&1)
	{
		fcntl(fd,F_SETFD,FD_CLOEXEC);
		sh.fdstatus[fd] |= IOCLEX;
	}
	sh.sftable[f2] = 0;
}

/*
 * Given a file descriptor <f1>, move it to a file descriptor number <f2>
 * If <f2> is needed move it, otherwise it is closed first.
 * The original stream <f1> is closed.
 *  The new file descriptor <f2> is returned;
 */
int sh_iorenumber(register int f1,register int f2)
{
	register Sfio_t *sp = sh.sftable[f2];
	if(f1!=f2)
	{
		/* see whether file descriptor is in use */
		if(sh_inuse(f2) || (f2>2 && sp))
		{
			io_preserve(sp,f2);
			sp = 0;
		}
		else if(f2==0)
			sh.st.ioset = 1;
		sh_close(f2);
		if(f2<=2 && sp)
		{
			register Sfio_t *spnew = sh_iostream(f1);
			sh.fdstatus[f2] = (sh.fdstatus[f1]&~IOCLEX);
			sfsetfd(spnew,f2);
			sfswap(spnew,sp);
			sfset(sp,SF_SHARE|SF_PUBLIC,1);
		}
		else 
		{
			sh.fdstatus[f2] = (sh.fdstatus[f1]&~IOCLEX);
			if((f2 = sh_fcntl(f1,F_DUPFD, f2)) < 0)
				errormsg(SH_DICT,ERROR_system(1),e_file+4);
			else if(f2 <= 2)
				sh_iostream(f2);
		}
		if(sp)
			sh.sftable[f1] = 0;
		sh_close(f1);
	}
	return(f2);
}

/*
 * close a file descriptor and update stream table and attributes 
 */
int sh_close(register int fd)
{
	register Sfio_t *sp;
	register int r = 0;
	if(fd<0)
		return(-1);
	if(!(sp=sh.sftable[fd]) || sfclose(sp) < 0)
	{
		if(fdnotify)
			(*fdnotify)(fd,SH_FDCLOSE);
		r=close(fd);
	}
	if(fd>2)
		sh.sftable[fd] = 0;
	sh.fdstatus[fd] = IOCLOSE;
	if(sh.fdptrs[fd])
		*sh.fdptrs[fd] = -1;
	sh.fdptrs[fd] = 0;
	return(r);
}

/*
 * Open a file for reading
 * On failure, print message.
 */
int sh_open(register const char *path, int flags, ...)
{
	register int		fd=0;
	mode_t			mode;
#ifdef SOCKET
	struct sockaddr_in	addr;
#endif /* SOCKET */
	va_list			ap;
	va_start(ap, flags);
	mode = (flags & O_CREAT) ? va_arg(ap, int) : 0;
	va_end(ap);
	errno = 0;
	if(*path==0)
	{
		errno = ENOENT;
		return(-1);
	}
	if (strmatch(path,e_devfdNN) || (fd=strmatch(path,"/dev/std@(in|out|err)")))
	{
		if(fd)
		{
			if(path[8]=='i')
				fd = 0;
			else if(path[8]=='o')
				fd = 1;
			else if(path[8]=='e')
				fd = 2;
		}
		else
			fd = (int)strtol(path+8, (char**)0, 10);
		if((mode=sh_iocheckfd(fd))==IOCLOSE)
			return(-1);
		flags &= O_ACCMODE;
		if(!(mode&IOWRITE) && ((flags==O_WRONLY) || (flags==O_RDWR)))
			return(-1);
		if(!(mode&IOREAD) && ((flags==O_RDONLY) || (flags==O_RDWR)))
			return(-1);
		return(fd);

	}
#ifdef SOCKET
	if (strmatch(path, "/dev/@(tcp|udp)/*/*") && str2inet(path + 5, &addr))
	{
		if ((fd = socket(AF_INET, path[5] == 't' ? SOCK_STREAM : SOCK_DGRAM, 0)) >= 0)
		{
			if(flags&O_SERVICE)
			{
				if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) || listen(fd,5))
				{
					close(fd);
					fd = -1;
				}
			}
			else if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)))
			{
				close(fd);
				fd = -1;
			}
		}
	}
	else
#endif /* SOCKET */
	if((fd = open(path, flags, mode)) < 0)
		return(-1);
	flags &= O_ACCMODE;
	if(flags==O_WRONLY)
		mode = IOWRITE;
	else if(flags==O_RDWR)
		mode = (IOREAD|IOWRITE);
	else
		mode = IOREAD;
	sh.fdstatus[fd] = mode;
	return(fd);
}

int sh_chkopen(register const char *name)
{
	register int fd = sh_open(name,O_RDONLY,0);
	if(fd < 0)
		errormsg(SH_DICT,ERROR_system(1),e_open,name);
	return(fd);
}

/*
 * move open file descriptor to a number > 2
 */
int sh_iomovefd(register int fdold)
{
	register int fdnew;
	if(fdold<0 || fdold>2)
		return(fdold);
	fdnew = sh_iomovefd(dup(fdold));
	sh.fdstatus[fdnew] = (sh.fdstatus[fdold]&~IOCLEX);
	close(fdold);
	sh.fdstatus[fdold] = IOCLOSE;
	return(fdnew);
}

/*
 * create a pipe and print message on failure
 */
int	sh_pipe(register int pv[])
{
	int fd[2];
	if(pipe(fd)<0 || (pv[0]=fd[0])<0 || (pv[1]=fd[1])<0)
		errormsg(SH_DICT,ERROR_system(1),e_pipe);
	pv[0] = sh_iomovefd(pv[0]);
	pv[1] = sh_iomovefd(pv[1]);
	sh.fdstatus[pv[0]] = IONOSEEK|IOREAD|IODUP;
	sh.fdstatus[pv[1]] = IONOSEEK|IOWRITE;
	sh_subsavefd(pv[0]);
	sh_subsavefd(pv[1]);
	return(0);
}

/*
 * close a pipe
 */
void sh_pclose(register int pv[])
{
	if(pv[0]>=2)
		sh_close(pv[0]);
	if(pv[1]>=2)
		sh_close(pv[1]);
	pv[0] = pv[1] = -1;
}

/*
 * I/O redirection
 * flag = 0 if files are to be restored
 * flag = 2 if files are to be closed on exec
 * flag = 3 when called from $( < ...), just open file and return
 */
int	sh_redirect(struct ionod *iop, int flag)
{
	register char *fname;
	register int 	fd, iof;
	const char *message = ERROR_dictionary(e_open);
	int o_mode;		/* mode flag for open */
	static char io_op[5];	/* used for -x trace info */
	static int	inuse_bits;	/* keep track of 3-9 in use */
	int clexec=0, fn, traceon;
	int indx = sh.topfd;
	if(flag==2)
		clexec = 1;
	if(iop)
		traceon = sh_trace(NIL(char**),0);
	for(;iop;iop=iop->ionxt)
	{
		iof=iop->iofile;
		fn = (iof&IOUFD);
		fname=iop->ioname;
		io_op[0] = '0'+(iof&IOUFD);
		if(iof&IOPUT)
		{
			io_op[1] = '>';
			o_mode = O_WRONLY|O_CREAT;
		}
		else
		{
			io_op[1] = '<';
			o_mode = O_RDONLY|O_NONBLOCK;
		}
		io_op[2] = 0;
		io_op[3] = 0;
		fname = iop->ioname;
		if(!(iof&IORAW))
			fname=sh_mactrim(fname,(!sh_isoption(SH_NOGLOB)&&sh_isoption(SH_INTERACTIVE))?2:0);
		errno=0;
		if(*fname)
		{
			if(iof&IODOC)
			{
				if(traceon)
				{
					io_op[2] = '<';
					sfputr(sfstderr,io_op,'\n');
				}
				fd = io_heredoc(iop);
				fname = 0;
			}
			else if(iof&IOMOV)
			{
				int dupfd,toclose= -1;
				io_op[2] = '&';
				if((fd=fname[0])>='0' && fd<='9')
				{
					char *number = fname;
					dupfd = strtol(fname,&number,10);
					if(*number=='-')
					{
						toclose = dupfd;
						number++;
					}
					if(*number || dupfd > IOUFD)
					{
						message = ERROR_dictionary(e_file);
						goto fail;
					}
					if(sh.subshell && dupfd==1)
					{
						sh_subtmpfile();
						dupfd = sffileno(sfstdout);
					}
				}
				else if(fd=='-' && fname[1]==0)
				{
					fd= -1;
					goto trace;
				}
				else if(fd=='p' && fname[1]==0)
				{
					if(iof&IOPUT)
						toclose = dupfd = sh.coutpipe;
					else
						toclose = dupfd = sh.cpipe[0];
				}
				else
				{
					message = ERROR_dictionary(e_file);
					goto fail;
				}
				if((fd=sh_fcntl(dupfd,F_DUPFD,3))<0)
					goto fail;
				sh_iocheckfd(dupfd);
				sh.fdstatus[fd] = (sh.fdstatus[dupfd]&~IOCLEX);
				if(toclose<0 && sh.fdstatus[fd]&IOREAD)
					sh.fdstatus[fd] |= IODUP;
				else if(dupfd==sh.cpipe[0])
					sh_pclose(sh.cpipe);
				else if(toclose>=0)
				{
					if(flag==0)
						sh_iosave(toclose,indx); /* save file descriptor */
					sh_close(toclose);
				}
			}
			else if(iof&IORDW)
			{
				io_op[2] = '>';
				o_mode = O_RDWR|O_CREAT;
				goto openit;
			}
			else if(!(iof&IOPUT))
				fd=sh_chkopen(fname);
			else if(sh_isoption(SH_RESTRICTED))
				errormsg(SH_DICT,ERROR_exit(1),e_restricted,fname);
			else
			{
				if(iof&IOAPP)
				{
					io_op[2] = '>';
					o_mode |= O_APPEND;
				}
				else
				{
					o_mode |= O_TRUNC;
					if(iof&IOCLOB)
						io_op[2] = '|';
					else if(sh_isoption(SH_NOCLOBBER))
					{
						struct stat sb;
						if(stat(fname,&sb)>=0)
						{
#ifdef SHOPT_FS_3D
							if(S_ISREG(sb.st_mode)&&
						                (!sh.lim.fs3d || iview(&sb)==0))
#else
							if(S_ISREG(sb.st_mode))
#endif /* SHOPT_FS_3D */
							{
								errno = EEXIST;
								errormsg(SH_DICT,ERROR_system(1),e_exists,fname);
							}
						}
						else
							o_mode |= O_EXCL;
					}
				}
			openit:
				if((fd=sh_open(fname,o_mode,RW_ALL)) <0)
					errormsg(SH_DICT,ERROR_system(1),((o_mode&O_CREAT)?e_create:e_open),fname);
			}
		trace:
			if(traceon && fname)
				sfprintf(sfstderr,"%s %s%c",io_op,fname,iop->ionxt?' ':'\n');
			if(flag==0 || sh_subsavefd(fn))
				sh_iosave(fn,indx); /* save file descriptor */
			if(fd<0)
			{
				if(sh_inuse(fn) || fn==sh.infd)
				{
					if(fn>9 || !(inuse_bits&(1<<fn)))
						io_preserve(sh.sftable[fn],fn);
				}
				sh_close(fn);
				if(fn<10)
					inuse_bits &= ~(1<<fn);
			}
			if(flag==3)
				return(fd);
			if(fd>=0)
			{
				if(fn>2 && fn<10)
				{
					if(inuse_bits&(1<<fn))
						sh_close(fn);
					inuse_bits |= (1<<fn);
				}
				fd = sh_iorenumber(sh_iomovefd(fd),fn);
			}
			if(fd >2 && clexec)
			{
				fcntl(fd,F_SETFD,FD_CLOEXEC);
				sh.fdstatus[fd] |= IOCLEX;
			}
		}
		else 
			goto fail;
	}
	return(indx);
fail:
	errormsg(SH_DICT,ERROR_system(1),message,fname);
	/* NOTREACHED */
	return(0);
}
/*
 * Create a tmp file for the here-document
 */
static int io_heredoc(register struct ionod *iop)
{
	register Sfio_t	*infile = 0, *outfile;
	register char		fd;
	if(!(iop->iofile&IOSTRG) && (!sh.heredocs || iop->iosize==0))
		return(sh_open(e_devnull,O_RDONLY));
	/* create an unnamed temporary file */
	if(!(outfile=sftmp(0)))
		errormsg(SH_DICT,ERROR_system(1),e_tmpcreate);
	if(!(iop->iofile&IOSTRG))
		infile = subopen(sh.heredocs,iop->iooffset,iop->iosize);
	if(iop->iofile&IOQUOTE)
	{
		/* This is a quoted here-document, not expansion */
		if(!infile)
			infile = sfopen(NIL(Sfio_t*),iop->ioname,"s");
		sfmove(infile,outfile,SF_UNBOUND,-1);
		sfclose(infile);
	}
	else
	{
		char *lastpath = sh.lastpath;
		if(sh_isoption(SH_XTRACE))
			sfdisc(outfile,&tee_disc);
		sh_machere(infile,outfile,iop->ioname);
		sh.lastpath = lastpath;
		if(infile)
			sfclose(infile);
	}
	/* close stream outfile, but save file descriptor */
	fd = sffileno(outfile);
	sfsetfd(outfile,-1);
	sfclose(outfile);
	lseek(fd,(off_t)0,SEEK_SET);
	sh.fdstatus[fd] = IOREAD;
	return(fd);
}

/*
 * This write discipline also writes the output on standard error
 * This is used when tracing here-documents
 */
#ifdef SF_BUFCONST
static ssize_t tee_write(Sfio_t *iop,const void *buff,size_t n,Sfdisc_t *unused)
#else
static int tee_write(Sfio_t *iop,const void *buff,int n,Sfdisc_t *unused)
#endif
{
	NOT_USED(unused);
	sfwrite(sfstderr,buff,n);
	return(write(sffileno(iop),buff,n));
}

/*
 * copy file <origfd> into a save place
 * The saved file is set close-on-exec
 * if <origfd> < 0, then -origfd is saved, but not duped so that it
 *   will be closed with sh_iorestore.
 */
void sh_iosave(register int origfd, int oldtop)
{
/*@
	assume oldtop>=0 && oldtop<sh.lim.open_max;
@*/
 
	register int	savefd;
	/* see if already saved, only save once */
	for(savefd=sh.topfd; --savefd>=oldtop; )
	{
		if(filemap[savefd].orig_fd == origfd)
			return;
	}
	/* make sure table is large enough */
	if(sh.topfd >= filemapsize)
	{
		filemapsize += 8;
		if(!(filemap = (struct fdsave*)realloc(filemap,filemapsize*sizeof(struct fdsave))))
			errormsg(SH_DICT,ERROR_exit(4),e_nospace);
			
	}
#ifdef SHOPT_DEVFD
	if(origfd <0)
	{
		savefd = origfd;
		origfd = -origfd;
	}
	else
#endif /* SHOPT_DEVFD */
	{
		if((savefd = sh_fcntl(origfd, F_DUPFD, 3)) < 0 && errno!=EBADF)
			errormsg(SH_DICT,ERROR_system(1),e_toomany);
	}
	filemap[sh.topfd].orig_fd = origfd;
	filemap[sh.topfd++].save_fd = savefd;
	if(savefd >=0)
	{
		register Sfio_t* sp = sh.sftable[origfd];
		/* make saved file close-on-exec */
		sh_fcntl(savefd,F_SETFD,FD_CLOEXEC);
		if(origfd==job.fd)
			job.fd = savefd;
		sh.fdstatus[savefd] = sh.fdstatus[origfd];
		sh.fdptrs[savefd] = &filemap[sh.topfd-1].save_fd;
		if(!(sh.sftable[savefd]=sp))
			return;
		sfsync(sp);
		if(origfd <=2)
		{
			/* copy standard stream to new stream */
			sp = sfswap(sp,NIL(Sfio_t*));
			sh.sftable[savefd] = sp;
		}
		else
			sh.sftable[origfd] = 0;
	}
}

/*
 *  close all saved file descriptors
 */
void	sh_iounsave(void)
{
	register int fd, savefd, newfd;
	for(newfd=fd=0; fd < sh.topfd; fd++)
	{
		if((savefd = filemap[fd].save_fd)< 0)
			filemap[newfd++] = filemap[fd];
		else
		{
			sh.sftable[savefd] = 0;
			sh_close(savefd);
		}
	}
	sh.topfd = newfd;
}

/*
 *  restore saved file descriptors from <last> on
 */
void	sh_iorestore(int last)
{
	register int 	origfd, savefd, fd;
	for (fd = sh.topfd - 1; fd >= last; fd--)
	{
		origfd = filemap[fd].orig_fd;
		sh_close(origfd);
		if ((savefd = filemap[fd].save_fd) >= 0)
		{
			sh_fcntl(savefd, F_DUPFD, origfd);
			if(savefd==job.fd)
				job.fd=origfd;
			sh.fdstatus[origfd] = sh.fdstatus[savefd];
			/* turn off close-on-exec if flag if necessary */
			if(sh.fdstatus[origfd]&IOCLEX)
				fcntl(origfd,F_SETFD,FD_CLOEXEC);
			if(origfd<=2)
			{
				sfswap(sh.sftable[savefd],sh.sftable[origfd]);
				if(origfd==0)
					sh.st.ioset = 0;
			}
			else
				sh.sftable[origfd] = sh.sftable[savefd];
			sh.sftable[savefd] = 0;
			sh_close(savefd);
		}
	}
	if(last < sh.topfd)
		sh.topfd = last;
}

/*
 * returns access information on open file <fd>
 * returns -1 for failure, 0 for success
 * <mode> is the same as for access()
 */
sh_ioaccess(int fd,register int mode)
{
	register int flags;
	if(mode==X_OK)
		return(-1);
	if((flags=sh_iocheckfd(fd))!=IOCLOSE)
	{
		if(mode==F_OK)
			return(0);
		if(mode==R_OK && (flags&IOREAD))
			return(0);
		if(mode==W_OK && (flags&IOWRITE))
			return(0);
	}
	return(-1);
}

#ifdef SOCKET

#ifndef INADDR_LOOPBACK
#define INADDR_LOOPBACK		0x7f000001L
#endif

/*
 * convert string to sockaddr_in
 * 0 returned on error
 */

static int str2inet(register const char *sp, struct sockaddr_in *addr)
{
	char		*proto = (char*)sp;
	register int	n=0,c,v;
	unsigned long	a=0;
	unsigned short	p=0;

	sp += 4;
	if(memcmp(sp,"local/",6)==0)
	{
		a = INADDR_LOOPBACK;
		n=4;
		sp+=6;
	}
	else if(!isdigit(*sp))
	{
		struct hostent *hp;
		char *cp = strchr(sp,'/');
		*cp = 0;
		hp = gethostbyname(sp);
		*cp = '/';
		if(!hp || hp->h_addrtype!=AF_INET || hp->h_length>sizeof(struct in_addr))
			return 0;
		a = (unsigned long)((struct in_addr*)hp->h_addr)->s_addr;
		n=6;
		sp = cp+1;
		if(!isdigit(*sp))
		{
			struct servent *xp;
			proto[3] = 0;
			if(xp = getservbyname(sp,proto))
			{
				p = xp->s_port;
				sp = "";
			}
			proto[3] = '/';
		}
	}
	while(*sp)
	{
		v = 0;
		while ((c = *sp++) >= '0' && c <= '9')
			v = v * 10 + c - '0';
		if (++n <= 4) a = (a << 8) | (v & 0xff);
		else
		{
			if (c) return(0);
			if (n <= 5)
				a = htonl(a);
			p = htons(v);
			break;
		}
		if (c != '.' && c != '/') return(0);
	}
	memset((char*)addr, 0, sizeof(*addr));
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = a;
	addr->sin_port = p;
	return(1);
}
#endif /* SOCKET */


/*
 *  Handle interrupts for slow streams
 */
#ifdef SF_BUFCONST
static int slowexcept(register Sfio_t *iop,int type,void *data,Sfdisc_t *handle)
#else
static int slowexcept(register Sfio_t *iop, int type, Sfdisc_t *handle)
#endif
{
	register int	n,fno;
	NOT_USED(handle);
	if(type==SF_DPOP || type==SF_FINAL)
		free((void*)handle);
	if(type!=SF_READ)
		return(0);
	if((sh.trapnote&(SH_SIGSET|SH_SIGTRAP)) && errno!=EIO && errno!=ENXIO)
		errno = EINTR;
	if((n=sfvalue(iop))<=0)
	{
		fno = sffileno(iop);
#ifndef FNDELAY
#   ifdef O_NDELAY
		if(errno==0 && (n=fcntl(fno,F_GETFL,0))&O_NDELAY)
		{
			n &= ~O_NDELAY;
			fcntl(fno, F_SETFL, n);
			return(1);
		}
#   endif /* O_NDELAY */
#endif /* !FNDELAY */
#ifdef O_NONBLOCK
		if(errno==EAGAIN)
		{
			n = fcntl(fno,F_GETFL,0);
			n &= ~O_NONBLOCK;
			fcntl(fno, F_SETFL, n);
			return(1);
		}
#endif /* O_NONBLOCK */
		if(errno!=EINTR)
			return(0);
		n=1;
	}
	errno = 0;
	if(sh.trapnote&SH_SIGSET)
	{
		sfputc(sfstderr,'\n');
		sh_exit(SH_EXITSIG);
	}
	if(sh.trapnote&SH_SIGTRAP)
		sh_chktrap();
	return(n);
}

/*
 * called when slowread times out
 */
static void time_grace(void *handle)
{
	NOT_USED(handle);
	timeout = 0;
	if(sh_isstate(SH_GRACE))
	{
		sh_offstate(SH_GRACE);
		if(!sh_isstate(SH_INTERACTIVE))
			return;
		((struct checkpt*)sh.jmplist)->mode = SH_JMPEXIT;
		errormsg(SH_DICT,2,e_timeout);
		sh.trapnote |= SH_SIGSET;
		return;
	}
	errormsg(SH_DICT,0,e_timewarn);
	sh_onstate(SH_GRACE);
	sigrelease(SIGALRM);
	sh.trapnote |= SH_SIGTRAP;
}

#ifdef SF_BUFCONST
static ssize_t piperead(Sfio_t *iop,void *buff,register size_t size,Sfdisc_t *handle)
#else
static int piperead(Sfio_t *iop,void *buff,register int size,Sfdisc_t *handle)
#endif
{
	int fd = sffileno(iop);
	NOT_USED(handle);
	if(sh_isstate(SH_INTERACTIVE) && io_prompt(iop,sh.nextprompt)<0 && errno==EIO)
		return(0);
	if(!(sh.fdstatus[sffileno(iop)]&IOCLEX) && sfset(iop,0,0)&SF_SHARE)
		size = ed_read(fd, (char*)buff, size);
	else
		size = read(fd, (char*)buff, size);
	return(size);
}
/*
 * This is the read discipline that is applied to slow devices
 * This routine takes care of prompting for input
 */
#ifdef SF_BUFCONST
static ssize_t slowread(Sfio_t *iop,void *buff,register size_t size,Sfdisc_t *handle)
#else
static int slowread(Sfio_t *iop,void *buff,register int size,Sfdisc_t *handle)
#endif
{
	int	(*readf)(int, char*, int);
	NOT_USED(handle);
	if(io_prompt(iop,sh.nextprompt)<0 && errno==EIO)
		return(0);
	if(sh.timeout)
		timeout = (void*)sh_timeradd(sh_isstate(SH_GRACE)?1000L*TGRACE:1000L*sh.timeout,0,time_grace,NIL(void*));
#   ifdef SHOPT_ESH
	if(sh_isoption(SH_EMACS|SH_GMACS))
		readf = ed_emacsread;
	else
#   endif	/* SHOPT_ESH */
#   ifdef SHOPT_VSH
#	ifdef SHOPT_RAWONLY
	    if(sh_isoption(SH_VI) || ((SHOPT_RAWONLY-0) && mbwide()))
#	else
	    if(sh_isoption(SH_VI))
#	endif
		readf = ed_viread;
	else
#   endif	/* SHOPT_VSH */
		readf = ed_read;
	size = (*readf)(sffileno(iop), (char*)buff, size);
	if(timeout)
		timerdel(timeout);
	timeout=0;
	return(size);
}

/*
 * check and return the attributes for a file descriptor
 */

int sh_iocheckfd(register int fd)
{
	register int flags, n;
	if((n=sh.fdstatus[fd])&IOCLOSE)
		return(n);
	if(!(n&(IOREAD|IOWRITE)))
	{
#ifdef F_GETFL
		if((flags=fcntl(fd,F_GETFL,0)) < 0)
			return(sh.fdstatus[fd]=IOCLOSE);
		if((flags&O_ACCMODE)!=O_WRONLY)
			n |= IOREAD;
		if((flags&O_ACCMODE)!=O_RDONLY)
			n |= IOWRITE;
#else
		struct stat statb;
		if((flags = fstat(fd,&statb))< 0)
			return(sh.fdstatus[fd]=IOCLOSE);
		n |= (IOREAD|IOWRITE);
		if(read(fd,"",0) < 0)
			n &= ~IOREAD;
#endif /* F_GETFL */
	}
	if(!(n&(IOSEEK|IONOSEEK)))
	{
		struct stat statb;
		/* /dev/null check is a workaround for select bug */
		static ino_t null_ino;
		static dev_t null_dev;
		if(null_ino==0 && stat(e_devnull,&statb) >=0)
		{
			null_ino = statb.st_ino;
			null_dev = statb.st_dev;
		}
		if(tty_check(fd))
			n |= IOTTY;
		if(lseek(fd,NIL(off_t),SEEK_CUR)<0)
		{
			n |= IONOSEEK;
#ifdef S_ISSOCK
			if((fstat(fd,&statb)>=0) && S_ISSOCK(statb.st_mode))
				n |= IOREAD|IOWRITE;
#endif /* S_ISSOCK */
		}
		else if((fstat(fd,&statb)>=0) && (
			S_ISFIFO(statb.st_mode) ||
#ifdef S_ISSOCK
			S_ISSOCK(statb.st_mode) ||
#endif /* S_ISSOCK */
			/* The following is for sockets on the sgi */
			(statb.st_ino==0 && (statb.st_mode & ~(S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH|S_IXUSR|S_IXGRP|S_IXOTH|S_ISUID|S_ISGID))==0) ||
			(S_ISCHR(statb.st_mode) && (statb.st_ino!=null_ino || statb.st_dev!=null_dev))
		))
			n |= IONOSEEK;
		else
			n |= IOSEEK;
	}
	sh.fdstatus[fd] = n;
	return(n);
}

/*
 * Display prompt PS<flag> on standard error
 */

static int	io_prompt(Sfio_t *iop,register int flag)
{
	register char *cp;
	char buff[1];
	char *endprompt;
	static short cmdno;
	int sfflags;
	if(flag<3 && !sh_isstate(SH_INTERACTIVE))
		flag = 0;
	if(flag==2 && sfpkrd(sffileno(iop),buff,1,'\n',0,1) >= 0)
		flag = 0;
	if(flag==0)
		return(sfsync(sfstderr));
	sfflags = sfset(sfstderr,SF_SHARE|SF_PUBLIC|SF_READ,0);
	if(!(sh.prompt=(char*)sfreserve(sfstderr,0,0)))
		sh.prompt = "";
	switch(flag)
	{
		case 1:
		{
			register int c;
#if defined(TIOCLBIC) && defined(LFLUSHO)
			if(!sh_isoption(SH_VI|SH_EMACS|SH_GMACS))
			{
				/*
				 * re-enable output in case the user has
				 * disabled it.  Not needed with edit mode
				 */
				int mode = LFLUSHO;
				ioctl(sffileno(sfstderr),TIOCLBIC,&mode);
			}
#endif	/* TIOCLBIC */
			cp = sh_mactry(nv_getval(nv_scoped(PS1NOD)));
			for(;c= *cp;cp++)
			{
				if(c==HIST_CHAR)
				{
					/* look at next character */
					c = *++cp;
					/* print out line number if not !! */
					if(c!= HIST_CHAR)
					{
						sfprintf(sfstderr,"%d", sh.hist_ptr?(int)sh.hist_ptr->histind:++cmdno);
					}
					if(c==0)
						goto done;
				}
				sfputc(sfstderr,c);
			}
			goto done;
		}
		case 2:
			cp = nv_getval(nv_scoped(PS2NOD));
			break;
		case 3:
			cp = nv_getval(nv_scoped(PS3NOD));
			break;
		default:
			goto done;
	}
	if(cp)
		sfputr(sfstderr,cp,-1);
done:
	if(*sh.prompt && (endprompt=(char*)sfreserve(sfstderr,0,0)))
		*endprompt = 0;
	sfset(sfstderr,sfflags&SF_READ|SF_SHARE|SF_PUBLIC,1);
	return(sfsync(sfstderr));
}

/*
 * This discipline is inserted on write pipes to prevent SIGPIPE
 * from causing an infinite loop
 */
#ifdef SF_BUFCONST
static int pipeexcept(Sfio_t* iop, int mode, void *data, Sfdisc_t* handle)
#else
static int pipeexcept(Sfio_t* iop, int mode, Sfdisc_t* handle)
#endif
{
	NOT_USED(iop);
	if(mode==SF_DPOP || mode==SF_FINAL)
		free((void*)handle);
	else if(mode==SF_WRITE && errno==EINTR && sh.lastsig==SIGPIPE)
		return(-1);
	return(0);
}

/*
 * keep track of each stream that is opened and closed
 */
static void	sftrack(Sfio_t* sp,int flag, int newfd)
{
	register int fd = sffileno(sp);
	register struct checkpt *pp;
	register int mode;
	if(flag==SF_SETFD || flag==SF_CLOSING)
	{
		if(newfd<0)
			flag = SF_CLOSING;
		if(fdnotify)
			(*fdnotify)(sffileno(sp),flag==SF_CLOSING?-1:newfd);
	}
#ifdef DEBUG
	if(flag==SF_READ || flag==SF_WRITE)
	{
		char *z = fmtbase((long)getpid(),0,0);
		write(ERRIO,z,strlen(z));
		write(ERRIO,": ",2);
		write(ERRIO,"attempt to ",11);
		if(flag==SF_READ)
			write(ERRIO,"read from",9);
		else
			write(ERRIO,"write to",8);
		write(ERRIO," locked stream\n",15);
		return;
	}
#endif
	if((unsigned)fd >= sh.lim.open_max)
		return;
	if(sh_isstate(SH_NOTRACK))
		return;
	mode = sfset(sp,0,0);
	if(sp==sh.heredocs && fd < 10 && flag==SF_NEW)
	{
		fd = sfsetfd(sp,10);
		fcntl(fd,F_SETFD,FD_CLOEXEC);
	}
	if(fd < 3)
		return;
	if(flag==SF_NEW)
	{
		if(!sh.sftable[fd] && sh.fdstatus[fd]==IOCLOSE)
		{
			sh.sftable[fd] = sp;
			flag = (mode&SF_WRITE)?IOWRITE:0;
			if(mode&SF_READ)
				flag |= IOREAD;
			sh.fdstatus[fd] = flag;
#if 0
			if(flag==IOWRITE)
				sfpool(sp,sh.outpool,SF_WRITE);
			else
#else
			if(flag!=IOWRITE)
#endif
				sh_iostream(fd);
		}
		if((pp=(struct checkpt*)sh.jmplist) && pp->mode==SH_JMPCMD)
		{
			struct openlist *item;
			/*
			 * record open file descriptors so they can
			 * be closed in case a longjmp prevents
			 * built-ins from cleanup
			 */
			item = new_of(struct openlist, 0);
			item->strm = sp;
			item->next = pp->olist;
			pp->olist = item;
		}
		if(fdnotify)
			(*fdnotify)(-1,sffileno(sp));
	}
	else if(flag==SF_CLOSING || (flag==SF_SETFD  && newfd<=2))
	{
		sh.sftable[fd] = 0;
		sh.fdstatus[fd]=IOCLOSE;
		if(pp=(struct checkpt*)sh.jmplist)
		{
			struct openlist *item;
			for(item=pp->olist; item; item=item->next)
			{
				if(item->strm == sp)
				{
					item->strm = 0;
					break;
				}
			}
		}
	}
}

struct eval
{
	Sfdisc_t	disc;
	char		**argv;
	short		slen;
	char		addspace;
};

/*
 * Create a stream consisting of a space separated argv[] list 
 */

Sfio_t *sh_sfeval(register char *argv[])
{
	register Sfio_t *iop;
	register char *cp;
	if(argv[1])
		cp = "";
	else
		cp = argv[0];
	iop = sfopen(NIL(Sfio_t*),(char*)cp,"s");
	if(argv[1])
	{
		register struct eval *ep;
		if(!(ep = new_of(struct eval,0)))
			return(NIL(Sfio_t*));
		ep->disc = eval_disc;
		ep->argv = argv;
		ep->slen  = -1;
		ep->addspace  = 0;
		sfdisc(iop,&ep->disc);
	}
	return(iop);
}

/*
 * This code gets called whenever an end of string is found with eval
 */

#ifdef SF_BUFCONST
static int eval_exceptf(Sfio_t *iop,int type, void *data, Sfdisc_t *handle)
#else
static int eval_exceptf(Sfio_t *iop,int type, Sfdisc_t *handle)
#endif
{
	register struct eval *ep = (struct eval*)handle;
	register char	*cp;
	register int	len;

	/* no more to do */
	if(type!=SF_READ || !(cp = ep->argv[0]))
	{
		if(type==SF_CLOSING)
			sfdisc(iop,SF_POPDISC);
		else if(ep && (type==SF_DPOP || type==SF_FINAL))
			free((void*)ep);
		return(0);
	}

	if(!ep->addspace)
	{
		/* get the length of this string */
		ep->slen = len = strlen(cp);
		/* move to next string */
		ep->argv++;
	}
	else /* insert space between arguments */
	{
		len = 1;
		cp = " ";
	}
	/* insert the new string */
	sfsetbuf(iop,cp,len);
	ep->addspace = !ep->addspace;
	return(1);
}

/*
 * This routine returns a stream pointer to a segment of length <size> from
 * the stream <sp> starting at offset <offset>
 * The stream can be read with the normal stream operations
 */

static Sfio_t *subopen(Sfio_t* sp, off_t offset, long size)
{
	register struct subfile *disp;
	if(sfseek(sp,offset,SEEK_SET) <0)
		return(NIL(Sfio_t*));
	if(!(disp = (struct subfile*)malloc(sizeof(struct subfile)+IOBSIZE+1)))
		return(NIL(Sfio_t*));
	disp->disc = sub_disc;
	disp->oldsp = sp;
	disp->offset = offset;
	disp->size = disp->left = size;
	sp = sfnew(NIL(Sfio_t*),(char*)(disp+1),IOBSIZE,sh.lim.open_max,SF_READ);
	sfdisc(sp,&disp->disc);
	return(sp);
}

/*
 * read function for subfile discipline
 */
#ifdef SF_BUFCONST
static ssize_t subread(Sfio_t* sp,void* buff,register size_t size,Sfdisc_t* handle)
#else
static int subread(Sfio_t* sp,void* buff,register int size,Sfdisc_t* handle)
#endif
{
	register struct subfile *disp = (struct subfile*)handle;
	NOT_USED(sp);
	if(disp->left == 0)
		return(0);
	if(size > disp->left)
		size = disp->left;
	disp->left -= size;
	return(sfread(disp->oldsp,buff,size));
}

/*
 * exception handler for subfile discipline
 */
#ifdef SF_BUFCONST
static int subexcept(Sfio_t* sp,register int mode, void *data, Sfdisc_t* handle)
#else
static int subexcept(Sfio_t* sp,register int mode, Sfdisc_t* handle)
#endif
{
	register struct subfile *disp = (struct subfile*)handle;
	if(mode==SF_CLOSING)
	{
		sfdisc(sp,SF_POPDISC);
		return(0);
	}
	else if(disp && (mode==SF_DPOP || mode==SF_FINAL))
	{
		free((void*)disp);
		return(0);
	}
#ifdef SF_ATEXIT
	else if (mode==SF_ATEXIT)
	{
		sfdisc(sp, SF_POPDISC);
		return(0);
	}
#endif
	else if(mode==SF_READ)
		return(0);
	return(-1);
}

#define NROW    15      /* number of rows before going to multi-columns */
#define LBLSIZ	3	/* size of label field and interfield spacing */
/* 
 * print a list of arguments in columns
 */
void	sh_menu(Sfio_t *outfile,int argn,char *argv[])
{
	register int i,j;
	register char **arg;
	int nrow, ncol=1, ndigits=1;
	int fldsize, wsize = ed_window();
	char *cp = nv_getval(nv_scoped(LINES));
	nrow = (cp?1+2*((int)strtol(cp, (char**)0, 10)/3):NROW);
	for(i=argn;i >= 10;i /= 10)
		ndigits++;
	if(argn < nrow)
	{
		nrow = argn;
		goto skip;
	}
	i = 0;
	for(arg=argv; *arg;arg++)
	{
		if((j=strlen(*arg)) > i)
			i = j;
	}
	i += (ndigits+LBLSIZ);
	if(i < wsize)
		ncol = wsize/i;
	if(argn > nrow*ncol)
	{
		nrow = 1 + (argn-1)/ncol;
	}
	else
	{
		ncol = 1 + (argn-1)/nrow;
		nrow = 1 + (argn-1)/ncol;
	}
skip:
	fldsize = (wsize/ncol)-(ndigits+LBLSIZ);
	for(i=0;i<nrow;i++)
	{
		if(sh.trapnote&SH_SIGSET)
			return;
		j = i;
		while(1)
		{
			arg = argv+j;
			sfprintf(outfile,"%*d) %s",ndigits,j+1,*arg);
			j += nrow;
			if(j >= argn)
				break;
			sfnputc(outfile,' ',fldsize-strlen(*arg));
		}
		sfputc(outfile,'\n');
	}
}

#undef read
/*
 * shell version of read() for user added builtins
 */
ssize_t sh_read(register int fd, void* buff, size_t n) 
{
	register Sfio_t *sp;
	if(sp=sh.sftable[fd])
		return(sfread(sp,buff,n));
	else
		return(read(fd,buff,n));
}

#undef write
/*
 * shell version of write() for user added builtins
 */
ssize_t sh_write(register int fd, const void* buff, size_t n) 
{
	register Sfio_t *sp;
	if(sp=sh.sftable[fd])
		return(sfwrite(sp,buff,n));
	else
		return(write(fd,buff,n));
}

#undef lseek
/*
 * shell version of lseek() for user added builtins
 */
off_t sh_seek(register int fd, off_t offset, int whence)
{
	register Sfio_t *sp;
	if((sp=sh.sftable[fd]) && (sfset(sp,0,0)&(SF_READ|SF_WRITE)))
		return(sfseek(sp,offset,whence));
	else
		return(lseek(fd,offset,whence));
}

#undef dup
int sh_dup(register int old)
{
	register int fd = dup(old);
	if(fd>=0)
	{
		if(sh.fdstatus[old] == IOCLOSE)
			sh.fdstatus[old] = 0;
		sh.fdstatus[fd] = (sh.fdstatus[old]&~IOCLEX);
		if(fdnotify)
			(*fdnotify)(old,fd);
	}
	return(fd);
}

#undef fcntl
int sh_fcntl(register int fd, int op, ...)
{
	int newfd, arg;
	va_list		ap;
	va_start(ap, op);
	arg =  va_arg(ap, int) ;
	va_end(ap);
	newfd = fcntl(fd,op,arg);
	if(newfd>=0) switch(op)
	{
	    case F_DUPFD:
		if(sh.fdstatus[fd] == IOCLOSE)
			sh.fdstatus[fd] = 0;
		sh.fdstatus[newfd] = (sh.fdstatus[fd]&~IOCLEX);
		if(fdnotify)
			(*fdnotify)(fd,newfd);
		break;
	    case F_SETFD:
		if(sh.fdstatus[fd] == IOCLOSE)
			sh.fdstatus[fd] = 0;
		if(arg&FD_CLOEXEC)
			sh.fdstatus[fd] |= IOCLEX;
		else
			sh.fdstatus[fd] &= ~IOCLEX;
	}
	return(newfd);
}

/*
 * give file descriptor <fd> and <mode>, return an iostream pointer
 * <mode> must be SF_READ or SF_WRITE
 * <fd> must be a non-negative number ofr SH_IOCOPROCESS or SH_IOHISTFILE. 
 * returns NULL on failure and may set errno.
 */

Sfio_t *sh_iogetiop(int fd, int mode)
{
	int n;
	Sfio_t *iop=0;
	if(mode!=SF_READ && mode!=SF_WRITE)
	{
		errno = EINVAL;
		return(iop);
	}
	switch(fd)
	{
	    case SH_IOHISTFILE:
		if(!sh_histinit())
			return(iop);
		fd = sffileno(sh.hist_ptr->histfp);
		break;
	    case SH_IOCOPROCESS:
		if(mode==SF_WRITE)
			fd = sh.coutpipe;
		else
			fd = sh.cpipe[0];
		break;
	    default:
		if(fd<0 || fd >= sh.lim.open_max)
			fd = -1;
	}
	if(fd<0)
	{
		errno = EBADF;
		return(iop);
	}
	if(!(n=sh.fdstatus[fd]))
		n = sh_iocheckfd(fd);
	if(mode==SF_WRITE && !(n&IOWRITE))
		return(iop);
	if(mode==SF_READ && !(n&IOREAD))
		return(iop);
	if(!(iop = sh.sftable[fd]))
		iop=sh_iostream(fd);
	return(iop);
}

typedef int (*Notify_f)(int,int);

Notify_f    sh_fdnotify(Notify_f notify)
{
	Notify_f old;
        old = fdnotify;
        fdnotify = notify;
        return(old);
}

Sfio_t	*sh_fd2sfio(int fd)
{
	register int status;
	Sfio_t *sp = sh.sftable[fd];
	if(!sp  && (status = sh_iocheckfd(fd))!=IOCLOSE)
	{
		register int flags=0;
		if(status&IOREAD)
			flags |= SF_READ;
		if(status&IOWRITE)
			flags |= SF_WRITE;
		sp = sfnew(NULL, NULL, -1, fd,flags);
		sh.sftable[fd] = sp;
	}
	return(sp);
}

Sfio_t *sh_pathopen(const char *cp)
{
	int n;
#ifdef PATH_BFPATH
	if((n=path_open(cp,path_get(cp))) < 0)
		n = path_open(cp,(Pathcomp_t*)0);
#else
	if((n=path_open(cp,path_get(cp))) < 0)
		n = path_open(cp,"");
#endif
	if(n < 0)
		errormsg(SH_DICT,ERROR_system(1),e_open,cp);
	return(sh_iostream(n));
}
