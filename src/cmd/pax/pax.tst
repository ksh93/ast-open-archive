# regression tests for the pax utility

dateformat='%(mtime:time=%Y-%m-%d/%H:%M:%S)s %(path)s'
modeformat='%(mtime:time=%K)s %(mode)s %(size)u %(path)s'

VIEW date y2k.dat
VIEW mode mode.dat

TEST 01 'different date formats'
	EXEC --nosummary -rf $date list.dat
	EXEC --listformat="$dateformat" -vf $date
		SAME OUTPUT list.dat
		ERROR - $date $'volume 1 in cpio format\n372 files, 99 blocks'
	EXEC -rf $date -wf y2k.pax -x asc
		OUTPUT -
		ERROR -
	EXEC --listformat="$dateformat" -vf y2k.pax
		SAME OUTPUT list.dat
		ERROR - y2k.pax $'volume 1 in asc format\n372 files, 116 blocks'
	EXEC -rf $date -wf y2k.pax -x aschk
		OUTPUT -
		ERROR -
	EXEC --listformat="$dateformat" -vf y2k.pax
		SAME OUTPUT list.dat
		ERROR - y2k.pax $'volume 1 in aschk format\n372 files, 116 blocks'
	EXEC -rf $date -wf y2k.pax -x binary
		OUTPUT -
		ERROR -
	EXEC --listformat="$dateformat" -vf y2k.pax
		SAME OUTPUT list.dat
		ERROR - y2k.pax $'volume 1 in binary format\n372 files, 54 blocks'
	EXEC -rf $date -wf y2k.pax -x cpio
		OUTPUT -
		ERROR -
	EXEC --listformat="$dateformat" -vf y2k.pax
		SAME OUTPUT list.dat
		ERROR - y2k.pax $'volume 1 in cpio format\n372 files, 99 blocks'
	EXEC -rf $date -wf y2k.pax -x pax
		OUTPUT -
		ERROR -
	EXEC --listformat="$dateformat" -vf y2k.pax
		SAME OUTPUT list.dat
		ERROR - y2k.pax $'volume 1 in pax format\n372 files, 1143 blocks'
	EXEC -rf $date -wf y2k.pax -x tar
		OUTPUT -
		ERROR -
	EXEC --listformat="$dateformat" -vf y2k.pax
		SAME OUTPUT list.dat
		ERROR - y2k.pax $'volume 1 in tar format\n372 files, 399 blocks'
	EXEC -rf $date -wf y2k.pax -x ustar
		OUTPUT -
		ERROR -
	EXEC --listformat="$dateformat" -vf y2k.pax
		SAME OUTPUT list.dat
		ERROR - y2k.pax $'volume 1 in ustar format\n372 files, 399 blocks'
	EXEC -rf $date -wf y2k.pax -x vdb
		OUTPUT -
		ERROR -
	EXEC --listformat="$dateformat" -vf y2k.pax
		SAME OUTPUT list.dat
		ERROR - y2k.pax $'volume 1 in vdb format\n372 files, 60 blocks'

TEST 02 'different file modes'
	o=

	EXEC --nosummary $o -rf $mode keep.lst mode.lst
		OUTPUT -
		ERROR -
	EXEC --listformat="$modeformat" -vf $mode
		SAME OUTPUT keep.lst
		ERROR - $mode $'volume 1 in cpio format\n19 files, 8 blocks'
	EXEC --nosummary $o -rf $mode
		OUTPUT -
		ERROR - $'pax: warning: mode.dir/m4000: mode --S------ not set
pax: warning: mode.dir/m4100: mode --S------ not set
pax: warning: mode.dir/m4200: mode --S------ not set
pax: warning: mode.dir/m4300: mode --S------ not set
pax: warning: mode.dir/m4400: mode --S------ not set
pax: warning: mode.dir/m4500: mode --S------ not set
pax: warning: mode.dir/m4600: mode --S------ not set
pax: warning: mode.dir/m4700: mode --S------ not set'
	EXEC --nosummary -wf mode.pax keep.lst mode.lst mode.dir
		ERROR -
	EXEC --listformat="$modeformat" -vf mode.pax
		SAME OUTPUT mode.lst
		ERROR - mode.pax $'volume 1 in cpio format\n19 files, 8 blocks'
	EXEC --nosummary $o -rf $mode
		OUTPUT -
		ERROR - $'pax: warning: mode.dir/m4000: mode --S------ not set
pax: warning: mode.dir/m4100: mode --S------ not set
pax: warning: mode.dir/m4200: mode --S------ not set
pax: warning: mode.dir/m4300: mode --S------ not set
pax: warning: mode.dir/m4400: mode --S------ not set
pax: warning: mode.dir/m4500: mode --S------ not set
pax: warning: mode.dir/m4600: mode --S------ not set
pax: warning: mode.dir/m4700: mode --S------ not set'
	EXEC --nosummary -wf mode.pax keep.lst mode.lst mode.dir
		ERROR -
	EXEC --listformat="$modeformat" -vf mode.pax
		SAME OUTPUT mode.lst
		ERROR - mode.pax $'volume 1 in cpio format\n19 files, 8 blocks'
	DO chmod u+rw,go+r mode.dir/*
	EXEC --nosummary $o -rf $mode
		OUTPUT -
		ERROR - $'pax: warning: mode.dir/m4000: mode --S------ not set
pax: warning: mode.dir/m4100: mode --S------ not set
pax: warning: mode.dir/m4200: mode --S------ not set
pax: warning: mode.dir/m4300: mode --S------ not set
pax: warning: mode.dir/m4400: mode --S------ not set
pax: warning: mode.dir/m4500: mode --S------ not set
pax: warning: mode.dir/m4600: mode --S------ not set
pax: warning: mode.dir/m4700: mode --S------ not set'
	EXEC --nosummary -wf mode.pax keep.lst mode.lst mode.dir
		ERROR -
	EXEC --listformat="$modeformat" -vf mode.pax
		SAME OUTPUT mode.lst
		ERROR - mode.pax $'volume 1 in cpio format\n19 files, 8 blocks'

	o=--preserve=ps

	EXEC --nosummary $o -rf $mode keep.lst mode.lst
		OUTPUT -
		ERROR -
	EXEC --listformat="$modeformat" -vf $mode
		SAME OUTPUT keep.lst
		ERROR - $mode $'volume 1 in cpio format\n19 files, 8 blocks'
	EXEC --nosummary $o -rf $mode
		OUTPUT -
		ERROR -
	EXEC --nosummary -wf mode.pax keep.lst mode.lst mode.dir
	EXEC --listformat="$modeformat" -vf mode.pax
		SAME OUTPUT keep.lst
		ERROR - mode.pax $'volume 1 in cpio format\n19 files, 8 blocks'
	EXEC --nosummary $o -rf $mode
		OUTPUT -
		ERROR -
	EXEC --nosummary -wf mode.pax keep.lst mode.lst mode.dir
	EXEC --listformat="$modeformat" -vf mode.pax
		SAME OUTPUT keep.lst
		ERROR - mode.pax $'volume 1 in cpio format\n19 files, 8 blocks'
	DO chmod u+rw,go+r mode.dir/*
	EXEC --nosummary $o -rf $mode
		OUTPUT -
		ERROR -
	EXEC --nosummary -wf mode.pax keep.lst mode.lst mode.dir
	EXEC --listformat="$modeformat" -vf mode.pax
		SAME OUTPUT keep.lst
		ERROR - mode.pax $'volume 1 in cpio format\n19 files, 8 blocks'
