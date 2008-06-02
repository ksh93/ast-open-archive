IMPLEMENTATION=$($UNIT)

VIEW tst $UNIT-$IMPLEMENTATION.tst

if	[[ -f $tst ]]
then	. $tst
else	print -u2 $UNIT: $IMPLEMENTATION regression tests must be manually generated
	exit 1
fi
