:PACKAGE: ast

LICENSE = since=2003,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

validate plugin=dss :LIBRARY: validate.c -ldss -lm

:TEST: dss

:: RELEASE bgp-validate.dss
