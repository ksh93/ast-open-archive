:PACKAGE: ast

LICENSE = since=2003,author=gsf

CCFLAGS = $(CC.DLL) $(CC.OPTIMIZE)

sum 1.0 plugin=sort :LIBRARY: sortsum.c -lrecsort -ldss

:TEST: sort

:: RELEASE data/nyse.dat
