
#define START (512)

//program start
/* comment */
//no multiline commands { } myst be on the same line
		{ld_rp,1,START/*comment*/},   
`start`	{cmp_rp,1,START},
		{beq_p,`skip`},
		{jmp_p,`end`},
`skip`	{ld_rp,2,1},
		{ad_rr,1,2},
/* comment */
		{cmp_rr,1,5},
		{beq_p,`l1`},
//comment
		{ad_rp,0,`l1`/*comment*/+2},
`l3` `l4`{pr_pi,0},
   `l1` `l3`
	`l2`
		{pr_pi,1},
#include "prg09.c"
		{jmp_p,`start`},
`end`
#include "prg10.c"
