.xx meta.description="pzip compresses/decompresses fixed length record data"
.xx meta.keywords="compression"
.MT 4
.TL
AT&T Labs Research pzip command
.AF "AT&T Labs Research - Florham Park NJ"
.AU "Glenn Fowler <gsf@research.att.com>"
.H 1 Abstract
Fixed length record data, although easy to access, is often viewed
as a waste of space.
Many projects go to great lengths optimizing data schemas to save space,
but complicate the data interface in the process.
.B pzip
shows that in many cases this view of fixed length data is wrong.
In fact, variable length data may become more compressible when
converted to a sparse, fixed length format.
Intense semantic schema
analysis can be replaced by an automated record partition, resulting in
compression space improvements of 2 to 10 times and decompression speed
improvements of 2 to 3 times over
.B gzip
for a large class of data.
.H 1 Description
.BR pzip (1)
compresses and decompresses data files of fixed length rows (records) and
columns (fields). It performs better than
.BR gzip (1)
in space/time on data that
has many (typically > 50%) columns that change at a low rate (columns with a
low rate of change are low frequency; columns with a high rate of change are
high frequency).
.PP
The
.B pzip
compress format is itself gzipped; decompressed data is reorganized
according to the user-specified partition file
before being passed to
.BR gzip .
Low frequency columns are difference
encoded and high frequency column groups are transposed to column-major order.
The
.B gzip
tables are flushed between each column partition group. This has a
positive space/time effect on the gzip string match and huffman tables.
.PP
Two other commands are part of the
.B pzip
package.
The
.BR pop (1)
command lists column frequencies for fixed length data and the
.BR pin (1)
command induces
.B pzip
partitions from training data.
.xx begin=internal
.PP
.B pzip
is in production use for several AT&T projects and data streams:
.xx link="../pzmps/pzmps.html	Greyhound",
.xx link="../pzama/pzama.html	AMA call detail",
and the AT&T AMA database of record
.xx link="http://fry.research.att.com/~rab/vetter_memo.html	Data Barn."
The
.B gzip
portion of
.BR pzip ,
which has been modified to handle large (>4GB) files, is also used by the
.xx link="http://ning09.research.att.com/~ningaui/index.html	ningaui project."
.B pzip
is also one of the internal AT&T CIO
.xx link="http://www.csb.att.com/cio/awards/circle99winners.html	Circle of Excellence"
award winners.
.xx end=internal
.H 1 Results
This table shows timing and size results for
.B pzip
and
.BR gzip ,
run on a 400MHz sgi mips processor.
.PP
.TS
center allbox;
c s s s s s
cb cb cb cb cb cb
cb n n n n n.
US Census field group 301 / all states
COMMAND	SIZE	RATE	REAL	USER	SYS
raw	342,279,796	1.0	\^	\^	\^
gzip	31,471,465	10.9	4m04.22s	3m58.79s	0m04.40s
pzip	17,549,599	19.5	2m26.95s	1m53.72s	0m03.58s
gunzip	\^	\^	0m29.82s	0m28.63s	0m00.67s
punzip	\^	\^	0m10.52s	0m09.81s	0m00.43s
.TE
.xx begin=internal
.PP
Although you can download the source and binaries for your own evaluation,
you can also send data to the
.xx link="http://pzip.research.att.com/survey/	survey"
site, which will determine if the data is suitable for
.BR pzip .
.xx end=internal
.H 1 Publications
.BL
.LI
.xx link="../publications/pzip-2002-1.pdf	Improving Table Compression with Combinatorial Optimization",
with Adam Buchsbaum and Raffaele Giancarlo,
in Proc. 13th ACM-SIAM Symp. on Discrete Algorithms, pp. 213-22, 2002. 
.xx begin=internal
.LI
.xx link=../publications/"pzip-1999-4.html	Dynamically Induced Compression",
with Don Caldwell and Ken Church,
AT&T Software Symposium, Middletown, NJ, 1999.
.xx end=internal
.LI
.xx link="../publications/pzip-2000-1.pdf	Engineering the Compression of Massive Tables: An Experimental Approach",
with Adam Buchsbaum and Don Caldwell and Ken Church and S. Muthukrishnan,
in Proc. 11th ACM-SIAM Symp. on Discrete Algorithms, pp. 175-184, 2000.
.xx begin=internal
.LI
.xx link="../publications/pzip-1999-2.pdf	A Lossless Data Partition Compressor",
with Don Caldwell and Ken Church,
a draft looking for a home,
.xx link="../publications/pzip-1999-2-talk.pdf	presentation".
.LI
.xx link="../publications/pzip-1999-1.pdf	Using Quantitative Methods to Compress Call Detail: From Data Warehouse to Data Publishing",
with Don Caldwell and Ken Church,
AT&T QUAC Symposium, February 1999,
.xx link="../publications/pzip-1999-1-talk.ppt	powerpoint presentation".
.xx end=internal
.LE
.H 1 Examples
Generate a partition, letting
.B pin
determine the row size and high frequency cutoff:
.EX
pin test.dat > test.prt
.EE
Generate a partition with a 10% high frequency cutoff for 100 byte record
fixed length data and trace the progress:
.EX
pin -v -r 100 -h 10% test.dat > test.prt
.EE
Compress the data:
.EX
pzip -p test.prt test.dat > test.pz
.EE
Decompress the data:
.EX
pzip test.pz > t
.EE
.sh download pzip $WWWTYPES
.H 1 Usage
The binary tarballs contain executables for
.BR pop ,
.BR pin ,
and
.BR pzip .
The man page for each command can be listed on the standard error
in text form using the
.B --man
option or in html form using the
.B --html
option.
Use the
.B --?help
option for help details.
