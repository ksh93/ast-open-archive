/* gmake with coshell and MAM */

REMOTE = cosh	/* one of remote-*.c below */

if "$(REMOTE)" == "cosh"
:PACKAGE: - coshell ast
"error" == make_error
end

LICENSE = type=special

HAVE_CONFIG_H == 1

gmake :: gmake.1 \
	commands.h dep.h file.h getopt.h job.h \
	make.h mam.h rule.h signame.h variable.h \
	commands.c job.c dir.c file.c misc.c main.c \
	read.c remake.c rule.c implicit.c default.c \
	variable.c expand.c function.c vpath.c version.c \
	ar.c arscan.c signame.c remote-$(REMOTE).c \
	getopt.c getopt1.c getloadavg.c \
	-lkstat -lkvm -lelf

-lkstat -lkvm -lelf : .DONTCARE

:: remote-cosh.c remote-cstms.c remote-stub.c alloca.c
:: COPYING ChangeLog NEWS README
:: configure config.h.in
:: remote.patch mam.patch
