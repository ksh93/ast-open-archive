# ast nmake recursion order tests

UNIT nmake

TEST 01 'recursion order basics'

	EXEC	-n --recurse=list
		INPUT Makefile $':MAKE:'
		INPUT cmd/Makefile $':MAKE:'
		INPUT cmd/tstutil/Makefile $':PACKAGE: ast libtstinc:order'
		INPUT cmd/genutil/Makefile $'genutil :: genutil.c -lnet -ldb -ltstasm -lsys'
		INPUT cmd/fe/Makefile $':PACKAGE: tstdata:order\nfe :: fe.c -ltstasm'
		INPUT cmd/be/Makefile $':PACKAGE: tstdata:order\nbe :: be.c -lnetgen -lsys'
		INPUT cmd/tstdata/Makefile $':PACKAGE: tstutil:order'
		INPUT lib/Makefile $':MAKE:'
		INPUT lib/libtstasm/Makefile $'tstasm :LIBRARY: tstasm.c +ljcl -lsys'
		INPUT lib/libusr/Makefile $'usr :LIBRARY: usr.c'
		INPUT lib/libtstgen/Makefile $'tstgen :LIBRARY: tstgen.c -ltstasm'
		INPUT lib/libnet/Makefile $':PACKAGE: be:order\nnet :LIBRARY: net.c -lnetgen'
		INPUT lib/libnetgen/Makefile $':PACKAGE: fe:order\nnetgen :LIBRARY: netgen.c -ldb -lsys'
		INPUT lib/libdb/Makefile $'db :LIBRARY: db.c -lusr -ltstgen -lzip'
		INPUT lib/libsort/Makefile $'sort :LIBRARY: sort.c -lsys'
		INPUT lib/libtstinc/Makefile $':PACKAGE: tst'
		OUTPUT - $'lib/libtstinc
-
cmd/tstutil
-
cmd/tstdata
-
lib/libtstasm
-
cmd/fe
lib/libusr
lib/libtstgen
-
lib/libdb
-
lib/libnetgen
-
cmd/be
-
-
lib/libnet
-
cmd/genutil
lib/libsort'

	EXEC	-n --recurse=prereqs
		OUTPUT - $'cmd/tstdata : cmd/tstutil
cmd/tstutil : lib/libtstinc
cmd/be : cmd/tstdata lib/libnetgen libsys
lib/libnetgen : cmd/fe lib/libdb libsys
cmd/fe : cmd/tstdata lib/libtstasm
lib/libtstasm : libjcl libsys
lib/libdb : lib/libusr lib/libtstgen libzip
lib/libtstgen : lib/libtstasm
cmd/genutil : lib/libnet lib/libdb lib/libtstasm libsys
lib/libnet : cmd/be lib/libnetgen
lib/libsort : libsys
all : cmd/genutil lib/libsort'
