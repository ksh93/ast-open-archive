:PACKAGE: ast

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

crypt 1.0 plugin=codex :LIBRARY: crypt.c \
		rar.c rar-13.c rar-20.c \
		zip.c \
		-lcodex

:TEST: codex

:: RELEASE
