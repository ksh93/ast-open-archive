:PACKAGE: ast

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

zip 1.0 plugin=codex :LIBRARY: \
	zip.c huff.c \
	shrink.c reduce.c implode.c deflate.c \
	-lcodex

:: RELEASE
