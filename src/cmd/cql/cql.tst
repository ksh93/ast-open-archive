# regression tests for the cql utility

export TZ=EST5EDT

VIEW data

KEEP "*.[cd]*"

TEST 01 'ciao big function reference'
	EXEC	-I$data -l -d '
	#include "cia.cql"
	schema=symbol;' -e '
	kind.value=="function" && name=="ZHzcnwm"' -e '
	void begin()
	{
		printf("file                           name sclass dtype          def     bline eline\n");
		printf("====================== ============ ====== ============== ======= ===== =====\n");
	}
	void action()
	{
		printf("%-22.22s %12.12s %6.6s %-14.14s %-7.7s %-5.5s %-5.5s\n", file.name, name, sclass.value, dtype, def.value, bline, eline);
	}' big.db
		OUTPUT - $'file                           name sclass dtype          def     bline eline
====================== ============ ====== ============== ======= ===== =====
4B3pr5HFjikzdg0EMd_jul      ZHzcnwm global CON_OGH        def     36    180  
4B3pr5gvu0vDFvsidlc.m       ZHzcnwm extern CON_OGH        dec     28    28   '
	EXEC
	EXEC	-I$data -l -d '
	#include "cia.cql"
	schema=reference;'  -e '
	kind1.value=="function" && id1.name=="ZHzcnwm" && kind2.value=="function" && id2.name=="*" '  -e '
	void begin()
	{
		printf("k1 file1             name1              k2 file2             name2\n");
		printf("== ================= ================== == ================= =================\n");
	}
	void action()
	{
		printf("%-2.2s %-17.17s %-18.18s %-2.2s %-17.17s %-17.17s \n", id1.kind, id1.file.name, id1.name, id2.kind, id2.file.name, id2.name);
	}' big.db
		OUTPUT - $'k1 file1             name1              k2 file2             name2
== ================= ================== == ================= =================
p  4B3pr5HFjikzdg0EM ZHzcnwm            p  4B3ils4hu4fARosbx QMtoeaw_          
p  4B3pr5HFjikzdg0EM ZHzcnwm            p  4B3ils4fs8hCRuggd OTmbenla          
p  4B3pr5HFjikzdg0EM ZHzcnwm            p  4B3ils4uu8lDHqacy DZrtqq            
p  4B3pr5HFjikzdg0EM ZHzcnwm            p  4B3ils4qy0OAthis. ZZdrkrta          
p  4B3pr5HFjikzdg0EM ZHzcnwm            p  4B3ils4tyu8fCTwvb CDfpyvxz          
p  4B3pr5HFjikzdg0EM ZHzcnwm            p  4B3ils4uu8lDHzndv LNtcdhd7          '
	EXEC

TEST 02 'ciao variable reference'
	EXEC	-I$data -l -d '
	#include "cia.cql"
	schema=symbol;'  -e '
	kind.value=="variable" && name=="*"  && file.name=="ciafan.c"' -e '
	void begin()
	{
		printf("file                           name sclass dtype          def     bline eline\n");
		printf("====================== ============ ====== ============== ======= ===== =====\n");
	}
	void action()
	{
		printf("%-22.22s %12.12s %6.6s %-14.14s %-7.7s %-5.5s %-5.5s\n", file.name, name, sclass.value, dtype, def.value, bline, eline);
	} ' var.db
		OUTPUT - $'file                           name sclass dtype          def     bline eline
====================== ============ ====== ============== ======= ===== =====
ciafan.c                   fancount static int            def     18    18   
ciafan.c                        dbs extern char           dec     21    21   
ciafan.c               ciafan_flags static int            def     19    19   '
	EXEC

TEST 03 'ciao file reference'
	EXEC	-I$data -l -d '#include "cia.cql"' -d 'schema=symbol' -e 'kind.value=="file" && name=="*"' file.db
		OUTPUT - $'2;<void>;f;2;;n;0;0;0;dc;100041c4
3;<libc.a>;f;3;;l;0;0;0;dc;10004204
4;/usr/include/stdio.h;f;4;;n;1;288;288;df;548d0998
5;libcbt/cbt.h;f;5;;n;1;104;104;df;ef6f7af
6;libpdb/file.h;f;6;;n;1;63;63;df;42e7403e
7;/usr/include/sgidefs.h;f;7;;n;1;193;193;df;141dad68
8;ast/re.h;f;8;;n;1;60;60;df;353cd5fd
9;libpdb/field.h;f;9;;n;1;171;171;df;b6e001a9
10;/usr/include/getopt.h;f;10;;n;1;46;46;df;2a031fc6
11;libpdb/pathmap.h;f;11;;n;1;17;17;df;88e02c94
12;libpdb/query.h;f;12;;n;1;146;146;df;13bfef1d
13;libpdb/view.h;f;13;;n;1;15;15;df;a7dd4e7c
14;libpdb/format.h;f;14;;n;1;54;54;df;bd9ebea8
15;libpdb/buffer.h;f;15;;n;1;19;19;df;581faffe
16;libpdb/index.h;f;16;;n;1;66;66;df;87f1ed40
17;ciafan.h;f;17;;n;1;600;31;df;1d2a8241
18;/usr/include/string.h;f;18;;n;1;113;113;df;7bdd1c95
19;libpdb/error.h;f;19;;n;1;63;63;df;42ae7868
20;main.c;f;20;;n;1;2121;99;df;ae98ae01
21;/usr/include/stdlib.h;f;21;;n;1;456;217;df;41cdd104
22;libpdb/infoview.h;f;22;;n;1;1133;65;df;be47ac
23;ast/stdio.h;f;23;;n;1;290;1;df;1f4ff3a4
737;ciafan.c;f;737;;n;1;2326;159;df;51eeb4bb
738;user_format.h;f;738;;n;1;435;33;df;1754c6e5
739;/usr/include/sys/types.h;f;739;;n;1;402;336;df;f91bb092
740;/usr/include/sys/select.h;f;740;;n;1;66;66;df;8fe695be
870;util.c;f;870;;n;1;2068;46;df;ad6a23f6'
	EXEC

TEST 04 'incl fanout'
	EXEC	-I$data -l -d '
	#include "incl.cql"	
	schema=incl_ref' -e '
	id1.name=="*.c" && id2.name=="*"'  -e '
	void begin()
	{
		printf("file1                            file2                             rkind\n");
		printf("================================ ================================ ======\n");
	}
	void action()
	{
		printf("%32.-32s %32.-32s %6.6s\n", id1.name, id2.name, rkind.value);
	}'  fanout.db
		OUTPUT - $'file1                            file2                             rkind
================================ ================================ ======
ciafan.c                         libpdb/query.h                      ref
ciafan.c                         libpdb/field.h                      ref
ciafan.c                         libpdb/format.h                     ref
ciafan.c                         libpdb/error.h                      ref
ciafan.c                         ast/stdio.h                         all
ciafan.c                         libpdb/infoview.h                   inc
ciafan.c                         ciafan.h                            all
ciafan.c                         user_format.h                       inc'
	EXEC
	EXEC	-I$data -l -d '
	#include "incl.cql"	
	schema = incl_ref;
	sort = { incl_ref.id1.name, incl_ref.id2.name };' -e '
	id1.name=="*.c" && id2.name=="*"'  -e '
	void begin()
	{
		printf("file1                            file2                             rkind\n");
		printf("================================ ================================ ======\n");
	}
	void action()
	{
		printf("%32.-32s %32.-32s %6.6s\n", id1.name, id2.name, rkind.value);
	}' fanout.db
		OUTPUT - $'file1                            file2                             rkind
================================ ================================ ======
ciafan.c                         libpdb/query.h                      ref
ciafan.c                         libpdb/format.h                     ref
ciafan.c                         libpdb/error.h                      ref
ciafan.c                         libpdb/field.h                      ref
ciafan.c                         user_format.h                       inc
ciafan.c                         ast/stdio.h                         all
ciafan.c                         libpdb/infoview.h                   inc
ciafan.c                         ciafan.h                            all'
	EXEC

TEST 05 '2 level view'
	EXEC	-I$data -l -d '
	#include "cia.cql"
	schema=symbol;' -e '
	kind.value=="file" && name=="*"' -e '
	void begin()
	{
		printf("filename                                        length cpplen\n");
		printf("=============================================== ====== ======\n");
	}
	void action()
	{
		printf("%-47.47s %-6.6s %-6.6s\n", file.name, eline, hline);
	}' bot.db
		OUTPUT - $'filename                                        length cpplen
=============================================== ====== ======
<void>                                          0      0     
<libc.a>                                        0      0     
f1.c                                            3      3     
f2.c                                            1      1     '
	EXEC
	EXEC	-I$data -l -d '
	#include "cia.cql"
	schema=symbol;' -e '
	kind.value=="file" && name=="*"' -e '
	void begin()
	{
		printf("filename                                        length cpplen\n");
		printf("=============================================== ====== ======\n");
	}
	void action()
	{
		printf("%-47.47s %-6.6s %-6.6s\n", file.name, eline, hline);
	}' bot.db
		OUTPUT - $'filename                                        length cpplen
=============================================== ====== ======
<void>                                          0      0     
<libc.a>                                        0      0     
f1.c                                            3      3     
f2.c                                            1      1     '
	EXEC
	EXEC	-I$data -l -d '
	#include "cia.cql"
	schema=symbol;' -e '
	kind.value=="file" && name=="*"' -e '
	void begin()
	{
		printf("filename                                        length cpplen\n");
		printf("=============================================== ====== ======\n");
	}
	void action()
	{
		printf("%-47.47s %-6.6s %-6.6s\n", file.name, eline, hline);
	}' top.db:bot.db
		OUTPUT - $'filename                                        length cpplen
=============================================== ====== ======
<void>                                          0      0     
<libc.a>                                        0      0     
f3.c                                            1      1     
f2.c                                            1      1     '
	EXEC

TEST 06 'ciao big print'
	EXEC	-I$data -l -d '
	#include "ciao_cc.cql"
	schema = relationship;
	relationship.input = "rel.db";
	entity.input = "ent.db";
	' -e '
	id1.name=="*" &&
	id2.name=="*" &&
	id1.kind.value=="file" &&
	id2.kind.value=="file"' -e '
        void action()
	{
        printf("%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s\n",
		kind1.value,
		id1.id,
		id1.name,
		id1.kind.value,
		id1.file.name,
		id1.dtype,
		id1.sclass.value,
		id1.bline,
		id1.hline,
		id1.eline,
		id1.def.value,
		id1.chksum,
		id1.selected,
		kind2.value,
		id2.id,
		id2.name,
		id2.kind.value,
		id2.file.name,
		id2.dtype,
		id2.sclass.value,
		id2.bline,
		id2.hline,
		id2.eline,
		id2.def.value,
		id2.chksum,
		id2.selected,
		usage,
		rkind);
        }
	' rel.db
		SAME OUTPUT $data/rel.out

	EXEC	-I$data -l -d '
	#include "ciao_cpp.cql"
	schema = entity;
	' -e '
        name=="*" && kind.value=="file" 
	' -e '
	void begin() {
		int isselected[];
		char* ispulled[];
	}
	void action() {
		if (!isselected[id]) {
			isselected[id] = 1;
			ispulled[id] = "";
			printf("[1] %s %s\n", id, name);
		}
		if (!isselected[file.id])
			ispulled[file.id] =
				sprintf("[2] %s %s\n", file.id, file.name);
		if (!isselected[pparam.id])
			ispulled[pparam.id] =
				sprintf("[3] %s %s\n", pparam.id, pparam.name);
		if (!isselected[tparam.id])
			ispulled[tparam.id] =
				sprintf("[4] %s %s\n", tparam.id, tparam.name);
		if (!isselected[ptype.id])
			ispulled[ptype.id] =
				sprintf("[5] %s %s\n", ptype.id, ptype.name);
	}
	void end() {
		char* s;
		for (ispulled[s]) 
			if (ispulled[s] != "")
				printf("%s", ispulled[s]);
	}
	'
		SAME OUTPUT $data/cpp.out
	EXEC

TEST 07 'functions'
	EXEC	-I$data -f null.cql -e '
		int fun(int a) {
			a += 1;
			return a;
		}
		int begin() {
			int a = 1;
		}
		int end() {
			printf("%d\n", a);
			printf("%d\n", fun(1));
			printf("%d\n", a);
			printf("%d\n", fun(a));
			printf("%d\n", a);
		}
		'
	OUTPUT - $'1\n2\n1\n2\n1'
	EXEC	-I$data -f null.cql -e '
		int rec(int n) {
			if (n == 0)
				return 0;
			if (n == 1)
				return 1;
			return n + rec(n - 1) + rec(n - 2);
		}
		int end() {
			int i;
			for (i = 0; i < 10; i++)
				printf("%d %d\n", i, rec(i));
		}
		'
	OUTPUT - $'0 0\n1 1\n2 3\n3 7\n4 14\n5 26\n6 46\n7 79\n8 133\n9 221'
	EXEC	-I$data -f null.cql -e '
		int rec(int n) {
			if (n == 0)
				return 0;
			if (n == 1)
				return 1;
			return n + rec(n - 1) + rec(n - 2);
		}
		int end() {
			int n;
			for (n = 0; n < 10; n++)
				printf("%d %d\n", n, rec(n));
		}
		'
	EXEC	-I$data -f null.cql -e '
		int ack(int a, int b) {
			if (a == 0)
				return b + 1;
			if (b == 0)
				return ack(a - 1, 1);
			return ack(a - 1, ack(a, b - 1));
		}
		int end() {
			printf("%d\n", ack(3, 3));
		}
		'
	OUTPUT - $'61'

TEST 08 'subfields and subschemas'
	EXEC	-I$data -l -f passwd.cql -e '
		gid.members == "*g*";
		'
	OUTPUT - $'dgk:*:1001:1001:David Korn:/home/dgk:/bin/ksh\ngsf:*:1002:1002:Glenn Fowler,gsf_loc:/home/gsf:/bin/ksh'
	EXEC	-I$data -l -f passwd.cql -e '
		select: gid.members == "*g*";
		'
	EXEC	-I$data -l -f passwd.cql -e '
		void select() {
			gid.members == "*g*";
		}
		'
	EXEC	-I$data -l -f passwd.cql -e '
		int select() {
			gid.members == "*g*";
		}
		'
	EXEC	-I$data -l -f passwd.cql -e '
		int select() {
			return gid.members == "*g*";
		}
		'
	EXEC	-I$data -l -f passwd.cql -e '
		void action() {
			if (gid.members == "*g*")
				printf("%s\n", cql.input);
		}
		'
	EXEC	-I$data -l -f passwd.cql
		OUTPUT - $'dgk:*:1001:1001:David Korn:/home/dgk:/bin/ksh\ngsf:*:1002:1002:Glenn Fowler,gsf_loc:/home/gsf:/bin/ksh\nkpv:*:1003:1003:Phong Vo,,kpv_room:/home/kpv:/bin/ksh\ndfwc:*:1004:1004:Don Caldwell,,,dfwc_ext:/home/dfwc:/bin/ksh\nbozo:*:1005:1005:Bozo T. Clown,Big Top,,bozo_ext:/home/bozo'
	EXEC	-I$data -l -f passwd.cql -e '
		int n;
		void action() {
			n++;
			if (gid.members == "*g*")
				printf("%s %d %s %s %s %s\n", name, uid, gid, gid.name, gid.gid, gid.members);
		}
		void end() {
			printf("%d total\n", n);
		}
		'
	OUTPUT - $'dgk 1001 1001 GR_dgk 1001 dgk\ngsf 1002 1002 GR_gsf 1002 gsf\n5 total'
	EXEC	-I$data -l -f passwd.cql -e '
		int n;
		int select() {
			n++;
			gid.members == "*g*";
		}
		void action() {
			printf("%s %d %s %s %s %s\n", name, uid, gid, gid.name, gid.gid, gid.members);
		}
		void end() {
			printf("%d total\n", n);
		}
		'
	EXEC	-I$data -l -f pwd.cql -e '
		int n;
		int select() {
			n++;
			gid.members == "*g*";
		}
		void action() {
			printf("%s %d %s %s %s %s\n", name, uid, gid, gid.name, gid.gid, gid.members);
		}
		void end() {
			printf("%d total\n", n);
		}
		'
	EXEC	-I$data -l -f pwd.cql -e '
		schema = group;

		void action() {
			int	i;

			printf("%s:\n", name);
			for (members[i])
				printf("	%s\n", members[i]);
		}
		'
		OUTPUT - $'GR_staff:
	dgk
	ek
	gsf
	kpv
	smb
	dfwc
GR_1124:
	ek
	smb
	dgb
	kpv
	griffin
	albert
	khp
	jap
GR_3d:
	dgk
	chen
	herman
GR_ftpdist:
	ftpdist
	smb
	dgk
	gsf
GR_ifs:
	kpv
	jjs
GR_reveng:
	ek
	kpv
GR_ship:
	dgk
	gsf
	kpv
	advsoft
	ek
GR_dgk:
	dgk
GR_gsf:
	gsf
GR_kpv:
	kpv
GR_dfwc:
	dfwc
GR_bozo:
	bozo'
	PROG rm *.hix
		OUTPUT -
	EXEC	-I$data -l -f pwd.cql -e '
		schema = group;

		void action() {
			int	i;

			printf("%s:\n", name);
			for (members[i])
				printf("	%s	%s\n", members[i], members[i].home);
		}
		'
		OUTPUT - $'GR_staff:
	dgk	/home/dgk
	ek	
	gsf	/home/gsf
	kpv	/home/kpv
	smb	
	dfwc	/home/dfwc
GR_1124:
	ek	
	smb	
	dgb	
	kpv	/home/kpv
	griffin	
	albert	
	khp	
	jap	
GR_3d:
	dgk	/home/dgk
	chen	
	herman	
GR_ftpdist:
	ftpdist	
	smb	
	dgk	/home/dgk
	gsf	/home/gsf
GR_ifs:
	kpv	/home/kpv
	jjs	
GR_reveng:
	ek	
	kpv	/home/kpv
GR_ship:
	dgk	/home/dgk
	gsf	/home/gsf
	kpv	/home/kpv
	advsoft	
	ek	
GR_dgk:
	dgk	/home/dgk
GR_gsf:
	gsf	/home/gsf
GR_kpv:
	kpv	/home/kpv
GR_dfwc:
	dfwc	/home/dfwc
GR_bozo:
	bozo	/home/bozo'
	ERROR - "cql: warning: $data/pwd.db: members index ek not found
cql: warning: $data/pwd.db: members index smb not found
cql: warning: $data/pwd.db: members index ek not found
cql: warning: $data/pwd.db: members index smb not found
cql: warning: $data/pwd.db: members index dgb not found
cql: warning: $data/pwd.db: members index griffin not found
cql: warning: $data/pwd.db: members index albert not found
cql: warning: $data/pwd.db: members index khp not found
cql: warning: $data/pwd.db: members index jap not found
cql: warning: $data/pwd.db: members index chen not found
cql: warning: $data/pwd.db: members index herman not found
cql: warning: $data/pwd.db: members index ftpdist not found
cql: warning: $data/pwd.db: members index smb not found
cql: warning: $data/pwd.db: members index jjs not found
cql: warning: $data/pwd.db: members index ek not found
cql: warning: $data/pwd.db: members index advsoft not found
cql: warning: $data/pwd.db: members index ek not found"
	EXEC
	EXEC	-I$data -l -f pwd.cql -e '
		schema = group;
		void action() {
			int	i;
			printf("%s:\n", name);
			for (members[i])
				printf("	%s %d %d\n", members[i], members[i].uid, members[i].gid);
		}
		'
		OUTPUT - $'GR_staff:
	dgk 1001 1001
	ek 0 0
	gsf 1002 1002
	kpv 1003 1003
	smb 0 0
	dfwc 1004 1004
GR_1124:
	ek 0 0
	smb 0 0
	dgb 0 0
	kpv 1003 1003
	griffin 0 0
	albert 0 0
	khp 0 0
	jap 0 0
GR_3d:
	dgk 1001 1001
	chen 0 0
	herman 0 0
GR_ftpdist:
	ftpdist 0 0
	smb 0 0
	dgk 1001 1001
	gsf 1002 1002
GR_ifs:
	kpv 1003 1003
	jjs 0 0
GR_reveng:
	ek 0 0
	kpv 1003 1003
GR_ship:
	dgk 1001 1001
	gsf 1002 1002
	kpv 1003 1003
	advsoft 0 0
	ek 0 0
GR_dgk:
	dgk 1001 1001
GR_gsf:
	gsf 1002 1002
GR_kpv:
	kpv 1003 1003
GR_dfwc:
	dfwc 1004 1004
GR_bozo:
	bozo 1005 1005'
		ERROR - "cql: warning: $data/pwd.db: members index ek not found
cql: warning: $data/pwd.db: members index ek not found
cql: warning: $data/pwd.db: members index ek not found
cql: warning: $data/pwd.db: members index smb not found
cql: warning: $data/pwd.db: members index smb not found
cql: warning: $data/pwd.db: members index smb not found
cql: warning: $data/pwd.db: members index ek not found
cql: warning: $data/pwd.db: members index ek not found
cql: warning: $data/pwd.db: members index ek not found
cql: warning: $data/pwd.db: members index smb not found
cql: warning: $data/pwd.db: members index smb not found
cql: warning: $data/pwd.db: members index smb not found
cql: warning: $data/pwd.db: members index dgb not found
cql: warning: $data/pwd.db: members index dgb not found
cql: warning: $data/pwd.db: members index dgb not found
cql: warning: $data/pwd.db: members index griffin not found
cql: warning: $data/pwd.db: members index griffin not found
cql: warning: $data/pwd.db: members index griffin not found
cql: warning: $data/pwd.db: members index albert not found
cql: warning: $data/pwd.db: members index albert not found
cql: warning: $data/pwd.db: members index albert not found
cql: warning: $data/pwd.db: members index khp not found
cql: warning: $data/pwd.db: members index khp not found
cql: warning: $data/pwd.db: members index khp not found
cql: warning: $data/pwd.db: members index jap not found
cql: warning: $data/pwd.db: members index jap not found
cql: warning: $data/pwd.db: members index jap not found
cql: warning: $data/pwd.db: members index chen not found
cql: warning: $data/pwd.db: members index chen not found
cql: warning: $data/pwd.db: members index chen not found
cql: warning: $data/pwd.db: members index herman not found
cql: warning: $data/pwd.db: members index herman not found
cql: warning: $data/pwd.db: members index herman not found
cql: warning: $data/pwd.db: members index ftpdist not found
cql: warning: $data/pwd.db: members index ftpdist not found
cql: warning: $data/pwd.db: members index ftpdist not found
cql: warning: $data/pwd.db: members index smb not found
cql: warning: $data/pwd.db: members index smb not found
cql: warning: $data/pwd.db: members index smb not found
cql: warning: $data/pwd.db: members index jjs not found
cql: warning: $data/pwd.db: members index jjs not found
cql: warning: $data/pwd.db: members index jjs not found
cql: warning: $data/pwd.db: members index ek not found
cql: warning: $data/pwd.db: members index ek not found
cql: warning: $data/pwd.db: members index ek not found
cql: warning: $data/pwd.db: members index advsoft not found
cql: warning: $data/pwd.db: members index advsoft not found
cql: warning: $data/pwd.db: members index advsoft not found
cql: warning: $data/pwd.db: members index ek not found
cql: warning: $data/pwd.db: members index ek not found
cql: warning: $data/pwd.db: members index ek not found"

TEST 09 'types and casts'
	EXEC	-I$data -l -f null.cql -e '
		begin:
			date_t	d1 = 805993046;
			date_t	d2 = "Fri Jul 17 10:57:26 EDT 1995";
			printf("%s %s\n", d1, d2);
			printf("%d %d\n", d1, d2);
	'
	OUTPUT - $'Jul 17  1995 Jul 17  1995\n805993046 805993046'
	EXEC	-I$data -l -f null.cql -e '
		begin:
			int	i1 = 123;
			int	i2 = "123";
			char*	s1 = "123";
			char*	s2 = 123;
			printf("%s %s %s %s\n", i1, i2, s1, s2);
			printf("%d %d %d %d\n", i1, i2, s1, s2);
			printf("%d %d %d %d\n", i1/10, i2/10, s1/10, s2/10);
	'
	OUTPUT - $'123 123 123 123\n123 123 123 123\n12 12 12 12'

TEST 10 'member reference'
	EXEC	-I$data -l -f mem.cql
		OUTPUT - $'AAA 3\nBBB 4\nCCC 2'

# TESTS 11..14 thanks to dfwc@research.att.com from awk example translations

TEST 11 'awk book page 24'
	EXEC	-I$data -l -d '
		#include "countries.cql"
		schema=Countries;
		' -e '
		void select() {
			Countries.country == "Asia";
		}
		'
		OUTPUT -
	EXEC	-I$data -l -d '
		#include "countries.cql"
		schema=Countries;
		' -e '
		void select() {
			Countries.continent == "Asia";
		}
		'
		OUTPUT - $'USSR	8649	275	Asia\nChina	3705	1032	Asia\nIndia	1267	746	Asia\nJapan	144	120	Asia'
	EXEC	-I$data -l -d '
		#include "country.cql"
		schema=Countries;
		' -e '
		void select() {
			Countries.area != "+([1-9])";
		}
		'
		OUTPUT - $'China	3705	1032	Asia\nElbonia	ffff	1	Moronica'
	EXEC	-I$data -l -d '
		#include "countries.cql"
		schema=Countries;
		' -e '
		void select() {
			(char*)Countries.area != "+([1-9])";
		}
		'
		OUTPUT - $'China	3705	1032	Asia\nElbonia	ffff	1	Moronica'
NOTE bug -- libexpr -- cast should avoid default conversion

TEST 12 'record number of max field value'
	EXEC	-I$data -l -d '
		#include "cmp.cql"
		schema=Cmp;
		' -e '
		void begin() {
			int max = 0;
			int maxline = 0;
		};
		void action() {
			if( big > max) {
				maxline = cql.record;
				max = big;
			}
		};
		void end() {
			printf("%d\n", maxline);
			printf("%d\n", max);
		};
		' cmp.db
		OUTPUT - $'8\n35433480199'

TEST 13 'date and elapsed time'
	EXEC	-I$data -l -e '#include "date.cql"'
		OUTPUT - $'independence;july 4, 1998;200y\nchristmas;Dec 25, 1998;2000y\nbirthday;Fri Oct 2 07:00:00 EDT 1998;600m'
	EXEC	-I$data -l -e "#include 'date.cql'"
	EXEC	-I$data -l -f date.cql
	EXEC	-I$data -f null.cql -e '
		void end() {
			date_t		d1 = 894401095;
			date_t		d2 = "1983-12-31/17:01";
			elapsed_t	e1 = 1234567;
			elapsed_t	e2 = "1M2w";
			int		i1 = 863197345;
			int		i2 = 7654321;
			printf("%d\t%s\n%d\t%s\n%d\t%s\n%d\t%s\n%d\t%s\n%d\t%s\n",
				d1, d1, d2, d2, e1, e1, e2, e2,
				i1, (date_t)i1,
				i2, (elapsed_t)i2);
		}
		'
	OUTPUT - $'894401095\tMay  5  1998\n441756060\tDec 31  1983\n1234567\t3h25m\n362880000\t1M11d\n863197345\tMay  9  1997\n7654321\t21h15m'

TEST 14 'format conversion'
	EXEC	-I$data -l -f emp.cql
		OUTPUT - $'highly paid hard workers\n\tName	Rate	Hours\n\tDave	10.50	55\n\tGlenn	10.50	95\n\tKen	9.10	60\n\tLefty	9.00	60'
	EXEC	-I$data -l -e '
		Countries {
			char* continent;
			char* country;
			int   pop;
			float poppct;
			int   area;
			float areapct;
			float density;
			int   poptot;
			int   areatot;
		}
		Countries.delimiter = ":";
		Countries.comment = "awkbook p94";
		Countries.input   = "prep3.dat";
		schema = Countries;
		void begin() {
			char* datefmt;
			char* hfmt, tfmt, TOTfmt;
			char* prev="", cs;
			float popp, areap;
			int   gpop, garea;
			float gpoppct, gareapct;
			int   spoptot, sareatot;

			datefmt = "January 1, 1988";
			hfmt = "%36s %8s %12s %7s %12s\n";
			tfmt = "%33s %10s %10s %9s\n";
			TOTfmt = "   TOTAL for %-13s%7d%11.1f%11d%10.1f\n";
		
			printf("%-18s %-40s %19s\n\n %-14s %-14s %-23s %-14s %-11s\n\n",
	       		"Report No. 3", "POPULATION, AREA, POPULATION DENSITY", datefmt,
	       		"CONTINENT", "COUNTRY", "POPULATION", "AREA", "POP. DEN.");
			printf(hfmt, "Millions ", "Pct. of", "Thousands ", "Pct. of", "People per");
			printf(hfmt, "of People", "Total ", "of Sq. Mi.", "Total ", "Sq. Mi. ");
			printf(hfmt, "---------", "-------", "----------", "-------", "----------");
			popp = 0.0; areap=0.0;
		}
		void action() {
			if(continent != prev) {
				if(cql.record > 1) {
	        			printf(tfmt, "----", "-----", "-----", "-----");
	 				printf(TOTfmt, prev, spoptot, popp, sareatot, areap);
					printf(tfmt, "====", "=====", "=====", "=====");
				}
				cs = continent; prev = continent;
				popp = poppct; areap = areapct;
			}
			else {
				cs = "";
				popp += poppct; areap += areapct;
			}
			printf(" %-15s%-10s %6d %10.1f %10d %9.1f %10.1f\n",
				cs, country, pop, poppct,area,areapct,density);
			gpop += pop; gpoppct += poppct;
			garea += area; gareapct += areapct;
			spoptot = poptot; sareatot = areatot;
		}
		void end() {
			printf(tfmt, "----", "-----", "-----", "-----");
			printf(TOTfmt, prev, spoptot, popp, sareatot, areap);
			printf(tfmt, "====", "=====", "=====", "=====");
			printf(" GRAND TOTAL %20d %10.1f %10d %9.1f\n",
				gpop, gpoppct, garea, gareapct);
			printf(tfmt, "=====", "======", "=====", "======");
		}
		'
	OUTPUT - 'Report No. 3       POPULATION, AREA, POPULATION DENSITY         January 1, 1988

 CONTINENT      COUNTRY        POPULATION              AREA           POP. DEN.  

                           Millions   Pct. of   Thousands  Pct. of   People per
                           of People   Total    of Sq. Mi.  Total      Sq. Mi. 
                           ---------  -------   ---------- -------   ----------
 Asia           Japan         120        4.3        144       0.6      833.3
                India         746       26.5       1267       4.9      588.8
                China        1032       36.6       3705      14.4      278.5
                USSR          275        9.8       8649      33.7       31.8
                             ----      -----      -----     -----
   TOTAL for Asia            2173       77.1      13765      53.6
                             ====      =====      =====     =====
 Europe         Germany        61        2.2         96       0.4      635.4
                England        56        2.0         94       0.4      595.7
                France         55        2.0        211       0.8      260.7
                             ----      -----      -----     -----
   TOTAL for Europe           172        6.1        401       1.6
                             ====      =====      =====     =====
 North America  Mexico         78        2.8        762       3.0      102.4
                USA           237        8.4       3615      14.1       65.6
                Canada         25        0.9       3852      15.0        6.5
                             ----      -----      -----     -----
   TOTAL for North America    340       12.1       8229      32.0
                             ====      =====      =====     =====
 South America  Brazil        134        4.8       3286      12.8       40.8
                             ----      -----      -----     -----
   TOTAL for South America    134        4.8       3286      12.8
                             ====      =====      =====     =====
 GRAND TOTAL                 2819      100.0      25681     100.0
                            =====     ======      =====    ======'

TEST 15 'multiple passes'
	EXEC	-I$data -l -e '
		#include "countries.cql"
		schema=Countries;
		void begin() {
			int arearr[]; int poparr[]; int aretot, poptot;
		}
		void action() {
			arearr[continent] += area;
			aretot += area;
			poparr[continent] += pop;
			poptot += pop;
		}
		void end() {
			char* i;
			printf("void begin() {\n\tint arearr[], poparr[], aretot, poptot;\n");
			printf("\tfloat den;\n\n");
			for (arearr[i]) {
				printf("\tarearr[\"%s\"] = %d;\n", i, arearr[i]);
				printf("\tpoparr[\"%s\"] = %d;\n", i, poparr[i]);
			}
			printf("\taretot = %d;\n", aretot);
			printf("\tpoptot = %d;\n}\n", poptot);
		}
		'
		MOVE OUTPUT pass1.out
	EXEC	-I$data -l -e '
		#include "countries.cql"
		schema=Countries;
		#include "pass1.out"
		void action() {
			den = area ? 1000.0*pop/area : 0;
			printf("%s:%s:%s:%f:%d:%f:%f:%d:%d\n",
				continent, country, pop, 100.0*pop/poptot,
				area, 100.0*area/aretot, den,
				poparr[continent], arearr[continent]);
		
		} 
		'
		OUTPUT - 'Asia:USSR:275:9.751773:8649:33.677284:31.795583:2173:13765
North America:Canada:25:0.886525:3852:14.998832:6.490135:340:8229
Asia:China:1032:36.595745:3705:14.426447:278.542510:2173:13765
North America:USA:237:8.404255:3615:14.076007:65.560166:340:8229
South America:Brazil:134:4.751773:3286:12.794954:40.779063:134:3286
Asia:India:746:26.453901:1267:4.933416:588.792423:2173:13765
North America:Mexico:78:2.765957:762:2.967059:102.362205:340:8229
Europe:France:55:1.950355:211:0.821587:260.663507:172:401
Asia:Japan:120:4.255319:144:0.560704:833.333333:2173:13765
Europe:Germany:61:2.163121:96:0.373803:635.416667:172:401
Europe:England:56:1.985816:94:0.366015:595.744681:172:401
Moronica:Elbonia:1:0.035461:0:0.000000:0.000000:1:0
Antarctica:Nilica:0:0.000000:1:0.003894:0.000000:0:1'
	EXEC	-I$data -l -e '
		#include "countries.cql"
		schema=Countries;
		void begin() {
			int ararr[], poparr[];
			int areatot, poptot;
			float den;
		}
		void action() {
			ararr[continent] += area; areatot += area;
			poparr[continent] += pop; poptot += pop;
		}
		void end() {
			cql.loop("", "action_2", "");
		}
		void action_2() {
			den = area ? 1000.0*pop/area : 0;
			printf("%s:%s:%s:%f:%d:%f:%f:%d:%d\n",
				continent, country, pop, 100.0*pop/poptot,
				area, 100.0*area/areatot, den,
				poparr[continent], ararr[continent]);
		} 
		'

TEST 16 'closure'
	EXEC	-I$data -l -f hw.cql
		OUTPUT - 'PARTITION0,1,0;Partition 0,1,0;pt;2.00;8168.38;xfs;y
PARTITION0,1,1;Partition 0,1,1;pt;8170.38;512.00;swap;y
pt;PARTITION0,1,0;dk;DISK0,1
DISK0,1;SCSI disk 0,1;dk;8682.38;ok;;y
pt;PARTITION0,1,1;dk;DISK0,1
dk;DISK0,1;ct;CTRL0
CTRL0;SCSI controller 0;ct;0;;;y
ct;CTRL0;io;IO1,io1,0
IO1,io1,0;IO board 1,io1,0;io;;;;y'
	EXEC	-I$data -l -f hw.cql
	EXEC	-I$data -l -f er.cql er.db
		SAME OUTPUT $data/e.db
	EXEC	-I$data -l -f er.cql er.db
		SAME OUTPUT $data/e.db
	DO	cp $data/e.db entity.db
	DO	cp $data/r.db relationship.db
	EXEC	-I$data -l -f er.cql
		SAME OUTPUT $data/e.db
	EXEC	-I$data -l -f er.cql
		SAME OUTPUT $data/e.db

TEST 17 'fixed with delimiter check'
	EXEC	-I$data -l -f fix.cql $data/fixok.db
		OUTPUT - $'aa:bb:cc:dd\nee:ff:gg:hh\nii:jj:kk:ll\nmm:nn:oo:pp'
	EXEC	-I$data -l -f fixcheck.cql $data/fixok.db
	EXEC	-I$data -l -f fix.cql $data/fixko.db
		OUTPUT - $'aa:bb:cc:dd\nee:ff:gh:h\n\nij:jj:kk:ll\nmm:nn:oo:pp'
	EXEC	-I$data -l -f fixcheck.cql -
		INPUT - $'aabbccdd\neeffghh\niijjjkkll\nmmnnoopp'
		OUTPUT - $'aa:bb:cc:dd\nmm:nn:oo:pp'
		ERROR - $'cql: warning: /dev/stdin: invalid record length 8 offset 9 -- ignored\ncql: warning: /dev/stdin: invalid record length 10 offset 17 -- ignored'
	EXEC	-I$data -l -f fixcheck.cql fixno.db
		INPUT fixno.db $'aabbccdd\neeffghh\niijjjkkll\nmmnnoopp'
		ERROR - $'cql: warning: fixno.db: invalid record length 8 offset 9 -- ignored\ncql: warning: fixno.db: invalid record length 10 offset 17 -- ignored'

TEST 18 'member arrays'
	EXEC	-I$data -l -f array1.cql $data/array1.db
		OUTPUT - $'gsf one\ngsf two\ngsf three\ndfwc five\ndfwc six'
	EXEC	-I$data -l -f array2.cql $data/array2.db

TEST 19 'buffer boundaries and hix steady state'
	EXEC	-l -e 't {int n; register char* a;}; a=="x";' $data/span.db
		OUTPUT - $'1;x\n3;x'
	EXEC

TEST 20 '-lcdb details'
	EXEC	-I$data -l -f space.cql -e 'action:printf(":%s:\n", name.last)'
		OUTPUT - $':Fowler:\n:Sword:\n:Bennett:\n:Haralick:'
	EXEC	-I$data -l -f space.cql -e 'dat.details="nospace";' -e 'action:printf(":%s:\n", name.last)'
		OUTPUT - $':Fowler:\n: Sword:\n:Bennett :\n: Haralick :'
	EXEC	-I$data -l -f fax.cql -e $'
Facility.details = "space";
Reference.details = "space";
schema = Facility;
void select() { FaxType  == "FNYAK"; }
'
		OUTPUT - $'TAVSZFFKEL|IJA5050489|CPOFUBKA   |HYGQPQF1341|FNYAK |81699|CPOFUBKA   EVTRUTI6412BDOEE 43639|O|J2784544       ||3|GYVRX5|53741897|95939197|5KMR9|...||      |SA
ZGPWVLDSWN|ORS5876243|CPOFUBKA   |HYGQPQF1341|FNYAK |61490|CPOFUBKA   EVTRUTI6412BDOEE 43630|Q|V8786507       ||6|JLAQO2|06470499|15915979|5ESL5|...||      |OK
BMRWDTZWUB|ILS7214423|CPOFUBKA   |HYGQPQF1341|FNYAK |21611|CPOFUBKA   EVTRUTI6412BDOEE 43631|W|N2126568       ||3|MAVFF1|73923815|69637416|2DRM4|...||      |HR
CLSHOGYZJM|DUX4347138|CPOFUBKA   |HYGQPQF1341|FNYAK |16380|CPOFUBKA   EVTRUTI6412BDOEE 43631|V|D6988043       ||9|KMPDH9|61495490|60682086|6TJQ6|...||      |WT
GNWJKQODRC|PUP4741576|CPOFUBKA   |HYGQPQF1341|FNYAK |12523|CPOFUBKA   EVTRUTI6412BDOEE 43632|X|I7457432       ||1|OADHZ3|79567340|82666606|2RRW8|...||      |LZ
SBEZUOSTXA|XAV2129512|CPOFUBKA   |HYGQPQF1341|FNYAK |78342|CPOFUBKA   EVTRUTI6412BDOEE 43633|R|T0349203       ||0|DRUMY2|00269327|17577111|3SQB3|...||      |OE
XSPCHNZKQT|CDU1458681|CPOFUBKA   |HYGQPQF1341|FNYAK |47450|CPOFUBKA   EVTRUTI6412BDOEE 43634|W|I7412338       ||5|CGTBF9|97772212|22008640|8TPO8|...||      |BV
CREPASGDRW|TUN6725398|CPOFUBKA   |HYGQPQF1341|FNYAK |72982|CPOFUBKA   EVTRUTI6412BDOEE 43635|J|V6561822       ||5|EQLTP9|77376890|64002480|2LTA2|...||      |PX
ORCNACCNLC|FAP6983914|CPOFUBKA   |HYGQPQF1341|FNYAK |34961|CPOFUBKA   EVTRUTI6412BDOEE 43636|Z|L2725474       ||1|UABZL7|97374254|48608282|2LJO6|...||      |NR
FUHQPPZWWT|QNS9834869|CPOFUBKA   |UMPXQX48   |FNYAK |67859|CPOFUBKA   RJCYVW57   ZVWKM 47815|W|X2764071       |TG|0|VHGOE4|02216564|06846822|6NJE0|...||      |TX
STGXYYSJXG|JYV8181710|CPOFUBKA   |UMPXQX48   |FNYAK |38561|CPOFUBKA   RJCYVW57   ZVWKM 47816|X|A1299589       |BC|8|NXKES4|88632680|00028244|0HTG6|...||      |HJ
SFYBSWIXPM|FEF4189176|CPOFUBKA   |UMPXQX48   |FNYAK |16348|CPOFUBKA   RJCYVW57   ZVWKM 47817|V|A3013942       |YL|3|GOVBD7|71321819|53991939|3CWH5|...||      |WO
POHKXVXOUD|QZW9658087|CPOFUBKA   |UMPXQX48   |FNYAK |43412|CPOFUBKA   RJCYVW57   ZVWKM 47818|I|P4382856       |AZ|3|CODTL3|79103533|31135579|7EUR9|...||      |KO
XWTEVJZGIX|ILQ9690649|CPOFUBKA   |UMPXQX48   |FNYAK |85635|CPOFUBKA   RJCYVW57   ZVWKM 47819|I|Y5299560       ||4|JDSOI8|38960994|82840028|4DBQ8|...||      |RR
GTOLQSQPRO|XMB2963314|CPOFUBKA   |UMPXQX48   |FNYAK |52509|CPOFUBKA   RJCYVW57   ZVWKM 47810|D|L2342072       ||6|LXMQK0|78922318|62062642|0PXE2|...||      |BZ
GJMZKEABVY|LCB8707598|CPOFUBKA   |UMPXQX48   |FNYAK |12702|CPOFUBKA   RJCYVW57   ZVWKM 47811|J|B2340402       ||4|NNGYE6|18744112|86400442|2BJU6|...||      |FZ
CFMVIKCLFE|LUT6387752|CPOFUBKA   |UMPXQX48   |FNYAK |92705|CPOFUBKA   RJCYVW57   ZVWKM 47812|F|J2706420       ||0|TJGMG0|36108778|02600200|2PTK8|...||      |NL'
	EXEC
	EXEC	-R -I$data -l -f fax.cql -e $'
Facility.details = "nospace";
Reference.details = "nospace";
schema = Facility;
void select() { FaxType  == "FNYAK "; }
'
	EXEC	-I$data -l -f fax.cql -e $'
Facility.details = "nospace";
Reference.details = "nospace";
schema = Facility;
void select() { FaxType  == "FNYAK "; }
'
	EXEC	-R -I$data -l -f fax.cql -e $'
Facility.details = "space";
Reference.details = "space";
schema = Reference;
void select() { f.FaxType  == "FNYAK"; }
'
		OUTPUT - $'CPOFUBKA   EVTRUTI6412BDOEE 43639
CPOFUBKA   EVTRUTI6412BDOEE 43630
CPOFUBKA   EVTRUTI6412BDOEE 43631
CPOFUBKA   EVTRUTI6412BDOEE 43631
CPOFUBKA   EVTRUTI6412BDOEE 43632
CPOFUBKA   EVTRUTI6412BDOEE 43633
CPOFUBKA   EVTRUTI6412BDOEE 43634
CPOFUBKA   EVTRUTI6412BDOEE 43635
CPOFUBKA   EVTRUTI6412BDOEE 43636
CPOFUBKA   RJCYVW57   ZVWKM 47815
CPOFUBKA   RJCYVW57   ZVWKM 47816
CPOFUBKA   RJCYVW57   ZVWKM 47817
CPOFUBKA   RJCYVW57   ZVWKM 47818
CPOFUBKA   RJCYVW57   ZVWKM 47819
CPOFUBKA   RJCYVW57   ZVWKM 47810
CPOFUBKA   RJCYVW57   ZVWKM 47811
CPOFUBKA   RJCYVW57   ZVWKM 47812'
	EXEC	-I$data -l -f fax.cql -e $'
Facility.details = "space";
Reference.details = "space";
schema = Reference;
void select() { f.FaxType  == "FNYAK"; }
'
	EXEC	-R -I$data -l -f fax.cql -e $'
Facility.details = "nospace";
Reference.details = "nospace";
schema = Reference;
void select() { f.FaxType  == "FNYAK "; }
'
	EXEC	-I$data -l -f fax.cql -e $'
Facility.details = "nospace";
Reference.details = "nospace";
schema = Reference;
void select() { f.FaxType  == "FNYAK "; }
'
