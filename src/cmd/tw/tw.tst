# tests for the tw utility

KEEP data link

mkdir data
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
mkdir -p link/x/y/z
ln -s x/y link/home

TEST 01 'basics'
	EXEC	-d data
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

TEST 02 'patterns'
	EXEC	-d data -e 'name=="*.c"'
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
	EXEC	-d data -e 'name=="*.[cd]"'
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
	EXEC	-d data -e 'name=="*.c" || name=="*.d"'

TEST 03 'types'
	EXEC	-d data -e type==DIR
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
	EXEC	-d data -e type==REG
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

TEST 04 'symlink hops'
	EXEC -d link -e sort:name -e "if(name=='home')status=FOLLOW;"
		OUTPUT - $'link
link/home
link/home/z
link/x
link/x/y
link/x/y/z'
