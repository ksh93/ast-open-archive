# : : generated from sort.rt by mktest : : #

# regression tests for the sort glean plugin

UNIT sort

TEST 01 basics

	EXEC	-t, -k1,1 -k2,2n '-lglean,count,min="3,3n",min="4,4",max="3,3n",max="4,4r"'
		INPUT - $'a,1,5,s
a,2,5,s
a,1,5,m
a,1,5,s
a,2,4,s
a,1,6,s
a,2,1,s
a,2,1,m
a,2,9,s
a,2,9,m
a,1,2,m
a,1,6,m
a,2,7,m
a,1,4,s
a,1,5,s
a,2,9,m'
		OUTPUT - $'1/1 a,1,5,s
1/2 a,2,5,s
2/3 a,1,5,m
2/5 a,2,4,s
4/6 a,1,6,s
3/7 a,2,1,s
4/8 a,2,1,m
5/9 a,2,9,s
6/10 a,2,9,m
5/11 a,1,2,m
6/12 a,1,6,m'

	EXEC	-t, -k1,1 -k2,2n '-lglean,count,min="3,3n"'
		OUTPUT - $'1/1 a,1,5,s\n1/2 a,2,5,s\n2/5 a,2,4,s\n3/7 a,2,1,s\n5/11 a,1,2,m'

	EXEC	-t, -k1,1 -k2,2n '-lglean,count,min="3,3n",min="4,4"'
		OUTPUT - $'1/1 a,1,5,s
1/2 a,2,5,s
2/3 a,1,5,m
2/5 a,2,4,s
3/7 a,2,1,s
4/8 a,2,1,m
5/11 a,1,2,m'

	EXEC	-t, -k1,1 -k2,2n '-lglean,count,min:="3,3n",min:="4,4"'

	EXEC	-t, -k1,1 -k2,2n '-lglean,count,max="3,3n"'
		OUTPUT - $'1/1 a,1,5,s\n1/2 a,2,5,s\n4/6 a,1,6,s\n5/9 a,2,9,s'

	EXEC	-t, -k1,1 -k2,2n '-lglean,count,max="3,3n",max="4,4r"'
		OUTPUT - $'1/1 a,1,5,s
1/2 a,2,5,s
2/3 a,1,5,m
4/6 a,1,6,s
5/9 a,2,9,s
6/10 a,2,9,m
6/12 a,1,6,m'

	EXEC	-t, -k1,1 -k2,2n '-lglean,count,max:="3,3n",max:="4,4r"'
		OUTPUT - $'1/1 a,1,5,s\n1/2 a,2,5,s\n2/3 a,1,5,m\n4/6 a,1,6,s\n4/8 a,2,1,m\n5/9 a,2,9,s'

	EXEC	-t, -k1,1 -k2,2n '-lglean,count,min="3,3n",min="4,4",max="3,3n",max="4,4r"'
		OUTPUT - $'1/1 a,1,5,s
1/2 a,2,5,s
2/3 a,1,5,m
2/5 a,2,4,s
4/6 a,1,6,s
3/7 a,2,1,s
4/8 a,2,1,m
5/9 a,2,9,s
6/10 a,2,9,m
5/11 a,1,2,m
6/12 a,1,6,m'

	EXEC	-t, -k1,1 -k2,2n '-lglean,count,min:="3,3n",min:="4,4",max="3,3n",max="4,4r"'

	EXEC	-t, -k1,1 -k2,2n '-lglean,count,min="3,3n",min="4,4",max:="3,3n",max:="4,4r"'
		OUTPUT - $'1/1 a,1,5,s
1/2 a,2,5,s
2/3 a,1,5,m
2/5 a,2,4,s
4/6 a,1,6,s
3/7 a,2,1,s
4/8 a,2,1,m
5/9 a,2,9,s
5/11 a,1,2,m'

	EXEC	-t, -k1,1 -k2,2n '-lglean,count,min:="3,3n",min:="4,4",max:="3,3n",max:="4,4r"'
