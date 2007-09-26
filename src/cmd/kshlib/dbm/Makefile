:PACKAGE: ast

LICENSE = since=2007,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

dbm 1.0 plugin=ksh :LIBRARY: dbm.c -ldbm -lshell

:TEST: dbm.tst $$(SHELL)

:: RELEASE
