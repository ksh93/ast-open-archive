:PACKAGE: ast

LICENSE = since=1993;author=dgk

pack :: pack.c libhuff.a

unpack :: unpack.c libhuff.a

pcat :: unpack.c libhuff.a

huff 1.0 :LIBRARY: huff.3 huffman.h \
		huffinit.c huffgethdr.c huffputhdr.c \
		huffencode.c huffdecode.c
