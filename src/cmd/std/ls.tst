# y2k regression tests for the ls utility

listformat='%(mtime:time=%Y-%m-%d/%H:%M:%S)s %(path)s'

VIEW data y2k.dat

function DATA
{
	pax -o 'nosummary' -rf $data
}

TEST 01 'down to the second'
	DO DATA
	EXEC -f "$listformat" *.dat
		SAME OUTPUT list.dat
