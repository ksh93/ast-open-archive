# tests for the tw utility

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
		link)	mkdir -p link/x/y/z
			ln -s x/y link/home
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
	DO	DATA data
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
	DO	DATA data
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
	DO	DATA link
	EXEC -d link -e sort:name -e "if(name=='home')status=FOLLOW;"
		OUTPUT - $'link
link/home
link/home/z
link/x
link/x/y
link/x/y/z'

TEST 05 '-name pattern'
	DO	DATA match
	EXEC	-d match -e sort:name
		OUTPUT - $'match
match/.abc
match/.ghi
match/.ghi/.mno
match/.ghi/pqr
match/def
match/jkl
match/jkl/.stu
match/jkl/vwx'
	EXEC	-d match -e sort:name -e 'name=="*"'
	EXEC	-d match -e sort:name -e 'path=="*"'
	EXEC	-d match -e sort:name -e 'name==".*"'
		OUTPUT - $'match/.abc
match/.ghi
match/.ghi/.mno
match/jkl/.stu'
	EXEC	-d match -e sort:name -e 'name=="*.*"'

TEST 06 'expressions and functions'

	EXEC 	-n -e '	int i;
			char* s = "abc pdq xyz";
			char* a, p, x;
			i = sscanf(s, "%s %s %s", &a, &p, &x);
			printf("i=%d a=\"%s\" p=\"%s\" x=\"%s\"\n", i, a, p, x);'
		OUTPUT - 'i=3 a="abc" p="pdq" x="xyz"'

	EXEC 	-n -e '	int i;
			char* s = "abc pdq xyz";
			char* a, p, x;
			i = sscanf(s, "%s %s %s", &a, &p, &x);
			printf("i=%d x=\"%s\" p=\"%s\" a=\"%s\"\n", i, x, p, a);'
		OUTPUT - 'i=3 x="xyz" p="pdq" a="abc"'

	EXEC 	-n -e '	int i;
			char* s = "abc pdq xyz";
			char* a, p, x;
			i = sscanf(s, "%s", &a);
			printf("i=%d a=\"%s\"\n", i, a);
			i = sscanf(s, "%s %s", &a, &p);
			printf("i=%d a=\"%s\" p=\"%s\"\n", i, a, p);
			i = sscanf(s, "%s %s %s", &a, &p, &x);
			printf("i=%d a=\"%s\" p=\"%s\" x=\"%s\"\n", i, a, p, x);
			i = sscanf(s, "%s %s %s", &a, &p, &x);
			printf("i=%d x=\"%s\" p=\"%s\" a=\"%s\"\n", i, x, p, a);
			printf("i=%d a=\"%s\" p=\"%s\" x=\"%s\"\n", i, a, p, x);'
		OUTPUT - 'i=1 a="abc"
i=2 a="abc" p="pdq"
i=3 a="abc" p="pdq" x="xyz"
i=3 x="xyz" p="pdq" a="abc"
i=3 a="abc" p="pdq" x="xyz"'

	EXEC 	-n -e '	int i,n;
			float f;
			char* s = "123 3.45e6";
			i = sscanf(s,"%d %g", &n, &f);
			printf("i=%d n=%d f=%g\n", i, n, f);'
		OUTPUT - 'i=2 n=123 f=3.45e+06'

	EXEC 	-n -e '	int i,n;
			float f;
			i = scanf("%d %g", &n, &f);
			printf("i=%d n=%d f=%g\n", i, n, f);'
		INPUT - '123 3.45e6'

	EXEC 	-n -e '	int i,n;
			float f;
			char* s = "123 3.45e6";
			i = sscanf(s,"%d %g", n, &f);
			printf("i=%d n=%d f=%g\n", i, n, f);'
		EXIT 1
		OUTPUT -
		ERROR - 'tw: line 4: i = sscanf(s,"%d %g", n, &f)<<< sscanf: address argument expected'

	EXEC 	-n -e '	int i,n;
			float f;
			char* s = "123 3.45e6";
			i = sscanf(s,"%d %g", &n, f);
			printf("i=%d n=%d f=%g\n", i, n, f);'
		ERROR - 'tw: line 4: i = sscanf(s,"%d %g", &n, f)<<< sscanf: address argument expected'

	EXEC 	-n -e '	int i,n;
			float f;
			char* s = "123 3.45e6";
			i = sscanf(s);
			printf("i=%d n=%d f=%g\n", i, n, f);'
		ERROR - 'tw: line 4: i = sscanf(s)<<< sscanf: format argument expected'

	EXEC 	-n -e '	int i,n;
			float f;
			char* s = "123 3.45e6";
			i = sscanf(s,"%d %g");
			printf("i=%d n=%d f=%g\n", i, n, f);'
		ERROR - 'tw: scanf: not enough arguments'

	EXEC 	-n -e '	int i,n;
			float f;
			char* s = "123 3.45e6";
			i = sscanf(s,"%d %g", &n);
			printf("i=%d n=%d f=%g\n", i, n, f);'
		ERROR - 'tw: scanf: not enough arguments'

	EXEC 	-n -e '	int i,n;
			float f;
			char* s = "123 3.45e6";
			i = sscanf(s,"%d %g", &n, &f, &i);
			printf("i=%d n=%d f=%g\n", i, n, f);'
		ERROR - 'tw: scanf: i: too many arguments'

	EXEC 	-n -e '	int i,n;
			float f;
			char* s = "123 3.45e6";
			i = sscanf(s,"%d %g", &f, &f);
			printf("i=%d n=%d f=%g\n", i, n, f);'
		ERROR - 'tw: scanf: f: integer variable address argument expected'

	EXEC 	-n -e '	int i,n;
			float f;
			char* s = "123 3.45e6";
			i = sscanf(s,"%d %g", &n, &n);
			printf("i=%d n=%d f=%g\n", i, n, f);'
		ERROR - 'tw: scanf: n: floating variable address argument expected'
