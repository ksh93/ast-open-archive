:PACKAGE: ast

LICENSE = since=1996,author=gsf+kpv+doug

sort :: README RELEASE PROMO.mm sort.1 main.c +lrecsort -ldll -lz

:MAKE: sortsum

:INSTALLDIR: gen testsort

gen :: gen.c

:TEST: sort

script :TEST: testsort.sh sort
