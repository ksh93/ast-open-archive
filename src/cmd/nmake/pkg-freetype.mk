/*
 * :PACKAGE: freetype support
 */

PACKAGE_freetype_INCLUDE = $(PKGDIRS:D:B=include/freetype*([-.0-9])/freetype/freetype.h:P=G:T=F:H>:O=1:D:D)
