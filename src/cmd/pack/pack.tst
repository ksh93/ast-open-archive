# regression tests for the pack utilitiy

VIEW data pack.c

TEST 01 'basics'
	DO cp $data .
	EXEC	pack.c
		OUTPUT - $'pack: pack.c : 31.8% Compression'
	PROG	unpack pack.c
		OUTPUT - $'unpack: pack.c.z: unpacked'
	PROG	cmp $data pack.c
		OUTPUT -
