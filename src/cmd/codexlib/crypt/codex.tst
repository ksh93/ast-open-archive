# regression tests for codex(1) { rar zip } encryption

function run # method
{
	typeset m x i j

	m=$1
	x=''
	for i in A B C D E F G H I J K L M
	do
		for j in '' 1 12
		do
			EXEC	-p passphrase ">$m"
				INPUT -n - "$x$j"
				COPY INPUT i
				MOVE OUTPUT e
			EXEC	-p passphrase "<$m"
				SAME INPUT e
				SAME OUTPUT i
		done
		x=${x}${i}${x}
	done
}

TEST 01 'rar 13'

	DO run crypt-rar-13

TEST 02 'rar 15'

	DO run crypt-rar-15

TEST 03 'rar 20'

	DO run crypt-rar-20

TEST 04 'zip'

	DO run crypt-zip
