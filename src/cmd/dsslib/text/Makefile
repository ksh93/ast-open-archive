:PACKAGE: ast

LICENSE = since=2002,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

text 1.0 plugin=dss :LIBRARY: text.c -ldss

if CC.HOSTTYPE == "unixware*.*" || "$(-mam)" == "static*"
text.o : .CC.NOOPTIMIZE
end

:TEST: dss

:: RELEASE data/pwd-txt.dat
