########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1992-2009 AT&T Intellectual Property          #
#                      and is licensed under the                       #
#                  Common Public License, Version 1.0                  #
#                    by AT&T Intellectual Property                     #
#                                                                      #
#                A copy of the License is available at                 #
#            http://www.opensource.org/licenses/cpl1.0.txt             #
#         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         #
#                                                                      #
#              Information and Software Systems Research               #
#                            AT&T Research                             #
#                           Florham Park NJ                            #
#                                                                      #
#                 Glenn Fowler <gsf@research.att.com>                  #
#                  David Korn <dgk@research.att.com>                   #
#                                                                      #
########################################################################
integer TEST=0 FAILURES=0 VERBOSE=0

trap 'rm -rf tail.tmp.*' 0

function failed
{
	(( FAILURES++ ))
	print -u2 TEST $TEST FAILED "$@"
}

function verbose
{
	print -u2 TEST $TEST "$@"
}

function running
{
	integer n=5

	while	(( --n ))
	do	kill -0 $1 2>/dev/null || return 1
		sleep 1
	done
	return 0
}

function test_1
{
	typeset tail_pid tail_exit

	TEST=1
	rm -rf tail.tmp.*
	mkfifo tail.tmp.fifo
	[[ $VERBOSE ]] && verbose "-f fifo"
	"$@" -f < tail.tmp.fifo > tail.tmp.out 2> tail.tmp.err &
	tail_pid=$!
	seq 20 > tail.tmp.fifo
	if	running $tail_pid
	then	failed "hung"
		kill -KILL ${tail_pid}
		wait
	else	wait
		tail_exit=$?
		(( tail_exit )) && failed "exit status $tail_exit"
	fi
	print $'11\n12\n13\n14\n15\n16\n17\n18\n19\n20' > tail.tmp.out.exp
	cmp -s tail.tmp.out tail.tmp.out.exp || {
		failed "stdout < got > expected"
		diff tail.tmp.out tail.tmp.out.exp
	}
	: > tail.tmp.err.exp
	cmp -s tail.tmp.err tail.tmp.err.exp || {
		failed "stderr < got > expected"
		diff tail.tmp.err tail.tmp.err.exp
	}
}

function test_2
{
	typeset tail_pid tail_exit

	TEST=2
	rm -rf tail.tmp.*
	mkfifo tail.tmp.fifo
	[[ $VERBOSE ]] && verbose "-f < fifo"
	"$@" -f tail.tmp.fifo > tail.tmp.out 2> tail.tmp.err &
	tail_pid=$!
	seq 20 > tail.tmp.fifo
	if	running $tail_pid
	then	print $'11\n12\n13\n14\n15\n16\n17\n18\n19\n20' > tail.tmp.out.exp
		cmp -s tail.tmp.out tail.tmp.out.exp || {
			failed "chunk 1 stdout < got > expected"
			diff tail.tmp.out tail.tmp.out.exp
		}
		seq 10 >> tail.tmp.fifo
		if	running $tail_pid
		then	kill -KILL $tail_pid
			wait
		else	wait
			tail_exit=$?
			(( tail_exit )) && failed "exit status $tail_exit"
		fi
	else	wait
		tail_exit=$?
		(( tail_exit )) && failed "exit status $tail_exit"
	fi
	print $'11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10' > tail.tmp.out.exp
	cmp -s tail.tmp.out tail.tmp.out.exp || {
		failed "stdout < got > expected"
		diff tail.tmp.out tail.tmp.out.exp
	}
	: > tail.tmp.err.exp
	cmp -s tail.tmp.err tail.tmp.err.exp || {
		failed "stderr < got > expected"
		diff tail.tmp.err tail.tmp.err.exp
	}
	
}

function test_tail
{
	test_1 "$@"
	test_2 "$@"
}

while	:
do	case $1 in
	-v|--verbose)
		VERBOSE=1
		;;
	-*)	;;
	*)	break
		;;
	esac
	shift
done

(( $# )) || set tail
test_tail "$@"
