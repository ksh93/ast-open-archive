# regression tests for codex(1) aes encryption

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
				NOTE encode aes $((${#x}+${#j}))
				INPUT -n - "$x$j"
				COPY INPUT i
				MOVE OUTPUT e
			EXEC	-p passphrase "<$m"
				NOTE decode aes $((${#x}+${#j}))
				SAME INPUT e
				SAME OUTPUT i
		done
		x=${x}${i}${x}
	done
}

TEST 01 '128 bit aes'

	DO run aes-128

TEST 02 '192 bit aes'

	DO run aes-192

TEST 03 '256 bit aes'

	DO run aes-256

TEST 04 'defaults'

	EXEC	-p passphrase ">aes-256"
		INPUT -n - "256 bit default key."
		COPY INPUT i
		MOVE OUTPUT e
	EXEC	-p passphrase "<aes"
		SAME INPUT e
		SAME OUTPUT i

	EXEC	-p passphrase ">aes"
		INPUT -n - "256 bit default key."
		COPY INPUT i
		MOVE OUTPUT e
	EXEC	-p passphrase "<aes-256"
		SAME INPUT e
		SAME OUTPUT i

TEST 05 'diagnostics'

	EXEC	-p passphrase "<aes"

	EXEC	-p passphrase "<aes"
		INPUT -n - "coax an error"
		ERROR - 'codex: truncated encryption header'
		EXIT 1

	EXEC	-p passphrase ">aes"
		INPUT -n - "coax an error"
		COPY INPUT i
		MOVE OUTPUT e
		ERROR -
		EXIT 0
	EXEC	-p passphrase "<aes-128"
		SAME INPUT e
		OUTPUT -
		ERROR - 'codex: encryption magic mismatch'
		EXIT 1
