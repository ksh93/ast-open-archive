if ! ( CC.HOSTTYPE == "*win*" && CC.SUFFIX.DYNAMIC == ".dll" || "$(PACKAGE_OPTIMIZE:N=space)" )
	libtype = :static
end

:PACKAGE: tk tcl shell$(libtype) ast$(libtype) X11

CCFLAGS = $(CC.OPTIMIZE)

tksh :: tksh.1 uinit.c tkMain.c

:: PROMO.mm README RELEASE
