/*
 * :PACKAGE: X11 support
 */

PACKAGE_X11 := $("/usr/openwin":T=F)

CC.REQUIRE.X11 = -lXext -lX11 -lnsl -lsocket -lw -ldl -lintl

$(CC.REQUIRE.X11) : .DONTCARE
