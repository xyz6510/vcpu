#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

enum {
	ld_rr,
	ld_rp,
	ldp_rr,
	ldp_pp,

	cp_rr,
	cp_rp,
	cpp_rr,
	cpp_pp,

	ad_rr,
	ad_rp,
	sb_rr,
	sb_rp,

	mul_rr,
	mul_rp,
	div_rr,
	div_rp,

	set_rr,
	set_rp,
	rst_rr,
	rst_rp,
	and_rr,
	and_rp,
	or_rr,
	or_rp,
	eor_rr,
	eor_rp,
	not_rr,

	cmp_rr,
	cmp_rp,

	beq_r,
	beq_p,
	blt_r,
	blt_p,
	bgt_r,
	bgt_p,
	bsr_r,
	bsr_p,
	bsrr_r,
	bsrr_p,

	jmp_r,
	jmp_p,
	jmpg_r,
	jmpg_p,
	jsr_r,
	jsr_p,
	jsrr_r,
	jsrr_p,
	jsrg_r,
	jsrg_p,
	rts,
	rtsr,
	rtsg,

	push_r,
	push_p,
	pop_r,

	pr_ri,
	pr_rh,
	pr_pi,
	pr_ph,

	de_r,
	de_p,
	sl_r,
	sl_p,

	nop,
	end,
};

#define NUM_I (4*1024)
#define NUM_R (8*1024)
#define INS (sizeof(uint64_t))
#define NP (4)
#define IS (INS*NP)
#define RAM (NUM_R*INS)
#define PRAM (NUM_I*IS)
#define NUM_S (4*1024)
#define STACK (NUM_S*INS)
#define PSTACK (NUM_S*INS)

#define OPC (128)
#define OPCMAX (OPC-1)

uint64_t *ram;
uint64_t *pram;
uint64_t *stack;
uint64_t *pstack;

uint64_t sg=0;
uint64_t ip=0;
uint64_t ssg=0;
uint64_t rs=0;
uint64_t sp=0;
uint64_t psp=0;

uint64_t ST=0;

uint64_t endvm=0;
uint64_t decoding=0;
uint64_t slowing=0;

void _ld_rr	(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]=ram[rs+p[sg+ip][2]];ip++; }
void _ld_rp	(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]=p[sg+ip][2];ip++; }

void _ldp_rr(uint64_t p[][NP]){ pram[ram[rs+p[sg+ip][1]]]=pram[ram[rs+p[sg+ip][2]]];ip++; }
void _ldp_pp(uint64_t p[][NP]){ pram[p[sg+ip][1]]=p[sg+ip][2];ip++; }

void _cp_rr (uint64_t p[][NP]){ int i;for(i=0;i<ram[rs+p[sg+ip][3]];i++) ram[rs+ram[rs+p[sg+ip][1]]+i]=ram[rs+ram[rs+p[sg+ip][2]]+i];ip++; }
void _cp_rp (uint64_t p[][NP]){ int i;for(i=0;i<p[sg+ip][3];i++) ram[rs+p[sg+ip][1]+i]=ram[rs+p[sg+ip][2]+i];ip++; }
void _cpp_rr(uint64_t p[][NP]){ int i;for(i=0;i<ram[rs+p[sg+ip][3]];i++) pram[ram[rs+p[sg+ip][1]]+i]=pram[ram[rs+p[sg+ip][2]]+i];ip++; }
void _cpp_pp(uint64_t p[][NP]){ int i;for(i=0;i<p[sg+ip][3];i++) pram[p[sg+ip][1]+i]=pram[p[sg+ip][2]+i];ip++; }

void _ad_rr	(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]+=ram[rs+p[sg+ip][2]];ip++; }
void _ad_rp	(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]+=p[sg+ip][2];ip++; }
void _sb_rr	(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]-=ram[rs+p[sg+ip][2]];ip++; }
void _sb_rp	(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]-=p[sg+ip][2];ip++; }

void _mul_rr(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]*=ram[rs+p[sg+ip][2]];ip++; }
void _mul_rp(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]*=p[sg+ip][2];ip++; }
void _div_rr(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]/=ram[rs+p[sg+ip][2]];ip++; }
void _div_rp(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]/=p[sg+ip][2];ip++; }

void _set_rr(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]|=(1<<ram[rs+p[sg+ip][2]]);ip++; }
void _set_rp(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]|=(1<<p[sg+ip][2]);ip++; }
void _rst_rr(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]&=(~(1<<ram[rs+p[sg+ip][2]]));ip++; }
void _rst_rp(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]&=(~(1<<p[sg+ip][2]));ip++; }

void _and_rr(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]&=ram[rs+p[sg+ip][2]];ip++; }
void _and_rp(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]&=p[sg+ip][2];ip++; }
void _or_rr (uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]|=ram[rs+p[sg+ip][2]];ip++; }
void _or_rp (uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]|=p[sg+ip][2];ip++; }
void _eor_rr(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]^=ram[rs+p[sg+ip][2]];ip++; }
void _eor_rp(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]^=p[sg+ip][2];ip++; }
void _not_rr(uint64_t p[][NP]){ ram[rs+p[sg+ip][1]]=~ram[rs+p[sg+ip][1]];ip++; }

void _cmp_rr(uint64_t p[][NP]){	ST=0;
								if(ram[rs+p[sg+ip][1]]==ram[rs+p[sg+ip][2]])ST=1;
								if(ram[rs+p[sg+ip][1]]<ram[rs+p[sg+ip][2]]) ST=2;
								if(ram[rs+p[sg+ip][1]]>ram[rs+p[sg+ip][2]]) ST=3;
								ip++; }
void _cmp_rp(uint64_t p[][NP]){ ST=0;
								if(ram[rs+p[sg+ip][1]]==p[sg+ip][2]) ST=1;
								if(ram[rs+p[sg+ip][1]]<p[sg+ip][2]) ST=2;
								if(ram[rs+p[sg+ip][1]]>p[sg+ip][2]) ST=3;
								ip++; }

void _beq_r	(uint64_t p[][NP]){ if (ST==1) ip=ram[rs+p[sg+ip][1]];else ip++; }
void _beq_p	(uint64_t p[][NP]){ if (ST==1) ip=p[sg+ip][1];else ip++; }
void _blt_r	(uint64_t p[][NP]){ if (ST==2) ip=ram[rs+p[sg+ip][1]];else ip++; }
void _blt_p	(uint64_t p[][NP]){ if (ST==2) ip=p[sg+ip][1];else ip++; }
void _bgt_r	(uint64_t p[][NP]){ if (ST==3) ip=ram[rs+p[sg+ip][1]];else ip++; }
void _bgt_p	(uint64_t p[][NP]){ if (ST==3) ip=p[sg+ip][1];else ip++; }
void _bsr_r	(uint64_t p[][NP]){ pstack[psp]=ip+1;
								if (ST==2) ip=ram[rs+p[sg+ip][1]];
								if (ST==3) ip=ram[rs+p[sg+ip][2]];
								psp++; }
void _bsr_p	(uint64_t p[][NP]){ pstack[psp]=ip+1;
								if (ST==2) ip=p[sg+ip][1];
								if (ST==3) ip=p[sg+ip][2];
								psp++; }
void _bsrr_r(uint64_t p[][NP]){ if (ST==1) {ip++;return;}
								pstack[psp]=ip+1;
								pstack[psp+1]=rs;
								uint64_t ors=rs,oip=ip;
								if (ST==2) ip=ram[ors+p[sg+ip][1]];
								if (ST==3) ip=ram[ors+p[sg+ip][2]];
								rs+=ram[ors+p[sg+oip][3]];
								psp+=2; }
void _bsrr_p(uint64_t p[][NP]){ if (ST==1) {ip++;return;}
								pstack[psp]=ip+1;
								pstack[psp+1]=rs;
								uint64_t oip=ip;
								if (ST==2) ip=p[sg+ip][1];
								if (ST==3) ip=p[sg+ip][2];
								rs+=p[sg+oip][3];
								psp+=2; }

void _jmp_r	(uint64_t p[][NP]){ ip=ram[rs+p[sg+ip][1]]; }
void _jmp_p	(uint64_t p[][NP]){ ip=p[sg+ip][1]; }
void _jmpg_r(uint64_t p[][NP]){ uint64_t osg=sg,ors=rs;
								sg=ram[ors+p[osg+ip][1]];
								ssg=ram[ors+p[osg+ip][2]];
								rs=ram[ors+p[osg+ip][3]];
								ip=0;sp=0; }
void _jmpg_p(uint64_t p[][NP]){ uint64_t osg=sg;
								sg=p[osg+ip][1];
								ssg=p[osg+ip][2];
								rs=p[osg+ip][3];
								ip=0;sp=0; }
void _jsr_r	(uint64_t p[][NP]){ pstack[psp]=ip+1;ip=ram[rs+p[sg+ip][1]];psp++; }
void _jsr_p	(uint64_t p[][NP]){ pstack[psp]=ip+1;ip=p[sg+ip][1];psp++; }
void _jsrr_r(uint64_t p[][NP]){ pstack[psp]=ip+1;pstack[psp+1]=rs;ip=ram[rs+p[sg+ip][1]],rs+=ram[rs+p[sg+ip][2]];psp+=2; }
void _jsrr_p(uint64_t p[][NP]){ pstack[psp]=ip+1;pstack[psp+1]=rs;ip=p[sg+ip][1],rs+=p[sg+ip][2];psp+=2; }
void _jsrg_r(uint64_t p[][NP]){ pstack[psp]=sg;
								pstack[psp+1]=ssg;
								pstack[psp+2]=ip+1;
								pstack[psp+3]=sp;
								pstack[psp+4]=rs;
								uint64_t osg=sg,ors=rs;
								sg=ram[ors+p[osg+ip][1]];
								ssg=ram[ors+p[osg+ip][2]];
								rs=ram[ors+p[osg+ip][3]];
								ip=0;sp=0;psp+=5; }
void _jsrg_p(uint64_t p[][NP]){ pstack[psp]=sg;
								pstack[psp+1]=ssg;
								pstack[psp+2]=ip+1;
								pstack[psp+3]=sp;
								pstack[psp+4]=rs;
								uint64_t osg=sg;
								sg=p[osg+ip][1];
								ssg=p[osg+ip][2];
								rs=p[osg+ip][3];
								ip=0;sp=0;psp+=5; }
void _rts	(uint64_t p[][NP]){ psp--;ip=pstack[psp]; }
void _rtsr	(uint64_t p[][NP]){ psp-=2;ip=pstack[psp];rs=pstack[psp+1]; }
void _rtsg	(uint64_t p[][NP]){ psp-=5;sg=pstack[psp];ssg=pstack[psp+1];ip=pstack[psp+2];sp=pstack[psp+3];rs=pstack[psp+4]; }

void _push_r(uint64_t p[][NP]){ stack[ssg+sp]=ram[rs+p[sg+ip][1]];sp++;ip++; }
void _push_p(uint64_t p[][NP]){ stack[ssg+sp]=p[sg+ip][1];sp++;ip++; }
void _pop_r (uint64_t p[][NP]){ sp--;ram[rs+p[sg+ip][1]]=stack[ssg+sp];ip++; }

void _pr_ri	(uint64_t p[][NP]){ if (decoding==0) printf("pr_ri:%lu\n",ram[rs+p[sg+ip][1]]);ip++;  }
void _pr_rh	(uint64_t p[][NP]){ if (decoding==0) printf("pr_rh:%#lx\n",ram[rs+p[sg+ip][1]]);ip++; }
void _pr_pi (uint64_t p[][NP]){ if (decoding==0) printf("pr_pi:%lu\n",p[sg+ip][1]);ip++; }
void _pr_ph (uint64_t p[][NP]){ if (decoding==0) printf("pr_ph:%#lx\n",p[sg+ip][1]);ip++; }

void _de_r  (uint64_t p[][NP]){ decoding=ram[rs+p[sg+ip][1]];ip++; }
void _de_p  (uint64_t p[][NP]){ decoding=p[sg+ip][1];ip++; }
void _sl_r  (uint64_t p[][NP]){ slowing=ram[rs+p[sg+ip][1]];ip++; }
void _sl_p  (uint64_t p[][NP]){ slowing=p[sg+ip][1];ip++; }

void _nop   (uint64_t p[][NP]){ ip++; }
void __end  (uint64_t p[][NP]){ endvm=1; }
void _invalid (uint64_t p[][NP]){ printf("invalid instruction sg:%04lx ip:%04lx\n",sg,ip);endvm=1; }

void decode(uint64_t p[NP])
{
	int np=0;
	int rr=0;
	int rp=0;
	int pr=0;
	int pp=0;
	printf("[");
	switch(p[0]) {
		case ld_rr:		{printf("ld_rr ");np=2;rr=2;break;}
		case ld_rp:		{printf("ld_rp ");np=2;rp=2;break;}
		case ldp_rr:	{printf("ldp_rr ");np=2;pr=2;break;}
		case ldp_pp:	{printf("ldp_pp ");np=2;pp=2;break;}
		case cp_rr:		{printf("cp_rr ");np=3;rr=3;break;}
		case cp_rp:		{printf("cp_rp ");np=3;rp=3;break;}
		case cpp_rr:	{printf("cpp_rr ");np=3;rr=3;break;}
		case cpp_pp:	{printf("cpp_pp ");np=3;rp=3;break;}
		case ad_rr:		{printf("ad_rr ");np=2;rr=2;break;}
		case ad_rp:		{printf("ad_rp ");np=2;rp=2;break;}
		case sb_rr:		{printf("sb_rr ");np=2;rr=2;break;}
		case sb_rp:		{printf("sb_rp ");np=2;rp=2;break;}
		case mul_rr:	{printf("mul_rr ");np=2;rr=2;break;}
		case mul_rp:	{printf("mul_rp ");np=2;rp=2;break;}
		case div_rr:	{printf("div_rr ");np=2;rr=2;break;}
		case div_rp:	{printf("div_rp ");np=2;rp=2;break;}
		case set_rr:	{printf("set_rr ");np=2;rr=2;break;}
		case set_rp:	{printf("set_rp ");np=2;rp=2;break;}
		case rst_rr:	{printf("rst_rr ");np=2;rr=2;break;}
		case rst_rp:	{printf("rst_rp ");np=2;rp=2;break;}
		case and_rr:	{printf("and_rr ");np=2;rr=2;break;}
		case and_rp:	{printf("and_rp ");np=2;rp=2;break;}
		case or_rr:		{printf("or_rr ");np=2;rr=2;break;}
		case or_rp:		{printf("or_rp ");np=2;rp=2;break;}
		case eor_rr:	{printf("eor_rr ");np=2;rr=2;break;}
		case eor_rp:	{printf("eor_rp ");np=2;rp=2;break;}
		case not_rr:	{printf("not_rr ");np=1;rr=1;break;}
		case cmp_rr:	{printf("cmp_rr ");np=2;rr=2;break;}
		case cmp_rp:	{printf("cmp_rp ");np=2;rp=2;break;}
		case beq_r:		{printf("beq_r ");np=1;rr=1;break;}
		case beq_p:		{printf("beq_p ");np=1;rp=1;break;}
		case blt_r:		{printf("blt_r ");np=1;rr=1;break;}
		case blt_p:		{printf("blt_p ");np=1;rp=1;break;}
		case bgt_r:		{printf("bgt_r ");np=1;rr=1;break;}
		case bgt_p:		{printf("bgt_p ");np=1;rp=1;break;}
		case bsr_r:		{printf("bsr_r ");np=2;rr=2;break;}
		case bsr_p:		{printf("bsr_p ");np=2;rp=2;break;}
		case bsrr_r:	{printf("bsrr_r ");np=3;rr=3;break;}
		case bsrr_p:	{printf("bsrr_p ");np=3;rp=3;break;}
		case jmp_r:		{printf("jmp_r ");np=1;rp=1;break;}
		case jmp_p:		{printf("jmp_p ");np=1;rp=1;break;}
		case jmpg_r:	{printf("jmpg_r ");np=3;rr=3;break;}
		case jmpg_p:	{printf("jmpg_p ");np=3;rp=3;break;}
		case jsr_r:		{printf("jsr_r ");np=1;rr=1;break;}
		case jsr_p:		{printf("jsr_p ");np=1;rp=1;break;}
		case jsrr_r:	{printf("jsrr_r ");np=2;rr=2;break;}
		case jsrr_p:	{printf("jsrr_p ");np=2;rp=2;break;}
		case jsrg_r:	{printf("jsrg_r ");np=3;rr=3;break;}
		case jsrg_p:	{printf("jsrg_p ");np=3;rp=3;break;}
		case rts:		{printf("rts");np=0;break;}
		case rtsr:		{printf("rtsr");np=0;break;}
		case rtsg:		{printf("rtsg");np=0;break;}
		case push_r:	{printf("push_r ");np=1;rp=1;break;}
		case push_p:	{printf("push_p ");np=1;rp=1;break;}
		case pop_r:		{printf("pop_r ");np=1;rr=1;break;}
		case pr_ri:		{printf("pr_ri ");np=1;rr=1;break;}
		case pr_rh:		{printf("pr_rh ");np=1;rr=1;break;}
		case pr_pi:		{printf("pr_pi ");np=1;rp=1;break;}
		case pr_ph:		{printf("pr_ph ");np=1;rp=1;break;}
		case de_r:		{printf("de_r ");np=1;rr=1;break;}
		case de_p:		{printf("de_p ");np=1;rp=1;break;}
		case sl_r:		{printf("sl_r ");np=1;rr=1;break;}
		case sl_p:		{printf("sl_p ");np=1;rp=1;break;}
		case nop:		{printf("nop");np=0;break;}
		case end:		{printf("end");np=0;break;}
	}
	switch(np) {
		case 1:printf("0x%lx",p[1]);break;
		case 2:printf("0x%lx,0x%lx",p[1],p[2]);break;
		case 3:printf("0x%lx,0x%lx,0x%lx",p[1],p[2],p[3]);break;
	}
	printf("] ");
	switch(rr) {
		case 1: printf("0x%lx %lu",ram[rs+p[1]],ram[rs+p[1]]);break;
		case 2: printf("0x%lx,0x%lx %lu,%lu",ram[rs+p[1]],ram[rs+p[2]],ram[rs+p[1]],ram[rs+p[2]]);break;
		case 3: printf("0x%lx,0x%lx,0x%lx %lu,%lu,%lu",ram[rs+p[1]],ram[rs+p[2]],ram[rs+p[3]],ram[rs+p[1]],ram[rs+p[2]],ram[rs+p[3]]);break;
	}
	switch(rp) {
		case 1: printf("0x%lx %lu",p[1],p[1]);break;
		case 2: printf("0x%lx,0x%lx %lu,%lu",ram[rs+p[1]],p[2],ram[rs+p[1]],p[2]);break;
		case 3: printf("0x%lx,0x%lx,0x%lx %lu,%lu,%lu",p[1],p[2],p[3],p[1],p[2],p[3]);break;
	}
	switch(pr) {
		case 2: printf("%#lx,%#lx %lu,%lu",pram[ram[rs+p[1]]],pram[p[2]],pram[ram[rs+p[1]]],pram[p[2]]);break;
	}
	switch(pp) {
		case 2: printf("%#lx,%#lx %lu,%lu",pram[p[1]],p[2],pram[p[1]],p[2]);break;
	}
	printf("\n");
}

void print_info(uint64_t ps)
{
	printf("\n");
	printf(" cpu:64bit\n");
	printf(" opc:%i\n",end+1);
	printf(" is:%lu\n",IS);
	printf(" np:%i\n",NP);
	printf(" ram:%lu \t %lu 64bit\n",RAM,RAM/INS);
	printf(" pram:%lu \t %i 64bit[%i]\n",PRAM,NUM_I,NP);
	printf(" stack:%lu \t %i 64bit\n",STACK,NUM_S);
	printf(" pstack:%lu \t %i 64bit\n",PSTACK,NUM_S);
	printf(" prg:%lu \t %lu instr\n",ps,ps/IS);
	printf("\n");
}


int main()
{
	ram=malloc(RAM);
	pram=malloc(PRAM);
	stack=malloc(STACK);
	pstack=malloc(STACK);
	memset(ram,0,RAM);
	memset(pram,0,PRAM);
	memset(stack,0,STACK);
	memset(pstack,0,PSTACK);

	uint64_t prg[][NP]={

		#include "/tmp/prg.c"

	};

	print_info(sizeof(prg));

	int f=open("/tmp/prg.bin",O_RDWR|O_CREAT|O_TRUNC,0644);
	write(f,prg,sizeof(prg));
	close(f);

	memcpy(pram,prg,sizeof(prg));

	typedef void (*inst_func)(uint64_t [][NP]);
	inst_func instr[OPC]={
		_ld_rr,
		_ld_rp,
		_ldp_rr,
		_ldp_pp,

		_cp_rr,
		_cp_rp,
		_cpp_rr,
		_cpp_pp,

		_ad_rr,
		_ad_rp,
		_sb_rr,
		_sb_rp,

		_mul_rr,
		_mul_rp,
		_div_rr,
		_div_rp,

		_set_rr,
		_set_rp,
		_rst_rr,
		_rst_rp,
		_and_rr,
		_and_rp,
		_or_rr,
		_or_rp,
		_eor_rr,
		_eor_rp,
		_not_rr,

		_cmp_rr,
		_cmp_rp,

		_beq_r,
		_beq_p,
		_blt_r,
		_blt_p,
		_bgt_r,
		_bgt_p,
		_bsr_r,
		_bsr_p,
		_bsrr_r,
		_bsrr_p,

		_jmp_r,
		_jmp_p,
		_jmpg_r,
		_jmpg_p,
		_jsr_r,
		_jsr_p,
		_jsrr_r,
		_jsrr_p,
		_jsrg_r,
		_jsrg_p,
		_rts,
		_rtsr,
		_rtsg,

		_push_r,
		_push_p,
		_pop_r,

		_pr_ri,
		_pr_rh,
		_pr_pi,
		_pr_ph,

		_de_r,
		_de_p,
		_sl_r,
		_sl_p,

		_nop,
		__end,
	};
	int i;for (i=end+1;i<OPC;i++) instr[i]=_invalid;

	uint64_t (*pm)[NP]=(uint64_t(*)[NP]) pram;

	for (ip=0;endvm==0;) {
		if (decoding) {
			printf("psp:%04lx ssg:%04lx sp:%04lx sg:%04lx rs:%04lx ip:%04lx ",psp,ssg,sp,sg,rs,ip);
			decode(pm[sg+ip]);
		}
		if (slowing) usleep(slowing);

		instr[pm[sg+ip][0]&OPCMAX](pm);
	}

	return 42;
}
