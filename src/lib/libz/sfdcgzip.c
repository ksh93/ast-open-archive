#pragma prototyped

/*
 * sfio gzip discipline
 */

#include <sfio_t.h>
#include <ast.h>
#include <zlib.h>
#include <sfdcgzip.h>

typedef struct
{
	Sfdisc_t	disc;		/* sfio discipline		*/
	Gz_t*		gz;		/* gz handle			*/
} Sfgzip_t;

/*
 * gzip exception handler
 * free on close
 */

static int
sfgzexcept(Sfio_t* sp, int op, void* val, Sfdisc_t* dp)
{
	register Sfgzip_t*	gz = (Sfgzip_t*)dp;
	int			r;

	NoP(sp);
#if 1
	{
		static char	aha[] = "AHA sfdcgzip event 0\n";
		static int	init;

		if (!init)
			init = getenv("SFGZ_DEBUG") ? 1 : -1;
		if (init > 0)
		{
			aha[sizeof(aha) - 3] = '0' + op;
			write(2, aha, sizeof(aha) - 1);
		}
	}
#endif
	switch (op)
	{
	case SF_ATEXIT:
		sfdisc(sp, SF_POPDISC);
		return 0;
	case SF_CLOSE:
	case SF_DPOP:
	case SF_FINAL:
		if (gz->gz)
		{
			r = gzclose(gz->gz) ? -1 : 0;
			gz->gz = 0;
		}
		else
			r = 0;
		if (op != SF_CLOSE)
			free(dp);
		return r;
	case SF_READ:
	case SF_WRITE:
		return *((ssize_t*)val) < 0 ? -1 : 0;
	case SF_SYNC:
		return gzsync(gz->gz, (z_off_t)(-1)) == -1 ? -1 : 0;
	case SFGZ_HANDLE:
		return (*((Gz_t**)val) = gz->gz) ? 1 : -1;
	case SFGZ_GETPOS:
		return (*((Sfoff_t*)val) = gzsync(gz->gz, (z_off_t)(-1))) == -1 ? -1 : 0;
	case SFGZ_SETPOS:
		return gzsync(gz->gz, (z_off_t)(*((Sfoff_t*)val))) == -1 ? -1 : 0;
	}
	return 0;
}

/*
 * sfio gzip discipline read
 */

static ssize_t
sfgzread(Sfio_t* fp, Void_t* buf, size_t size, Sfdisc_t* dp)
{
	register Sfgzip_t*	gz = (Sfgzip_t*)dp;

	return gzread(gz->gz, buf, size);
}

/*
 * sfio gzip discipline write
 */

static ssize_t
sfgzwrite(Sfio_t* fp, const Void_t* buf, size_t size, Sfdisc_t* dp)
{
	register Sfgzip_t*	gz = (Sfgzip_t*)dp;

	return (gzwrite(gz->gz, (void*)buf, size) < 0) ? -1 : size;
}

/*
 * create and push the sfio gzip discipline
 *
 * (flags&SFGZ_VERIFY) return
 *	>0	is a gzip/compress file
 *	 0	not a gzip/compress file
 *	<0	error
 * otherwise return
 *	>0	discipline pushed (gzip or lzw)
 *	 0	discipline not needed
 *	<0	error
 */

int
sfdcgzip(Sfio_t* sp, int flags)
{
	char*		m;
	Sfgzip_t*	gz;
	char		mode[10];
int phony = 0;

	if (sp->flags & SF_READ)
	{
		register unsigned char*	s;
		register int		n;

		/*
		 * peek the first 2 bytes to verify the magic
		 *
		 *	0x1f8b	sfdcgzip	gzip	
		 *	0x1f9d	sfdclzw		compress
		 */
		
		if (!(s = (unsigned char*)sfreserve(sp, 2, 1)))
			return -1;
		if (s[0] != 0x1f)
			n = -1;
		else if (s[1] == 0x8b)
			n = 0;
		else if (s[1] == 0x9d)
			n = 1;
		else
			n = -1;
		sfread(sp, s, 0);
		if (n < 0)
			return 0;
		if (flags & SFGZ_VERIFY)
			return n >= 0;
		if (n > 0)
			return sfdclzw(sp, flags);
	}
	else if (flags & SFGZ_VERIFY)
		return -1;
	if (!(gz = newof(0, Sfgzip_t, 1, 0)))
		return -1;
	gz->disc.exceptf = sfgzexcept;
	if (sp->flags & SF_READ)
		gz->disc.readf = sfgzread;
	else
		gz->disc.writef = sfgzwrite;
	m = mode;
	*m++ = (sp->flags & SF_READ) ? 'r' : 'w';
	*m++ = 'b';
	if (flags & SFGZ_NOCRC)
		*m++ = 'n';
if ((sp->flags & SF_READ) && getenv("SFGZ_phony")) { phony = 1; } else
	*m++ = 'o';
	if ((flags &= 0xf) > 0 && flags <= 9)
		*m++ = '0' + flags;
	*m = 0;
	if (sfdisc(sp, &gz->disc) != &gz->disc || !(gz->gz = gzdopen(sffileno(sp), mode)))
	{
		free(gz);
		return -1;
	}
#if 0
{
	char*	v;
	long	n;

	if ((v = getenv("SFGZ_sfdcgzip_bufsize")) || (v = getenv("SFGZ_bufsize")))
	{
		if ((n = strtol(v, NiL, 0)) > 0)
			sfsetbuf(sp, NiL, n);
	}
	else
		sfsetbuf(sp, NiL, SF_BUFSIZE);
}
#else
	sfsetbuf(sp, NiL, SF_BUFSIZE);
#endif
if (phony) sp->file = open("/dev/null", 0);
	if (!(sp->flags & SF_READ))
		sfset(sp, SF_IOCHECK, 1);
	return 1;
}

#if __OBSOLETE__ < 19990717

#if defined(__EXPORT__)
#define extern	__EXPORT__
#endif

extern int
sfgzip(Sfio_t* sp)
{
	return sfdcgzip(sp, 0);
}

#undef	extern

#endif
