#ifndef __MY_BITS
#define __MY_BITS
const unsigned long long  bitmask[64]  = {
0x8000000000000000LL, 0x4000000000000000LL, 0x2000000000000000LL, 0x1000000000000000LL, 0x0800000000000000LL, 0x0400000000000000LL, 0x0200000000000000LL, 0x0100000000000000LL,
0x80000000000000LL,   0x40000000000000LL,   0x20000000000000LL,   0x10000000000000LL,   0x08000000000000LL,   0x04000000000000LL,   0x02000000000000LL,   0x01000000000000LL,
0x800000000000LL,     0x400000000000LL,     0x200000000000LL,     0x100000000000LL,     0x080000000000LL,     0x040000000000LL,     0x020000000000LL,     0x010000000000LL,
0x8000000000LL,       0x4000000000LL,       0x2000000000LL,       0x1000000000LL,       0x0800000000LL,       0x0400000000LL,       0x0200000000LL,       0x0100000000LL,
0x80000000LL,         0x40000000LL,         0x20000000LL,         0x10000000LL,         0x08000000LL,         0x04000000LL,         0x02000000LL,         0x01000000LL,
0x800000LL,           0x400000LL,           0x200000LL,           0x100000LL,           0x080000LL,           0x040000LL,           0x020000LL,           0x010000LL,
0x8000LL,             0x4000LL,             0x2000LL,             0x1000LL,             0x0800LL,             0x0400LL,             0x0200LL,             0x0100LL,
0x80LL,               0x40LL,               0x20LL,               0x10LL,               0x08,                 0x04,                 0x02,                 0x01
};


#define BIT(a, b) ((a & bitmask[b]) >> (63 - b))

#define BITS8(a) BIT(a,0),BIT(a,1),BIT(a,2),BIT(a,3),BIT(a,4),BIT(a,5),BIT(a,6),BIT(a,7),BIT(a,8),BIT(a,9),BIT(a,10),BIT(a,11),BIT(a,12),BIT(a,13),BIT(a,14),BIT(a,15),BIT(a,16),BIT(a,17),BIT(a,18),BIT(a,19),BIT(a,20),BIT(a,21),BIT(a,22),BIT(a,23),BIT(a,24),BIT(a,25),BIT(a,26),BIT(a,27),BIT(a,28),BIT(a,29),BIT(a,30),BIT(a,31),BIT(a,32),BIT(a,33),BIT(a,34),BIT(a,35),BIT(a,36),BIT(a,37),BIT(a,38),BIT(a,39),BIT(a,40),BIT(a,41),BIT(a,42),BIT(a,43),BIT(a,44),BIT(a,45),BIT(a,46),BIT(a,47),BIT(a,48),BIT(a,49),BIT(a,50),BIT(a,51),BIT(a,52),BIT(a,53),BIT(a,54),BIT(a,55),BIT(a,56),BIT(a,57),BIT(a,58),BIT(a,59),BIT(a,60),BIT(a,61),BIT(a,62),BIT(a,63)
#define BITS8ix(a) 0LL,BIT(a,32),0LL,BIT(a,33),0LL,BIT(a,34),0LL,BIT(a,35),0LL,BIT(a,36),0LL,BIT(a,37),0LL,BIT(a,38),0LL,BIT(a,39),0LL,BIT(a,40),0LL,BIT(a,41),0LL,BIT(a,42),0LL,BIT(a,43),0LL,BIT(a,44),0LL,BIT(a,45),0LL,BIT(a,46),0LL,BIT(a,47),0LL,BIT(a,48),0LL,BIT(a,49),0LL,BIT(a,50),0LL,BIT(a,51),0LL,BIT(a,52),0LL,BIT(a,53),0LL,BIT(a,54),0LL,BIT(a,55),0LL,BIT(a,56),0LL,BIT(a,57),0LL,BIT(a,58),0LL,BIT(a,59),0LL,BIT(a,60),0LL,BIT(a,61),0LL,BIT(a,62),0LL,BIT(a,63)
#define BITS8iy(a) BIT(a,32),0LL,BIT(a,33),0LL,BIT(a,34),0LL,BIT(a,35),0LL,BIT(a,36),0LL,BIT(a,37),0LL,BIT(a,38),0LL,BIT(a,39),0LL,BIT(a,40),0LL,BIT(a,41),0LL,BIT(a,42),0LL,BIT(a,43),0LL,BIT(a,44),0LL,BIT(a,45),0LL,BIT(a,46),0LL,BIT(a,47),0LL,BIT(a,48),0LL,BIT(a,49),0LL,BIT(a,50),0LL,BIT(a,51),0LL,BIT(a,52),0LL,BIT(a,53),0LL,BIT(a,54),0LL,BIT(a,55),0LL,BIT(a,56),0LL,BIT(a,57),0LL,BIT(a,58),0LL,BIT(a,59),0LL,BIT(a,60),0LL,BIT(a,61),0LL,BIT(a,62),0LL,BIT(a,63),0LL
#define BITS4(a) BIT(a,32),BIT(a,33),BIT(a,34),BIT(a,35),BIT(a,36),BIT(a,37),BIT(a,38),BIT(a,39),BIT(a,40),BIT(a,41),BIT(a,42),BIT(a,43),BIT(a,44),BIT(a,45),BIT(a,46),BIT(a,47),BIT(a,48),BIT(a,49),BIT(a,50),BIT(a,51),BIT(a,52),BIT(a,53),BIT(a,54),BIT(a,55),BIT(a,56),BIT(a,57),BIT(a,58),BIT(a,59),BIT(a,60),BIT(a,61),BIT(a,62),BIT(a,63)
/*BIT(a,0),BIT(a,1),BIT(a,2),BIT(a,3),BIT(a,4),BIT(a,5),BIT(a,6),BIT(a,7),BIT(a,8),BIT(a,9),BIT(a,10),BIT(a,11),BIT(a,12),BIT(a,13),BIT(a,14),BIT(a,15),BIT(a,16),BIT(a,17),BIT(a,18),BIT(a,19),BIT(a,20),BIT(a,21),BIT(a,22),BIT(a,23),BIT(a,24),BIT(a,25),BIT(a,26),BIT(a,27),BIT(a,28),BIT(a,29),BIT(a,30),BIT(a,31) */
#define BITS2(a) BIT(a,48),BIT(a,49),BIT(a,50),BIT(a,51),BIT(a,52),BIT(a,53),BIT(a,54),BIT(a,55),BIT(a,56),BIT(a,57),BIT(a,58),BIT(a,59),BIT(a,60),BIT(a,61),BIT(a,62),BIT(a,63)
/* BIT(a,16),BIT(a,17),BIT(a,18),BIT(a,19),BIT(a,20),BIT(a,21),BIT(a,22),BIT(a,23),BIT(a,24),BIT(a,25),BIT(a,26),BIT(a,27),BIT(a,28),BIT(a,29),BIT(a,30),BIT(a,31) */
/* BIT(a,0),BIT(a,1),BIT(a,2),BIT(a,3),BIT(a,4),BIT(a,5),BIT(a,6),BIT(a,7),BIT(a,8),BIT(a,9),BIT(a,10),BIT(a,11),BIT(a,12),BIT(a,13),BIT(a,14),BIT(a,15) */

#define BIT_PRINT8(a)   fprintf(stdout,"%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld\n", BITS8(a));
#define BIT_PRINT8ix(a) fprintf(stdout,"%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld\n", BITS8ix(a));
#define BIT_PRINT8iy(a) fprintf(stdout,"%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld%lld\n", BITS8iy(a));
#define BIT_PRINT4(a)   fprintf(stdout,"%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", BITS4(a));
#define BIT_PRINT2(a)   fprintf(stdout,"%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", BITS2(a));

#endif /* __MY_BITS */
