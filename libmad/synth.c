/*
 * mad - MPEG audio decoder
 * Copyright (C) 2000-2001 Robert Leslie
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: synth.c,v 1.2 2001-05-06 00:10:37 peter Exp $
 */

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# include "global.h"

# include "fixed.h"
# include "frame.h"
# include "synth.h"

/*
 * NAME:	synth->init()
 * DESCRIPTION:	initialize synth struct
 */
void mad_synth_init(struct mad_synth *synth)
{
  synth->phase = 0;
  mad_synth_mute(synth);
}

/*
 * An optional optimization called here the Subband Synthesis Optimization
 * (SSO) improves the performance of subband synthesis at the expense of
 * accuracy.
 *
 * The idea is to simplify 32x32->64-bit multiplication to 32x32->32 such
 * that extra scaling and rounding are not necessary. This often allows the
 * compiler to use faster 32-bit multiply-accumulate instructions instead of
 * explicit 64-bit multiply, shift, and add instructions.
 *
 * SSO works like this: a full 32x32->64-bit multiply of two mad_fixed_t
 * values requires the result to be right-shifted 28 bits to be properly
 * scaled to the same fixed-point format. Right shifts can be applied at any
 * time to either operand or to the result, so the optimization involves
 * careful placement of these shifts to minimize the loss of accuracy.
 *
 * First, a 14-bit shift is applied with rounding at compile-time to the D[]
 * table of coefficients for the subband synthesis window. This only loses 2
 * bits of accuracy because the lower 12 bits are always zero. A second
 * 12-bit shift occurs after the DCT calculation. This loses 12 bits of
 * accuracy. Finally, a third 2-bit shift occurs just before the sample is
 * saved in the PCM buffer. 14 + 12 + 2 == 28 bits.
 */

/* FPM_DEFAULT without OPT_SSO will actually lose accuracy and performance */

# if defined(FPM_DEFAULT) && !defined(OPT_SSO)
#  define OPT_SSO
# endif

/* second SSO shift, with rounding */

# if defined(OPT_SSO)
#  define SHIFT(x)  (((x) + (1L << 11)) >> 12)
# else
#  define SHIFT(x)  (x)
# endif

/* possible DCT speed optimization */

# if defined(OPT_SPEED) && defined(MAD_F_MLX)
#  define OPT_DCTO
#  define MUL(x, y)  \
    ({ mad_fixed64hi_t hi;  \
       mad_fixed64lo_t lo;  \
       MAD_F_MLX(hi, lo, (x), (y));  \
       hi << (32 - MAD_F_SCALEBITS - 3);  \
    })
# else
#  undef OPT_DCTO
#  define MUL(x, y)  mad_f_mul((x), (y))
# endif

  /* costab[i] = cos(PI / (2 * 32) * i) */

#if 0
#define COS(X,Y) costab##X = Y
enum {
#include "costab.h"
};
#else
mad_fixed_t costab[] =
{
#define COS(X,Y) Y,
#include "costab.h"
#undef COS
};
#define costab1 costab[1]
#define costab2 costab[2]
#define costab3 costab[3]
#define costab4 costab[4]
#define costab5 costab[5]
#define costab6 costab[6]
#define costab7 costab[7]
#define costab8 costab[8]
#define costab9 costab[9]
#define costab10 costab[10]

#define costab11 costab[11]
#define costab12 costab[12]
#define costab13 costab[13]
#define costab14 costab[14]
#define costab15 costab[15]
#define costab16 costab[16]
#define costab17 costab[17]
#define costab18 costab[18]
#define costab19 costab[19]
#define costab20 costab[20]
    
#define costab21 costab[21]
#define costab22 costab[22]
#define costab23 costab[23]
#define costab24 costab[24]
#define costab25 costab[25]
#define costab26 costab[26]
#define costab27 costab[27]
#define costab28 costab[28]
#define costab29 costab[29]
#define costab30 costab[30]

#define costab31 costab[31]
#endif


/*
 * NAME:	dct32()
 * DESCRIPTION:	perform fast in[32]->out[32] DCT
 */
static
void dct32(mad_fixed_t const in[32], unsigned int slot,
	   mad_fixed_t lo[16][8], mad_fixed_t hi[16][8])
{
#if 0
  static mad_fixed_t work_area[176];
# define t0 work_area[0]
# define t1 work_area[1]
# define t2 work_area[2]
# define t3 work_area[3]
# define t4 work_area[4]
# define t5 work_area[5]
# define t6 work_area[6]
# define t7 work_area[7]
# define t8 work_area[8]
# define t9 work_area[9]
# define t10 work_area[10]
# define t11 work_area[11]
# define t12 work_area[12]
# define t13 work_area[13]
# define t14 work_area[14]
# define t15 work_area[15]
# define t16 work_area[16]
# define t17 work_area[17]
# define t18 work_area[18]
# define t19 work_area[19]
# define t20 work_area[20]
# define t21 work_area[21]
# define t22 work_area[22]
# define t23 work_area[23]
# define t24 work_area[24]
# define t25 work_area[25]
# define t26 work_area[26]
# define t27 work_area[27]
# define t28 work_area[28]
# define t29 work_area[29]
# define t30 work_area[30]
# define t31 work_area[31]
# define t32 work_area[32]
# define t33 work_area[33]
# define t34 work_area[34]
# define t35 work_area[35]
# define t36 work_area[36]
# define t37 work_area[37]
# define t38 work_area[38]
# define t39 work_area[39]
# define t40 work_area[40]
# define t41 work_area[41]
# define t42 work_area[42]
# define t43 work_area[43]
# define t44 work_area[44]
# define t45 work_area[45]
# define t46 work_area[46]
# define t47 work_area[47]
# define t48 work_area[48]
# define t49 work_area[49]
# define t50 work_area[50]
# define t51 work_area[51]
# define t52 work_area[52]
# define t53 work_area[53]
# define t54 work_area[54]
# define t55 work_area[55]
# define t56 work_area[56]
# define t57 work_area[57]
# define t58 work_area[58]
# define t59 work_area[59]
# define t60 work_area[60]
# define t61 work_area[61]
# define t62 work_area[62]
# define t63 work_area[63]
# define t64 work_area[64]
# define t65 work_area[65]
# define t66 work_area[66]
# define t67 work_area[67]
# define t68 work_area[68]
# define t69 work_area[69]
# define t70 work_area[70]
# define t71 work_area[71]
# define t72 work_area[72]
# define t73 work_area[73]
# define t74 work_area[74]
# define t75 work_area[75]
# define t76 work_area[76]
# define t77 work_area[77]
# define t78 work_area[78]
# define t79 work_area[79]
# define t80 work_area[80]
# define t81 work_area[81]
# define t82 work_area[82]
# define t83 work_area[83]
# define t84 work_area[84]
# define t85 work_area[85]
# define t86 work_area[86]
# define t87 work_area[87]
# define t88 work_area[88]
# define t89 work_area[89]
# define t90 work_area[90]
# define t91 work_area[91]
# define t92 work_area[92]
# define t93 work_area[93]
# define t94 work_area[94]
# define t95 work_area[95]
# define t96 work_area[96]
# define t97 work_area[97]
# define t98 work_area[98]
# define t99 work_area[99]
# define t100 work_area[100]
# define t101 work_area[101]
# define t102 work_area[102]
# define t103 work_area[103]
# define t104 work_area[104]
# define t105 work_area[105]
# define t106 work_area[106]
# define t107 work_area[107]
# define t108 work_area[108]
# define t109 work_area[109]
# define t110 work_area[110]
# define t111 work_area[111]
# define t112 work_area[112]
# define t113 work_area[113]
# define t114 work_area[114]
# define t115 work_area[115]
# define t116 work_area[116]
# define t117 work_area[117]
# define t118 work_area[118]
# define t119 work_area[119]
# define t120 work_area[120]
# define t121 work_area[121]
# define t122 work_area[122]
# define t123 work_area[123]
# define t124 work_area[124]
# define t125 work_area[125]
# define t126 work_area[126]
# define t127 work_area[127]
# define t128 work_area[128]
# define t129 work_area[129]
# define t130 work_area[130]
# define t131 work_area[131]
# define t132 work_area[132]
# define t133 work_area[133]
# define t134 work_area[134]
# define t135 work_area[135]
# define t136 work_area[136]
# define t137 work_area[137]
# define t138 work_area[138]
# define t139 work_area[139]
# define t140 work_area[140]
# define t141 work_area[141]
# define t142 work_area[142]
# define t143 work_area[143]
# define t144 work_area[144]
# define t145 work_area[145]
# define t146 work_area[146]
# define t147 work_area[147]
# define t148 work_area[148]
# define t149 work_area[149]
# define t150 work_area[150]
# define t151 work_area[151]
# define t152 work_area[152]
# define t153 work_area[153]
# define t154 work_area[154]
# define t155 work_area[155]
# define t156 work_area[156]
# define t157 work_area[157]
# define t158 work_area[158]
# define t159 work_area[159]
# define t160 work_area[160]
# define t161 work_area[161]
# define t162 work_area[162]
# define t163 work_area[163]
# define t164 work_area[164]
# define t165 work_area[165]
# define t166 work_area[166]
# define t167 work_area[167]
# define t168 work_area[168]
# define t169 work_area[169]
# define t170 work_area[170]
# define t171 work_area[171]
# define t172 work_area[172]
# define t173 work_area[173]
# define t174 work_area[174]
# define t175 work_area[175]
# define t176 work_area[176]
#else
  mad_fixed_t t0,   t1,   t2,   t3,   t4,   t5,   t6,   t7;
  mad_fixed_t t8,   t9,   t10,  t11,  t12,  t13,  t14,  t15;
  mad_fixed_t t16,  t17,  t18,  t19,  t20,  t21,  t22,  t23;
  mad_fixed_t t24,  t25,  t26,  t27,  t28,  t29,  t30,  t31;
  mad_fixed_t t32,  t33,  t34,  t35,  t36,  t37,  t38,  t39;
  mad_fixed_t t40,  t41,  t42,  t43,  t44,  t45,  t46,  t47;
  mad_fixed_t t48,  t49,  t50,  t51,  t52,  t53,  t54,  t55;
  mad_fixed_t t56,  t57,  t58,  t59,  t60,  t61,  t62,  t63;
  mad_fixed_t t64,  t65,  t66,  t67,  t68,  t69,  t70,  t71;
  mad_fixed_t t72,  t73,  t74,  t75,  t76,  t77,  t78,  t79;
  mad_fixed_t t80,  t81,  t82,  t83,  t84,  t85,  t86,  t87;
  mad_fixed_t t88,  t89,  t90,  t91,  t92,  t93,  t94,  t95;
  mad_fixed_t t96,  t97,  t98,  t99,  t100, t101, t102, t103;
  mad_fixed_t t104, t105, t106, t107, t108, t109, t110, t111;
  mad_fixed_t t112, t113, t114, t115, t116, t117, t118, t119;
  mad_fixed_t t120, t121, t122, t123, t124, t125, t126, t127;
  mad_fixed_t t128, t129, t130, t131, t132, t133, t134, t135;
  mad_fixed_t t136, t137, t138, t139, t140, t141, t142, t143;
  mad_fixed_t t144, t145, t146, t147, t148, t149, t150, t151;
  mad_fixed_t t152, t153, t154, t155, t156, t157, t158, t159;
  mad_fixed_t t160, t161, t162, t163, t164, t165, t166, t167;
  mad_fixed_t t168, t169, t170, t171, t172, t173, t174, t175;
  mad_fixed_t t176;
#endif
  
  t0   = in[0]  + in[31];  t16  = MUL(in[0]  - in[31], costab1);
  t1   = in[15] + in[16];  t17  = MUL(in[15] - in[16], costab31);

  t41  = t16 + t17;
  t59  = MUL(t16 - t17, costab2);
  t33  = t0  + t1;
  t50  = MUL(t0  - t1,  costab2);

  t2   = in[7]  + in[24];  t18  = MUL(in[7]  - in[24], costab15);
  t3   = in[8]  + in[23];  t19  = MUL(in[8]  - in[23], costab17);

  t42  = t18 + t19;
  t60  = MUL(t18 - t19, costab30);
  t34  = t2  + t3;
  t51  = MUL(t2  - t3,  costab30);

  t4   = in[3]  + in[28];  t20  = MUL(in[3]  - in[28], costab7);
  t5   = in[12] + in[19];  t21  = MUL(in[12] - in[19], costab25);

  t43  = t20 + t21;
  t61  = MUL(t20 - t21, costab14);
  t35  = t4  + t5;
  t52  = MUL(t4  - t5,  costab14);

  t6   = in[4]  + in[27];  t22  = MUL(in[4]  - in[27], costab9);
  t7   = in[11] + in[20];  t23  = MUL(in[11] - in[20], costab23);

  t44  = t22 + t23;
  t62  = MUL(t22 - t23, costab18);
  t36  = t6  + t7;
  t53  = MUL(t6  - t7,  costab18);

  t8   = in[1]  + in[30];  t24  = MUL(in[1]  - in[30], costab3);
  t9   = in[14] + in[17];  t25  = MUL(in[14] - in[17], costab29);

  t45  = t24 + t25;
  t63  = MUL(t24 - t25, costab6);
  t37  = t8  + t9;
  t54  = MUL(t8  - t9,  costab6);

  t10  = in[6]  + in[25];  t26  = MUL(in[6]  - in[25], costab13);
  t11  = in[9]  + in[22];  t27  = MUL(in[9]  - in[22], costab19);

  t46  = t26 + t27;
  t64  = MUL(t26 - t27, costab26);
  t38  = t10 + t11;
  t55  = MUL(t10 - t11, costab26);

  t12  = in[2]  + in[29];  t28  = MUL(in[2]  - in[29], costab5);
  t13  = in[13] + in[18];  t29  = MUL(in[13] - in[18], costab27);

  t47  = t28 + t29;
  t65  = MUL(t28 - t29, costab10);
  t39  = t12 + t13;
  t56  = MUL(t12 - t13, costab10);

  t14  = in[5]  + in[26];  t30  = MUL(in[5]  - in[26], costab11);
  t15  = in[10] + in[21];  t31  = MUL(in[10] - in[21], costab21);

  t48  = t30 + t31;
  t66  = MUL(t30 - t31, costab22);
  t40  = t14 + t15;
  t57  = MUL(t14 - t15, costab22);

  t69  = t33 + t34;  t89  = MUL(t33 - t34, costab4);
  t70  = t35 + t36;  t90  = MUL(t35 - t36, costab28);
  t71  = t37 + t38;  t91  = MUL(t37 - t38, costab12);
  t72  = t39 + t40;  t92  = MUL(t39 - t40, costab20);
  t73  = t41 + t42;  t94  = MUL(t41 - t42, costab4);
  t74  = t43 + t44;  t95  = MUL(t43 - t44, costab28);
  t75  = t45 + t46;  t96  = MUL(t45 - t46, costab12);
  t76  = t47 + t48;  t97  = MUL(t47 - t48, costab20);

  t78  = t50 + t51;  t100 = MUL(t50 - t51, costab4);
  t79  = t52 + t53;  t101 = MUL(t52 - t53, costab28);
  t80  = t54 + t55;  t102 = MUL(t54 - t55, costab12);
  t81  = t56 + t57;  t103 = MUL(t56 - t57, costab20);

  t83  = t59 + t60;  t106 = MUL(t59 - t60, costab4);
  t84  = t61 + t62;  t107 = MUL(t61 - t62, costab28);
  t85  = t63 + t64;  t108 = MUL(t63 - t64, costab12);
  t86  = t65 + t66;  t109 = MUL(t65 - t66, costab20);

  t113 = t69  + t70;
  t114 = t71  + t72;

  /*  0 */ hi[15][slot] = SHIFT(t113 + t114);
  /* 16 */ lo[ 0][slot] = SHIFT(MUL(t113 - t114, costab16));

  t115 = t73  + t74;
  t116 = t75  + t76;

  t32  = t115 + t116;

  /*  1 */ hi[14][slot] = SHIFT(t32);

  t118 = t78  + t79;
  t119 = t80  + t81;

  t58  = t118 + t119;

  /*  2 */ hi[13][slot] = SHIFT(t58);

  t121 = t83  + t84;
  t122 = t85  + t86;

  t67  = t121 + t122;

  t49  = (t67 << 1) - t32;

  /*  3 */ hi[12][slot] = SHIFT(t49);

  t125 = t89  + t90;
  t126 = t91  + t92;

  t93  = t125 + t126;

  /*  4 */ hi[11][slot] = SHIFT(t93);

  t128 = t94  + t95;
  t129 = t96  + t97;

  t98  = t128 + t129;

  t68  = (t98 << 1) - t49;

  /*  5 */ hi[10][slot] = SHIFT(t68);

  t132 = t100 + t101;
  t133 = t102 + t103;

  t104 = t132 + t133;

  t82  = (t104 << 1) - t58;

  /*  6 */ hi[ 9][slot] = SHIFT(t82);

  t136 = t106 + t107;
  t137 = t108 + t109;

  t110 = t136 + t137;

  t87  = (t110 << 1) - t67;

  t77  = (t87 << 1) - t68;

  /*  7 */ hi[ 8][slot] = SHIFT(t77);

  t141 = MUL(t69 - t70, costab8);
  t142 = MUL(t71 - t72, costab24);
  t143 = t141 + t142;

  /*  8 */ hi[ 7][slot] = SHIFT(t143);
  /* 24 */ lo[ 8][slot] =
	     SHIFT((MUL(t141 - t142, costab16) << 1) - t143);

  t144 = MUL(t73 - t74, costab8);
  t145 = MUL(t75 - t76, costab24);
  t146 = t144 + t145;

  t88  = (t146 << 1) - t77;

  /*  9 */ hi[ 6][slot] = SHIFT(t88);

  t148 = MUL(t78 - t79, costab8);
  t149 = MUL(t80 - t81, costab24);
  t150 = t148 + t149;

  t105 = (t150 << 1) - t82;

  /* 10 */ hi[ 5][slot] = SHIFT(t105);

  t152 = MUL(t83 - t84, costab8);
  t153 = MUL(t85 - t86, costab24);
  t154 = t152 + t153;

  t111 = (t154 << 1) - t87;

  t99  = (t111 << 1) - t88;

  /* 11 */ hi[ 4][slot] = SHIFT(t99);

  t157 = MUL(t89 - t90, costab8);
  t158 = MUL(t91 - t92, costab24);
  t159 = t157 + t158;

  t127 = (t159 << 1) - t93;

  /* 12 */ hi[ 3][slot] = SHIFT(t127);

  t160 = (MUL(t125 - t126, costab16) << 1) - t127;

  /* 20 */ lo[ 4][slot] = SHIFT(t160);
  /* 28 */ lo[12][slot] =
	     SHIFT((((MUL(t157 - t158, costab16) << 1) - t159) << 1) - t160);

  t161 = MUL(t94 - t95, costab8);
  t162 = MUL(t96 - t97, costab24);
  t163 = t161 + t162;

  t130 = (t163 << 1) - t98;

  t112 = (t130 << 1) - t99;

  /* 13 */ hi[ 2][slot] = SHIFT(t112);

  t164 = (MUL(t128 - t129, costab16) << 1) - t130;

  t166 = MUL(t100 - t101, costab8);
  t167 = MUL(t102 - t103, costab24);
  t168 = t166 + t167;

  t134 = (t168 << 1) - t104;

  t120 = (t134 << 1) - t105;

  /* 14 */ hi[ 1][slot] = SHIFT(t120);

  t135 = (MUL(t118 - t119, costab16) << 1) - t120;

  /* 18 */ lo[ 2][slot] = SHIFT(t135);

  t169 = (MUL(t132 - t133, costab16) << 1) - t134;

  t151 = (t169 << 1) - t135;

  /* 22 */ lo[ 6][slot] = SHIFT(t151);

  t170 = (((MUL(t148 - t149, costab16) << 1) - t150) << 1) - t151;

  /* 26 */ lo[10][slot] = SHIFT(t170);
  /* 30 */ lo[14][slot] =
	     SHIFT((((((MUL(t166 - t167, costab16) << 1) -
		       t168) << 1) - t169) << 1) - t170);

  t171 = MUL(t106 - t107, costab8);
  t172 = MUL(t108 - t109, costab24);
  t173 = t171 + t172;

  t138 = (t173 << 1) - t110;

  t123 = (t138 << 1) - t111;

  t139 = (MUL(t121 - t122, costab16) << 1) - t123;

  t117 = (t123 << 1) - t112;

  /* 15 */ hi[ 0][slot] = SHIFT(t117);

  t124 = (MUL(t115 - t116, costab16) << 1) - t117;

  /* 17 */ lo[ 1][slot] = SHIFT(t124);

  t131 = (t139 << 1) - t124;

  /* 19 */ lo[ 3][slot] = SHIFT(t131);

  t140 = (t164 << 1) - t131;

  /* 21 */ lo[ 5][slot] = SHIFT(t140);

  t174 = (MUL(t136 - t137, costab16) << 1) - t138;

  t155 = (t174 << 1) - t139;

  t147 = (t155 << 1) - t140;

  /* 23 */ lo[ 7][slot] = SHIFT(t147);

  t156 = (((MUL(t144 - t145, costab16) << 1) - t146) << 1) - t147;

  /* 25 */ lo[ 9][slot] = SHIFT(t156);

  t175 = (((MUL(t152 - t153, costab16) << 1) - t154) << 1) - t155;

  t165 = (t175 << 1) - t156;

  /* 27 */ lo[11][slot] = SHIFT(t165);

  t176 = (((((MUL(t161 - t162, costab16) << 1) -
	     t163) << 1) - t164) << 1) - t165;

  /* 29 */ lo[13][slot] = SHIFT(t176);
  /* 31 */ lo[15][slot] =
	     SHIFT((((((((MUL(t171 - t172, costab16) << 1) -
			 t173) << 1) - t174) << 1) - t175) << 1) - t176);

  /*
   * Totals:
   *  80 multiplies
   *  80 additions
   * 119 subtractions
   *  49 shifts (not counting SSO)
   */
}

# undef MUL
# undef SHIFT

/* third SSO shift and/or D[] optimization preshift */

# if defined(OPT_SSO)
#  if MAD_F_FRACBITS != 28
#   error "MAD_F_FRACBITS must be 28 to use OPT_SSO"
#  endif
#  define ML0(hi, lo, x, y)	((lo)  = (x) * (y))
#  define MLA(hi, lo, x, y)	((lo) += (x) * (y))
#  define MLZ(hi, lo)		((void) (hi), (mad_fixed_t) (lo))
#  define SHIFT(x)		((x) >> 2)
#  define PRESHIFT(x)		((MAD_F(x) + (1L << 13)) >> 14)
# else
#  define ML0(hi, lo, x, y)	MAD_F_ML0((hi), (lo), (x), (y))
#  define MLA(hi, lo, x, y)	MAD_F_MLA((hi), (lo), (x), (y))
#  define MLZ(hi, lo)		MAD_F_MLZ((hi), (lo))
#  define SHIFT(x)		(x)
#  if defined(MAD_F_SCALEBITS)
#   undef  MAD_F_SCALEBITS
#   define MAD_F_SCALEBITS	(MAD_F_FRACBITS - 12)
#   define PRESHIFT(x)		(MAD_F(x) >> 12)
#  else
#   define PRESHIFT(x)		MAD_F(x)
#  endif
# endif

static
mad_fixed_t const D[17][32] = {
# include "D.dat"
};

/*
 * NAME:	synth->frame()
 * DESCRIPTION:	perform PCM synthesis of frame subband samples
 */
void mad_synth_frame(struct mad_synth *synth, struct mad_frame const *frame)
{
  unsigned int nch, ns, ch, s, phase = 0;

  nch = MAD_NCHANNELS(&frame->header);
  ns  = MAD_NSBSAMPLES(&frame->header);

  for (ch = 0; ch < nch; ++ch) {
    mad_fixed_t *pcm1, *pcm2;
    mad_fixed_t (*even)[2][16][8], (*odd)[2][16][8];
    mad_fixed_t const (*sbsample)[36][32];

    phase = synth->phase;
    pcm1  = synth->pcm.samples[ch];

    even = &synth->filter[ch][0];
    odd  = &synth->filter[ch][1];

    sbsample = &frame->sbsample[ch];

    for (s = 0; s < ns; ++s) {
      unsigned int pe, po;
      register mad_fixed_t (*fe)[8], (*fx)[8], (*fo)[8];
      register mad_fixed_t const (*Dptr)[32], *ptr;
      register mad_fixed64hi_t hi;
      register mad_fixed64lo_t lo;
      unsigned int sb;

      dct32((*sbsample)[s], phase >> 1,
	    (*even)[~phase & 1], (*odd)[~phase & 1]);

      pe = phase & ~1;
      po = ((phase - 1) & 0xf) | 1;

      /* calculate 32 samples */

      fe = &(*even)[~phase & 1][0];
      fx = &(*even)[ phase & 1][0];
      fo =  &(*odd)[ phase & 1][0];

      Dptr = &D[0];

      ptr = *Dptr + pe;
      ML0(hi, lo, (*fe)[0], ptr[ 0]);
      MLA(hi, lo, (*fe)[1], ptr[14]);
      MLA(hi, lo, (*fe)[2], ptr[12]);
      MLA(hi, lo, (*fe)[3], ptr[10]);
      MLA(hi, lo, (*fe)[4], ptr[ 8]);
      MLA(hi, lo, (*fe)[5], ptr[ 6]);
      MLA(hi, lo, (*fe)[6], ptr[ 4]);
      MLA(hi, lo, (*fe)[7], ptr[ 2]);

      ptr = *Dptr + po;
      MLA(hi, lo, (*fx)[0], -ptr[ 0]);
      MLA(hi, lo, (*fx)[1], -ptr[14]);
      MLA(hi, lo, (*fx)[2], -ptr[12]);
      MLA(hi, lo, (*fx)[3], -ptr[10]);
      MLA(hi, lo, (*fx)[4], -ptr[ 8]);
      MLA(hi, lo, (*fx)[5], -ptr[ 6]);
      MLA(hi, lo, (*fx)[6], -ptr[ 4]);
      MLA(hi, lo, (*fx)[7], -ptr[ 2]);

      *pcm1++ = SHIFT(MLZ(hi, lo));

      pcm2 = pcm1 + 30;

      for (sb = 1; sb < 16; ++sb) {
	++fe;
	++Dptr;

	/* D[32 - sb][i] == -D[sb][31 - i] */

	ptr = *Dptr + pe;
	ML0(hi, lo, (*fe)[7], ptr[ 2]);
	MLA(hi, lo, (*fe)[6], ptr[ 4]);
	MLA(hi, lo, (*fe)[5], ptr[ 6]);
	MLA(hi, lo, (*fe)[4], ptr[ 8]);
	MLA(hi, lo, (*fe)[3], ptr[10]);
	MLA(hi, lo, (*fe)[2], ptr[12]);
	MLA(hi, lo, (*fe)[1], ptr[14]);
	MLA(hi, lo, (*fe)[0], ptr[ 0]);

	ptr = *Dptr + po;
	MLA(hi, lo, (*fo)[0], -ptr[ 0]);
	MLA(hi, lo, (*fo)[1], -ptr[14]);
	MLA(hi, lo, (*fo)[2], -ptr[12]);
	MLA(hi, lo, (*fo)[3], -ptr[10]);
	MLA(hi, lo, (*fo)[4], -ptr[ 8]);
	MLA(hi, lo, (*fo)[5], -ptr[ 6]);
	MLA(hi, lo, (*fo)[6], -ptr[ 4]);
	MLA(hi, lo, (*fo)[7], -ptr[ 2]);

	*pcm1++ = SHIFT(MLZ(hi, lo));

	ptr = *Dptr - po;
	ML0(hi, lo, (*fo)[7], ptr[31 -  2]);
	MLA(hi, lo, (*fo)[6], ptr[31 -  4]);
	MLA(hi, lo, (*fo)[5], ptr[31 -  6]);
	MLA(hi, lo, (*fo)[4], ptr[31 -  8]);
	MLA(hi, lo, (*fo)[3], ptr[31 - 10]);
	MLA(hi, lo, (*fo)[2], ptr[31 - 12]);
	MLA(hi, lo, (*fo)[1], ptr[31 - 14]);
	MLA(hi, lo, (*fo)[0], ptr[31 - 16]);

	ptr = *Dptr - pe;
	MLA(hi, lo, (*fe)[0], ptr[31 - 16]);
	MLA(hi, lo, (*fe)[1], ptr[31 - 14]);
	MLA(hi, lo, (*fe)[2], ptr[31 - 12]);
	MLA(hi, lo, (*fe)[3], ptr[31 - 10]);
	MLA(hi, lo, (*fe)[4], ptr[31 -  8]);
	MLA(hi, lo, (*fe)[5], ptr[31 -  6]);
	MLA(hi, lo, (*fe)[6], ptr[31 -  4]);
	MLA(hi, lo, (*fe)[7], ptr[31 -  2]);

	*pcm2-- = SHIFT(MLZ(hi, lo));

	++fo;
      }

      ++Dptr;

      ptr = *Dptr + po;
      ML0(hi, lo, (*fo)[0], ptr[ 0]);
      MLA(hi, lo, (*fo)[1], ptr[14]);
      MLA(hi, lo, (*fo)[2], ptr[12]);
      MLA(hi, lo, (*fo)[3], ptr[10]);
      MLA(hi, lo, (*fo)[4], ptr[ 8]);
      MLA(hi, lo, (*fo)[5], ptr[ 6]);
      MLA(hi, lo, (*fo)[6], ptr[ 4]);
      MLA(hi, lo, (*fo)[7], ptr[ 2]);

      *pcm1 = SHIFT(-MLZ(hi, lo));
      pcm1 += 16;

      phase = (phase + 1) % 16;
    }
  }

  synth->phase      = phase;
  synth->pcm.length = 32 * ns;
}

/*
 * NAME:	synth->mute()
 * DESCRIPTION:	zero all polyphase filterbank values, resetting synthesis
 */
void mad_synth_mute(struct mad_synth *synth)
{
  unsigned int ch, s, v;

  for (ch = 0; ch < 2; ++ch) {
    for (s = 0; s < 16; ++s) {
      for (v = 0; v < 8; ++v) {
	synth->filter[ch][0][0][s][v] = synth->filter[ch][0][1][s][v] =
	synth->filter[ch][1][0][s][v] = synth->filter[ch][1][1][s][v] = 0;
      }
    }
  }
}
