/*
 * :PACKAGE: X11 support
 */

PACKAGE_X11_DIRS = /usr/openwin
PACKAGE_X11 := $(PACKAGE_X11_DIRS:T=F:O=1)
PACKAGE_X11_VERSION = 6

CC.REQUIRE.X11 = -lXext -lX11 -lnsl -lsocket -lw -ldl -lintl -ldnet_stub

$(CC.REQUIRE.X11) : .DONTCARE
