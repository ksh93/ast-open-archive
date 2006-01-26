# tests for the dss fix query

TITLE + fix

export TZ=EST5EDT

VIEW data ../netflow/data

TEST 01 'basics'
	EXEC -x netflow '{fix --stamp=2003-01-31}' $data/netflow-5.dat
		OUTPUT - $'<METHOD>flat</>
<FLAT>
	<NAME>netflow</>
	<DESCRIPTION>Cisco router netflow dump fixed width binary format.</>
	<IDENT>@(#)$Id: netflow bin 2003-01-31 $</>
	<MAGIC>
		<STRING>netflow</>
		<VERSION>20030131</>
		<SWAP>be</>
	</>
	<COMPRESS>pzip netflow-bin</>
	<LIBRARY>time_t</>
	<LIBRARY>ip_t</>
	<LIBRARY>num_t</>
	<FIELD>
		<NAME>end</>
		<DESCRIPTION>Flow end time in 64 bit nanoseconds since the epoch.</>
		<TYPE>ns_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>8</>
		</>
	</>
	<FIELD>
		<NAME>start</>
		<DESCRIPTION>Flow start time in 64 bit nanoseconds since the epoch.</>
		<TYPE>ns_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>8</>
		</>
	</>
	<FIELD>
		<NAME>bytes</>
		<DESCRIPTION>Number of bytes sent in duration.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>dst_addr</>
		<DESCRIPTION>Destination IP address.</>
		<TYPE>ipaddr_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>first</>
		<DESCRIPTION>Elapsed milliseconds at flow start.</>
		<TYPE>elapsed_t</>
		<PHYSICAL>
			<TYPE>be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>hop</>
		<DESCRIPTION>Next hop IP address.</>
		<TYPE>ipaddr_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>last</>
		<DESCRIPTION>Elapsed milliseconds at flow end.</>
		<TYPE>elapsed_t</>
		<PHYSICAL>
			<TYPE>be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>nsec</>
		<DESCRIPTION>Residual nanoseconds.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>router_sc</>
		<DESCRIPTION>IP address of router shortcut by switch.</>
		<TYPE>ipaddr_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>src_addr</>
		<DESCRIPTION>Source IP address.</>
		<TYPE>ipaddr_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>time</>
		<DESCRIPTION>Current time in seconds since the epoch.</>
		<TYPE>time_t</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>uptime</>
		<DESCRIPTION>Elapsed milliseconds since the router booted.</>
		<TYPE>elapsed_t</>
		<PHYSICAL>
			<TYPE>be_t</>
			<WIDTH>4</>
		</>
	</>
	<FIELD>
		<NAME>count</>
		<DESCRIPTION>Number of records that follow in packet.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>dst_as</>
		<DESCRIPTION>Originating AS of destination address.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>dst_mask</>
		<DESCRIPTION>Destination address prefix mask bits.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>dst_port</>
		<DESCRIPTION>TCP/UDP destination port number.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>engine_id</>
		<DESCRIPTION>Switching engine type id.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>engine_type</>
		<DESCRIPTION>Switching engine type.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>flags</>
		<DESCRIPTION>Reason flow was discarded, etc...</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>flow_sequence</>
		<DESCRIPTION>Flow sequence counter.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>input</>
		<DESCRIPTION>Input interface index.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>output</>
		<DESCRIPTION>Output interface index.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>packets</>
		<DESCRIPTION>Packets sent in duration.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>prot</>
		<DESCRIPTION>IP protocol, e.g., 6=TCP, 17=UDP, ...</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>sampling_interval</>
		<DESCRIPTION>Sampling interval.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>sampling_mode</>
		<DESCRIPTION>Sampling mode.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>src_as</>
		<DESCRIPTION>Originating AS of source address.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>src_mask</>
		<DESCRIPTION>Source address prefix mask bits.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>src_port</>
		<DESCRIPTION>TCP/UDP source port number.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>tcp_flags</>
		<DESCRIPTION>Cumulative OR of tcp flags.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>tcp_misseq_cnt</>
		<DESCRIPTION>Number of mis-sequenced tcp pkts.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>tcp_retx_cnt</>
		<DESCRIPTION>Number of mis-seq with delay > 1sec.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>tcp_retx_secs</>
		<DESCRIPTION>Number of seconds between mis-sequenced pkts.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>tos</>
		<DESCRIPTION>IP type-of-service.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>version</>
		<DESCRIPTION>Record version.</>
		<TYPE>number</>
		<PHYSICAL>
			<TYPE>unsigned be_t</>
			<WIDTH>2</>
		</>
	</>
	<FIELD>
		<NAME>_PAD_</>
		<DESCRIPTION>Fixed size roundup pad.</>
		<TYPE>void</>
		<PHYSICAL>
			<WIDTH>26</>
		</>
	</>
</>'
