# tests for the find utility

function DATA
{
	typeset f i j k
	for f
	do	test -d $f && continue
		KEEP $f
		case $f in
		data)	mkdir data
			for i in aaa zzz
			do	i=data/$i
				mkdir $i
				for j in 111 222 333
				do	mkdir $i/$j
					for k in 4 5 6 7
					do	mkdir $i/$j/$k
						for l in q.c r.d s.z
						do	print $i $j $k $l > $i/$j/$k/$l
							chmod $k$k$k $i/$j/$k/$l
						done
					done
				done
			done
			;;
		match)	mkdir -p match/.ghi match/jkl
			: > match/.abc > match/def
			: > match/.ghi/.mno > match/.ghi/pqr
			: > match/jkl/.stu > match/jkl/vwx
			;;
		mode)	mkdir mode
			for i in 0 1 2 3 4 5 6 7
			do	: > mode/$i$i$i
				chmod $i$i$i mode/$i$i$i
			done
			;;
		esac
	done
}

TEST 01 'basics'
	DO	DATA data
	EXEC	data
		OUTPUT - $'data
data/aaa
data/aaa/111
data/aaa/111/4
data/aaa/111/4/q.c
data/aaa/111/4/r.d
data/aaa/111/4/s.z
data/aaa/111/5
data/aaa/111/5/q.c
data/aaa/111/5/r.d
data/aaa/111/5/s.z
data/aaa/111/6
data/aaa/111/6/q.c
data/aaa/111/6/r.d
data/aaa/111/6/s.z
data/aaa/111/7
data/aaa/111/7/q.c
data/aaa/111/7/r.d
data/aaa/111/7/s.z
data/aaa/222
data/aaa/222/4
data/aaa/222/4/q.c
data/aaa/222/4/r.d
data/aaa/222/4/s.z
data/aaa/222/5
data/aaa/222/5/q.c
data/aaa/222/5/r.d
data/aaa/222/5/s.z
data/aaa/222/6
data/aaa/222/6/q.c
data/aaa/222/6/r.d
data/aaa/222/6/s.z
data/aaa/222/7
data/aaa/222/7/q.c
data/aaa/222/7/r.d
data/aaa/222/7/s.z
data/aaa/333
data/aaa/333/4
data/aaa/333/4/q.c
data/aaa/333/4/r.d
data/aaa/333/4/s.z
data/aaa/333/5
data/aaa/333/5/q.c
data/aaa/333/5/r.d
data/aaa/333/5/s.z
data/aaa/333/6
data/aaa/333/6/q.c
data/aaa/333/6/r.d
data/aaa/333/6/s.z
data/aaa/333/7
data/aaa/333/7/q.c
data/aaa/333/7/r.d
data/aaa/333/7/s.z
data/zzz
data/zzz/111
data/zzz/111/4
data/zzz/111/4/q.c
data/zzz/111/4/r.d
data/zzz/111/4/s.z
data/zzz/111/5
data/zzz/111/5/q.c
data/zzz/111/5/r.d
data/zzz/111/5/s.z
data/zzz/111/6
data/zzz/111/6/q.c
data/zzz/111/6/r.d
data/zzz/111/6/s.z
data/zzz/111/7
data/zzz/111/7/q.c
data/zzz/111/7/r.d
data/zzz/111/7/s.z
data/zzz/222
data/zzz/222/4
data/zzz/222/4/q.c
data/zzz/222/4/r.d
data/zzz/222/4/s.z
data/zzz/222/5
data/zzz/222/5/q.c
data/zzz/222/5/r.d
data/zzz/222/5/s.z
data/zzz/222/6
data/zzz/222/6/q.c
data/zzz/222/6/r.d
data/zzz/222/6/s.z
data/zzz/222/7
data/zzz/222/7/q.c
data/zzz/222/7/r.d
data/zzz/222/7/s.z
data/zzz/333
data/zzz/333/4
data/zzz/333/4/q.c
data/zzz/333/4/r.d
data/zzz/333/4/s.z
data/zzz/333/5
data/zzz/333/5/q.c
data/zzz/333/5/r.d
data/zzz/333/5/s.z
data/zzz/333/6
data/zzz/333/6/q.c
data/zzz/333/6/r.d
data/zzz/333/6/s.z
data/zzz/333/7
data/zzz/333/7/q.c
data/zzz/333/7/r.d
data/zzz/333/7/s.z'
	EXEC	data -print
	EXEC	data --print
	EXEC	-print data
	EXEC	--print data

TEST 02 'patterns'
	DO	DATA data
	EXEC	data -name '*.c'
		OUTPUT - $'data/aaa/111/4/q.c
data/aaa/111/5/q.c
data/aaa/111/6/q.c
data/aaa/111/7/q.c
data/aaa/222/4/q.c
data/aaa/222/5/q.c
data/aaa/222/6/q.c
data/aaa/222/7/q.c
data/aaa/333/4/q.c
data/aaa/333/5/q.c
data/aaa/333/6/q.c
data/aaa/333/7/q.c
data/zzz/111/4/q.c
data/zzz/111/5/q.c
data/zzz/111/6/q.c
data/zzz/111/7/q.c
data/zzz/222/4/q.c
data/zzz/222/5/q.c
data/zzz/222/6/q.c
data/zzz/222/7/q.c
data/zzz/333/4/q.c
data/zzz/333/5/q.c
data/zzz/333/6/q.c
data/zzz/333/7/q.c'
	EXEC	data -name '*.c' -print
	EXEC	data -name '*.[cd]'
		OUTPUT - $'data/aaa/111/4/q.c
data/aaa/111/4/r.d
data/aaa/111/5/q.c
data/aaa/111/5/r.d
data/aaa/111/6/q.c
data/aaa/111/6/r.d
data/aaa/111/7/q.c
data/aaa/111/7/r.d
data/aaa/222/4/q.c
data/aaa/222/4/r.d
data/aaa/222/5/q.c
data/aaa/222/5/r.d
data/aaa/222/6/q.c
data/aaa/222/6/r.d
data/aaa/222/7/q.c
data/aaa/222/7/r.d
data/aaa/333/4/q.c
data/aaa/333/4/r.d
data/aaa/333/5/q.c
data/aaa/333/5/r.d
data/aaa/333/6/q.c
data/aaa/333/6/r.d
data/aaa/333/7/q.c
data/aaa/333/7/r.d
data/zzz/111/4/q.c
data/zzz/111/4/r.d
data/zzz/111/5/q.c
data/zzz/111/5/r.d
data/zzz/111/6/q.c
data/zzz/111/6/r.d
data/zzz/111/7/q.c
data/zzz/111/7/r.d
data/zzz/222/4/q.c
data/zzz/222/4/r.d
data/zzz/222/5/q.c
data/zzz/222/5/r.d
data/zzz/222/6/q.c
data/zzz/222/6/r.d
data/zzz/222/7/q.c
data/zzz/222/7/r.d
data/zzz/333/4/q.c
data/zzz/333/4/r.d
data/zzz/333/5/q.c
data/zzz/333/5/r.d
data/zzz/333/6/q.c
data/zzz/333/6/r.d
data/zzz/333/7/q.c
data/zzz/333/7/r.d'
	EXEC	data \( -name '*.c' \|\| -name '*.d' \)
	EXEC	data \( -name '*.c' -or -name '*.d' \)
	EXEC	data \( -name '*.c' -or -name '*.d' \) -print
	EXEC	data \( -name '*.c' -prune \) -or \( -name '*.d' -print \)
		OUTPUT - $'data/aaa/111/4/r.d
data/aaa/111/5/r.d
data/aaa/111/6/r.d
data/aaa/111/7/r.d
data/aaa/222/4/r.d
data/aaa/222/5/r.d
data/aaa/222/6/r.d
data/aaa/222/7/r.d
data/aaa/333/4/r.d
data/aaa/333/5/r.d
data/aaa/333/6/r.d
data/aaa/333/7/r.d
data/zzz/111/4/r.d
data/zzz/111/5/r.d
data/zzz/111/6/r.d
data/zzz/111/7/r.d
data/zzz/222/4/r.d
data/zzz/222/5/r.d
data/zzz/222/6/r.d
data/zzz/222/7/r.d
data/zzz/333/4/r.d
data/zzz/333/5/r.d
data/zzz/333/6/r.d
data/zzz/333/7/r.d'
	EXEC	data \( -name 'zzz|*.c' -prune \) -or \( -name '*.d' -print \)
		OUTPUT - $'data/aaa/111/4/r.d
data/aaa/111/5/r.d
data/aaa/111/6/r.d
data/aaa/111/7/r.d
data/aaa/222/4/r.d
data/aaa/222/5/r.d
data/aaa/222/6/r.d
data/aaa/222/7/r.d
data/aaa/333/4/r.d
data/aaa/333/5/r.d
data/aaa/333/6/r.d
data/aaa/333/7/r.d'

TEST 03 'types'
	DO	DATA data
	EXEC	data -type d
		OUTPUT - $'data
data/aaa
data/aaa/111
data/aaa/111/4
data/aaa/111/5
data/aaa/111/6
data/aaa/111/7
data/aaa/222
data/aaa/222/4
data/aaa/222/5
data/aaa/222/6
data/aaa/222/7
data/aaa/333
data/aaa/333/4
data/aaa/333/5
data/aaa/333/6
data/aaa/333/7
data/zzz
data/zzz/111
data/zzz/111/4
data/zzz/111/5
data/zzz/111/6
data/zzz/111/7
data/zzz/222
data/zzz/222/4
data/zzz/222/5
data/zzz/222/6
data/zzz/222/7
data/zzz/333
data/zzz/333/4
data/zzz/333/5
data/zzz/333/6
data/zzz/333/7'
	EXEC	data -type f
		OUTPUT - $'data/aaa/111/4/q.c
data/aaa/111/4/r.d
data/aaa/111/4/s.z
data/aaa/111/5/q.c
data/aaa/111/5/r.d
data/aaa/111/5/s.z
data/aaa/111/6/q.c
data/aaa/111/6/r.d
data/aaa/111/6/s.z
data/aaa/111/7/q.c
data/aaa/111/7/r.d
data/aaa/111/7/s.z
data/aaa/222/4/q.c
data/aaa/222/4/r.d
data/aaa/222/4/s.z
data/aaa/222/5/q.c
data/aaa/222/5/r.d
data/aaa/222/5/s.z
data/aaa/222/6/q.c
data/aaa/222/6/r.d
data/aaa/222/6/s.z
data/aaa/222/7/q.c
data/aaa/222/7/r.d
data/aaa/222/7/s.z
data/aaa/333/4/q.c
data/aaa/333/4/r.d
data/aaa/333/4/s.z
data/aaa/333/5/q.c
data/aaa/333/5/r.d
data/aaa/333/5/s.z
data/aaa/333/6/q.c
data/aaa/333/6/r.d
data/aaa/333/6/s.z
data/aaa/333/7/q.c
data/aaa/333/7/r.d
data/aaa/333/7/s.z
data/zzz/111/4/q.c
data/zzz/111/4/r.d
data/zzz/111/4/s.z
data/zzz/111/5/q.c
data/zzz/111/5/r.d
data/zzz/111/5/s.z
data/zzz/111/6/q.c
data/zzz/111/6/r.d
data/zzz/111/6/s.z
data/zzz/111/7/q.c
data/zzz/111/7/r.d
data/zzz/111/7/s.z
data/zzz/222/4/q.c
data/zzz/222/4/r.d
data/zzz/222/4/s.z
data/zzz/222/5/q.c
data/zzz/222/5/r.d
data/zzz/222/5/s.z
data/zzz/222/6/q.c
data/zzz/222/6/r.d
data/zzz/222/6/s.z
data/zzz/222/7/q.c
data/zzz/222/7/r.d
data/zzz/222/7/s.z
data/zzz/333/4/q.c
data/zzz/333/4/r.d
data/zzz/333/4/s.z
data/zzz/333/5/q.c
data/zzz/333/5/r.d
data/zzz/333/5/s.z
data/zzz/333/6/q.c
data/zzz/333/6/r.d
data/zzz/333/6/s.z
data/zzz/333/7/q.c
data/zzz/333/7/r.d
data/zzz/333/7/s.z'

TEST 04 'modes'
	DO	DATA mode
	EXEC	mode -perm -000
		OUTPUT - 'mode
mode/000
mode/111
mode/222
mode/333
mode/444
mode/555
mode/666
mode/777'
	EXEC	mode -perm  000
		OUTPUT - 'mode/000'
	EXEC	mode -perm +000
		OUTPUT -
	EXEC	mode -perm -111
		OUTPUT - 'mode
mode/111
mode/333
mode/555
mode/777'
	EXEC	mode -perm  111
		OUTPUT - 'mode/111'
	EXEC	mode -perm +111
		OUTPUT - 'mode
mode/111
mode/333
mode/555
mode/777'
	EXEC	mode -perm -222
		OUTPUT - 'mode/222
mode/333
mode/666
mode/777'
	EXEC	mode -perm  222
		OUTPUT - 'mode/222'
	EXEC	mode -perm +222
		OUTPUT - 'mode
mode/222
mode/333
mode/666
mode/777'
	EXEC	mode -perm -333
		OUTPUT - 'mode/333
mode/777'
	EXEC	mode -perm  333
		OUTPUT - 'mode/333'
	EXEC	mode -perm +333
		OUTPUT - 'mode
mode/111
mode/222
mode/333
mode/555
mode/666
mode/777'
	EXEC	mode -perm -444
		OUTPUT - 'mode
mode/444
mode/555
mode/666
mode/777'
	EXEC	mode -perm  444
		OUTPUT - 'mode/444'
	EXEC	mode -perm +444
		OUTPUT - 'mode
mode/444
mode/555
mode/666
mode/777'
	EXEC	mode -perm -555
		OUTPUT - 'mode
mode/555
mode/777'
	EXEC	mode -perm  555
		OUTPUT - 'mode/555'
	EXEC	mode -perm +555
		OUTPUT - 'mode
mode/111
mode/333
mode/444
mode/555
mode/666
mode/777'
	EXEC	mode -perm -666
		OUTPUT - 'mode/666
mode/777'
	EXEC	mode -perm  666
		OUTPUT - 'mode/666'
	EXEC	mode -perm +666
		OUTPUT - 'mode
mode/222
mode/333
mode/444
mode/555
mode/666
mode/777'
	EXEC	mode -perm -777
		OUTPUT - 'mode/777'
	EXEC	mode -perm  777
		OUTPUT - 'mode/777'
	EXEC	mode -perm +777
		OUTPUT - 'mode
mode/111
mode/222
mode/333
mode/444
mode/555
mode/666
mode/777'

TEST 05 '-name pattern'
	DO	DATA match
	EXEC	match
		OUTPUT - $'match
match/def
match/.abc
match/jkl
match/jkl/vwx
match/jkl/.stu
match/.ghi
match/.ghi/pqr
match/.ghi/.mno'
	EXEC	match -name '*'
	EXEC	match -name '.*'
		OUTPUT - $'match/.abc
match/jkl/.stu
match/.ghi
match/.ghi/.mno'
