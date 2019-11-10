
#define sg1 (128)
#define num_ins (64)

		{ld_rp,0x01,0},
`mod3`
		{ld_rp,0x02,10000000},
`mod2`
		{cpp_pp,sg1*NP,0,num_ins*IS},
		{ldp_pp,(sg1*NP)+(`mod2`*NP),jmp_p},
		{ldp_pp,(sg1*NP)+(`mod2`*NP)+1,`l2`},
		{ldp_pp,(sg1*NP)+(`mod3`*NP)+2,11},
		{ldp_pp,(sg1*NP)+(`mod1`*NP),rtsg},
		{push_p,123},
		{push_p,321},
		{jsrg_p,sg1,256,128},
`l2`
		{ad_rp,0x01,1},
		{sb_rp,0x02,1},
		{cmp_rp,0x02,0},
`mod1`
		{beq_p,`l1`},
		{jmp_p,`l2`},
`l1`
		{pr_ri,0x01},
		{pr_ri,0x02},
		{nop},
		{ld_rp,0,5},
		{ld_rp,1,6},
		{ld_rp,2,7},
		{ad_rr,0,1},
		{pr_ri,0},
		{jsrr_p,`sum`,1},
		{pr_ri,0},
		{de_p,0},
		{sl_p,16000*0},
		{cmp_rp,1,14},
		{bsrr_p,`less`,`greater`,1},
		{nop},
		{pr_ri,1},
		{end},
`sum`
		{ad_rr,0,1},
		{pr_ri,0},
		{rtsr},
`less`
		{ad_rp,0,5},
//		{pr_ri,0},
		{rtsr},
`greater`
		{ad_rp,0,7},
//		{pr_ri,0},
		{rtsr},
