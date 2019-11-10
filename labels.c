#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

typedef struct {
	int n;
	int s;
	int e;
	int l;
} LABEL;

int main()
{
	int f=open("/tmp/prg.cpp",O_RDONLY);
	if (f<0) {
		printf("error open input file\n");
		exit(-1);
	}
	int len=lseek(f,0,SEEK_END);
	int align=getpagesize();
    int size=((len/align)+1)*align;
    char *mem=NULL;
    posix_memalign((void**)&mem,align,size);
    memset(mem,0,size);
    mem=mmap(mem,size,PROT_READ,MAP_SHARED|MAP_FIXED,f,0);
    if (mem==MAP_FAILED) {
        printf("error mmap file\n");
        exit(-1);
    }
    close(f);

	int line=0;
	int i;
	for (i=0;i<len;i++) {
		printf("%04i ",line);
		for (;i<len;i++) {
			printf("%c",mem[i]);
			if (mem[i]=='\n') {
				line++;
				break;
			}
		}
	}
	printf("\n");

	LABEL *list_i=malloc(1024*sizeof(LABEL));
	LABEL *list_o=malloc(1024*sizeof(LABEL));

	line=0;
	int start=-1;
	int inside=0;
	int ins=0;
	int out=0;
	for (i=0;i<len;i++) {
		if (mem[i]=='{') {
			inside=1;
		}
		if (mem[i]=='}') {
			start=-1;
			inside=0;
			line++;
		}
		if (mem[i]=='`') {
			if (start==-1) {
				if (inside) printf("inside label ");
				else printf("outside label ");
				printf("[%i] ",line);
				start=i;
			} else {
				int end=i+1;
				printf("[%.*s] [%i] [%i]\n",end-start,&mem[start],start,i-start);
				if (inside) {
					list_i[ins].n=line;
					list_i[ins].s=start;
					list_i[ins].e=end;
					list_i[ins].l=end-start;
					ins++;
				} else {
					list_o[out].n=line;
					list_o[out].s=start;
					list_o[out].e=end;
					list_o[out].l=end-start;
					out++;
				}
				start=-1;
			}
		}
	}
	printf("\n");
	printf(":inside list\n");
	for (i=0;i<ins;i++) {
		int s=list_i[i].s;
		int l=list_i[i].l;
		printf("%.*s\n",l,&mem[s]);
	}
	printf(":outside list\n");
	for (i=0;i<out;i++) {
		int s=list_o[i].s;
		int l=list_o[i].l;
		printf("%.*s\n",l,&mem[s]);
	}
	f=open("/tmp/prg.0",O_RDWR|O_CREAT|O_TRUNC,0644);
	if (f<0) {
		printf("error open output file\n");
		exit(-1);
	}
	int next=0;
	char buf[16];
	for (i=0;i<ins;i++) {
		int s=list_i[i].s;
		//int e=list_i[i].e;
		int l=list_i[i].l;
		write(f,&mem[next],s-next);
		int found=0;
		char *str1=&mem[s];
		int j;
		for(j=0;j<out;j++) {
			int os=list_o[j].s;
			int on=list_o[j].n;
			char *str2=&mem[os];
			if (strncmp(str1,str2,l)==0) {
				write(f,buf,sprintf(buf,"%i",on));
				next=s;
				//next=e;
				found=1;
				break;
			}
		}
		if (!found) {
			printf("label %.*s not found\n",l,&mem[s]);
			exit(-1);
		}
	}
	write(f,&mem[next],len-next);
	close(f);
	munmap(mem,size);

	int f1=open("/tmp/prg.0",O_RDONLY);
	if (f1<0) {
		printf("error open in asm file\n");
		exit(-1);
	}
	int f2=open("/tmp/prg.1",O_RDWR|O_CREAT|O_TRUNC,0644);
	if (f2<0) {
		printf("error open in asm file\n");
		exit(-1);
	}
	char c;
	inside=0;
	for (i=read(f1,&c,1);i>0;i=read(f1,&c,1)) {
		if (c=='`') {
			inside^=1;
			continue;
		}
		if (inside) continue;
		write(f2,&c,1);
	}
	close(f1);
	close(f2);

	return 0;
}
