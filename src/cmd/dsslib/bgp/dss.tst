# tests for the dss bgp method

TITLE + bgp

export TZ=EST5EDT

VIEW data

TEST 01 'cisco format'
	EXEC -x bgp -c - $data/cisco.dat
		OUTPUT - $'9941/9941'
	EXEC -q -x bgp -c 'path=~"701$"' $data/cisco.dat
		OUTPUT - $'148/9941'
	EXEC -q -x bgp -c - $data/cisco.dat
		OUTPUT - $'9941/9941'
	EXEC -q -x bgp -c 'hop=="192.121.154.25"' $data/cisco.dat
		OUTPUT - $'709/9941'
	EXEC -q -x bgp -c 'hop!="192.121.154.25"' $data/cisco.dat
		OUTPUT - $'9232/9941'
	EXEC -q -x bgp - $data/cisco-split.dat
		OUTPUT - $'Status codes: s suppressed, d damped, h history, * valid, > best, i internal, r rib-failure, S stale
Origin codes: i IGP, e EGP, ? incomplete
   Network          Next Hop            Metric LocPrf Weight Path
*>i3.0.0.0/8        12.123.205.79                 100      0 80 i
* i                 12.123.205.79                 100      0 80 i
*> 4.0.0.0/8        4.24.224.53                    82      0 1 ?
*> 4.24.224.52/30   0.0.0.0                            32768 i
*>i4.25.111.8/30    12.123.29.245                 100      0 i
* i                 12.123.29.245                 100      0 i
*> 6.0.0.0/20       208.51.6.25                    82      0 3549 i
*>i6.1.0.0/16       192.205.31.34                  80      0 668 7170 1455 i
* i                 192.205.31.34                  80      0 668 7170 1455 i
* i152.158.208.196/30
                    12.123.29.235           10    100      0 2687 ?
*>i                 12.123.29.235           10    100      0 2687 ?
* i152.158.208.244/30
                    12.123.29.235           10    100      0 2687 ?
*>i                 12.123.29.235           10    100      0 2687 ?
*>i12.24.174.125/32 12.127.5.250                  100      0 ?
*>i12.224.174.125/32
                    12.127.5.250                  100      0 ?
*>i112.224.174.125/32
                    12.127.5.250                  100      0 ?'
	EXEC -q -x bgp -a 'hop=="12.127.5.250"' $data/cisco-split.dat
		OUTPUT - $'*>i12.24.174.125/32 12.127.5.250                  100      0 ?
*>i12.224.174.125/32
                    12.127.5.250                  100      0 ?
*>i112.224.174.125/32
                    12.127.5.250                  100      0 ?'

TEST 02 'mrt format'
	EXEC -x bgp -c - $data/mrt.dat
		OUTPUT - $'19649/19649'
	EXEC -q -x bgp -c 'path=~"701$"' $data/mrt.dat
		OUTPUT - $'446/19649'
		ERROR -
	EXEC -q -x bgp -c - $data/mrt.dat
		OUTPUT - $'19649/19649'
	EXEC -q -x bgp -c 'hop=="202.12.28.190"' $data/mrt.dat
		OUTPUT - $'1159/19649'
	EXEC -q -x bgp -c 'hop!="202.12.28.190"' $data/mrt.dat
		OUTPUT - $'18490/19649'
	EXEC -q -x bgp -p '%-16(prefix)s %(path)s' 'path=~"1 701$"' $data/mrt.dat
		OUTPUT - $'198.153.142.0/24 4777,2497,1,701
198.71.67.0/24   4777,2497,1,701
198.71.67.0/24   9177,6730,1,701
147.81.0.0/16    4777,2497,1,701'
	EXEC -q -x bgp -p '%-16(prefix)s %(path)s %(src_as)s %(src_as)d %(src_as)u' 'prefix=="147.81.0.0/16"&&path=~"4777 - 1"' $data/mrt.dat
		OUTPUT - $'147.81.0.0/16    4777,2497,1,701 4777 4777 4777'
	EXEC -x bgp -p '%(path)s' 'path=~"26788$"' $data/ripe-08.dat
		OUTPUT - $'6066,701,6327,19752,{26788}
6066,6395,19752,26788
6066,6395,19752,{14456,26230,26788,26872,27168}
6066,701,6327,19752,{26788}
6066,6395,19752,26788
6066,6395,19752,{14456,26230,26788,26872,27168}
6066,6395,19752,26788
6066,6395,19752,26788
6066,6395,19752,26788
6066,6395,19752,26788
6066,6395,19752,26788
6066,701,6347,12059,26788,26788,26788,26788'
	EXEC -x bgp -p '%(path)s' 'path=~"19752,26788$"' $data/ripe-08.dat
		OUTPUT - $'6066,701,6327,19752,{26788}
6066,6395,19752,26788
6066,6395,19752,{14456,26230,26788,26872,27168}
6066,701,6327,19752,{26788}
6066,6395,19752,26788
6066,6395,19752,{14456,26230,26788,26872,27168}
6066,6395,19752,26788
6066,6395,19752,26788
6066,6395,19752,26788
6066,6395,19752,26788
6066,6395,19752,26788'
	EXEC -x bgp -p '%(path)s' 'path=~"26872 27168$"' $data/ripe-08.dat
		OUTPUT -

TEST 03 'value maps, route_btoa style'
	EXEC -q -I$data -x dss:bgp-map -p $'TIME: %(time:%m/%d/%y %H:%M:%S)s\nTYPE: %(type)s\nFROM: %(src_addr)s %(src_as)s\nTO: %(dst_addr)s AS%(dst_as)d\nORIGIN: %(origin)s\nMULTI_EXIT_DISC: %(med)d\nAGGREGATOR: %(agg_addr)s AS%(agg_as)d\nASPATH: %(path:s | )s\nNEXT_HOP: %(hop)s\nCOMMUNITY: %(community:u )s\nANNOUNCE: %(prefix)s\n' 'path=~"1 701$"' $data/mrt.dat
		OUTPUT - $'TIME: 08/16/01 10:41:19
TYPE: announce
FROM: 202.12.28.190 Asia Pacific Network Information Centre
TO: 193.0.0.1 AS12654
ORIGIN: IGP
MULTI_EXIT_DISC: 0
AGGREGATOR: 0.0.0.0 AS0
ASPATH: Asia Pacific Network Information Centre | IIJ | BBN Planet | UUNET Technologies, Inc.
NEXT_HOP: 202.12.28.190
COMMUNITY: 
ANNOUNCE: 198.153.142.0/24

TIME: 08/16/01 10:43:16
TYPE: announce
FROM: 202.12.28.190 Asia Pacific Network Information Centre
TO: 193.0.0.1 AS12654
ORIGIN: IGP
MULTI_EXIT_DISC: 0
AGGREGATOR: 0.0.0.0 AS0
ASPATH: Asia Pacific Network Information Centre | IIJ | BBN Planet | UUNET Technologies, Inc.
NEXT_HOP: 202.12.28.190
COMMUNITY: 
ANNOUNCE: 198.71.67.0/24

TIME: 08/16/01 10:43:56
TYPE: announce
FROM: 212.47.190.1 Nextra (Schweiz) AG
TO: 193.0.0.1 AS12654
ORIGIN: IGP
MULTI_EXIT_DISC: 0
AGGREGATOR: 0.0.0.0 AS0
ASPATH: Nextra (Schweiz) AG | sunrise (TDC Switzerland AG) | BBN Planet | UUNET Technologies, Inc.
NEXT_HOP: 212.47.190.1
COMMUNITY: 
ANNOUNCE: 198.71.67.0/24

TIME: 08/16/01 10:55:58
TYPE: announce
FROM: 202.12.28.190 Asia Pacific Network Information Centre
TO: 193.0.0.1 AS12654
ORIGIN: IGP
MULTI_EXIT_DISC: 0
AGGREGATOR: 0.0.0.0 AS0
ASPATH: Asia Pacific Network Information Centre | IIJ | BBN Planet | UUNET Technologies, Inc.
NEXT_HOP: 202.12.28.190
COMMUNITY: 
ANNOUNCE: 147.81.0.0/16
'
	EXEC -q -I$data -x bgp-map -p $'TIME: %(time:%m/%d/%y %H:%M:%S)s\nTYPE: %(type)s\nFROM: %(src_addr)s %(src_as)s\nTO: %(dst_addr)s AS%(dst_as)d\nORIGIN: %(origin)s\nMULTI_EXIT_DISC: %(med)d\nAGGREGATOR: %(agg_addr)s AS%(agg_as)d\nASPATH: %(path:s | )s\nNEXT_HOP: %(hop)s\nCOMMUNITY: %(community:u )s\nANNOUNCE: %(prefix)s\n' 'path=~"1 701$"' $data/mrt.dat

TEST 04 'path regex syntax'
	EXEC -q -x bgp 'path=~"[]"' $data/cisco.dat
		OUTPUT - $'Status codes: s suppressed, d damped, h history, * valid, > best, i internal, r rib-failure, S stale
Origin codes: i IGP, e EGP, ? incomplete
   Network          Next Hop            Metric LocPrf Weight Path'
		ERROR - $'dss::scan: []<<<: invalid regular expression'
		EXIT 1
	EXEC -q -x bgp 'path=~"[!]"' $data/cisco.dat
		ERROR - $'dss::scan: [!]<<<: invalid regular expression'
	EXEC -q -x bgp 'path=~/[^]/' $data/cisco.dat
		ERROR - $'dss::scan: [^]<<<: invalid regular expression'

TEST 05 'path regex matching'
	EXEC -q -x bgp -p '%(path)s' 'path=~"^1755* 6461"' $data/cisco.dat
		OUTPUT - $'1755,6461,3786,3786,3786
1755,6461,3786,3786,3786'
	EXEC -q -x bgp -p '%(path)s' 'path=~"^1755+ 6461"' $data/cisco.dat

TEST 06 'expression syntax'
	EXEC -q -x bgp -c "type=='A'" $data/mrt.dat
		OUTPUT - $'16740/19649'
	EXEC -q -x bgp -c "'A'==type" $data/mrt.dat
	EXEC -q -x bgp "(type=='A')|{count}" $data/mrt.dat
	EXEC -q -x bgp -c "type=='K'" $data/mrt.dat
		OUTPUT - $'61/19649'
	EXEC -q -x bgp "(type=='K')|{count}" $data/mrt.dat
	EXEC -q -x bgp -c "type=='N'" $data/mrt.dat
		OUTPUT - $'0/19649'
	EXEC -q -x bgp "(type=='N')|{count}" $data/mrt.dat
	EXEC -q -x bgp -c "type=='O'" $data/mrt.dat
	EXEC -q -x bgp "(type=='O')|{count}" $data/mrt.dat
	EXEC -q -x bgp -c "type=='S'" $data/mrt.dat
		OUTPUT - $'36/19649'
	EXEC -q -x bgp "(type=='S')|{count}" $data/mrt.dat
	EXEC -q -x bgp -c "type=='T'" $data/mrt.dat
		OUTPUT - $'0/19649'
	EXEC -q -x bgp "(type=='T')|{count}" $data/mrt.dat
	EXEC -q -x bgp -c "type=='W'" $data/mrt.dat
		OUTPUT - $'2812/19649'
	EXEC -q -x bgp "(type=='W')|{count}" $data/mrt.dat
	EXEC -q -x bgp -c "type!='A'" $data/mrt.dat
		OUTPUT - $'2909/19649'
	EXEC -q -x bgp "(type!='A')|{count}" $data/mrt.dat

TEST 07 'ipma router dump summaries'
	EXEC -x bgp -c - $data/ipma.dat
		OUTPUT - $'6730/6730'
	EXEC -x bgp -c - $data/ipma-txt.dat
		OUTPUT - $'6730/6730'
	EXEC -x bgp -c - $data/ipma-tab.dat
		OUTPUT - $'1433/1433'
	EXEC -x bgp -c - $data/ipma-lcl.dat
		OUTPUT - $'219/219'

TEST 08 'misc fields'
	EXEC -q -x bgp "(community_len>26)|{print '%(path_len)u|%(community_len)u|%(community)s'}" $data/mrt.dat
		OUTPUT - $'4|28|1299:1000,1755:30,1755:1000,1755:2001,1755:2101,1755:2103,1755:2104,1755:2106,1755:2109,1755:2110,1755:2111,1833:1000,3301:1000,13129:3010
5|30|1299:50,1299:1000,1755:20,1755:1000,1755:2001,1755:2101,1755:2103,1755:2104,1755:2106,1755:2109,1755:2110,1755:2111,1833:50,1833:1000,13129:3010
5|30|1299:50,1299:1000,1755:30,1755:1000,1755:2001,1755:2101,1755:2103,1755:2104,1755:2106,1755:2109,1755:2110,1755:2111,1833:50,1833:1000,13129:3010'
	EXEC -q -x bgp "<mrt.exp" $data/mrt.dat
		INPUT mrt.exp $'(community_len>26)|{print "%(path_len)u|%(community_len)u|%(community)s"}'
	EXEC -q -x bgp '(community_len>26)|{print "%(prefix:%2$d|%1$08x)s"}' $data/mrt.dat
		OUTPUT - $'24|a0080a00
19|c3b68000
19|c3b68000'

TEST 09 'expression values'
	EXEC -q -x bgp '{print "%(hop)08x %(hop)s"}' $data/cisco-split.dat
		OUTPUT - $'0c7bcd4f 12.123.205.79
0c7bcd4f 12.123.205.79
0418e035 4.24.224.53
00000000 0.0.0.0
0c7b1df5 12.123.29.245
0c7b1df5 12.123.29.245
d0330619 208.51.6.25
c0cd1f22 192.205.31.34
c0cd1f22 192.205.31.34
0c7b1deb 12.123.29.235
0c7b1deb 12.123.29.235
0c7b1deb 12.123.29.235
0c7b1deb 12.123.29.235
0c7f05fa 12.127.5.250
0c7f05fa 12.127.5.250
0c7f05fa 12.127.5.250'
	EXEC -x bgp '(prefix=="9.2.0.0/16")|{print "%(prefix)s %(path)s"}' $data/cisco.dat
		OUTPUT - $'9.2.0.0/16 1755,701
9.2.0.0/16 3333,5378,6660,{701,11}
9.2.0.0/16 2914,{701,11,22}
9.2.0.0/16 1,{701,11,22,33}
9.2.0.0/16 3557,6461,{701,11,22,33,44}
9.2.0.0/16 3557,6461,{701,11,22,33,44,55}
9.2.0.0/16 2914,{701,11,22,33,44,55,66}
9.2.0.0/16 8517,9000,2548,{701,11,22,33,44,55,66,77}
9.2.0.0/16 10764,1,{701,11,22,33,44,55,66,77,88}
9.2.0.0/16 715,{701,11,22,33,44,55,66,77,88,99}
9.2.0.0/16 3561,701
9.2.0.0/16 2551,701
9.2.0.0/16 2828,701
9.2.0.0/16 7018,701
9.2.0.0/16 2497,701'

TEST 10 'string OP prefix and generic vs explicit prefix type'

	EXEC -q -x bgp '(prefixv4 =~ "65.168.0.0/20")|{count}' $data/mrt.dat
		OUTPUT - $'26/19649'
	EXEC -q -x bgp '(prefix =~ "65.168.0.0/20")|{count}' $data/mrt.dat
	EXEC -q -x bgp '("65.168.0.0/20" =~ prefixv4)|{count}' $data/mrt.dat
		OUTPUT - $'123/19649'
#######	EXEC -q -x bgp '("65.168.0.0/20" =~ prefix)|{count}' $data/mrt.dat

	EXEC -q -x bgp '(prefix != "0.0.0.0/0" && "65.168.0.0/20" =~ prefixv4)|{count}' $data/mrt.dat
		OUTPUT - $'26/19649'
#######	EXEC -q -x bgp '(prefix != "0.0.0.0/0" && "65.168.0.0/20" =~ prefix)|{count}' $data/mrt.dat

TEST 11 'message group index and indicator'
	EXEC -q -x bgp '(src_addr=="195.211.29.254")|{print "%4(message)d %(MESSAGE)d %(type)c %(src_addr)s"}' $data/mrt.dat
		OUTPUT - $' 241 1 S 195.211.29.254
 915 1 S 195.211.29.254
2179 1 S 195.211.29.254
2727 1 S 195.211.29.254
3801 1 S 195.211.29.254
4502 0 S 195.211.29.254
5532 0 S 195.211.29.254
6062 0 S 195.211.29.254
6932 0 S 195.211.29.254
7473 1 S 195.211.29.254'

TEST 12 "Tim Griffin's bgpdump format"
	EXEC -q -x bgp "
		(src_addr=='212.47.190.1'&&time<=997972890)?{}:{return};
		(!PART&&(type=='A'||type=='W'))?
			{print 'H|BGP4MP|%(time)d|%(src_addr)s|%(src_as)d|%(path:u )s|%(origin)s|%(hop)s|%(local)d|%(med)d|%(community:u )s|%(atomic)d|%(agg_addr)s|%(agg_as)d|%(originator)s|%(cluster:.)s'}:
			{};
		(type=='A'||type=='W')?
			{print '%(type)c|%(prefix)s'}:
		(type=='S')?
			{print '%(type)c|%(src_addr)s|%(src_as)d|%(old_state)s|%(new_state)s'}:
			{print '%(type)c|%(src_addr)s|%(src_as)d'}
		" $data/mrt.dat
		OUTPUT - $'H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 701 6506|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|204.238.232.0/24
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 701 15034 6429 6429 6429|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|199.186.25.0/24
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 9057 13126 9154 12829|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|80.80.128.0/20
A|213.137.33.0/24
A|213.137.34.0/23
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 5549 5549 5378 5551 5713 8094|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|143.160.4.0/22
A|143.160.8.0/22
A|143.160.16.0/22
A|143.160.24.0/22
A|143.160.28.0/22
A|143.160.32.0/22
A|143.160.64.0/22
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 701 6461 715|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|131.161.200.0/22
A|131.161.200.0/21
A|131.161.208.0/20
A|131.161.217.0/24
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 701 14788|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|63.89.29.0/24
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 701 7046|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|198.71.64.0/24
A|198.71.65.0/24
A|198.71.66.0/24
A|198.71.68.0/24
A|198.71.69.0/24
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 5669 8495|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|141.187.0.0/16
A|195.34.160.0/19
H|BGP4MP|997972878|212.47.190.1|9177|9177 6730 3561 701 10455|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|135.118.6.0/24
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 1239 3549 11042 8876 8831|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|62.108.64.0/22
A|62.108.68.0/22
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 701 1239 5676|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|156.70.0.0/16
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 1239 3549 11042 8876 15406|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|193.255.106.0/24
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 701|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|65.126.138.0/23
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 701 15290 2169|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|204.104.132.0/24
A|204.104.133.0/24
A|204.104.134.0/24
A|204.104.135.0/24
H|BGP4MP|997972878|212.47.190.1|9177|9177 6730 3561 701|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|198.71.67.0/24
A|198.153.142.0/24
H|BGP4MP|997972878|212.47.190.1|9177|9177 6730 3549 1239 568 721 5972 1778|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|159.133.0.0/16
A|164.65.0.0/16
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 1239 3549 11042 8876 20871|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|80.71.128.0/20
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 1239 3549 11042 8876 8456|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|193.140.192.0/20
A|193.140.208.0/21
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 701 209|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|209.211.0.0/22
H|BGP4MP|997972878|212.47.190.1|9177|9177 6730 701 1767|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|165.139.0.0/20
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 8297 17426|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|203.115.120.0/24
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 701 6429 19338 19338 19338 19632|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|64.76.152.0/24
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 9057 3356 19786 17443 17426|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|203.115.100.0/22
K|212.47.190.1|9177
H|BGP4MP|997972890|212.47.190.1|9177|9177 8210 1239 701|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|198.71.67.0/24
A|198.153.142.0/24
H|BGP4MP|997972890|212.47.190.1|9177|9177 8210 2874|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|160.8.0.0/16
H|BGP4MP|997972890|212.47.190.1|9177|9177 8210 1239 701 5713 8094|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|143.160.4.0/22
A|143.160.8.0/22
A|143.160.16.0/22
A|143.160.24.0/22
A|143.160.28.0/22
A|143.160.32.0/22
A|143.160.64.0/22
H|BGP4MP|997972890|212.47.190.1|9177|9177 8210 1299 12476 12476 12992|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|193.218.115.0/24
H|BGP4MP|997972890|212.47.190.1|9177|9177 6730 6453 1239 6461 715|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|131.161.200.0/22
A|131.161.200.0/21
A|131.161.208.0/20
A|131.161.217.0/24
H|BGP4MP|997972890|212.47.190.1|9177|9177 6730 5400 5727 7018 3549 11042 8876 20871|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|80.71.128.0/20
H|BGP4MP|997972890|212.47.190.1|9177|9177 8210 3561 3561 3967 6354|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|64.89.33.0/24
H|BGP4MP|997972890|212.47.190.1|9177|9177 8210 1299 12476 12476|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|212.76.32.0/19
H|BGP4MP|997972890|212.47.190.1|9177|9177 8210 5511 8866 8866 8795 9154 9154 9154 9154 9154 9154 9154 9154|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|213.137.32.0/24
H|BGP4MP|997972890|212.47.190.1|9177|9177 8210 5511 8866 8866 8795 8795 8795 8795 12829 12829 12829 12829 12829 12829 12829 12829 12829|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|80.80.128.0/20
A|213.137.33.0/24
A|213.137.34.0/23
H|BGP4MP|997972890|212.47.190.1|9177|9177 6730 5400 5727 7018 3549 11042 8876 8456|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|193.140.192.0/20
A|193.140.208.0/21
H|BGP4MP|997972890|212.47.190.1|9177|9177 8210 1299 12476 12476 15930|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|193.41.120.0/22
H|BGP4MP|997972890|212.47.190.1|9177|9177 8210 3561 3561 701 10455|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|135.118.6.0/24
H|BGP4MP|997972890|212.47.190.1|9177|9177 8210 1755 8246 20486|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|193.178.213.0/24
H|BGP4MP|997972890|212.47.190.1|9177|9177 8210 1755 286 209 13663 11836|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|216.231.135.0/24
H|BGP4MP|997972890|212.47.190.1|9177|9177 6730 5400 5727 7018 3549 11042 8876 8831|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|62.108.64.0/22
A|62.108.68.0/22
H|BGP4MP|997972890|212.47.190.1|9177|9177 6730 5400 5727 7018 3549 11042 8876 15406|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|193.255.106.0/24'
	EXEC -q -x bgp "
		(src_addr=='212.47.190.1'&&time<=997972890)?{}:{return};
		(path=~'701')?{}:{return terminate};
		(!PART&&(type=='A'||type=='W'))?
			{print 'H|BGP4MP|%(time)d|%(src_addr)s|%(src_as)d|%(path:u )s|%(origin)s|%(hop)s|%(local)d|%(med)d|%(community:u )s|%(atomic)d|%(agg_addr)s|%(agg_as)d|%(originator)s|%(cluster:.)s'}:
			{};
		(type=='A'||type=='W')?
			{print '%(type)c|%(prefix)s'}:
		(type=='S')?
			{print '%(type)c|%(src_addr)s|%(src_as)d|%(old_state)s|%(new_state)s'}:
			{print '%(type)c|%(src_addr)s|%(src_as)d'}
		" $data/mrt.dat
		OUTPUT - $'H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 701 6506|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|204.238.232.0/24
H|BGP4MP|997972878|212.47.190.1|9177|9177 8210 701 15034 6429 6429 6429|105|212.47.190.1|0|0||0|0.0.0.0|0|0.0.0.0|
A|199.186.25.0/24'
		EXIT 1

TEST 13 "ipv6 bgp rib and update data from http://archive.routeviews.org"

	EXEC -I$data -x bgp '(prefixv6=~"2001:400::/26")|{print "%(type)c %-24(prefix)s %-24(hop)s %(path)s"}' updates.20080601.0000.bz2
		OUTPUT - $'A 2001:400::/32            2001:918:0:8::1          3303,6939,293
A 2001:418::/32            2001:918:0:8::1          3303,2914
A 2001:418:3F6::/48        2001:918:0:8::1          3303,2914
A 2001:420::/32            2001:918:0:8::1          3303,6453,109
A 2001:420::/35            2001:918:0:8::1          3303,6453,109
A 2001:428::/32            2001:918:0:8::1          3303,6939,209
W 2001:418:3F6::/48        0.0.0.0                  
A 2001:418::/32            2001:918:0:8::1          3303,6175,2914
A 2001:400::/32            2001:918:0:8::1          3303,6175,293
A 2001:420::/32            2001:918:0:8::1          3303,6175,109
A 2001:420::/35            2001:918:0:8::1          3303,6175,109
A 2001:428::/32            2001:918:0:8::1          3303,6175,2497,209
A 2001:400::/32            2001:918:0:8::1          3303,6939,293
A 2001:418::/32            2001:918:0:8::1          3303,2914
A 2001:418:3F6::/48        2001:918:0:8::1          3303,2914
A 2001:420::/32            2001:918:0:8::1          3303,6453,109
A 2001:420::/35            2001:918:0:8::1          3303,6453,109
A 2001:428::/32            2001:918:0:8::1          3303,6939,209
W 2001:418:3F6::/48        0.0.0.0                  
A 2001:418::/32            2001:918:0:8::1          3303,6175,2914
A 2001:400::/32            2001:918:0:8::1          3303,6175,293
A 2001:420::/32            2001:918:0:8::1          3303,6175,109
A 2001:420::/35            2001:918:0:8::1          3303,6175,109
A 2001:428::/32            2001:918:0:8::1          3303,6175,2497,209
A 2001:400::/32            2001:918:0:8::1          3303,6939,293
A 2001:418::/32            2001:918:0:8::1          3303,2914
A 2001:418:3F6::/48        2001:918:0:8::1          3303,2914
A 2001:420::/32            2001:918:0:8::1          3303,6453,109
A 2001:420::/35            2001:918:0:8::1          3303,6453,109
A 2001:428::/32            2001:918:0:8::1          3303,6939,209
W 2001:418:3F6::/48        0.0.0.0                  
A 2001:418::/32            2001:918:0:8::1          3303,6175,2914
A 2001:400::/32            2001:918:0:8::1          3303,6175,293
A 2001:420::/32            2001:918:0:8::1          3303,6175,109
A 2001:420::/35            2001:918:0:8::1          3303,6175,109
A 2001:428::/32            2001:918:0:8::1          3303,6175,2497,209
A 2001:400::/32            2001:918:0:8::1          3303,6939,293
A 2001:418::/32            2001:918:0:8::1          3303,2914
A 2001:418:3F6::/48        2001:918:0:8::1          3303,2914
A 2001:420::/32            2001:918:0:8::1          3303,6453,109
A 2001:420::/35            2001:918:0:8::1          3303,6453,109
A 2001:428::/32            2001:918:0:8::1          3303,6939,209
W 2001:418:3F6::/48        0.0.0.0                  
A 2001:418::/32            2001:918:0:8::1          3303,6175,2914
A 2001:428::/32            2001:918:0:8::1          3303,6175,2497,209
A 2001:420::/32            2001:918:0:8::1          3303,6175,109
A 2001:420::/35            2001:918:0:8::1          3303,6175,109
A 2001:400::/32            2001:918:0:8::1          3303,6175,293'

	EXEC -I$data -x bgp '(prefixv6=~"2001:700::/28")|{print "%(type)c %-24(prefix)s %-24(hop)s %(path)s"}' rib.20080601.0108.bz2
		OUTPUT - $'T 2001:700::/32            0.0.0.0                  18084,6175,2603,224
T 2001:700::/32            0.0.0.0                  3303,6175,2603,224
T 2001:700::/32            0.0.0.0                  3257,1299,2603,224
T 2001:700::/32            0.0.0.0                  2497,1273,1299,2603,224
T 2001:700::/32            0.0.0.0                  30071,41692,2603,224
T 2001:700::/32            0.0.0.0                  30071,41692,2603,224
T 2001:700::/32            0.0.0.0                  33437,6939,2603,224
T 2001:700::/32            0.0.0.0                  30071,41692,2603,224
T 2001:700::/32            0.0.0.0                  13237,2603,224
T 2001:700::/32            0.0.0.0                  2914,2603,224
T 2001:700::/32            0.0.0.0                  2914,2603,224
T 2001:700::/32            0.0.0.0                  7781,6175,2603,224
T 2001:700::/32            0.0.0.0                  6939,2603,224
T 2001:700::/32            0.0.0.0                  7660,24287,24489,20965,2603,224
T 2001:700::/32            0.0.0.0                  3292,2603,224
T 2001:700::/32            0.0.0.0                  7018,3356,20965,2603,224
T 2001:708::/32            0.0.0.0                  18084,6175,2603,1741
T 2001:708::/32            0.0.0.0                  3303,6175,2603,1741
T 2001:708::/32            0.0.0.0                  3257,1299,2603,1741
T 2001:708::/32            0.0.0.0                  2497,1273,1299,2603,1741
T 2001:708::/32            0.0.0.0                  30071,41692,2603,1741
T 2001:708::/32            0.0.0.0                  30071,41692,2603,1741
T 2001:708::/32            0.0.0.0                  33437,6939,2603,1741
T 2001:708::/32            0.0.0.0                  30071,41692,2603,1741
T 2001:708::/32            0.0.0.0                  13237,2603,1741
T 2001:708::/32            0.0.0.0                  2914,2603,1741
T 2001:708::/32            0.0.0.0                  2914,2603,1741
T 2001:708::/32            0.0.0.0                  7781,6175,2603,1741
T 2001:708::/32            0.0.0.0                  6939,2603,1741
T 2001:708::/32            0.0.0.0                  7660,24287,24489,20965,2603,1741
T 2001:708::/32            0.0.0.0                  7018,6175,2603,1741'

	EXEC -I$data -x bgp '{print "%(prefix)s %(path)s"}' rib.20090316.2200.dat
		OUTPUT - $'339:752F:14::/48 7781,6453,6175,1257,1880
339:752F:14::/48 33437,30071,1257,1880
339:752F:14::/48 7018,6175,1257,1880
2001::/32 7781,30071,12637
2001::/32 33437,2914,7019
2001::/32 2497,1273,8767,29259
2001::/32 18084,3549,12637
2001::/32 27664,12956,6762,1257
2001::/32 13237,8767,29259
2001::/32 6939,12859
2001::/32 209,2914,7019
2001::/32 2914,7019
2001::/32 7018,6453,39556
2001::/32 7660,22388,11537,59
2001::/32 209,6939,12859
2001::/32 2914,7019
2001::/32 209,2914,7019
2001:200::/32 7781,6453,2914,2500
2001:200::/32 33437,2914,2500
2001:200::/32 2497,2500
2001:200::/32 18084,6939,2500
2001:200::/32 27664,12956,6939,2500
2001:200::/32 6939,2500
2001:200::/32 13237,2914,2500
2001:200::/32 209,2914,2500
2001:200::/32 2914,2500
2001:200::/32 7018,2914,2500
2001:200::/32 7660,2500
2001:200::/32 209,6939,2500
2001:200::/32 2914,2500
2001:200::/32 209,2914,2500'

TEST 14 "generic anonymized parser exercises"

	EXEC	-x bgp '{print --all}' $data/a01.mrt
		SAME OUTPUT $data/a01.out

	EXEC	-x bgp '{print --all}' $data/a02.mrt
		SAME OUTPUT $data/a02.out

	EXEC	-x bgp '{print --all}' $data/a03.mrt
		SAME OUTPUT $data/a03.out

	EXEC	-x bgp '{print --all}' $data/a04.mrt
		SAME OUTPUT $data/a04.out

	EXEC	-x bgp '{print --all}' $data/a05.mrt
		SAME OUTPUT $data/a05.out

	EXEC	-x bgp '{print --all}' $data/a06.mrt
		SAME OUTPUT $data/a06.out

	EXEC	-x bgp '{print --all}' $data/a07.mrt
		SAME OUTPUT $data/a07.out

	EXEC	-x bgp '{print --all}' $data/a08.mrt
		SAME OUTPUT $data/a08.out

	EXEC	-x bgp '{print --all}' $data/a10.mrt
		SAME OUTPUT $data/a10.out

	EXEC	-x bgp '{print --all}' $data/a11.mrt
		SAME OUTPUT $data/a11.out

	EXEC	-x bgp '{print --all}' $data/a12.mrt
		SAME OUTPUT $data/a12.out

	EXEC	-x bgp '{print --all}' $data/a13.mrt
		SAME OUTPUT $data/a13.out

	EXEC	-x bgp '{print --all}' $data/a14.mrt
		SAME OUTPUT $data/a14.out

	EXEC	-x bgp '{print --all}' $data/a15.mrt
		SAME OUTPUT $data/a15.out

TEST 15 "feature specific parser exercises"

	EXEC	-x bgp '(path =~ "26677 $")|{print "%(typeof(path))s %(sizeof(path))u %(path)s"}' $data/rib.20080601.0108.bz2
		OUTPUT - $'as16path_t 7 18084,2500,7660,22388,11537,6509,{271,2884,7860,8111,15296,26677}
as16path_t 9 3303,6175,2497,2500,7660,22388,11537,6509,{271,2884,7860,8111,15296,26677}
as16path_t 8 3257,2497,2500,7660,22388,11537,6509,{271,2884,7860,8111,15296,26677}
as16path_t 7 2497,2500,7660,22388,11537,6509,{271,2884,7860,8111,15296,26677}
as16path_t 9 30071,6175,2497,2500,7660,22388,11537,6509,{271,2884,7860,8111,15296,26677}
as16path_t 9 30071,6175,2497,2500,7660,22388,11537,6509,{271,2884,7860,8111,15296,26677}
as16path_t 8 33437,6939,2516,7660,22388,11537,6509,{271,2884,7860,8111,15296,26677}
as16path_t 9 30071,6175,2497,2500,7660,22388,11537,6509,{271,2884,7860,8111,15296,26677}
as16path_t 6 2914,7660,22388,11537,6509,{271,2884,7860,8111,15296,26677}
as16path_t 6 2914,7660,22388,11537,6509,{271,2884,7860,8111,15296,26677}
as16path_t 9 7781,6175,2497,2500,7660,22388,11537,6509,{271,2884,7860,8111,15296,26677}
as16path_t 7 6939,2516,7660,22388,11537,6509,{271,2884,7860,8111,15296,26677}
as16path_t 5 7660,22388,11537,6509,{271,2884,7860,8111,15296,26677}
as16path_t 7 3292,2914,7660,22388,11537,6509,{271,2884,7860,8111,15296,26677}
as16path_t 10 7018,30071,6175,2497,2500,7660,22388,11537,6509,{271,2884,7860,8111,15296,26677}'

	EXEC	-x bgp '{print "%(typeof(path))s %(sizeof(path))u %(path)s"}' $data/f01.mrt
		SAME OUTPUT $data/f01.out

	EXEC	-x bgp '{print "%(mvpn)s"}' $data/mvpn-key-01.dat
		OUTPUT - '( key=( afi=1 mvpn=( group_addrv6=6:20F:405:60A:809:A0E:C0D:E12 originatorv4=0.6.2.16 rd=( as16=6 number=393741 ) src_addrv6=6:20E:405:609:809:A0D:C0D:E11 type=3 ) origin=48 safi=5 ) originatorv4=0.6.2.17 type=4 )'

	EXEC	-x bgp '{print "%(mvpn.key)s"}' $data/mvpn-key-01.dat
		OUTPUT - '( afi=1 mvpn=( group_addrv6=6:20F:405:60A:809:A0E:C0D:E12 originatorv4=0.6.2.16 rd=( as16=6 number=393741 ) src_addrv6=6:20E:405:609:809:A0D:C0D:E11 type=3 ) origin=48 safi=5 )'

	EXEC	-x bgp '{print "%(mvpn.key.mvpn)s"}' $data/mvpn-key-01.dat
		OUTPUT - '( group_addrv6=6:20F:405:60A:809:A0E:C0D:E12 originatorv4=0.6.2.16 rd=( as16=6 number=393741 ) src_addrv6=6:20E:405:609:809:A0D:C0D:E11 type=3 )'

	EXEC	-x bgp '{print "%(mvpn.key.mvpn.rd)s"}' $data/mvpn-key-01.dat
		OUTPUT - '( as16=6 number=393741 )'

	EXEC	-x bgp '{print "%(mvpn.key.mvpn.rd.number)s"}' $data/mvpn-key-01.dat
		OUTPUT - '393741'
