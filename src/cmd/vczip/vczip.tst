# regression tests for the vczip command

UNIT vczip

TEST 01 builtin methods

	EXEC	 -m rle.0,huffman
		INPUT t.dat 'hello world'
		SAME INPUT t.dat
		COPY OUTPUT t.rz
		IGNORE OUTPUT

	EXEC	 -u
		SAME INPUT t.rz
		SAME OUTPUT t.dat

	EXEC	 -m rle.0,huffman
		SAME INPUT t.dat
		COPY OUTPUT t.rz

	EXEC	 -u
		SAME INPUT t.rz
		SAME OUTPUT t.dat

	EXEC	 -m rle.0,huffman
		SAME INPUT t.dat
		COPY OUTPUT t.rz

	EXEC	 -u
		SAME INPUT t.rz
		SAME OUTPUT t.dat
