# tests for the dss stats query

TITLE + stats

export TZ=EST5EDT

VIEW data ../netflow/data

TEST 01 'default output'
	EXEC -x netflow '{stats --count --sum}' $data/netflow-5.dat
		OUTPUT - $'          FIELD           COUNT             SUM
       src_addr             247    656138212302
       dst_addr             247    332121641119
            hop             247     28896210442
          input             247            1943
         output             247             992
        packets             247           22691
          bytes             247        30393693
          first             247     38181216616
           last             247     38184764492
       src_port             247         2208170
       dst_port             247          877684
          flags             247               0
      tcp_flags             247            5038
           prot             247            1533
            tos             247            4224
         src_as             247         1725584
         dst_as             247         2775870
       src_mask             247            4979
       dst_mask             247            6156
      router_sc             247               0
   tcp_retx_cnt             247               0
  tcp_retx_secs             247               0
 tcp_misseq_cnt             247               0
        version             247            1235
          count             247             997
         uptime             247     38186327004
           time             247    250434664657
           nsec             247    166142393718
  flow_sequence             247         9032815
    engine_type             247             138
      engine_id             247            1242
sampling_interval             247               0
  sampling_mode             247               0
          start             247  2.50434660e+20
            end             247  2.50434663e+20'
	EXEC -x netflow '{stats --group=prot --range packets}' $data/netflow-5.dat
		OUTPUT - $'          FIELD             MIN             MAX            prot
        packets               1               6               1
        packets               1           20902               6
        packets               1              24              17'
	EXEC -x netflow '{stats --group=prot --group=tos --count packets}' $data/netflow-5.dat
		OUTPUT - $'          FIELD           COUNT            prot             tos
        packets              69               1               0
        packets             120               6               0
        packets              22               6             192
        packets              36              17               0'
	EXEC -x netflow '{stats --group=prot --group=tos --count --deviation packets}' $data/netflow-5.dat
		OUTPUT - $'          FIELD           COUNT       DEVIATION            prot             tos
        packets              69  2.01468134e+00               1               0
        packets             120  1.89836045e+03               6               0
        packets              22  1.67334945e+01               6             192
        packets              36  5.91008651e+00              17               0'

TEST 02 'print output'
	EXEC -x netflow '{stats --group=prot --group=tos --print="%(prot)u|%(tos)u|%(COUNT)u|%(DEVIATION)f" packets}' $data/netflow-5.dat
	OUTPUT - $'1|0|69|2.014681
6|0|120|1898.360454
6|192|22|16.733495
17|0|36|5.910087'
