:PACKAGE: ast

LICENSE = since=1996,author=gsf

DEBUG == 1

if CC.HOSTTYPE == "win32*"
AT_STRICT == 1
end

at :: RELEASE at.h at.c -lcoshell -lcs

batch :LINK: at

$(LIBDIR)/at/jobs :INSTALLDIR: mode=u=rwx,go=x

$(LIBDIR)/at/jobs :INSTALLDIR: mode=u=rx,go=x atx

atx :: atx.c +last

at :SERVICE: atd.c -lcs +last

crontab :: crontab.sh
