# regression tests for the POSIX grep utilitiy

KEEP "*.[pd]at"

function DATA
{
	typeset f
	integer i
	typeset -i8 n
	for f
	do	test -f $f && continue
		case $f in
		big.dat)for ((i = 0; i <= 10000; i++))
			do	print $i
			done
			;;
		chars.dat)
			for ((n = 1; n < 256; n++))
			do	if	((n != 10))
				then	eval c=\$\'\\${n#8#}\'
					print -r -- "$c"
				fi
			done
			;;
		g1.dat)	cat <<'!'
.xxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
!
			;;
		g4.dat)	cat <<'!'
   1 ZIPPORI, Israel
	/usr/spool/ap/88/07/15/a0471: Israel-MonaLisa
   1 ZERIFIN, Israel
	/usr/spool/ap/88/05/17/a0823: Israel-Baez
   1 ZEPHYRHILLS, Fla.
	/usr/spool/ap/88/04/27/a0963: HelicopterEscape
   1 ZENICA, Yugoslavia
	/usr/spool/ap/88/07/13/a0814: Yugoslavia
   1 ZAP, N.D.
	/usr/spool/ap/88/03/13/a0776: CoalStrike
   1 ZAMBRANO, Honduras
	/usr/spool/ap/88/03/24/a0512: Honduras-Soldiers
   1 ZACHARY, La.
	/usr/spool/ap/88/04/05/a0745: Brites
   1 YUCCA VALLEY, Calif.
	/usr/spool/ap/88/08/26/a0624: BRF--SoCalQuake
   1 YORKVILLE, Ill.
	/usr/spool/ap/88/08/31/a0687: ReformedStudent
   1 YORK, Maine
	/usr/spool/ap/88/10/09/a0772: SeaSearches
   1 YENAN, China
	/usr/spool/ap/88/02/24/a0419: China-Yenan
   1 YELOWSTONE NATIONAL PARK, Wyo.
	/usr/spool/ap/88/09/15/a0792: Dukakis
   1 YEADON, Pa.
	/usr/spool/ap/88/05/14/a0689: Brites
   1 YATTA, Occupied West Bank
	/usr/spool/ap/88/10/29/a0417: Israel-Undercover
   1 YASSIHOYUK, Turkey
	/usr/spool/ap/88/09/09/a0423: MidasTomb
   1 YAPHANK, N.Y.
	/usr/spool/ap/88/05/10/a0686: Brites
   1 YAMOUSSOUKRO, Ivory Coast
	/usr/spool/ap/88/09/25/a0635: Africa-UN
!
			;;
		g5.dat)	cat <<'!'
com 1037117850
com -113451303
com -253844186
com -591640727
com -192085666
com 875206176
com -688908411
com 116220732
com -815364609
com 393021566
com -197586762
com -979497332
com 580876342
com 857752251
com -282427433
com 440265772
com 903702654
com 377371259
com -790446649
com -407893353
com 601447097
com 311585929
com -990601410
com 273028495
com -421520583
com -620551282
com -768217422
com 722547274
com 313902943
com -729597068
com 306062132
com 773754585
com -678639313
com -345701409
com -290065002
com -974307104
com 1047184566
com 210828681
com 108982822
com 68031245
com -1047141482
com 227569703
com -530798398
com -822779044
com 440691738
com 624275796
com 843073732
com 228971433
com 258376249
com -308161170
com -995590232
com 856677272
com 132296249
com 633658628
com 25935234
com -1063085400
com 148654970
com -824172925
com -659459669
com 196909720
com -393774825
com 736667556
com 674673107
com 1007653812
com -261383312
com 263123663
com -946595190
com -396442
com -506832213
com 149702652
com -937852087
com -500943193
com -288026147
com -653808189
com 801559288
com -653395420
com -405217270
com -749529781
com 965720542
com 396739912
com 250804267
com 1058925867
com 121948720
com 129329115
com -503214654
com 758365427
com -569717820
com 191932303
com 1041195498
com -178872661
com 719024931
com 389365053
com -695930677
com -720993320
com 659352079
com -445359373
com -405581235
com -495515453
com -861910553
com -35979929
com 1056535300
com 188042833
com -220408267
com -766533595
com 718865736
com -614647852
com 637296265
com 607439702
com -996163547
com -354301843
com 187216170
com -524246340
com 165453004
com -922340816
com -392313676
com 933400965
com -357455062
com 876069330
com 619850004
com 34785127
com -204461692
com -1021142281
com 261505948
com 713447396
com -264424205
com -757624021
com -697742264
com -67902535
com 813305897
com 611213298
com 810009586
com -351033158
com -757580248
com -754765998
com 96550293
com 818835421
com 625544984
com -301866740
com -363940120
com 196940655
com -990799410
com -650380493
com -823008037
com 229313079
com 480371766
com 934025272
com -223072319
com 481173087
com 101019846
com -954562179
com -267806909
com 1004678320
com 267997081
com -691653747
com 821221633
com 11472834
com -852175935
com 145665121
com 636788309
com -38553220
com -594562227
com 893269786
com -515632420
com -504118519
com -795555924
com -896489800
com 381679431
com 451163332
com 945690716
com -474968721
com -181646048
com -477705084
com 179336691
com 944752723
com -106013482
com 295161509
com -1026918852
com -1008494120
com -368542058
com 6153383
com 269567191
com 221084616
com -1015567145
com 326752359
com -253427460
com -990923267
com -745673545
com -772482393
com 12783572
com 695087221
com 782623860
com 239322275
com -920492686
com -461345191
com 304590436
com -141131273
com -1024267294
com -289620401
com -495626460
com 948528218
com 87006518
com 395454722
com 577392034
com 814343604
com 497169207
com -567127307
com 764271483
com -866721319
com -387005272
com -501938820
com 567881079
com 453665993
com -790328887
com 390097892
com 141055035
com 990378016
com -730626518
com 732985962
com -286073373
com 22747858
com -326949321
com 1022500944
com 905679100
com -448120658
com 363118089
com 819248817
com -691522154
com 59581781
com -450349154
com -729823626
com 646115018
com -65922779
com -373376656
com 1004572328
com 466654801
com 128208377
com 958497476
com 22952708
com -822443770
com 689913706
com 726815914
com -128674860
com 779809535
com -316931412
com -1025891272
com 4804418
com 309313283
com 536922264
com -876904372
com 700688221
com 186984467
com 791829735
com 237211732
com 515173384
com -911728294
com -783718602
com 160345621
com -716237348
com -185346360
com -634816499
com -845917397
com 460946577
com 777785415
com -579223277
com -127944050
com -351414763
com -1006508563
com 934284417
com -414601720
com -328845777
com 701421432
com -680992028
com 444048798
com -277796693
com -1014985030
com 213438258
com -863232710
com -236044310
com -593324426
com -269273068
com -163992668
com -1026411186
com 537134594
com 321391768
com -872419201
com -795875760
com 373186979
com 616631783
com -567696334
com 554407297
com 723377442
com 1062001538
com 152160308
com 43834651
com 902450760
com -390697289
com 431114551
com -851289267
com 454377388
com 470923853
com -950885734
com -313255930
com -388083168
com -267037738
com -601696282
com -848277038
com 745209391
com -423687675
com 646585818
com -613632730
com 151442994
com 868010020
com -589969477
com 756495308
com 482257575
com -546245706
com -56416295
com -922688644
com -927591869
com -193091648
com 505183574
com -696294953
com -676843648
com -458233039
com 1016060900
com 235279194
com 255314418
com 821562352
com 677435672
com -137977226
com -296008805
com -284837634
com 992052324
com 848130900
com -612135722
com -242663012
com 40910582
com -633235255
!
			;;
		g6.pat)	cat <<'!'
^Aconv\(
^Cconv\(
^Uconv\(
^bconv\(
^begrewrite\(
^bi_bread\(
^bi_close\(
^bi_open\(
^bi_rand\(
^bi_read\(
^bi_write\(
^bltinval\(
^compattype\(
^compile\(
^concat\(
^constants\(
^curproc\(
^dclformals\(
^declare\(
^decref\(
^dump\(
^dupgen\(
^dupnode\(
^econv\(
^elemrewr\(
^emalloc\(
^emit\(
^emitconst\(
^emitspace\(
^eqtype\(
^erealloc\(
^errflush\(
^error\(
^etypeof\(
^etypeoft\(
^execute\(
^exits\(
^fileline\(
^freenode\(
^gen\(
^genfreeauto\(
^halt\(
^here\(
^iconv\(
^idnode\(
^idump\(
^istart\(
^istopscope\(
^length\(
^lerror\(
^lexinit\(
^lfileline\(
^lgen\(
^lookup\(
^mconv\(
^mk\(
^mkcheck\(
^nargs\(
^nconv\(
^ndump\(
^new\(
^newc\(
^newfile\(
^newi\(
^newl\(
^panic\(
^patch\(
^popscope\(
^pprint\(
^printable\(
^processes\(
^procinit\(
^proglocals\(
^pushscope\(
^pushval\(
^recrewrite\(
^rerror\(
^rpanic\(
^run\(
^scopedecrefgen\(
^setprog\(
^tconv\(
^topofstack\(
^topscope\(
^type\(
^typeinit\(
^typeof\(
^typeoftid\(
^warn\(
^yylex\(
^yyparse\(
!
			;;
		g8.dat)	cat <<'!'
b
ba
!
			;;
		g12.dat)cat <<'!'
AAA	n
AAAS	n
Aaron	n
AAU	n
AAUP	d
AAUW	d
ABA	n
Ababa	pc
aback	d
abacus	n
abaft	d
abalone	n
abandon	v,er,va
abase	v,er,va
abash	v,er,va
abate	v,er,va
abattoir	n
abbe	n
abbess	n
abbey	n
abbot	n
Abbott	n
abbreviate	v,ion
abc	n
abdicate	v,ion,va
abdomen	n
abdominal	a
abduct	v,ion
Abe	pc
abeam	d
abed	d
Abel	pc
Abelian	pc
Abelson	n
Aberdeen	pc
Abernathy	n
aberrant	n,a
aberrate	v,ion
abet	v,va,ms
abettor	n
abeyant	a
abhor	v,er,ms
abhorrent	a
abide	v,er
Abidjan	pc
Abigail	pc
abject	a,ion
abjuration	n
abjure	v,er
ablate	v,ion
ablaut	n
ablaze	d
able	v,a,comp,va
abloom	d
ablution	n
ABM	n
abnegate	v,ion
Abner	pc
abnormal	n,a
aboard	d
abode	n
abolish	v,er,va
abolition	n,na
abolitionary	n
abominable	a
abominate	v,ion
aboriginal	n,a
aborigine	n
aborning	d
abort	v,er,ion
abortifacient	n
abound	vi
about	d,nopref
above	d
aboveboard	d
aboveground	d
abovementioned	d
abracadabra	n
abrade	v,er,va
Abraham	n
Abram	n
Abramson	n
abrasion	n,na
abreact	v,ion
abreast	d
abridge	v,er,va
abridgment	n
abroad	d
abrogate	v,ion
abrupt	a,ion
abscess	n,v
abscissa	n
abscissae	d
abscission	n
abscond	v,er
absent	v,a
absentee	n
absenteeism	n
absentia	n
absentminded	a
absinthe	n
absolute	n,a,na
absolution	n
absolve	v,er
absorb	v,er,va
absorbent	a
absorption	n,na
abstain	v,er
abstemious	a
abstention	n
abstinent	a
abstract	n,v,a,er,ion
abstruse	a
absurd	a,na
abuilding	d
abundant	a
abuse	n,v,er,va
abusive	a
abut	v,er,va,ms
abysmal	a
abyss	n
abyssal	a
Abyssinia	pc
AC	d
acacia	n
academe	pc,na
academia	pc
academic	n,na
academician	n
academy	n
Acadia	pc
acanthus	n
Acapulco	pc
accede	v
accelerando	d
accelerate	v,ion
accelerometer	n
accent	n,v,na
accentual	a
accentuate	v,ion
accept	v,er,va
acceptant	a
acceptor	n
access	n,v
accessible	a,in
accession	n,v,na
accessory	n,na
accident	n,a
accidental	a
accipiter	n
acclaim	n,v,er
acclamation	n
acclimate	n,v,ion
acclimatize	v,er,ion
accolade	n
accommodate	v,ion
accompaniment	n
accompanist	n
accompany	v,na
accompli	d
accomplice	n
accomplish	v,er,va
accord	n,v
accordant	a
accordion	n,na
accost	v
account	n,v,va
accountant	n,a,na
Accra	pc
accredit	v,va
accreditation	n
accrete	v
accretion	n,na
accretionary	n
accrual	a
accrue	v,va
acculturate	v,ion
accumulate	v,ion
accuracy	n,in
accurate	a,in
accursed	a
accusation	n,na
accusatory	d
accuse	v,er
accustom	v
ace	n,v,nopref
acentric	d
acerb	a
acerbic	a
acetaldehyde	n
acetate	n
acetic	d
acetone	n
acetyl	n,na
acetylene	n
ache	n,v,er
achieve	v,er,va
Achilles	pc
aching	a
achondrite	n,na
achromatic	a
acid	n,a
acidic	d
acidify	v,er,ion
acidimeter	n,na
acidulous	a
Ackerman	n
Ackley	n
acknowledge	v,va
acknowledgeable	d
ACLU	pc
ACM	pc
acme	pc
acne	n
acolyte	n
acorn	n
acoustic	n,a
acoustician	n
acoustoelectric	a,na
acoustooptic	n,a,na
acquaint	v
acquaintance	n,na
acquiesce	v
acquiescent	a
acquire	v,va
acquisition	n,na
acquit	v,ms
acquittal	n
acre	n
acreage	pc
acrid	a
acrimonious	a
acrimony	n
acrobat	n
acrobatic	n,na
acrolein	n
acronym	n
acrophobia	n
acropolis	n
across	d
acrostic	a
acrylate	n
acrylic	n
ACS	pc
act	n,v,ion,va
Actaeon	pc
actinic	na
actinide	n
actinium	n
actinometer	n,na
activate	v,ion,in
activism	pc
Acton	n
actress	n
actual	a,na
actuarial	a
actuary	n
actuate	v,ion
acuity	n
acumen	n
acupuncture	n
acute	a
acyclic	a
acyl	n
ad	n
Ada	pc
adage	n
adagio	n
Adair	pc
Adam	pc
adamant	a
adamantine	a
Adams	n
Adamson	n
adapt	v,er,ion,va
adaptation	n,na
adaptive	a
adaptor	n
add	v,er,va
addend	n
addenda	pc
addendum	pc
addict	n,v,ion
Addis	pc
Addison	n
addition	n,na
addle	v
address	n,v,er,na,va
addressee	n
Addressograph	pc
adduce	v,er,va
Adelaide	pc
Adele	pc
Adelia	pc
Aden	pc
adenoid	n,na
adenoma	n
adenosine	n
adept	n,a
adequacy	n,in
adequate	a,in
adhere	v
adherent	n,a
adhesion	n,na
adiabatic	n
adieu	n
adipose	a
Adirondack	n
adjacent	a
adjectival	a
adjective	n,a
adjoin	v
adjoint	n
adjourn	v,va
adjudge	v
adjudicate	v,ion
adjunct	n,a,ion
adjuration	n
adjure	v
adjust	v,er,va
adjutant	n,a
Adkins	n
Adler	n
administer	v
administrable	d
administrate	v,ion
administratrix	d
admiral	n
admiralty	n
admiration	n
admire	v,er,va
admissible	a,in
admission	n,na
admit	v,er,ms
admittance	n
admix	v
admixture	n
admonish	v,er,va
admonition	n
admonitory	a
ado	d,nopref
adobe	n,er
adolescent	n,a
Adolph	pc
Adolphus	pc
Adonis	pc
adopt	v,er,ion,va
adoration	n
adore	v,er,va
adorn	v,er,va
adrenal	n,a
adrenaline	n
Adrian	pc
Adriatic	pc
Adrienne	pc
adrift	d
adroit	a,comp
adsorb	v,va
adsorbate	n
adsorbent	n
adsorption	n,na
adulate	v,ion
adult	n,a
adulterant	n
adulterate	v,ion
adulterer	n
adulteress	n
adulterous	a
adultery	n
adumbrate	v,ion
advance	v,er,va
advantage	n,v
advantageous	a
advection	n,na
advent	n,na
adventitial	a
adventitious	a
adventure	n,v,er,na
adventuresome	a
adventuress	n
adventurous	a
adverb	n
adverbial	a
adversary	n,a
adverse	a
advert	n,v
advertent	a,in
advertise	v,er,va
advice	pc
advise	v,er,va
advisee	n
advisor	n,y
advocacy	n
advocate	v,ion
adz	n
adze	n
Aegean	pc
aegis	n
Aeneas	pc
Aeneid	pc
aeolian	d
Aeolus	pc
aerate	v,a,ion,va
aerial	n,a,na
aerie	n
Aerobacter	pc
aerobatic	n
aerobic	n,na
aerodynamic	n,na
aeronautic	n,na
aerosol	n,na
aerospace	n
Aeschylus	pc
aesthete	n
aesthetic	n,na
afar	d
affable	a,va
affair	n
affect	n,v,ion,va
affectation	n
affectionate	a
afferent	a
affiance	n,v
affidavit	n
affiliate	n,v,ion
affine	n,ed,a
affinity	n
affirm	v,va
affirmation	n,na
affix	v,va
affixation	n
afflatus	n
afflict	v,er,ion
affluent	n,a
afford	v,va
afforest	v
afforestation	n
affray	n,v
affright	n,v
affront	n,v
afghan	n
Afghanistan	pc
aficionado	n
afield	d
afire	d
AFL	d
aflame	d
afloat	d
aflutter	d
afoot	d
aforementioned	d
aforesaid	d
aforethought	d
afoul	d
afraid	d
afresh	d
Africa	pc
Afrikaner	pc
afro	n
aft	er
afterbirth	n
afterburner	n
afterdeck	n
aftereffect	n
afterglow	n
afterimage	n
afterlife	n
aftermath	pc
aftermost	pc
afternoon	n
aftershock	n
aftertaste	n
afterthought	n
afterward	n
afterworld	n
again	d,nopref
against	d,nopref
Agamemnon	pc
agamic	a
agape	d
agar	n,nopref
agate	n,nopref
Agatha	pc
agave	n
age	n,v
Agee	n
agelong	d
agenda	pc,na
agendum	pc
agent	n,a
agglomerate	v,ion
agglutinate	v,ion
agglutinin	n
aggrade	v
aggravate	v,ion
aggregate	n,v,a,ion
aggression	n,na
aggressor	n
aggrieve	v
aghast	d
agile	a
agitate	v,ion
agitprop	pc
agleam	d
agley	d
aglitter	d
aglow	d
Agnes	pc
Agnew	n
agnomen	n
agnostic	n,na
ago	d,nopref
agog	d
agon	n
agone	na
agony	n
agora	n
agoraphobia	n
agouti	n
agrarian	n,na
agree	v,va
agreeable	a
agreeing	d
agribusiness	n
Agricola	pc
agricultural	a,na
agriculture	n,na
agrimony	n
agronomist	n
agronomy	n,na
aground	d
ague	n
Agway	pc
ah	n,nopref
Ahab	pc
ahead	d
ahem	d
Ahmadabad	pc
ahoy	d
aid	n,v,er,nopref
Aida	pc
aide	n,nopref
Aiken	pc
ail	n,v,nopref
ailanthus	pc
Aileen	pc
aileron	n
ailment	n
aim	n,v
ain't	d
Ainu	n
air	n,v,man,y
airborne	d
airbrush	n,v
Airbus	n
aircraft	n
airdrop	n,v,va
Aires	pc
airfare	n
airfield	n
airflow	n
airfoil	n
airframe	n
airhead	n
airlift	n,v
airline	n,er
airlock	n
airmail	n,v
airmass	n
airpark	n
airport	n
airscrew	n
airsick	a
airspace	n
airspeed	n
!
			;;
		g6.dat)	cat <<'!'
#include "alloc.h"
#include <libc.h>

char *
emalloc(unsigned long n)
{
	char *p;
	p=malloc((unsigned)n);
	if(p==0){
		warn("out of memory; exiting");
		exits("out of memory");
	}
	return p;
}
char *
erealloc(char *p, unsigned long n)
{
	p=realloc(p, (unsigned)n);
	if(p==0){
		warn("out of memory; exiting");
		exits("out of memory");
	}
	return p;
}
#include "alloc.h"
#include "word.h"
#include "store.h"
#include "comm.h"
#include <libc.h>

/*
 * Push constants
 */

ipushconst(Proc *proc)
{
	*proc->sp++=(SWord)*++proc->pc;
	return 1;
}

ipush_2(Proc *proc)
{
	*proc->sp++=-2;
	return 1;
}

ipush_1(Proc *proc)
{
	*proc->sp++=-1;
	return 1;
}

ipush0(Proc *proc)
{
	*proc->sp++=0;
	return 1;
}

ipush1(Proc *proc)
{
	*proc->sp++=1;
	return 1;
}

ipush2(Proc *proc)
{
	*proc->sp++=2;
	return 1;
}

ipush3(Proc *proc)
{
	*proc->sp++=3;
	return 1;
}

ipush4(Proc *proc)
{
	*proc->sp++=4;
	return 1;
}

ipush5(Proc *proc)
{
	*proc->sp++=5;
	return 1;
}

ipush6(Proc *proc)
{
	*proc->sp++=6;
	return 1;
}

ipush7(Proc *proc)
{
	*proc->sp++=7;
	return 1;
}

ipush8(Proc *proc)
{
	*proc->sp++=8;
	return 1;
}

ipush9(Proc *proc)
{
	*proc->sp++=9;
	return 1;
}

ipush10(Proc *proc)
{
	*proc->sp++=10;
	return 1;
}

/*
 * Binary operators
 */
ige(Proc *proc)
{
	--proc->sp;
	proc->sp[-1]=proc->sp[-1]>=proc->sp[0];
	return 1;
}

ile(Proc *proc)
{
	--proc->sp;
	proc->sp[-1]=proc->sp[-1]<=proc->sp[0];
	return 1;
}

ine(Proc *proc)
{
	--proc->sp;
	proc->sp[-1]=proc->sp[-1]!=proc->sp[0];
	return 1;
}

ieq(Proc *proc)
{
	--proc->sp;
	proc->sp[-1]=proc->sp[-1]==proc->sp[0];
	return 1;
}

igt(Proc *proc)
{
	--proc->sp;
	proc->sp[-1]=proc->sp[-1]>proc->sp[0];
	return 1;
}

ilt(Proc *proc)
{
	--proc->sp;
	proc->sp[-1]=proc->sp[-1]<proc->sp[0];
	return 1;
}

iadd(Proc *proc)
{
	--proc->sp;
	proc->sp[-1]+=proc->sp[0];
	return 1;
}

isub(Proc *proc)
{
	--proc->sp;
	proc->sp[-1]-=proc->sp[0];
	return 1;
}

imul(Proc *proc)
{
	long l0, l1, l;
	--proc->sp;
	l0=proc->sp[-1];
	l1=proc->sp[0];
	l=l0*l1;
	if(l1 && l/l1 != l0)
		rerror("product overflow");
	proc->sp[-1]=l;
	return 1;
}

idiv(Proc *proc)
{
	--proc->sp;
	if(proc->sp[0]==0)
		rerror("zero divide");
	proc->sp[-1]/=proc->sp[0];
	return 1;
}

imod(Proc *proc)
{
	--proc->sp;
	if(proc->sp[0]==0)
		rerror("zero modulo");
	proc->sp[-1]%=proc->sp[0];
	return 1;
}

iand(Proc *proc)
{
	--proc->sp;
	proc->sp[-1]&=proc->sp[0];
	return 1;
}

ior(Proc *proc)
{
	--proc->sp;
	proc->sp[-1]|=proc->sp[0];
	return 1;
}

ixor(Proc *proc)
{
	--proc->sp;
	proc->sp[-1]^=proc->sp[0];
	return 1;
}

ilsh(Proc *proc)
{
	--proc->sp;
	proc->sp[-1]<<=proc->sp[0];
	return 1;
}

irsh(Proc *proc)
{
	--proc->sp;
	proc->sp[-1]>>=proc->sp[0];
	return 1;
}

imax(Proc *proc)
{
	SWord l;
	l=*--proc->sp;
	if(l>proc->sp[-1])
		proc->sp[-1]=l;
	return 1;
}

/*
 * Unary operators
 */

ineg(Proc *proc)
{
	proc->sp[-1]=-proc->sp[-1];
	return 1;
}

inot(Proc *proc)
{
	proc->sp[-1]=~proc->sp[-1];
	return 1;
}

ilnot(Proc *proc)
{
	proc->sp[-1]=!proc->sp[-1];
	return 1;
}

iref(Proc *proc)
{
	Store *s=(Store *)*--proc->sp;
	*proc->sp++=s->ref-1;
	decref(&s);
	return 1;
}

ilen(Proc *proc)
{
	Store *s=(Store *)*--proc->sp;
	*proc->sp++=s->len;
	decref(&s);
	return 1;
}

/*
 * String comparison: put value of strcmp() on stack
 */

istrcmp(Proc *proc)
{
	int cmp;
	Store *s1, *s2;
	s1=(Store *)proc->sp[-2];
	s2=(Store *)proc->sp[-1];
	cmp=strcmp((char *)s1->data, (char *)s2->data);
	decref(&s1);
	decref(&s2);
	proc->sp--;
	proc->sp[-1]=cmp;
	return 1;
}

/*
 * Print
 */

iprintint(Proc *proc)
{
	pprint(proc, "%ld", *--proc->sp);
	return 1;
}

iprintnewline(Proc *proc)
{
	pprint(proc, "\n");
	return 1;
}

iprintblank(Proc *proc)
{
	pprint(proc, " ");
	return 1;
}

iprintunit(Proc *proc)
{
	pprint(proc, "(unit)");
	return 1;
}

iprintchar(Proc *proc)
{
	pprint(proc, "%c", *--proc->sp);
	return 1;
}

pprint(proc, fmt, a, b, c, d, e)
	Proc *proc;
	char *fmt;
{
	char buf[1024];
	long n;
	n=sprint(buf, fmt, a, b, c, d, e);
	if(proc->prbuf==0){
		proc->prbuf=emalloc(64+n);
		proc->maxprbuf=64+n;
		proc->nprbuf=0;
	}
	if(n+proc->nprbuf+1>proc->maxprbuf){
		proc->prbuf=erealloc(proc->prbuf, proc->maxprbuf+64+n);
		proc->maxprbuf+=64+n;
	}
	strcpy(proc->prbuf+proc->nprbuf, buf);
	proc->nprbuf+=n;
}
/*
 * Stack management
 */

ipop(Proc *proc)
{
	--proc->sp;
	return 1;
}

ipopptr(Proc *proc)
{
	decref((Store **)(proc->sp-1));
	--proc->sp;
	return 1;
}

idup(Proc *proc)
{
	proc->sp++;
	proc->sp[-1]=proc->sp[-2];
	return 1;
}

idupptr(Proc *proc)
{
	proc->sp++;
	proc->sp[-1]=proc->sp[-2];
	((Store *)(proc->sp[-1]))->ref++;
	return 1;
}
#include "node.h"
#include "symbol.h"
#include "alloc.h"
#include "word.h"
#include "store.h"
#include "comm.h"
#include "inst.h"
#include <libc.h>

#define	FNS
#include "lib.h"
#undef	FNS

#define	C	0x40000000
#define	I	0x20000000
#define	F	0x10000000
#define	M(x)	((x)&~(C|I|F))

long call0[]={	/* plain function, 0 arguments */
	I+Ipushfp,	C+0,	F,	I+Iret,	C+0*WS,	I+Idone,	0
};
long call1[]={	/* plain function, 1 argument */
	I+Ipushfp,	C+0,	F,	I+Iret,	C+1*WS,	I+Idone,	0
};
long call2[]={	/* plain function, 2 arguments */
	I+Ipushfp,	C+0,	F,	I+Iret,	C+2*WS,	I+Idone,	0
};
long call3[]={	/* plain function, 3 arguments */
	I+Ipushfp,	C+0,	F,	I+Iret,	C+3*WS,	I+Idone,	0
};
long call4[]={	/* plain function, 4 arguments */
	I+Ipushfp,	C+0,	F,	I+Iret,	C+4*WS,	I+Idone,	0
};
long call5[]={	/* plain function, 5 arguments */
	I+Ipushfp,	C+0,	F,	I+Iret,	C+5*WS,	I+Idone,	0
};
long call2_0[]={/* two-step function, 0 arguments */
	I+Ipushfp,	C+0,	F+0,	F+1,	I+Iret,	C+0*WS,	I+Idone,	0
};

struct{
	char	*name;
	int	(*fn[3])();
	int	nargs;
	long	*template;
}bltin[]={
#include "lib.h"
	0,	{0,	0,	0},	0,	0,
};

bltinlookup(char *s)
{
	int i;
	for(i=0; bltin[i].name; i++)
		if(strcmp(s, bltin[i].name)==0)
			return i;
	error("%s not a builtin", s);
	return -1;
}

long
bltinval(char *name, Node *t)
{
	int i, nargs, len;
	long *template, *p;
	Store *s;
	SWord *d;
	if(t->o.t!=TProg)
		error("builtin %s not a function", name);
	i=bltinlookup(name);
	nargs=bltin[i].nargs;
	if(nargs!=length(t->l))	/* necessary but not sufficient */
		error("wrong #args to builtin %s: %d (should be %d)", name, length(t->l), nargs);
	template=bltin[i].template;
	p=template;
	for(len=0; *p; p++)
		len++;
	s=(Store *)emalloc(SHSZ+len*LWS);
	s->ref=1;
	s->type=Sprog;
	s->sbits=0;
	s->len=len;
	d=s->data;
	for(p=template; *p; p++)
		if(*p&C)
			*d++=(SWord)M(*p);
		else if(*p&I)
			*d++=(SWord)insttab[M(*p)].fp;
		else if(*p&F)
			*d++=(SWord)bltin[i].fn[M(*p)];
	return (long)s;
}

Store *
mk(type, len)
{
	Store *s;
	if(type==Sstruct)
		len++;
	s=(Store *)emalloc(SHSZ+len*LWS);
	s->ref=1;
	s->type=type;
	if(type==Sstruct){
		s->sbits=1;
		s->data[0]=0;
	}else
		s->sbits=0;
	s->len=len;
	return s;
}
#include "node.h"
#include "symbol.h"
#include "alloc.h"
#include "ydefs.h"
#include "word.h"
#include "store.h"
#include "comm.h"
#include "inst.h"
#include "errjmp.h"
#include <libc.h>

long		resultloc;
long		returnloc;
Node		*formals;
long		autooffset;
extern int	bflag;
extern int	cflag;
extern int	nscope;
extern Node	arychartype;

compile(n)	/* called from parser only */
	Node *n;
{
	extern long autooffset;
	Errjmp x;
	n=constants(n);
	if(cflag){
		fileline();
		fprint(2, "constants:\n");
		dump(n, 0);
	}
	errsave(x);
	if(errmark()){
		autooffset=0;
		freenode(n);
		errrest(x);
		errjmp();
	}
	istart();
	gen(n, 0);
	freenode(n);
	errrest(x);
}

gen(Node *n, int retain)
{
	int i;
	if(n==0)
		return;
	switch(n->t){
	case NArrayref:
		arygen(n->l, n->r, 0, 0L);
		if(!retain)
			popgen(n->l->o.s->val->type->r);
		return;
	case NBecome:
		if(n->l->t==NCall && !bflag){
			callgen(n->l, Ibecome);
			return;
		}
		gen(n->l, 1);
		n=n->r;
		if(n->o.t==TID)
			n=typeoftid(n);
		switch(n->o.t){
		case TInt:
		case TChar:
			emit(Istoreauto);
			emitconst(-LWS*(3+length(formals)));
			break;
		case TArray:
		case TChan:
		case TProg:
		case TStruct:
			emit(Istoreptrauto);
			emitconst(-LWS*(3+length(formals)));
			break;
		case TUnit:
			break;
		default:
			panic("can't compile %t become", n->o.t);
		}
		scopedecrefgen();
		trlrgen();
		return;
	case NBegin:
		callgen(n->l, Ibegin);
		return;
	case NCall:
		callgen(n, Icall);
		if(!retain)
			popgen(etypeoft(n->l)->r);
		return;
	case NDecl:
	case NDeclsc:
		declare(n, 0, 0, 1);
		return;
	case NExpr:
		switch(n->o.i){
		case GE:
			i=Ige;
		Binop:
			gen(n->l, 1);
			gen(n->r, 1);
			if(eqtype(etypeof(n->l), &arychartype)){
				emit(Istrcmp);
				constgen(0L);
			}
			emit(i);
		Popit:
			if(!retain)
				emit(Ipop);
			return;
		case LE:
			i=Ile;
			goto Binop;
		case NE:
			i=Ine;
			goto Binop;
		case EQ:
			i=Ieq;
			goto Binop;
		case '>':
			i=Igt;
			goto Binop;
		case '<':
			i=Ilt;
			goto Binop;
		case '+':
			i=Iadd;
			goto Binop;
		case '-':
			i=Isub;
			goto Binop;
		case '*':
			i=Imul;
			goto Binop;
		case '/':
			i=Idiv;
			goto Binop;
		case '%':
			i=Imod;
			goto Binop;
		case '&':
			i=Iand;
			goto Binop;
		case '|':
			i=Ior;
			goto Binop;
		case '^':
			i=Ixor;
			goto Binop;
		case LSH:
			i=Ilsh;
			goto Binop;
		case RSH:
			i=Irsh;
			goto Binop;
		case ANDAND:
			condgen(n->l, n->r, Ijmptrue, Ijmpfalse, 0L, 1L, retain);
			return;
		case OROR:
			condgen(n->l, n->r, Ijmpfalse, Ijmptrue, 1L, 0L, retain);
			return;
		case PRINT:
			gen(n->l, 1);
			printgen(n->l);
			emit(Isprint);
			if(!retain)
				emit(Iprint);
			return;
		case SND:
			gen(n->l, 1);
			constgen((long)Cissnd);
			emit(Icommset1);
			emit(Icommcln1);
			gen(n->r, 1);
			if(isptrtype(etypeoft(n->l)->r))
				emit(Isndptr);
			else
				emit(Isnd);
			if(!retain)
				popgen(etypeof(n));
			return;
		case RCV:
			gen(n->l, 1);
			constgen(0L);	/* not Cissnd */
			emit(Icommset1);
			emit(Icommcln1);
			return;
		case '=':
			gen(n->r, 1);
			if(retain)
				dupgen(etypeof(n->r), 1);
			lgen(n->l);
			return;
		case LEN:
			gen(n->l, 1);
			emit(Ilen);
			goto Popit;
		case REF:
			if(isptrtype(etypeof(n->l))){
				gen(n->l, 1);
				emit(Iref);
			}else
				constgen(1L);
			goto Popit;
		case DEF:
			if(retain && n->l->t==NID && isinttype(etypeof(n->l))){
				constgen(1L);
				return;
			}
			/*
			 * don't really need to call lgen1, which will uniquify our
			 * array for us, but it does no harm, and it's easy.
			 */
			lgen1(n->l, Idefauto, Idef, Idefary);
			goto Popit;
		case UMINUS:
			gen(n->l, 1);
			emit(Ineg);
			goto Popit;
		case '~':
			gen(n->l, 1);
			emit(Inot);
			goto Popit;
		case '!':
			gen(n->l, 1);
			emit(Ilnot);
			goto Popit;
		case INC:
			lgen1(n->l, Iincauto, Iinc, Iincary);
			goto Popit;
		case DEC:
			lgen1(n->l, Idecauto, Idec, Idecary);
			goto Popit;
		default:
			panic("can't compile %e expression", n->o.i);
		}

	case NExprlist:
		/*
		 * This is an arg or element list; first is pushed last
		 */
		gen(n->r, 1);
		gen(n->l, 1);
		return;
	case NID:
		if(!retain)
			return;
		switch(typeof(n)->o.t){
		case TInt:
		case TChar:
			if(n->o.s->val->isauto){
				emit(Ipushauto);
				emitconst(n->o.s->val->store.off);
			}else{
				emit(Ipush);
				emitconst((long)&n->o.s->val->store.l);
			}
			return;
		case TProg:
		case TArray:
		case TChan:
		case TStruct:
			if(n->o.s->val->isauto){
				emit(Ipushptrauto);
				emitconst(n->o.s->val->store.off);
			}else{
				emit(Ipushptr);
				emitconst((long)&n->o.s->val->store.l);
			}
			return;
		case TUnit:
			if(retain)
				constgen(0L);
			return;
		case TType:
			lerror(n, "attempt to evaluate type variable %m", n);
		default:
			panic("can't compile type %t", n->o.s->val->type->o.t);
		}
	case NIf:
		ifgen(n);
		return;
	case NList:
		gen(n->l, 0);
		gen(n->r, 0);
		return;
	case NLoop:
		loopgen(n);
		return;
	case NMk:
		mkgen(n->l, n->r);
		return;
	case NNum:
		if(retain)
			constgen(n->o.l);
		return;
	case NProg:
		if(retain)
			proggen(n->l, n->r);
		return;
	case NResult:
		gen(n->l, 1);
		emit(Ijmp);
		emitconst((long)(resultloc-here()-1)*WS);
		return;
	case NScope:
		pushscope();
		if(nscope==1){
			int nauto;
			autooffset=0;
			emit(Ipushfp);
			nauto=here();
			emitconst(0L);
			gen(n->l, 0);
			patch((int)nauto, autooffset);
		}else
			gen(n->l, 0);
		scopedecrefgen();
		popscope();
		return;
	case NSelect:
		selgen(n->l);
		return;
	case NSmash:{
		Value *vl, *vr;
		vl=n->l->o.s->val;
		vr=n->r->o.s->val;
		if(vr->type->o.t==TType){
			freenode(vl->type);
			vl->type=dupnode(vr->type);
			return;
		}
		gen(n->r, 1);
		/*
		 * Free old values; tricky: push as int, pop as ptr
		 */
		if(isptrtype(vl->type)){
			if(vl->isauto){
				emit(Ipushauto);
				emitconst(vl->store.off);
			}else{
				emit(Ipush);
				emitconst((long)&vl->store.l);
			}
			emit(Ipopptr);
		}
		if(vl->isauto){
			emit(Istoreauto);
			emitconst(vl->store.l);
			return;
		}
		emit(Istore);
		emitconst((long)&vl->store.l);
		return;
	}
	case NString:
		if(retain){
			Store *s;
			s=(Store *)emalloc(SHSZ+strlen(n->o.c)+1);
			strcpy((char *)(s->data), n->o.c);
			s->ref=1;
			s->len=strlen(n->o.c);
			s->type=Sarychar;
			emit(Ipushdata);
			emitconst((long)s);
		}
		return;
	case NStructref:
		arygen(n->l, n->r, 1, n->o.l);
		return;
	case NSwitch:
		switchgen(n->l, n->r);
		return;
	case NUnit:
		if(retain)
			constgen(0L);
		return;
	case NVal:
		valgen(n->l);
		if(!retain)
			popgen(n->o.n);
		return;
	}
	panic("can't compile node %n", n->t);
	return;
}

arygen(Node *a, Node *i, int isstr, long off)
{
	int ptr, ischar;
	if(isstr){
		ptr=isptrtype(i);
		constgen(off);
		ischar=0;
	}else{
		Node *t=etypeoft(a)->r;
		ptr=isptrtype(t);
		gen(i, 1);
		ischar=t->o.t==TChar;
	}
	if(a->t!=NID){
		gen(a, 1);
		emit(ptr? Ipusharyptrexpr :
			(ischar? Ipusharycharexpr :Ipusharyexpr));
	}else if(a->o.s->val->isauto){
		emit(ptr? Ipusharyptrauto :
			(ischar? Ipusharycharauto :Ipusharyauto));
		emitconst(a->o.s->val->store.off);
	}else{
		emit(ptr? Ipusharyptr :
			(ischar? Ipusharychar :Ipushary));
		emitconst((long)&a->o.s->val->store.l);
	}
}

lgen(Node *n)
{
	switch(n->t){
	case NID:
		switch(typeof(n)->o.t){
		case TChar:
			if(n->o.s->val->isauto){
				emit(Istorecharauto);
				emitconst(n->o.s->val->store.off);
				return;
			}
			emit(Istorechar);
			emitconst((long)&n->o.s->val->store.l);
			return;
		case TInt:
		case TUnit:
			if(n->o.s->val->isauto){
				emit(Istoreauto);
				emitconst(n->o.s->val->store.off);
				return;
			}
			emit(Istore);
			emitconst((long)&n->o.s->val->store.l);
			return;
		case TArray:
		case TChan:
		case TProg:
		case TStruct:
			if(n->o.s->val->isauto){
				emit(Istoreptrauto);
				emitconst(n->o.s->val->store.off);
				return;
			}
			emit(Istoreptr);
			emitconst((long)&n->o.s->val->store.l);
			return;

		default:
			panic("lgen: ID type %t", n->o.s->val->type->o.t);
			return;
		}
	case NArrayref:
		gen(n->r, 1);
		goto Genref;
	case NStructref:
		constgen(n->o.l);
	Genref:
		lgen1(n->l, Ipushuniqauto, Ipushuniq, Ipushuniqary);
		emit(Istoreary);
		return;
	default:
		panic("lgen: lvalue node %n", n->t);
	}
}

/*
 * n is a compound object about to be assigned into
 */
lgen1(Node *n, int Iauto, int Ivar, int Iary)
{
	switch(n->t){
	case NID:
		if(n->o.s->val->isauto){
			emit(Iauto);
			emitconst(n->o.s->val->store.off);
			return;
		}
		emit(Ivar);
		emitconst((long)&n->o.s->val->store.l);
		return;
	case NArrayref:
		gen(n->r, 1);
		goto Genref;
	case NStructref:
		constgen(n->o.l);
	Genref:
		lgen1(n->l, Ipushuniqauto, Ipushuniq, Ipushuniqary);
		emit(Iary);
		return;
	default:
		panic("lgen1: lvalue node %n", n->t);
	}
}

ifgen(Node *n)
{
	int loc1, loc2;
	gen(n->o.n, 1);
	emit(Ijmpfalse);
	loc1=here();
	emit(0);
	gen(n->l, 0);
	if(n->r==0){
		patch(loc1, (long)(here()-loc1-1)*WS);
		return;
	}
	emit(Ijmp);
	loc2=here();
	emit(0);
	patch(loc1, (long)(here()-loc1-1)*WS);
	gen(n->r, 0);
	patch(loc2, (long)(here()-loc2-1)*WS);
	return;
}

valgen(Node *n)
{
	int loc1, loc2;
	int orl;
	emit(Ijmp);
	loc1=here();
	emitconst(0L);
	orl=resultloc;
	resultloc=here();
	emit(Ijmp);
	loc2=here();
	emitconst(0L);
	patch(loc1, (long)(here()-loc1-1)*WS);
	gen(n, 1);
	emit(Ivalnoresult);
	patch(loc2, (long)(here()-loc2-1)*WS);
	resultloc=orl;
}

loopgen(Node *n)
{
	int loc0, loc1, loc2;
	if(n->o.i){	/* enter loop at top, so jump to body */
		emit(Ijmp);
		loc0=here();
		emit(0);
	}
	gen(n->r->l, 0);	/* left expr */
	if(n->r->r){		/* jump to condition */
		emit(Ijmp);
		loc1=here();
		emit(0);
	}
	if(n->o.i)
		patch(loc0, (here()-loc0-1)*LWS);
	loc2=here();
	gen(n->l, 0);		/* body */
	gen(n->r->o.n, 0);	/* right expr */
	if(n->r->r){
		patch(loc1, (here()-loc1-1)*LWS);
		gen(n->r->r, 1);
		emit(Ijmptrue);
	}else
		emit(Ijmp);
	emitconst((loc2-here()-1)*LWS);
}

condgen(Node *l, Node *r, Inst i1, Inst i2, long t1, long t2, int retain)
{
	int loc1, loc2, loc3;
	gen(l, 1);
	emit(i1);
	loc1=here();
	emit(0);
	loc2=here();
	if(retain)
		constgen(t1);
	emit(Ijmp);
	loc3=here();
	emit(0);
	patch(loc1, (long)(here()-loc1-1)*WS);
	gen(r, 1);
	emit(i2);
	emitconst((long)(loc2-here()-1)*WS);
	if(retain)
		constgen(t2);
	patch(loc3, (long)(here()-loc3-1)*WS);
}

callgen(Node *n, int callinst)
{
	Node *pt;
	pt=etypeof(n->l);
	/*
	 * Space for result
	 */
	constgen(0L);
	/*
	 * Args
	 */
	gen(n->r, 1);
	/*
	 * Call
	 */
	emit(Ipushconst);
	if(n->l->t==NID)
		emitconst((long)n->l->o.s->name);
	else{
		char buf[128];
		char *p;
		sprint(buf, "prog(){call on line %d}", n->line);
		p=emalloc((unsigned long)strlen(buf)+1);
		strcpy(p, buf);
		emitconst((long)p);
	}
	gen(n->l, 1);
	switch(callinst){
	case Icall:
		emit(Icall);
		return;
	case Ibegin:
		constgen(LWS*(1+1+length(pt->l)));	/* result+procname+args */
		emit(Ibegin);
		return;
	case Ibecome:
		constgen(LWS*(1+1+length(pt->l)));	/* result+procname+args */
		scopedecrefgen();
		fdecrefgen(formals, -3L*WS);
		emit(Ibecome);
		if(formals)
			emitconst(length(formals)*LWS);
		else
			emitconst(0L);
		return;
	}
	panic("callgen");
}

selgen(Node *n)
{
	int tbl, i;
	long l;
	int ends[200];
	selchangen(n);
	l=length(n);
	constgen(l);
	emit(Icommset);
	emit(Icommcln);
	if(l>(sizeof ends/sizeof ends[0]))
		panic("selgen table too small");
	tbl=here();
	emitspace(l);
	i=0;
	seltblgen(n, tbl, ends, &i);
	for(i=0; i<l; i++)
		patch(ends[i], (long)(here()-ends[i]-1)*WS);
}

selchangen(Node *n)
{
	long flags;
	if(n->t==NList){
		selchangen(n->l);
		selchangen(n->r);
		return;
	}
	if(n->t!=NCase)
		panic("selchangen");
	n=n->l->l;
	if(n->o.t=='=')
		n=n->r;		/* n is now RCV or SND */
	flags=0;
	if(n->o.t==SND)
		flags|=Cissnd;
	n=n->l;			/* n is now channel */
	if(n->t==NArraycom){
		flags|=Cisary;
		n=n->l;
	}else if(etypeoft(n)->o.t==TArray)
		flags|=Cisary;
	gen(n, 1);
	constgen(flags);
}

seltblgen(Node *n, int tbl, int *ends, int *ip)
{
	Node *c, *s, *l, *t;
	if(n->t==NList){
		/* chans are eval'ed from the top, so table is backwards */
		seltblgen(n->r, tbl, ends, ip);
		seltblgen(n->l, tbl, ends, ip);
		return;
	}
	if(n->t!=NCase)
		panic("seltblgen");
	if(n->l->t==NList)
		error("sorry, empty cases not implemented");
	patch(tbl+*ip, (long)(here()-tbl)*WS);
	c=n->l->l;	/* communication */
	s=n->r;		/* statement */
	l=0;
	if(c->o.t=='='){
		l=c->l;	/* lvalue */
		c=c->r;
	}
	if(c->o.t==SND){
		gen(c->r, 1);
		if(isptrtype(etypeoft(c->l)->r))
			emit(Isndptr);
		else
			emit(Isnd);
	}
	c=c->l;	/* channel expression */
	/*
	 * The value is still on the stack; save it or toss it
	 */
	if(l)
		lgen(l);
	else if(c->t==NArraycom){
		t=etypeoft(c->l)->r;
		if(t->o.t==TID)
			t=typeoftid(t);
		popgen(t->r);
	}else
		popgen(etypeoft(c)->r);
	if(c->t==NArraycom){	/* save array index */
		if(c->r)
			lgen(c->r);
		else
			emit(Ipop);
	}
	gen(s, 0);
	emit(Ijmp);
	ends[*ip]=here();
	(*ip)++;
	emitconst(0L);
}

switchgen(Node *s, Node *e)
{
	int isptr, out;
	isptr=isptrtype(etypeof(e));
	gen(e, 1);
	emit(Ijmp);
	emitconst(2*LWS);
	emit(Ijmp);	/* each case jumps to here to get out */
	out=here();
	emitconst(0L);
	switchgen1(s, isptr, out-1);
	/* pop leftover value if no case matched */
	if(isptr)
		emit(Ipopptr);
	else
		emit(Ipop);
	patch(out, (here()-out-1)*LWS);
}

switchgen1(Node *s, int isptr, int out)
{
	Node *e;
	int loc;
	if(s->t==NList){
		switchgen1(s->l, isptr, out);
		switchgen1(s->r, isptr, out);
		return;
	}
	if(s->t!=NCase)
		panic("switchgen1");
	if(s->r==0)
		error("sorry; can't fold cases together yet");
	if(s->l->t==NDefault)
		loc=-1;
	else{
		e=s->l->l;
		if(isptr){	/* string */
			emit(Idupptr);
			gen(e, 1);
			emit(Istrcmp);
			constgen(0L);
		}else{
			emit(Idup);
			gen(e, 1);
		}
		emit(Ieq);
		emit(Ijmpfalse);
		loc=here();
		emitconst(0L);
	}
	if(isptr)
		emit(Ipopptr);
	else
		emit(Ipop);
	gen(s->r, 0);
	emit(Ijmp);
	emitconst((out-here()-1)*LWS);
	if(loc!=-1)
		patch(loc, (here()-loc-1)*LWS);
}

popgen(Node *t)
{
	if(isptrtype(t))
		emit(Ipopptr);
	else if(isinttype(t) || t->o.t==TUnit)
		emit(Ipop);
	else
		panic("popgen %t\n", t->o.t);
}

genfreeauto(Symbol *s)
{
	if(!s->val->isauto)
		panic("genfreeauto");
	if(isptrtype(s->val->type)){
		emit(Idecrefauto);
		emitconst(s->val->store.off);
	}
}

printgen(Node *n)
{
	Node *t;
	if(n==0)
		return;
	if(n->t==NExprlist){
		printgen(n->l);
		printgen(n->r);
		return;
	}
	t=etypeoft(n);
	switch(t->o.t){
	case TArray:
	case TChan:
	case TProg:
	case TStruct:
		emit(Iprintary);
		break;
	case TChar:
		emit(Iprintchar);
		break;
	case TInt:
		emit(Iprintint);
		break;
	case TUnit:
		emit(Iprintunit);
		break;
	default:
		panic("printgen: bad type %t", t->o.t);
	}
}

proggen(Node *t, Node *n)
{
	int or;
	Node *of;
	Errjmp s;
	Store *p;
	long len, loc;
	long nauto, oao;
	extern int (*prog[])();
	oao=autooffset;
	or=returnloc;
	of=formals;
	autooffset=0;
	returnloc=0;
	formals=t->l;
	errsave(s);
	if(errmark()){
		returnloc=or;
		formals=of;
		autooffset=oao;
		errrest(s);
		errjmp();
	}
	loc=here();
	pushscope();
	dclformals(t->l);
	autooffset=0;
	emit(Ipushfp);
	nauto=here();
	emitconst(0L);
	gen(n, 0);
	trlrgen();
	patch((int)nauto, autooffset);
	popscope();
	errrest(s);
	autooffset=oao;
	returnloc=or;
	formals=of;
	len=here()-loc+1;
	p=(Store *)emalloc(SHSZ+len*LWS);
	memcpy((char *)(p->data), (char *)(prog+loc), len*LWS);
	p->ref=1;
	p->len=len;
	p->type=Sprog;
	setprog(loc);
	emit(Ipushdata);
	emitconst((long)p);
}

trlrgen()
{
	if(returnloc){
		emit(Ijmp);
		emitconst((long)(returnloc-here()-1)*WS);
		return;
	}
	returnloc=here();
	fdecrefgen(formals, -3L*WS);
	emit(Iret);
	if(formals)
		emitconst(length(formals)*LWS);
	else
		emitconst(0L);
}

fdecrefgen(Node *types, long offset)
{
	if(types==0)
		return 0;
	if(types->t==NList){
		offset=fdecrefgen(types->l, offset);
		return fdecrefgen(types->r, offset);
	}
	if(types->t!=NFormal)
		panic("fdecrefgen");
	types=types->r;
	if(isptrtype(types)){
		emit(Idecrefauto);
		emitconst(offset);
	}
	return offset-WS;
}

dupgen(Node *t, int n)
{
	while(n--)
		emit(isptrtype(t)? Idupptr : Idup);
}

mkgen(Node *t, Node *v)
{
	switch(t->o.t){
	case TChar:
	case TInt:
	case TUnit:
		if(v)
			gen(v, 1);
		else
			constgen(0L);
		return;
	case TID:
		mkgen(typeoftid(t), v);
		return;
	case TChan:
		if(v)
			gen(v, 1);
		else{
			constgen((long)(sizeof(Chan)-sizeof(Store)));
			mallocgen(t);
		}
		return;
	case TArray:
		if(v==0){
			gen(t->l, 1);
			mallocgen(t);
			return;
		}
		gen(v, 1);
		if(v->t!=NExprlist && eqtype(t, etypeof(v)))
			return;
		if(v->t==NString)
			constgen((long)strlen(v->o.c));
		else
			constgen((long)length(v));
		emit(Idup);
		if(t->l)
			gen(t->l, 1);
		else
			constgen(0L);
		emit(Imax);
		mallocgen(t);
		if(t->r->o.t==TChar){
			if(v->t==NString)
				emit(Imemcpychar);
			else
				emit(Imemcpycharint);
		}else
			emit(Imemcpy);
		return;
	case TProg:
		if(v==0){
			v=new(NProg, dupnode(t), (Node *)0, (Node *)0);
			gen(v, 1);
			freenode(v);
			return;
		}
		gen(v, 1);
		return;
	case TStruct:
		if(v==0){
			mallocgen(t);
			return;
		}
		gen(v, 1);
		if(v->t!=NExprlist && eqtype(t, etypeof(v)))
			return;
		constgen((long)length(v));
		mallocgen(t);
		emit(Imemcpystruct);
		return;		
	default:
		panic("mkgen: bad type %t", t->o.t);
	}
}

mallocgen(Node *t)
{
	switch(t->o.t){
	case TArray:
		t=t->r;
		if(t->o.t==TID)
			t=typeoftid(t);
		if(isptrtype(t)){
			constgen((long)Saryptr);
			emit(Imalloc);
		}else if(t->o.t==TInt || t->o.t==TUnit){
			constgen((long)Saryint);
			emit(Imalloc);
		}else if(t->o.t==TChar)
			emit(Imallocarychar);
		else
			panic("mallocgen array of %t", t->o.t);
		return;
	case TStruct:{
		int pos=0;
		long bits=0;
		t=t->l;
		elembitsgen(t, &pos, &bits);
		if(pos)
			constgen(bits);
		constgen((long)length(t));
		emit(Imallocstruct);
		return;
	}
	case TChan:
		constgen((long)Schan);
		emit(Imalloc);
		return;
	}
	panic("mallocgen of %t", t->o.t);
}

elembitsgen(Node *t, int *pos, long *bits)
{
	int i;
	if(t->t==NList){
		elembitsgen(t->l, pos, bits);
		elembitsgen(t->r, pos, bits);
		return;
	}
	if(t->t!=NElem)
		panic("elembitsgen %n", t->t);
	for(i=length(t); --i>=0; ){
		if(*pos==BPW){
			constgen(*bits);
			*pos=0;
			*bits=0;
		}
		if(isptrtype(t->r))
			*bits|=1L<<*pos;
		(*pos)++;
	}
}

constgen(long l)
{
	if(l<-2 || l>10){
		emit(Ipushconst);
		emitconst(l);
		return;
	};
	switch((int)l){
	case -2:
		emit(Ipush_2);
		break;
	case -1:
		emit(Ipush_1);
		break;
	case 0:
		emit(Ipush0);
		break;
	case 1:
		emit(Ipush1);
		break;
	case 2:
		emit(Ipush2);
		break;
	case 3:
		emit(Ipush3);
		break;
	case 4:
		emit(Ipush4);
		break;
	case 5:
		emit(Ipush5);
		break;
	case 6:
		emit(Ipush6);
		break;
	case 7:
		emit(Ipush7);
		break;
	case 8:
		emit(Ipush8);
		break;
	case 9:
		emit(Ipush9);
		break;
	case 10:
		emit(Ipush10);
		break;
	default:
		panic("constgen");
	}
}

printable(Node *n)
{
	if(n==0)
		return 0;
	switch(n->t){
	case NExpr:
		return n->o.t!='=';
	case NArrayref:
	case NCall:
	case NID:
	case NMk:
	case NNum:
	case NProg:
	case NString:
	case NStructref:
	case NUnit:
	case NVal:
		return 1;
	}
	return 0;
}
#include "alloc.h"
#include "node.h"
#include "symbol.h"
#include "ydefs.h"
#include "word.h"
#include "store.h"
#include <libc.h>

Node		*doconst();
extern int	Cflag;

Node *
constants(Node *n)
{
	if(n==0)
		return 0;
	if(Cflag)
		return n;
	switch(n->t){
	case NArrayref:
		if(isconst(n))
			return doconst(n);
		break;
	case NArraycom:
		break;
	case NBecome:
		break;
	case NBegin:
		break;
	case NCall:
		break;
	case NCase:
		break;
	case NDecl:
		n->r=constants(n->r);
		n->o.n=constants(n->o.n);
		declare(n, 0, 0, 0);
		return n;
	case NDeclsc:
		break;
	case NDefault:
		return n;
	case NElem:
		n->r=constants(n->r);
		return n;
	case NExpr:
		switch(n->o.i){
		case GE:
		case LE:
		case NE:
		case EQ:
		case '>':
		case '<':
		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
		case '&':
		case '|':
		case '^':
		case ANDAND:
		case OROR:
		case LSH:
		case RSH:
			if(isconst(n->l) && isconst(n->r))
				return doconst(n);
			break;
		case DEF:
		case REF:
		case LEN:
		case UMINUS:
		case '~':
		case '!':
			if(isconst(n->l))
				return doconst(n);
			break;
		case PRINT:
		case RCV:
		case SND:
		case INC:
		case DEC:
			break;
		case '=':
			break;
		default:
			fprint(2, "can't const expression %e\n", n->o.i);
			return n;
		}
		break;
	case NExprlist:
		break;
	case NFormal:
		n->r=constants(n->r);
		return n;
	case NLabel:
		break;
	case NID:
		if(isconst(n))
			return doconst(n);
		break;
	case NIf:
		n->l=constants(n->l);
		n->r=constants(n->r);
		n->o.n=constants(n->o.n);
		if(isconst(n->o.n)){
			Node *m;
			gen(n->o.n, 1);
			execute();
			if(topofstack()){
				m=n->l;
				n->l=0;
			}else{
				m=n->r;
				n->r=0;
			}
			freenode(n);
			return m;
		}
		return n;
	case NList:
		break;
	case NLoop:
		break;
	case NLoopexpr:
		n->o.n=constants(n->o.n);
		break;
	case NMk:
		break;
	case NNum:
		return n;
	case NProg:
		pushscope();
		dclformals(n->l->l);
		n->r=constants(n->r);
		popscope();
		return n;
	case NResult:
		break;
	case NScope:
		pushscope();
		n->l=constants(n->l);
		popscope();
		return n;
	case NSelect:
		break;
	case NSmash:
		return n;
	case NString:
		return n;
	case NSwitch:
		break;
	case NStructref:
		if(isconst(n))
			return (n);
		break;
	case NType:
		break;
	case NUnit:
		break;
	case NVal:
		if(isconst(n->l))
			return doconst(n);
		break;
	default:
		fprint(2, "can't const node %n\n", n->t);
		return n;
	}
	n->l=constants(n->l);
	n->r=constants(n->r);
	return n;
}

isconst(Node *n)
{
	if(n==0)
		return 1;
	switch(n->t){
	case NArrayref:
		return isconst(n->l) && isconst(n->r);
	case NCall:
		return 0;
	case NExpr:
		switch(n->o.i){
		case GE:
		case LE:
		case NE:
		case EQ:
		case '>':
		case '<':
		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
		case '&':
		case '|':
		case '^':
		case ANDAND:
		case OROR:
		case LSH:
		case RSH:
			return isconst(n->l) && isconst(n->r);
		case DEF:
		case LEN:
		case UMINUS:
		case '~':
		case '!':
			return isconst(n->l);
		case REF:
		case '=':
		case RCV:
		case SND:
		case INC:
		case DEC:
			return 0;
		}
		fprint(2, "can't isconst expression %e", n->o.i);
		return 0;
	case NID:
		return n->o.s->val->scope==0 && (n->o.s->val->stclass&SCconst);
	case NIf:
		return isconst(n->o.n) && isconst(n->l) && isconst(n->r);
	case NList:
		return 0;
	case NLoop:
		return 0;
	case NNum:
		return 1;
	case NResult:
		return isconst(n->l);
	case NScope:
		return isconst(n->l);
	case NString:
		return 1;
	case NStructref:
		return isconst(n->l);
	case NVal:
		return isconst(n->l);
	case NUnit:
		return 1;
	}
	fprint(2, "can't isconst node %n\n", n->t);
	return 0;
}

Node *
doconst(Node *n)
{
	Node *t;
	if(n->t==NNum || n->t==NString || n->t==NUnit)
		return n;	/* already const */
	t=etypeoft(n);
	switch(t->o.t){
	case TChar:
	case TInt:
		gen(n, 1);
		freenode(n);
		execute();
		return new(NNum, (Node *)0, (Node *)0, (Node *)topofstack());
	case TUnit:
		return new(NUnit, (Node *)0, (Node *)0, (Node *)0);
	case TArray:
		if(t->r->o.t==TChar){
			Store *s;
			char *c;
			gen(n, 1);
			freenode(n);
			execute();
			s=(Store *)topofstack();
			c=emalloc(s->len+1);
			strncpy(c, (char *)s->data, (int)s->len);
			return newc(NString, (Node *)0, (Node *)0, c);
		}
		return n;
	}
	return n;
}
#include "alloc.h"
#include "word.h"
#include "store.h"
#include "comm.h"
#include <libc.h>

extern	int	pflag;

/*
 * Jumps
 */

ijmp(Proc *proc)
{
	SWord l;
	l=(SWord)*++proc->pc;
	proc->pc+=l/WS;
	return 1;
}

ijmpfalse(Proc *proc)
{
	SWord l;
	l=(SWord)*++proc->pc;
	if(*--proc->sp==0)
		proc->pc+=l/WS;
	return 1;
}

ijmptrue(Proc *proc)
{
	SWord l;
	l=(SWord)*++proc->pc;
	if(*--proc->sp!=0)
		proc->pc+=l/WS;
	return 1;
}

ivalnoresult(Proc *proc)
{
	rerror("val produces no result");
	return 0;
}

/*
 * Progs
 *
 *   Layout of a stack frame
 *
 *	sp:
 *		automatics
 *	fp:	old fp
 *		old pc
 *		symbol
 *		arg1
 *		arg2
 *		...
 *		result
 */

iret(Proc *proc)
{
	SWord nargs;
	nargs=(SWord)(proc->pc[1]);
	proc->sp=(SWord *)proc->fp+1;
	proc->fp=(SWord *)*--proc->sp;
	proc->pc=(int (**)())*--proc->sp;
	proc->sp-=(sizeof(char *)+nargs)/WS;
	if(proc->pc==0){
		if(pflag)
			fprint(2, "%d halts\n", proc->procnum);
		halt(proc);
		return 0;
	}
	return 1;
}

ibecome(Proc *proc)
{
	int nargs;
	int (**newpc)();
	SWord oldfp, oldpc, *oldresultaddr, *newresultaddr;
	Store *s;
	nargs=*--proc->sp/LWS;
	nargs+=2;	/* includes result and sym; add pc, fp */
	s=(Store *)*--proc->sp;
	if(--(s->ref)==0)
		rpanic("ibecome ref==0");
	newpc=((int (**)())s->data);
	oldfp=proc->fp[0];
	oldpc=proc->fp[-1];
	*proc->sp++=oldpc;
	*proc->sp++=oldfp;
	oldresultaddr=proc->fp-3-(long)(*++proc->pc)/LWS;
	newresultaddr=proc->sp-nargs;
	memcpy((char *)oldresultaddr, (char *)newresultaddr, LWS*nargs);
	/* args in place.  do the call by hand, jmp to pushfp */
	proc->sp=oldresultaddr+(nargs-2);
	*proc->sp++=oldpc;
	proc->fp=(SWord *)oldfp;
	proc->pc=newpc-1;
	return 1;
}

ipushfp(Proc *proc)
{
	int nauto;
	*proc->sp=(SWord)proc->fp;
	proc->fp=proc->sp++;
	nauto=((SWord)*++proc->pc)/WS;
	while(nauto--)
		*proc->sp++=0;
	if(proc->sp>=&proc->stack[NSTACK])
		rerror("stack overflow");
	return 1;
}

icall(Proc *proc)
{
	int (**newpc)();
	Store *s;
	s=(Store *)*--proc->sp;
	if(--(s->ref)==0)
		rpanic("icall ref==0");
	newpc=((int (**)())s->data);
	*proc->sp++=(SWord)proc->pc;
	proc->pc=newpc-1;
	return 1;
}
#include "node.h"
#include "symbol.h"
#include "alloc.h"
#include "ydefs.h"
#include "word.h"
#include "store.h"
#include <libc.h>

extern int	nscope;

declare(Node *n, int stclass, int dotypchk, int docomp)
{
	extern int iflag;
	if(n==0)
		return;
	if(n->t==NList){
		declare(n->l, stclass, dotypchk, docomp);
		declare(n->r, stclass, dotypchk, docomp);
		return;
	}
	if(n->t==NDeclsc){
		declare(n->l, n->o.i, dotypchk, docomp);
		return;
	}
	if(dotypchk)
		type(n->o.n, 0);
	if(n->r==0){
		if(n->o.n==0)
			panic("declare: no type");
		if(n->o.n->t==NMk && n->o.n->l==0)
			lerror(n, "can't derive type in declaration");
		n->r=dupnode(etypeof(n->o.n));
	}
	if(dotypchk){
		type(n->r, 0);
		if(n->o.n){
			/*
			 * Make it a mk
			 */
			if(n->o.n->t!=NMk)
				n->o.n=new(NMk, (Node *)0, n->o.n, (Node *)0);
			/*
			 * Default type for mk
			 */
			if(n->o.n->l==0)
				n->o.n->l=dupnode(n->r);
			else if(!compattype(n->r, n->o.n->l))
				lerror(n, "type clash in declaration (%t %t)\n",
					n->r->o.t, etypeof(n->o.n)->o.t);
			mkcheck(n->o.n->l, n->o.n->r);
		}
	}
	if(docomp && n->o.n){
		if(dotypchk)	/* top level declaration */
			n->o.n=constants(n->o.n);
		gen(n->o.n, 1);
		dupgen(n->r, length(n->l)-1);
	}else
		docomp=0;
	dcl(n->l, n->r, stclass, n->o.n, docomp);
	if(n->o.n && docomp && nscope==0){
		if(iflag)
			idump();
		execute();
	}
}

dcl(id, typ, stclass, val, docomp)
	Node *id, *typ, *val;
{
	if(id->t==NList){
		dcl(id->l, typ, stclass, val, docomp);
		dcl(id->r, typ, stclass, val, docomp);
		return;
	}
	if(typ->o.t==TID && typ->l->o.s->val->type->o.t!=TType)
		error("%m not a type", typ->l);
	if(id->t!=NID)
		panic("dcl not ID");
	pushval(id->o.s, dupnode(typ));
	if(stclass&SCbltin)
		id->o.s->val->store.l=bltinval(id->o.s->name, typ);
	if(docomp)
		lgen(id);
	id->o.s->val->stclass=stclass;
}

/*
 * To compile this
 * 	rec {
 * 		x : chan of T = f(x,y);
 * 		y : chan of T = g(x,y);
 * 	};
 * convert it to this
 * 	x : chan of T = mk();
 * 	y : chan of T = mk();
 * 	x1 : chan of T = f(x,y);
 * 	y1 : chan of T = g(x,y);
 * 	x <- x1;
 * 	y <- y1;
 *	toss x1, y1;
 * where the operator x <- x1 means copy the representation of x1 into x.
 *
 *	rec type T: struct of { t:T; };
 *
 * is handled similarly.
 */

Node *
op1(Node *n)
{
	Node *m;
	if(n->t==NDeclsc){
		m=op1(n->l);
		return newi(NDeclsc, m, (Node *)0, n->o.i);
	}
	if(n->r==0){
		if(n->o.n && (n->o.n->t==NProg || (n->o.n->t==NMk && n->o.n->l)))
			n->r=dupnode(n->o.n->l);
		else			
			lerror(n, "can't deduce type for rec decl");
	}else if(n->r->o.t==TType){
		m=newi(NType, (Node *)0, (Node *)0, n->r->l->o.t);
		m=new(NDecl, dupnode(n->l), m, (Node *)0);
		return m;
	}
	m=new(NMk, dupnode(n->r), (Node *)0, (Node *)0);
	m=new(NDecl, dupnode(n->l), dupnode(n->r), m);
	return m;
}

Node *
op2(Node *n)
{
	Node *m;
	char s[Namesize+2];
	if(n->t==NDeclsc){
		m=op2(n->l);
		return newi(NDeclsc, m, (Node *)0, n->o.i);
	}
	if(n->l->t==NList)
		error("no identifier lists in rec's, please");
	strcpy(s+1, n->l->o.s->name);
	s[0]='*';
	m=new(NDecl, idnode(lookup(s, ID)), dupnode(n->r), dupnode(n->o.n));
	return m;
}

Node *
op3(Node *n)
{
	Node *m;
	char s[Namesize+2];
	if(n->t==NDeclsc)
		return op3(n->l);
	if(n->l->t==NList)
		error("no lists in rec's, please");
	strcpy(s+1, n->l->o.s->name);
	s[0]='*';
	m=new(NSmash, idnode(lookup(s+1, ID)), idnode(lookup(s, ID)), (Node *)0);
	return m;
}

Node *
rewr(Node *n, Node *(*f)())
{
	if(n->t==NList)
		return new(NList, rewr(n->l, f), rewr(n->r, f), (Node *)0);
	return (*f)(n);
}

recrewrite(Node *n)
{
	Node *n1, *n2, *n3;
	n1=rewr(n->l, op1);
	n2=rewr(n->l, op2);
	n3=rewr(n->l, op3);
	freenode(n->l);
	n->t=NList;
	n->r=n3;
	n->l=new(NList, n1, n2, (Node *)0);
	ndump(n);
}

/*
 *
 * To compile this
 *
 *	prog(a:int){
 *		begin prog(b:int){ f(a, b); }(b);
 *	}
 *
 * convert it to this
 *
 *	prog(a:int){
 *		begin prog(b:int, a:int){ f(a, b); }(b, a);
 *	}
 *
 */

Node 	*begf;
Node	*bega;
int	fscope;
int	progerr;

proglocals(Node *n)
{
	progerr=1;
	pushscope();
	fscope=nscope;
	begf=n->l->l;
	bega=0;
	dclformals(begf);
	progid(n->r);
	popscope();
}

begrewrite(Node *n)
{
	progerr=0;
	pushscope();
	fscope=nscope;
	begf=n->l->l->l;
	bega=n->r;
	dclformals(begf);
	progid(n->l->r);
	popscope();
	n->l->l->l=begf;
	n->r=bega;
}

addformal(Node *n)
{
	Node *nf;
	if(!alreadyformal(n, begf)){
		nf=new(NFormal, dupnode(n), dupnode(n->o.s->val->type), (Node *)0);
		if(begf)
			begf=new(NList, begf, nf, (Node *)0);
		else
			begf=nf;
		nf=dupnode(n);
		if(bega)
			bega=new(NExprlist, bega, nf, (Node *)0);
		else
			bega=nf;
	}		
}

alreadyformal(Node *n, Node *f)
{
	if(f==0)
		return 0;
	if(f->t==NList)
		return alreadyformal(n, f->l) || alreadyformal(n, f->r);
	return strcmp(n->o.s->name, f->l->o.s->name)==0;
}

progid(Node *n)
{
	if(n==0)
		return;
	switch(n->t){
	case NArrayref:
	case NArraycom:
	case NBecome:
	case NBegin:
	case NCall:
	case NCase:
		break;
	case NDecl:
		progid(n->r);
		progid(n->o.n);
		declare(n, 0, 0, 0);
		return;
	case NDeclsc:
	case NDefault:
		break;
	case NElem:
		return;
	case NExpr:
	case NExprlist:
	case NFormal:
		break;
	case NID:
		if(n->o.s->val)
		if(0<n->o.s->val->scope && n->o.s->val->scope<fscope){
			if(progerr)
				lerror(n, "%m not in an accessible scope", n);
			addformal(n);
		}
		return;
	case NLabel:
	case NList:
	case NLoop:
		break;
	case NLoopexpr:
		progid(n->o.n);
		break;
	case NIf:
		progid(n->o.n);
		break;
	case NMk:
		break;
	case NNum:
		return;
	case NProg:
		pushscope();
		dclformals(n->l->l);
		progid(n->r);
		popscope();
		return;
	case NResult:
		break;
	case NScope:
		pushscope();
		progid(n->l);
		popscope();
		return;
	case NSelect:
		break;
	case NSmash:
		return;	/* ?? */
	case NString:
		return;
	case NSwitch:
	case NStructref:
		break;
	case NType:
		break;
	case NUnit:
		return;
	case NVal:
		break;
	default:
		fprint(2, "can't progid node %n\n", n->t);
		return;
	}
	progid(n->l);
	progid(n->r);
}

#include "nodenames.h"
#include "typenames.h"
#include "errjmp.h"
#include "node.h"
#include "symbol.h"
#include "ydefs.h"
#include <libc.h>

lerror(Node *n, char *s, a, b, c, d, e, f)
{
	lfileline(n->line);
	fprint(2, s, a, b, c, d, e, f);
	if(s[strlen(s)-1]!='\n')
		fprint(2, "\n");
	errflush();
	errjmp();
}

error(char *s, a, b, c, d, e, f)
{
	fileline();
	fprint(2, s, a, b, c, d, e, f);
	if(s[strlen(s)-1]!='\n')
		fprint(2, "\n");
	errflush();
	errjmp();
}

rerror(char *s, a, b, c, d, e, f)
{
	fileline();
	fprint(2, s, a, b, c, d, e, f);
	fprint(2, "\n");
	processes(0);
	errflush();
	errjmp();
}

warn(char *s, a, b, c, d, e, f)
{
	fileline();
	fprint(2, "warning: ");
	fprint(2, s, a, b, c, d, e, f);
	fprint(2, "\n");
}

panic(char *s, a, b, c, d, e, f)
{
	fileline();
	fprint(2, "internal error: ");
	fprint(2, s, a, b, c, d, e, f);
	fprint(2, "\n");
	abort();
}

rpanic(char *s, a, b, c, d, e, f)
{
	fileline();
	processes(0);
	fprint(2, "internal error: ");
	fprint(2, s, a, b, c, d, e, f);
	fprint(2, "\n");
	abort();
}

bconv(int *o, int f1, int f2)
{
	extern int printcol;
	while(printcol<*o-8)
		strconv("\t", f1, f2);
	strconv("        "+(8-(*o-printcol)), f1, f2);
	return sizeof(int);
}

nconv(int *o, int f1, int f2)
{
	if(*o<0 || sizeof(Ntypename)/sizeof(Ntypename[0])<=*o)
		strconv("mystery node", f1, f2);
	else
		strconv(Ntypename[*o], f1, f2);
	return sizeof(int);
}

tconv(int *o, int f1, int f2)
{
	if(*o<0 || sizeof(Ttypename)/sizeof(Ttypename[0])<=*o)
		strconv("mystery type", f1, f2);
	else
		strconv(Ttypename[*o], f1, f2);
	return sizeof(int);
}

char	bufx[128][10];
int	bufno=9;

char *
prbuf(){
	if(++bufno==10)
		bufno=0;
	return bufx[bufno];
}

econv(int *o, int f1, int f2)
{
	char *buf=prbuf();
	char *x;
	int t=*o;
	if(t<128 && strchr("+-*/%|&^~?!><=", t))
		sprint(buf, "%c", t);
	else{
		switch(t){
		case GE:
			x=">=";
			break;
		case LE:
			x="<=";
			break;
		case NE:
			x="!=";
			break;
		case EQ:
			x="==";
			break;
		case ANDAND:
			x="&&";
			break;
		case OROR:
			x="||";
			break;
		case REF:
			x="ref";
			break;
		case LEN:
			x="len";
			break;
		case UMINUS:
			x="unary -";
			break;
		case RCV:
			x="rcv";
			break;
		case SND:
			x="send";
			break;
		case LSH:
			x="<<";
			break;
		case RSH:
			x=">>";
			break;
		case DEC:
			x="--";
			break;
		case INC:
			x="++";
			break;
		default:
			x="mystery expression";
			break;
		}
		strcpy(buf, x);
	}
	strconv(buf, f1, f2);
	return sizeof(int);
}

mconv(int *o, int f1, int f2)
{
	char *buf=prbuf();
	Node *n=(Node *)*o;
	switch(n->t){
!
			;;
		pat.dat)for ((i = 900; i <= 1000; i++))
			do	print $i
			done
			;;
		x.dat)	print x
			;;
		y.dat)	print y
			;;
		xyz.dat)print x
			print y
			print z
			;;
		AB.dat|BC.dat)
			print $f
			;;
		esac > $f
	done
}

TEST 01 '-q, -v combinations'
	EXEC	-q . /dev/null
		OUTPUT -
		EXIT '[12]'
	EXEC	-q -v . /dev/null
	EXEC	-q -v .
		INPUT - 'x'
	EXEC	-q .
		EXIT 0

TEST 02 'BRE, ERE, -x, -v, -e sanity'
	DO	DATA big.dat
	EXEC	'^10*$' big.dat
		OUTPUT - $'1\n10\n100\n1000\n10000'
	EXEC	-x '10*' big.dat
	EXEC	-x -e '10*' big.dat
	EXEC	-E '^1(00)*0?$' big.dat
	EXEC	-x '[^[:digit:]]*[[=1=]][[.0.]]\{0,\}' big.dat
	EXEC	-E -x '[^[:digit:]]*[[=1=]][[.0.]]{0,}' big.dat
	EXEC	-E -e $'^1((0)\\2)*$' -e $'^10((0)\\2)*$' big.dat
	EXEC	-E -e $'^1((0)\\2)*$\n^10((0)\\2)*$' big.dat
	EXEC	-E -e $'^1((0)\\2)*$|^10((0)\\4)*$' big.dat
	EXEC	-e $'^1\\(\\(0\\)\\2\\)*$' -e $'^10\\(\\(0\\)\\2\\)*$' big.dat
	EXEC	-e $'^1\\(\\(0\\)\\2\\)*$\n^10\\(\\(0\\)\\2\\)*$' big.dat
	EXEC	-e '^1\(\(0\)\2\)*$' -e '^10\(\(0\)\2\)*$' big.dat
	EXEC	-x -e '1\(\(0\)\2\)*' -e '10\(\(0\)\2\)*' big.dat
	EXEC	-E -v '[2-9]|1.*1|^0' big.dat
	EXEC	-E -x '1(0{0,2}){1,2}' big.dat
	EXEC	-E '1*^10*$' big.dat
	EXEC	-E -x 'a.|b'
		INPUT - $'a\nb\nab\nba'
		OUTPUT - $'b\nab'
	EXEC	'state.var.folder'
		INPUT - $'XXXXXXXXXXXXXXXXXXXXXXX\nfolder state.var.folder'
		OUTPUT - $'folder state.var.folder'
	EXEC	'XXXXXX'
		INPUT - $'..XXXXXX'
		OUTPUT - $'..XXXXXX'
	EXEC	-v /usr/include
		INPUT - $'aaa\n/usr/include/signal.h\nzzz'
		OUTPUT - $'aaa\nzzz'
	EXEC	-E -e $'(abc)' -e $'(def)\\1'
		INPUT - $'abc\ndefdef'
		OUTPUT - $'abc\ndefdef'
	EXEC	-E -e $'(abc)\n(def)\\1'
	EXEC	-E -e $'(abc)|(def)\\2'

TEST 03 'data chars except \0 \n'
	DO	DATA chars.dat
	EXEC	-c . chars.dat
		OUTPUT - 254
	EXEC	-c -e '' chars.dat
	EXEC	-c -e $'\na' chars.dat
	EXEC	-c -e $'a\n' chars.dat
	EXEC	-c -e $'a\n' chars.dat
	EXEC	-c -e 'a' chars.dat
		OUTPUT - 1
	EXEC	-c -x '' chars.dat
		EXIT 1
		OUTPUT - 0
	EXEC	-E -c -x '' chars.dat

TEST 04 'char class on data chars except \0 \n'
	DO	DATA chars.dat
	EXEC	-c '[[:alnum:]]' chars.dat
		OUTPUT - 62
	EXEC	-c -i '[[:alnum:]]' chars.dat
	EXEC	-c '[[:alpha:]]' chars.dat
		OUTPUT - 52
	EXEC	-c '[[:blank:]]' chars.dat
		OUTPUT - 2
	EXEC	-c '[[:cntrl:]]' chars.dat
		OUTPUT - 31
	EXEC	-c '[[:digit:]]' chars.dat
		OUTPUT - 10
	EXEC	-c '[[:graph:]]' chars.dat
		OUTPUT - 94
	EXEC	-c '[[:lower:]]' chars.dat
		OUTPUT - 26
	EXEC	-c '[[:upper:]]' chars.dat
	EXEC	-c '[[:print:]]' chars.dat
		OUTPUT - 95
	EXEC	-c '[[:punct:]]' chars.dat
		OUTPUT - 32
	EXEC	-c '[[:space:]]' chars.dat
		OUTPUT - 5
	EXEC	-c '[[:xdigit:]]' chars.dat
		OUTPUT - 22
	EXEC	-c -i '[[:alpha:]]' chars.dat
		OUTPUT - 52
	EXEC	-c -i '[[:lower:]]' chars.dat
	EXEC	-c -i '[[:upper:]]' chars.dat

TEST 05 '-f, -F, big pattern'
	DO	DATA big.dat pat.dat
	DO	{ cp big.dat INPUT ;}
	EXEC	-c -f pat.dat
		OUTPUT - 1902
	EXEC	-c -E -fpat.dat
	EXEC	-c -F -fpat.dat
	EXEC	-v -c -f pat.dat
		OUTPUT - 8099
	EXEC	-v -c -F -fpat.dat
	EXEC	-v -c -E -fpat.dat
	EXEC	-c -x -fpat.dat
		OUTPUT - 101
	EXEC	-c -x -F -f pat.dat
	EXEC	-c -x -E -f pat.dat
	EXEC	-v -c -x -fpat.dat
		OUTPUT - 9900
	EXEC	-v -c -x -F -f pat.dat
	EXEC	-v -c -x -E -f pat.dat

TEST 06 '-f, -F, big pattern'
	DO	DATA big.dat
	EXEC	-n '\(.\)\(.\)\2\1' big.dat
		IGNORE	OUTPUT
	DO	cp OUTPUT out
	EXEC	-c . out
		OUTPUT - 91
	DO	cp out INPUT
	EXEC	-l . out
		OUTPUT - out
	EXEC	-l .
		OUTPUT - '/dev/stdin'
	EXEC	-l . big.dat big.dat
		OUTPUT - $'big.dat\nbig.dat'
	EXEC	-l . /dev/null big.dat big.dat /dev/null
	EXEC	-q -l . big.dat big.dat
		OUTPUT -
	EXEC	-c . big.dat big.dat
		OUTPUT - $'big.dat:10001\nbig.dat:10001'
	EXEC	-c . /dev/null
		OUTPUT - 0
		EXIT	1
	EXEC	-v -l . big.dat big.dat
		OUTPUT -

TEST 07 '-h, -H'
	DO	DATA x.dat xyz.dat
	EXEC	z x.dat xyz.dat
		OUTPUT - $'xyz.dat:z'
	EXEC	-h z x.dat xyz.dat
		OUTPUT - $'z'
	EXEC	-H z xyz.dat
		OUTPUT - $'xyz.dat:z'

TEST 08 'exit status, -s, -q, -e, -c, -l combinations'
	IGNORE	OUTPUT ERROR
	DO	DATA x.dat AB.dat BC.dat
	DO	for opt in -e -c -l
		do	

		EXEC	$opt . /dev/null
			EXIT 1
		EXEC	-q $opt . /dev/null
			EXIT 1
		EXEC	$opt .
			INPUT -	x
			EXIT 0
		EXEC	-q $opt .
			INPUT -	x
			EXIT 0
		EXEC	$opt . not_a_file
			EXIT '[!01]'
		EXEC	-q $opt . not_a_file
			EXIT 1
		EXEC	-s $opt . not_a_file
			EXIT 2
		EXEC	-q -s $opt . not_a_file
			EXIT 1
		EXEC	-s $opt . x.dat not_a_file
			EXIT 2
		EXEC	-q -s $opt . x.dat not_a_file
			EXIT 0
		EXEC	-q -s $opt . not_a_file x.dat
			EXIT 0

		done
	EXEC	-l A AB.dat BC.dat
		OUTPUT - $'AB.dat'
		EXIT 0
	EXEC	-L C AB.dat BC.dat
	EXEC	-v -l C AB.dat BC.dat
	EXEC	-l C AB.dat BC.dat
		OUTPUT - $'BC.dat'
	EXEC	-L A AB.dat BC.dat
	EXEC	-v -l A AB.dat BC.dat
	EXEC	-l B AB.dat BC.dat
		OUTPUT - $'AB.dat\nBC.dat'
	EXEC	-L Z AB.dat BC.dat
	EXEC	-l Z AB.dat BC.dat
		OUTPUT -
		EXIT 1
	EXEC	-L B AB.dat BC.dat

TEST 09 'file not found'
	DIAGNOSTICS
	DO	DATA x.dat y.dat
	EXEC	y nope.dat
		EXIT 2
	EXEC	-F -f nope.dat x.dat
		EXIT 2
	EXEC	-F -f nope.dat -f x.dat xyz.dat
		EXIT 2
	EXEC	y x.dat nope.dat y.dat
		OUTPUT - $'y.dat:y'
		EXIT 2

TEST 10 'simple gre tests from Andrew Hume'
	EXEC	-q $'a'
		INPUT - $'a'
		OUTPUT -
	EXEC	-q $'a'
		INPUT - $'ba'
	EXEC	-q $'a'
		INPUT - $'bab'
	EXEC	-q $'.'
		INPUT - $'x'
	EXEC	-q $'.'
		INPUT - $'xxx'
	EXEC	-q $'.a'
		INPUT - $'xa'
	EXEC	-q $'.a'
		INPUT - $'xxa'
	EXEC	-q $'.a'
		INPUT - $'xax'
	EXEC	-q $'$'
		INPUT - $'x'
	EXEC	-q $'$'
		INPUT - $''
	EXEC	-q $'.$'
		INPUT - $'x'
	EXEC	-q $'a$'
		INPUT - $'a'
	EXEC	-q $'a$'
		INPUT - $'ba'
	EXEC	-q $'a$'
		INPUT - $'bbba'
	EXEC	-q $'^'
		INPUT - $'x'
	EXEC	-q $'^'
		INPUT - $''
	EXEC	-q $'^'
		INPUT - $'^'
	EXEC	-q $'^a$'
		INPUT - $'a'
	EXEC	-q $'^a.$'
		INPUT - $'ax'
	EXEC	-q $'^a.$'
		INPUT - $'aa'
	EXEC	-q $'^$'
		INPUT - $''
	EXEC	-q $'^.a'
		INPUT - $'xa'
	EXEC	-q $'^.a'
		INPUT - $'xaa'
	EXEC	-q $'^.*a'
		INPUT - $'a'
	EXEC	-q $'^.*a'
		INPUT - $'xa'
	EXEC	-q $'^.*a'
		INPUT - $'xxxxxxa'
	EXEC	-q -E $'^.+a'
		INPUT - $'xa'
	EXEC	-q -E $'^.+a'
		INPUT - $'xxxxxxa'
	EXEC	-q $'a*'
		INPUT - $''
	EXEC	-q $'a*'
		INPUT - $'a'
	EXEC	-q $'a*'
		INPUT - $'aaaa'
	EXEC	-q $'a*'
		INPUT - $'xa'
	EXEC	-q $'a*'
		INPUT - $'xxxx'
	EXEC	-q $'aa*'
		INPUT - $'a'
	EXEC	-q $'aa*'
		INPUT - $'aaa'
	EXEC	-q $'aa*'
		INPUT - $'xa'
	EXEC	-q $'\\$'
		INPUT - $'x$'
	EXEC	-q $'\\$'
		INPUT - $'$'
	EXEC	-q $'\\$'
		INPUT - $'$x'
	EXEC	-q $'\\.'
		INPUT - $'.'
	EXEC	-q -G $'.^$'
		INPUT - $'a^'
	EXEC	-q -G $'^x$'
		INPUT - $'x'
	EXEC	-q -G $'a\\$'
		INPUT - $'a$'
	EXEC	-q -G $'\\(ab\\)$'
		INPUT - $'cab'
	EXEC	-q -G $'\\(ab\\)$'
		INPUT - $'ab'
	EXEC	-q -E $'xr+y'
		INPUT - $'xry'
	EXEC	-q -E $'xr+y'
		INPUT - $'xrry'
	EXEC	-q -E $'xr+y'
		INPUT - $'xrrrrrry'
	EXEC	-q -E $'xr?y'
		INPUT - $'xy'
	EXEC	-q -E $'xr?y'
		INPUT - $'xry'
	EXEC	-q -E $'a(bc|def)g'
		INPUT - $'abcg'
	EXEC	-q -E $'a(bc|def)g'
		INPUT - $'adefg'
	EXEC	-q $'[0-9]'
		INPUT - $'1'
	EXEC	-q $'[0-9]'
		INPUT - $'567'
	EXEC	-q $'[0-9]'
		INPUT - $'x0y'
	EXEC	-q $'[^0-9]'
		INPUT - $'abc'
	EXEC	-q $'[^0-9]'
		INPUT - $'x0y'
	EXEC	-q -E $'x[0-9]+y'
		INPUT - $'x0y'
	EXEC	-q -E $'x[0-9]+y'
		INPUT - $'x23y'
	EXEC	-q -E $'x[0-9]+y'
		INPUT - $'x12345y'
	EXEC	-q -E $'x[0-9]?y'
		INPUT - $'xy'
	EXEC	-q -E $'x[0-9]?y'
		INPUT - $'x1y'
	EXEC	-q -i $'X'
		INPUT - $'x'
	EXEC	-q -x $'read'
		INPUT - $'read'
	EXEC	-q -xF $'read'
		INPUT - $'read'
	EXEC	-q -F $'read'
		INPUT - $'read'
	EXEC	-q -F $'read'
		INPUT - $'xy read'
	EXEC	-q -F $'read'
		INPUT - $'x read y'
	EXEC	-q -F $'read'
		INPUT - $'xread'
	EXEC	-q -F $'read'
		INPUT - $'readx'
	EXEC	-q $'[.]de..'
		INPUT - $'.dexx'
	EXEC	-q $'[.]de..'
		INPUT - $'.deyyy'
	EXEC	-q -G $'^|s'
		INPUT - $'|sec'
	EXEC	-q -G $'..B'
		INPUT - $'CDAB'
	EXEC	-q -G $'$.*tt.*\\$'
		INPUT - $'$tt$'
	EXEC	-q -E $'^([a-z]+)\\1$'
		INPUT - $'vivi'
	EXEC	-q -E $'([a-z]+)\\1'
		INPUT - $'vivi'
	EXEC	-q -E $'([a-z]+)\\1'
		INPUT - $'vivify'
	EXEC	-q -E $'([a-z]+)\\1'
		INPUT - $'revivi'
	EXEC	-q -G $'\\(....\\).*\\1'
		INPUT - $'beriberi'
	EXEC	-q -E $'(....).*\\1'
		INPUT - $'beriberi'
	EXEC	-q $'^$'
		INPUT - $''
	EXEC	-q -G $'^$'
		INPUT - $''
	EXEC	-q $'[ab]\\{2\\}k'
		INPUT - $'abk'
	EXEC	-q $'[ab]\\{2\\}k'
		INPUT - $'xyaak'
	EXEC	-q $'[ab]\\{2\\}k'
		INPUT - $'zabak'
	EXEC	-q $'[ab]\\{2,\\}d'
		INPUT - $'abd'
	EXEC	-q $'[ab]\\{2,\\}d'
		INPUT - $'abababad'
	EXEC	-q $'q[ab]\\{2,4\\}d'
		INPUT - $'qabd'
	EXEC	-q $'q[ab]\\{2,4\\}d'
		INPUT - $'qababd'
	EXEC	-q $'q[ab]\\{2,4\\}d'
		INPUT - $'qaaad'
	EXEC	-q -E $'a[]]b'
		INPUT - $'a]b'
	EXEC	-q -G $'a[]]b'
		INPUT - $'a]b'
	EXEC	-q -E $'a[^]b]c'
		INPUT - $'adc'
	EXEC	-q -G $'a[^]b]c'
		INPUT - $'adc'
	EXEC	-q -i $'angel[^e]'
		INPUT - $'angelo'
	EXEC	-q -i $'angel[^e]'
		INPUT - $'ANGELH'
	EXEC	-q -G $'^[^-].*>'
		INPUT - $'abc>'
	EXEC	-q -i $'^[A-Z]'
		INPUT - $'abc'
	EXEC	-q -i $'^[A-Z]'
		INPUT - $'ABC'
	EXEC	-q -i $'^[^A-Z]'
		INPUT - $'123'
	EXEC	-q -G $'|abc'
		INPUT - $'|abc'
	EXEC	-q -G $'\\(ac*\\)c*d[ac]*\\1'
		INPUT - $'acdacaaa'
	EXEC	-q -E $'(ac*)c*d[ac]*\\1'
		INPUT - $'acdacaaa'
	EXEC	-q -E $'ram|am'
		INPUT - $'am'
	EXEC	-q -E $'[a-za-za-za-za-za-za-za-za-za-z]'
		INPUT - $'for this line'
	EXEC	-q $'[a-za-za-za-za-za-za-za-za-za-z]'
		INPUT - $'for this line'
	EXEC	-q -E $'[a-za-za-za-za-za-za-za-za-z]'
		INPUT - $'but watch out'
	EXEC	-q $'[a-za-za-za-za-za-za-za-za-z]'
		INPUT - $'but watch out'
	EXEC	-q -E $'[ ]*([^ ]+)[ ]*'
		INPUT - $'foo'
	EXEC	-q -E $'[ ]*([^ ]+)[ ]*'
		INPUT - $'foo '
	EXEC	-q -E $'[ ]*([^ ]+)[ ]*([(].*[)])?'
		INPUT - $'foo'
	EXEC	-q -E $'[ ]*([^ ]+)[ ]*([(].*[)])?'
		INPUT - $'foo '
	EXEC	-q -E $'((foo)|(bar))!bas'
		INPUT - $'bar!bas'
	EXEC	-q -E $'((foo)|(bar))!bas'
		INPUT - $'foo!bar!bas'
	EXEC	-q -E $'((foo)|(bar))!bas'
		INPUT - $'bar!bas'
	EXEC	-q -E $'((foo)|(bar))!bas'
		INPUT - $'foo!bar!bas'
	EXEC	-q -E $'((foo)|bar)!bas'
		INPUT - $'bar!bas'
	EXEC	-q -E $'((foo)|bar)!bas'
		INPUT - $'foo!bar!bas'
	EXEC	-q -E $'(foo|(bar))!bas'
		INPUT - $'bar!bas'
	EXEC	-q -E $'(foo|(bar))!bas'
		INPUT - $'foo!bar!bas'
	EXEC	-q -E $'^([^!]+!)?([^!]+)$|^.+!([^!]+!)([^!]+)$'
		INPUT - $'bar!bas'
	EXEC	-q -E $'^[abf][0-9][0-9][0-9]([0-9]|(\\[[0-9]+\\]))'
		INPUT - $'b051[89]'
	EXEC	-q -E $'^[abf][0-9][0-9][0-9]([0-9]|(\\[[0-9]+\\]))'
		INPUT - $'b0123'
	EXEC	-q $'a'
		INPUT - $''
		EXIT 1
	EXEC	-q $'a'
		INPUT - $'x'
	EXEC	-q $'a'
		INPUT - $'xxxxx'
	EXEC	-q $'.'
		INPUT - $''
	EXEC	-q $'.a'
		INPUT - $'a'
	EXEC	-q $'.a'
		INPUT - $'ab'
	EXEC	-q $'.a'
		INPUT - $''
	EXEC	-q $'.$'
		INPUT - $''
	EXEC	-q $'a$'
		INPUT - $'ab'
	EXEC	-q $'a$'
		INPUT - $'x'
	EXEC	-q $'a$'
		INPUT - $''
	EXEC	-q $'^a$'
		INPUT - $'xa'
	EXEC	-q $'^a$'
		INPUT - $'ax'
	EXEC	-q $'^a$'
		INPUT - $'xax'
	EXEC	-q $'^a$'
		INPUT - $''
	EXEC	-q $'^a.$'
		INPUT - $'xa'
	EXEC	-q $'^a.$'
		INPUT - $'aaa'
	EXEC	-q $'^a.$'
		INPUT - $'axy'
	EXEC	-q $'^a.$'
		INPUT - $''
	EXEC	-q $'^$'
		INPUT - $'x'
	EXEC	-q $'^$'
		INPUT - $'^'
	EXEC	-q $'^.a'
		INPUT - $'a'
	EXEC	-q $'^.a'
		INPUT - $''
	EXEC	-q $'^.*a'
		INPUT - $''
	EXEC	-q -E $'^.+a'
		INPUT - $''
	EXEC	-q -E $'^.+a'
		INPUT - $'a'
	EXEC	-q -E $'^.+a'
		INPUT - $'ax'
	EXEC	-q $'aa*'
		INPUT - $'xxxx'
	EXEC	-q $'aa*'
		INPUT - $''
	EXEC	-q $'\\$'
		INPUT - $''
	EXEC	-q $'\\$'
		INPUT - $'x'
	EXEC	-q $'\\.'
		INPUT - $'x'
	EXEC	-q $'\\.'
		INPUT - $''
	EXEC	-q -G $'.^$'
		INPUT - $''
	EXEC	-q -G $'.^$'
		INPUT - $'a^$'
	EXEC	-q -G $'^x$'
		INPUT - $'yx'
	EXEC	-q -G $'^x$'
		INPUT - $'xy'
	EXEC	-q -G $'a\\$'
		INPUT - $'a'
	EXEC	-q -G $'\\(ab\\)$'
		INPUT - $'ab$'
	EXEC	-q -E $'xr+y'
		INPUT - $'ry'
	EXEC	-q -E $'xr+y'
		INPUT - $'xy'
	EXEC	-q -E $'xr?y'
		INPUT - $'xrry'
	EXEC	-q -E $'a(bc|def)g'
		INPUT - $'abc'
	EXEC	-q -E $'a(bc|def)g'
		INPUT - $'abg'
	EXEC	-q -E $'a(bc|def)g'
		INPUT - $'adef'
	EXEC	-q -E $'a(bc|def)g'
		INPUT - $'adeg'
	EXEC	-q $'[0-9]'
		INPUT - $'abc'
	EXEC	-q $'[0-9]'
		INPUT - $''
	EXEC	-q $'[^0-9]'
		INPUT - $'1'
	EXEC	-q $'[^0-9]'
		INPUT - $'567'
	EXEC	-q $'[^0-9]'
		INPUT - $''
	EXEC	-q -E $'x[0-9]+y'
		INPUT - $'0y'
	EXEC	-q -E $'x[0-9]+y'
		INPUT - $'xy'
	EXEC	-q -E $'x[0-9]?y'
		INPUT - $'x23y'
	EXEC	-q -x $'read'
		INPUT - $'xy read'
	EXEC	-q -x $'read'
		INPUT - $'x read y'
	EXEC	-q -x $'read'
		INPUT - $'xread'
	EXEC	-q -x $'read'
		INPUT - $'readx'
	EXEC	-q -xF $'read'
		INPUT - $'xy read'
	EXEC	-q -xF $'read'
		INPUT - $'x read y'
	EXEC	-q -xF $'read'
		INPUT - $'xread'
	EXEC	-q -xF $'read'
		INPUT - $'readx'
	EXEC	-q $'[.]de..'
		INPUT - $'.de'
	EXEC	-q $'[.]de..'
		INPUT - $'.dex'
	EXEC	-q -G $'^|s'
		INPUT - $'sec'
	EXEC	-q -G $'..B'
		INPUT - $'ABCD'
	EXEC	-q -E $'^([a-z]+)\\1$'
		INPUT - $'vivify'
	EXEC	-q -E $'([a-z]+)\\1'
		INPUT - $'vovify'
	EXEC	-q -E $'([a-z]+)\\1'
		INPUT - $'viv'
	EXEC	-q $'[ab]\\{2\\}k'
		INPUT - $'zad'
	EXEC	-q $'[ab]\\{2\\}k'
		INPUT - $'bq'
	EXEC	-q $'[ab]\\{2\\}k'
		INPUT - $'abq'
	EXEC	-q $'[ab]\\{2,\\}d'
		INPUT - $'ad'
	EXEC	-q $'[ab]\\{2,\\}d'
		INPUT - $'ababaq'
	EXEC	-q $'q[ab]\\{2,4\\}d'
		INPUT - $'qad'
	EXEC	-q $'q[ab]\\{2,4\\}d'
		INPUT - $'qababad'
	EXEC	-q -i $'angel[^e]'
		INPUT - $'angel'
	EXEC	-q -i $'angel[^e]'
		INPUT - $'ANGEL'
	EXEC	-q -i $'angel[^e]'
		INPUT - $'angele'
	EXEC	-q -i $'angel[^e]'
		INPUT - $'ANGELE'
	EXEC	-q -G $'^[^-].*>'
		INPUT - $'-a>'
	EXEC	-q -i $'^[^A-Z]'
		INPUT - $'abc'
	EXEC	-q -i $'^[^A-Z]'
		INPUT - $'ABC'
	EXEC	-q -G $'|abc'
		INPUT - $'abc'
	EXEC	-q -x $'.|..'
		INPUT - $'abc'

TEST 11 'complex gre tests from Andrew Hume'
	DO DATA g1.dat g4.dat g5.dat g6.pat g6.dat g8.dat g12.dat
	EXEC -xF $'defg\nabcd'
		INPUT - $'x\nabcd\nabcde\neabcd\ndefg\nxdefg\ndefgx\nabcd defg'
		OUTPUT - $'abcd\ndefg'
	EXEC	abc
		INPUT - 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaxyz
abc'
		OUTPUT - 'abc'
	EXEC	-E 'pm|xdhu'
		INPUT - $'/p1/usr/bin/pmxpc:
\t pm.sl\t2.94
\t xdhu.sl\t1.8
\t/lib/crt1.o.sl 1.1 4.0 01/15/86 12744 AT&T-SF
\t/usr/include/stdio.h.sl 1.1 4.0 01/15/86 4140 AT&T-SF
\t/usr/include/ctype.h.sl 1.1 4.0 01/15/86 45671 AT&T-SF
\t/usr/include/string.h.sl 1.1 4.0 01/15/86 51235 AT&T-SF
\t/usr/include/signal.h.sl 1.1 4.0 01/15/86 34302 AT&T-SF
\t/usr/include/sys/signal.h.sl 1.5 3.2 09/02/87 33640 AT&T-SF
\t/usr/include/sys/types.h.sl 1.3 3.1 06/02/86 48113 AT&T-SF
\t/usr/include/sys/stat.h.sl 1.3 3.0 12/19/85 41824 
\t/usr/include/termio.h.sl 1.1 4.0 01/15/86 29141 AT&T-SF'
		OUTPUT - $'/p1/usr/bin/pmxpc:
\t pm.sl\t2.94
\t xdhu.sl\t1.8'
	EXEC	-v : g4.dat
		OUTPUT - $'   1 ZIPPORI, Israel
   1 ZERIFIN, Israel
   1 ZEPHYRHILLS, Fla.
   1 ZENICA, Yugoslavia
   1 ZAP, N.D.
   1 ZAMBRANO, Honduras
   1 ZACHARY, La.
   1 YUCCA VALLEY, Calif.
   1 YORKVILLE, Ill.
   1 YORK, Maine
   1 YENAN, China
   1 YELOWSTONE NATIONAL PARK, Wyo.
   1 YEADON, Pa.
   1 YATTA, Occupied West Bank
   1 YASSIHOYUK, Turkey
   1 YAPHANK, N.Y.
   1 YAMOUSSOUKRO, Ivory Coast'
	EXEC	'^com ' g5.dat
		SAME OUTPUT g5.dat
	EXEC	-E -f g6.pat g6.dat
		OUTPUT - $'emalloc(unsigned long n)
erealloc(char *p, unsigned long n)
pprint(proc, fmt, a, b, c, d, e)
bltinval(char *name, Node *t)
mk(type, len)
compile(n)\t/* called from parser only */
gen(Node *n, int retain)
lgen(Node *n)
genfreeauto(Symbol *s)
dupgen(Node *t, int n)
printable(Node *n)
constants(Node *n)
declare(Node *n, int stclass, int dotypchk, int docomp)
recrewrite(Node *n)
proglocals(Node *n)
begrewrite(Node *n)
lerror(Node *n, char *s, a, b, c, d, e, f)
error(char *s, a, b, c, d, e, f)
rerror(char *s, a, b, c, d, e, f)
warn(char *s, a, b, c, d, e, f)
panic(char *s, a, b, c, d, e, f)
rpanic(char *s, a, b, c, d, e, f)
bconv(int *o, int f1, int f2)
nconv(int *o, int f1, int f2)
tconv(int *o, int f1, int f2)
econv(int *o, int f1, int f2)
mconv(int *o, int f1, int f2)'
	EXEC	'^[^`]*`[^`]*$'
		INPUT - $'if [ `cat $HISTFILE | lct` -gt "$HISTMAXL" ]

for i in `ls [0-9]*# | egrep \'^[0-9]+##?$\' | sed -e \'s/#*$//\'`

do case "`ps -lx$i" in ?*);; *) rm -f ${i}# ${i}##;; esac

NBRFILES=`ls -f $pubdir/jbk | fgrep -vi -x \'.

..\'|lct`'
		OUTPUT - $'do case "`ps -lx$i" in ?*);; *) rm -f ${i}# ${i}##;; esac
NBRFILES=`ls -f $pubdir/jbk | fgrep -vi -x \'.
..\'|lct`'
	EXEC	-F -x -f g8.dat g8.dat
		SAME OUTPUT g8.dat
	EXEC	-F -f g8.dat
		INPUT - $'aba\ncad\nbad\nacb'
		OUTPUT - $'aba\nbad\nacb'
	EXEC	-v '^\.x' g1.dat
		OUTPUT -
		EXIT 1
	EXEC	-xvF -f g12.dat g12.dat
	EXEC	-xvF -f INPUT
		INPUT - $'at\nhematic'

TEST 12 'alternating BM tests'
	EXEC	-F $':::1:::0:\n:::1:1:0:'
		INPUT - ':::0:::1:::1:::0:'
		SAME OUTPUT INPUT
	EXEC	-F $':::1:::0:\n:::1:1:1:'
	EXEC	-E $':::1:::0:|:::1:1:0:'
	EXEC	-E $':::1:::0:|:::1:1:1:'
