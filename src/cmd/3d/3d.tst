: 3d regression tests
#
# 3d.tst (AT&T Research) 2004-05-03
#
# the first section defines the test harness
# the next section defines individual test functions
# the tests are in the last section
#

( vpath ) >/dev/null 2>&1 || {
	print -u2 $0: must be run from 3d shell
	exit 1
}

export LC_ALL=C

integer seconds=1

COMMAND=3d

FILE=
FORMAT="%Y-%m-%d+%H:%M:%S"
GROUP=
INIT=
NEW="new-and-improved"
NUKE=
OLD="original"
pwd=$PWD
PREFIX=
STAMP="2005-07-17+04:05:06"
VIRTUAL=

trap "(( ERRORS++ ))" ERR
trap "CLEANUP 0" 0
tmp=/tmp/$$.3d
mkdir $tmp || exit
cd $tmp
mkdir bottom

function ACTIVE
{
	DO return 0
}

function TEST
{
	case $INIT in
	"")	INIT=1
		print "TEST	$COMMAND"
		;;
	esac
	cd $tmp
	case $NUKE in
	?*)	rm -rf $NUKE; NUKE= ;;
	esac
	PREFIX=
	GROUP=$1
	ACTIVE || return
	vpath - -
	shift
	print "$GROUP	$*"
}

function FAIL # file message
{
	print -u2 "	FAIL $@"
	rm -rf $1
	(( ERRORS++ ))
}

function PREFIX
{
	ACTIVE || return
	PREFIX=$1
	case $1 in
	/*)	NUKE="$NUKE $1" ;;
	*)	NUKE="$NUKE $PWD/$1" ;;
	esac
}

function VIRTUAL
{
	ACTIVE || return
	case $VIRTUAL in
	?*)	pwd=$PWD
		cd $tmp
		rm -rf $tmp/$VIRTUAL
		cd $pwd
		;;
	esac
	VIRTUAL=$1
}

function CD
{
	ACTIVE || return
	cd $tmp/$1
}

function VPATH
{
	ACTIVE || return
	vpath "$@"
}

function CP
{
	ACTIVE || return
	cp "$@"
	shift $#-1
	NUKE="$NUKE $1"
}

function LN
{
	ACTIVE || return
	ln "$@"
	shift $#-1
	NUKE="$NUKE $1"
}

function MV
{
	ACTIVE || return
	mv "$@"
	shift $#-1
	NUKE="$NUKE $1"
}

function MKDIR
{
	ACTIVE || return
	mkdir -p $*
	for i
	do	case $i in
		/*)	NUKE="$NUKE $i" ;;
		*)	NUKE="$NUKE $i" ;;
		esac
	done
}

function DATA
{
	ACTIVE || return 1
	VIRTUAL $VIRTUAL
	case $1 in
	-)	remove=1; shift ;;
	*)	remove=0 ;;
	esac
	case $# in
	0)	return 0 ;;
	1)	;;
	*)	return 1 ;;
	esac
	(( TESTS++ ))
	path=$1
	case $PREFIX in
	"")	FILE=$path ;;
	*)	FILE=$PREFIX/$path ;;
	esac
	file=bottom/$path
	if	[[ ! -f $tmp/$file ]]
	then	case $remove in
		0)	if	[[ $path == */* && ! -d $tmp/${file%/*} ]]
			then	mkdir -p $tmp/${file%/*} || FAIL $tmp/${file%/*} DATA mkdir
			fi
			print $OLD > $tmp/$file
			mode=${file%???}
			mode=${file#$mode}
			chmod $mode $tmp/$file || FAIL $tmp/$file DATA chmod
			;;
		esac
	else	case $remove in
		1)	rm -f $tmp/$file ;;
		esac
	fi
	return 0
}

#
# the remaining functions implement individiual parameterized tests
#

function APPEND
{
	ACTIVE && DATA $* || return
	print "$NEW" >> $FILE || FAIL $FILE write error
	if	[[ $(<$FILE) != "$OLD"$'\n'"$NEW" ]]
	then	FAIL $FILE unchanged by $0
	elif	[[ -f $FILE/... && $(<$FILE/...) != "$OLD" ]]
	then	FAIL $FILE/... changed by $0
	fi
}

function MODE
{
	ACTIVE && DATA $* || return
	chmod 000 $FILE || FAIL $FILE chmod error
	if	[[ -f $FILE/... && ! -r $FILE/... ]]
	then	FAIL $FILE/... changed by $0
	elif	[[ -r $FILE ]]
	then	FAIL $FILE unchanged by $0
	fi
}

function REMOVE
{
	ACTIVE && DATA $* || return
	rm $FILE || FAIL $FILE rm error
	if	[[ ! -f $FILE/... ]]
	then	FAIL $FILE/... changed by $0
	fi
	print "$NEW" > $FILE || FAIL $FILE write error
	rm $FILE || FAIL $FILE rm error
	if	[[ $(<$FILE) != "$OLD" ]]
	then	FAIL $FILE unchanged by $0
	elif	[[ $(<$FILE/...) != "$OLD" ]]
	then	FAIL $FILE/... changed by $0
	fi
}

function TOUCH
{
	ACTIVE && DATA $* || return
	touch -r $FILE -t "$seconds seconds" $tmp/reference || FAIL $tmp/reference touch error
	(( seconds++ ))
	touch -t "$seconds seconds" $FILE || FAIL $FILE touch error
	if	[[ $FILE/... -nt $tmp/reference ]]
	then	FAIL $FILE/... changed by $0
	elif	[[ ! $FILE -nt $tmp/reference ]]
	then	FAIL $FILE unchanged by $0
	fi
	touch -t $STAMP $FILE
	if	[[ $(date -m -f $FORMAT $FILE) != "$STAMP" ]]
	then	FAIL $FILE modfiy time does not match $STAMP
	fi
}

function UPDATE
{
	ACTIVE && DATA $* || return
	print "$NEW" 1<> $FILE || FAIL $FILE write error
	if	[[ $(<$FILE) != "$NEW" ]]
	then	FAIL $FILE unchanged by $0
	elif	[[ -f $FILE/... && $(<$FILE/...) != "$OLD" ]]
	then	FAIL $FILE/... changed by $0
	fi
}

function WRITE
{
	ACTIVE && DATA $* || return
	print "$NEW" > $FILE || FAIL $FILE write error
	if	[[ $(<$FILE) != "$NEW" ]]
	then	FAIL $FILE unchanged by $0
	elif	[[ -f $FILE/... && $(<$FILE/...) != "$OLD" ]]
	then	FAIL $FILE/... changed by $0
	fi
}

function RUN
{
	ACTIVE && DATA $* || return
	WRITE	w666
	WRITE	w600
	TOUCH	t777
	MODE	m444
	WRITE	dir/w666
	WRITE	dir/w600
	TOUCH	dir/t777
	MODE	dir/m444
	UPDATE	u644
	UPDATE	u640
	APPEND	a644
	APPEND	a640
	UPDATE	dir/u644
	UPDATE	dir/u640
	APPEND	dir/a644
	APPEND	dir/a640
	VIRTUAL
	REMOVE	r644
	WRITE	r644
	REMOVE	r644
}

#
# finally the tests
#

TEST 01 PWD==top top exists
	VPATH top bottom
	MKDIR top
	CD top
	RUN

TEST 02 PWD!=top top exists
	VPATH top bottom
	MKDIR top
	MKDIR junk
	CD junk
	PREFIX ../top
	RUN

TEST 03 PWD==top top virtual
	VIRTUAL top
	VPATH top bottom
	CD top
	RUN

TEST 04 PWD!=top top virtual
	VIRTUAL top
	VPATH top bottom
	MKDIR junk
	CD junk
	PREFIX ../top
	RUN

TEST 05 top symlink
	if	LN -s text link
	then	[[ -L link ]] || FAIL lstat does stat
	fi

TEST 06 symlink spaghetti
	MKDIR usr/bin sbin
	ACTIVE && echo : > sbin/cmd && chmod +x sbin/cmd
	LN -s usr/bin bin
	LN -s ../../sbin/cmd usr/bin/cmd
	CD bin
	ACTIVE && cmd

TEST 07 PWD==top top exists, bot virtual
	VPATH top bot
	MKDIR top
	CD top
	ACTIVE && echo foo > foo && echo bar > bar
	CP foo ...
	MV bar ...
	CP foo ...
	ACTIVE && [[ $(ls ...) != $'bar\nfoo' ]] && FAIL bottom incomplete -- $(ls ...)
	ACTIVE && [[ $(2d ls) != $'foo' ]] && FAIL top incomplete -- $(2d ls)
	CP * ...
	ACTIVE && [[ $(cat bar) != bar ]] && FAIL bottom to bottom garbled -- $(cat bar)
	ACTIVE && [[ $(cat foo) != foo ]] && FAIL top garbled -- $(cat foo)
	ACTIVE && [[ $(cat .../foo) != foo ]] && FAIL top to bottom garbled -- $(cat .../foo)
