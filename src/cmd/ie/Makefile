:PACKAGE: ast

LICENSE = since=1984,author='dgk+Pat Sullivan'

CCFLAGS = -O $(CC.DLL)

VERSION = 1.0

ESH == 1
VSH == 1
LDYNAMIC == 1

.SOURCE : configure

edit $(VERSION) :LIBRARY: editlib.c vi.c emacs.c history.c edit.c +ldll

$(FUNDIR) :INSTALLDIR: mode=+x ie

sh_config.h : configure config (CC)
	$(*:O>1) $(*:O=1) $(CC) > $(<:B:S=.tmp)
	if	test -r $(<:B:S=.tmp) && cmp -s $(<:B:S=.tmp) $(<)
	then	rm $(<:B:S=.tmp)
	else	mv $(<:B:S=.tmp) $(<)
	fi

:: RELEASE OPTIONS \
	bcopy.c config data.c fastfork.c fcntl.c ftime.c getgroups.c \
	getppid.c getrlimit.c hz.c int16.c ioctl.c ispipe.c \
	lock.c lstat.c memset.c nfile.c nice.c opendir.c \
	poll.c proto.c rand.c rename.c select.c setjmp.c setreuid.c \
	shellmagic sibuf.c sigrelse.c sigrestart.c sigvec.c socket.c \
	strchr.c syscall.c vlimit.c void.c wait3.c waitpid.c winsize.c \
	yellowp.c
