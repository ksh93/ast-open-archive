/*
 * :PACKAGE: X11 support
 */

PACKAGE_X11_DIRS = \
	/usr/X11 $("/usr/X11([Rr][0-9]*([-.0-9]))":P=G:H>) \
	$("/usr/contrib/X11?([Rr][0-9]*([-.0-9]))":P=G:H>) \
	/usr/contrib \
	/usr/openwin
PACKAGE_X11 := $(PACKAGE_X11_DIRS:T=F:O=1)
PACKAGE_X11_VERSION = 6

CC.REQUIRE.X11 = -lXext -lX11 -lnsl -lsocket -lw -ldl -lintl -ldnet_stub \
	$(CC.HOSTTYPE:N=openbsd.*:@?-lc_r??)
CC.REQUIRE.Xaw = -lXaw -lXt
CC.REQUIRE.Xaw3d = -lXaw3d -lXt
CC.REQUIRE.Xm = -lXm -lXt
CC.REQUIRE.Xt = -lXt -lXmu -lX11 -lSM -lICE -lm

$(CC.REQUIRE.X11) \
$(CC.REQUIRE.Xaw) $(CC.REQUIRE.Xaw3d) \
$(CC.REQUIRE.Xm) $(CC.REQUIRE.Xt) \
: .DONTCARE
