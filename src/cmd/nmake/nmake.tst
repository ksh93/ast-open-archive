# nmake regression tests

function DATA
{
	typeset f i
	for f
	do	case $f in
		L.dir)	mkdir L.dir
			for i in 001 002 003 004
			do	print $i > L.dir/$i
			done
			;;
		esac
	done
}

TEST 001 ':H: sort edit op'

	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H)'
		OUTPUT - $'a a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=)'
		OUTPUT - $'a a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H<)'
		OUTPUT - $'a a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H>)'
		OUTPUT - $'c b a a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H!)'
		OUTPUT - $'a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H!=)'
		OUTPUT - $'a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=F)'
		OUTPUT - $'a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=I)'
		OUTPUT - $'c b a a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=N)'
		OUTPUT - $'a a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=U)'
		OUTPUT - $'a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=V)'
		OUTPUT - $'a a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=NF)'
		OUTPUT - $'a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=NI)'
		OUTPUT - $'c b a a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=NU)'
		OUTPUT - $'a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=NV)'
		OUTPUT - $'a a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=VF)'
		OUTPUT - $'a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=VI)'
		OUTPUT - $'c b a a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=VU)'
		OUTPUT - $'a b c'

	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H)'
		OUTPUT - $'02 1 10 2'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=)'
		OUTPUT - $'1 02 2 10'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H<)'
		OUTPUT - $'02 1 10 2'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H>)'
		OUTPUT - $'2 10 1 02'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H!)'
		OUTPUT - $'02 1 10 2'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H!=)'
		OUTPUT - $'1 02 10'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=F)'
		OUTPUT - $'02'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=I)'
		OUTPUT - $'2 10 1 02'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=N)'
		OUTPUT - $'1 02 2 10'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=U)'
		OUTPUT - $'02 1 10 2'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=V)'
		OUTPUT - $'10 2 02 1'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=NF)'
		OUTPUT - $'1'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=NI)'
		OUTPUT - $'10 2 02 1'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=NU)'
		OUTPUT - $'1 02 10'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=NV)'
		OUTPUT - $'10 2 02 1'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=VF)'
		OUTPUT - $'10'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=VI)'
		OUTPUT - $'1 02 2 10'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=VU)'
		OUTPUT - $'10 2 1'

	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H<)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H>)'
		OUTPUT - $'a-1.2.3 a-02.3.4 a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H!)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=F)'
		OUTPUT - $'a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=I)'
		OUTPUT - $'a-1.2.3 a-02.3.4 a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=N)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=U)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=V)'
		OUTPUT - $'a-02.3.4 a-1.2.3 a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=NF)'
		OUTPUT - $'a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=NI)'
		OUTPUT - $'a-1.2.3 a-02.3.4 a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=NU)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=NV)'
		OUTPUT - $'a-02.3.4 a-1.2.3 a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=VF)'
		OUTPUT - $'a-02.3.4'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=VI)'
		OUTPUT - $'a-1.2.3 a-01.2.3 a-02.3.4'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=VU)'
		OUTPUT - $'a-02.3.4 a-1.2.3'

TEST 002 ':I: intersection edit op'

	EXEC	-n -f - . 'A = c 2 b 10 a 02 1 a' 'print -- $(A:I)'
		OUTPUT - $''
	EXEC	-n -f - . 'A = c 2 b 10 a 02 1 a' 'print -- $(A:I<)'
		OUTPUT - $''

TEST 003 ':L: glob list edit op'

	DO	DATA L.dir
	EXEC	-n -f - . 'print -- $("L.dir":L)'
		OUTPUT - $'001 002 003 004'
	EXEC	-n -f - . 'print -- $("L.dir":L>)'
		OUTPUT - $'004'
	EXEC	-n -f - . 'print -- $("L.dir":L<)'
		OUTPUT - $'001'
	EXEC	-n -f - . 'print -- $("L.dir":L=)'
		OUTPUT - $'001 002 003 004'
	EXEC	-n -f - . 'print -- $("L.dir":L!)'
		OUTPUT - $'L.dir/001 L.dir/002 L.dir/003 L.dir/004'

TEST 004 ':N: file match edit op'

	EXEC	-n -f - . 'A = c 2 b 10 a 02 1 a' 'print -- $(A:N)'
		OUTPUT - $''

TEST 005 ':O: ordinal edit op'

	EXEC	-n -f - . 'A = c 2 b 10 a 02 1 a' 'print -- $(A:O)'
		OUTPUT - $'8'
	EXEC	-n -f - . 'A = c 2 b 10 a 02 1 a' 'print -- $(A:O=)'
		OUTPUT - $'8'

TEST 006 ':Q: quoting'

	EXEC	-n -f - . 'A = "a z"' 'print -- $(A:Q)'
		OUTPUT - $'\\""a z"\\"'
	EXEC	-n -f - . "A = 'a z'" 'print -- $(A:Q)'
		OUTPUT - $'"\'a z\'"'
	EXEC	-n -f - . 'A = a z' 'print -- $(A:Q)'
		OUTPUT - $'a z'
	EXEC	-n -f - . 'A = a\z' 'print -- $(A:Q)'
		OUTPUT - $'\'a\\z\''
	EXEC	-n -f - . 'A = a$z' 'print -- $(A:Q)'
		OUTPUT - $'\'a\$z\''
	EXEC	-n -f - . 'A = "a z"' 'print -- $(A:@Q)'
		OUTPUT - $'\\""a z"\\"'
	EXEC	-n -f - . "A = 'a z'" 'print -- $(A:@Q)'
		OUTPUT - $'"\'a z\'"'
	EXEC	-n -f - . 'A = a z' 'print -- $(A:@Q)'
		OUTPUT - $'\'a z\''
	EXEC	-n -f - . 'A = a\z' 'print -- $(A:@Q)'
		OUTPUT - $'\'a\\z\''
	EXEC	-n -f - . 'A = a$z' 'print -- $(A:@Q)'
		OUTPUT - $'\'a\$z\''

TEST 007 ':T=D: quoting'

	EXEC	-n -f - . 'A == "a z"' 'print -- $("(A)":T=D)'
		OUTPUT - -DA=$'\\""a z"\\"'
	EXEC	-n -f - . "A == 'a z'" 'print -- $("(A)":T=D)'
		OUTPUT - -DA=$'"\'a z\'"'
	EXEC	-n -f - . 'A == a z' 'print -- $("(A)":T=D)'
		OUTPUT - -DA=$'\'a z\''
	EXEC	-n -f - . 'A == a\z' 'print -- $("(A)":T=D)'
		OUTPUT - -DA=$'\'a\\z\''
	EXEC	-n -f - . 'A == a$z' 'print -- $("(A)":T=D)'
		OUTPUT - -DA=$'\'a\$z\''

TEST 008 ':T=E: quoting'

	EXEC	-n -f - . 'A == "a z"' 'print -- $("(A)":T=E)'
		OUTPUT - A=$'"a z"'
	EXEC	-n -f - . "A == 'a z'" 'print -- $("(A)":T=E)'
		OUTPUT - A=$'\'a z\''
	EXEC	-n -f - . 'A == a z' 'print -- $("(A)":T=E)'
	EXEC	-n -f - . 'A == a\z' 'print -- $("(A)":T=E)'
		OUTPUT - A=$'\'a\\z\''
	EXEC	-n -f - . 'A == a$z' 'print -- $("(A)":T=E)'
		OUTPUT - A=$'\'a\$z\''
