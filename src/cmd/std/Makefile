:PACKAGE: ast

LICENSE = since=1989,author=gsf

:ALL:

RM = $(STDRM)

:: PROMO.mm README RELEASE ls-color.sh ls-dir.sh

banner :: banner.c LICENSE=since=1989,author=dgk

dd :: dd.c

df :: df.c

du :: du.c

file :: file.c

iconv :: iconv.c

locale :: locale.c

ls :: ls.c

mesg :: mesg.c LICENSE=since=1995,author=dgk

$(PACKAGE_LOCAL)mount :: mount.1 mount.c LICENSE=since=1998,author=gsf

$(PACKAGE_LOCAL)ps :: ps.1 ps.c LICENSE=since=1998,author=gsf

shar :: shar.sh LICENSE=since=1999,author=dgk

split :: split.c LICENSE=since=1995,author=dgk+gsf

csplit :LINK: split

sum :: sum.c -lsum

cksum :LINK: sum

md5sum :LINK: sum

touch :: touch.c

tsort :: tsort.c LICENSE=since=1995,author=dgk+gsf

unexpand :: unexpand.c LICENSE=since=1995,author=dgk

expand :LINK: unexpand

:TEST: locale ls sum touch tsort

:: y2k.dat
